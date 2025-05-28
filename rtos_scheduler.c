#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include "rtos.h"

#define DEFAULT_CPU_QUANTA_MS 1000 // CPU 점유 시간의 default 값

static int timer_fd;                  // 타이머 파일 디스크립터
Task tasks[MAX_TASKS];         // Task 구조체를 담는 배열
int task_count = 0;            // 스케줄러에 등록된 태스크 수
volatile uint64_t ticks = 0;            // 1ms가 몇 번 지났는가? = tick
static int remaining_time[MAX_TASKS]; // 각 태스크별 남은 실행 시간(ms)
extern int slice_ms;                  // DIP switch의 값에 따라서 1 또는 5
extern volatile int yield_flag;       // 선점 요청 플래그

// 스케쥴러 초기화 함수 == 타이머 생성하기
int init_scheduler(void)
{
    struct itimerspec its = {
        .it_value = {0, 1 * 1000000},    // 첫 만료: 1 ms
        .it_interval = {0, 1 * 1000000}, // 첫 만료 이후 만료되는 주기: 1 ms
    };

    /*
    타이머 디스크립터가 잘 생성되었다면 non-negative value를 return
    즉, 음수의 값을 return 하면 에러 메시지 출력하도록 설정
    */
    if ((timer_fd = timerfd_create(CLOCK_MONOTONIC_RAW, 0)) < 0)
    {
        perror("timerfd_create");
        return -1;
    }

    /*
    timerfd_settime 함수를 통해서 timer_fd의 시간을 its 구조체에서 선언한 값들로 설정해주기
    */
    if (timerfd_settime(timer_fd, 0, &its, NULL) < 0)
    {
        perror("timerfd_settime");
        close(timer_fd);
        return -1;
    }

    printf("Scheduler initialized: 1 ms tick\n");
    return 0;
}

// 실행할 태스크들을 배열에 등록해주는 함수
void register_task(int period_ms, void (*func)(void))
{
    // 배열이 꽉 차있다면 에러 메시지
    if (task_count >= MAX_TASKS)
    {
        fprintf(stderr, "Error: Max tasks reached\n");
        return;
    }

    /*
    배열의 마지막 칸에 새로운 태스크 등록해주고
    task_count 변수 ++
    */
    tasks[task_count].period_ms = period_ms;
    tasks[task_count].func = func;
    
    if (period_ms == -1) {
        // 백그라운드 태스크 (무한 실행)
        remaining_time[task_count] = -1;
        printf("Background task %d registered (infinite)\n", task_count);
    } else {
        // 일반 태스크 (정해진 실행 시간)
        remaining_time[task_count] = period_ms;
        printf("Normal task %d registered (period_ms=%d)\n", task_count, period_ms);
    }
    
    task_count++;
}

// 스케줄링 메인 함수
void rtos_start(void)
{
    // 등록 된 태스크가 없다면 에러 메시지 출력
    if (task_count == 0)
    {
        fprintf(stderr, "Error: No tasks registered. Cannot start scheduler.\n");
        return;
    }

    printf("RTOS started. Round-Robin scheduling with slice_ms = %d\n", slice_ms);

    int idx = 0; // 실행할 task의 index를 가리키는 변수

    while (1)
    {
        uint64_t expirations;

        /*
        1ms가 지날 때 마다 expirations가 1씩 증가하게 됨
        즉 음수가 나왔다는 것은 타이머 디스크립터에 문제가 생겼다는 것 -> 에러 메시지 출력
        */
        if (read(timer_fd, &expirations, sizeof(expirations)) < 0)
        {
            perror("read(timer_fd)");
            break;
        }

        // ticks = 몇 번 만료가 되었는가? = 1ms(단위시간)이 몇 번 흘렀는가?
        ticks += expirations;

        // 일반 태스크가 모두 완료되었는지 확인
        int normal_tasks_completed = 1;
        for (int i = 0; i < task_count; i++) {
            if (tasks[i].period_ms != -1 && remaining_time[i] > 0) {
                normal_tasks_completed = 0;
                break;
            }
        }
        
        if (normal_tasks_completed) {
            printf("All normal tasks completed. Exiting RTOS.\n");
            break;
        }

        // remaining_time이 0이면 태스크를 건너뛰기 (백그라운드 태스크는 제외)
        if (remaining_time[idx] == 0 && tasks[idx].period_ms != -1)
        {
            printf("Task %d skipped (remaining_time=0)\n", idx);
            // 다음 태스크로 전환 (round-robin)
            idx = (idx + 1) % task_count;
            continue;
        }

        // 백그라운드 태스크는 항상 실행
        int is_background = (tasks[idx].period_ms == -1);
        
        // 현재 CPU 점유 시간(quantum) 계산
        int quantum_ms = DEFAULT_CPU_QUANTA_MS * slice_ms;

        // 이 태스크가 실제로 실행할 최대 시간
        int alloc_ms;
        if (is_background) {
            // 백그라운드 태스크는 quantum 시간만큼 실행
            alloc_ms = quantum_ms;
        } else {
            // 일반 태스크는 min(남은 실행 시간, 할당된 quantum)
            alloc_ms = remaining_time[idx] < quantum_ms
                           ? remaining_time[idx]
                           : quantum_ms;
        }

        // alloc_ms 만큼 실행하되, yield_flag가 세트되면 즉시 중단
        printf("Starting task %d (%s) for up to %d ms (ticks=%llu)\n",
               idx, is_background ? "background" : "normal", alloc_ms, (unsigned long long)ticks);

        // 태스크 함수 호출
        tasks[idx].func();

        // 할당 된 시간이 아직 지나지 않았고,
        // yiled_flag의 값아 false라면 계속 유지.
        int slept = 0;
        while (slept < alloc_ms && !yield_flag)
        {
            usleep(1000); // 1 ms 지연
            ticks++;
            slept++;
        }

        if (yield_flag)
        {
            printf(" → Task %d preempted after %d ms\n", idx, slept);
            yield_flag = 0; // 플래그 클리어
        }

        // 남은 실행 시간 갱신 (백그라운드 태스크는 제외)
        if (!is_background) {
            remaining_time[idx] -= slept;
            if (remaining_time[idx] == 0)
            {
                printf(" → Task %d completed!\n", idx);
            }
        }

        // 다음 태스크로 전환 (round-robin)
        idx = (idx + 1) % task_count;
        printf("→ Switching to task %d (remaining_time=%d ms)\n",
               idx, remaining_time[idx]);
    }

    close(timer_fd);
    printf("RTOS stopped.\n");
}

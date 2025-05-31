#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include "rtos.h"
#include "task.h"
#include "config.h"

#define DEFAULT_CPU_QUANTA_MS 1000 // CPU 점유 시간의 default 값

// static int timer_fd;                  // 타이머 파일 디스크립터
Task tasks[MAX_TASKS]; // Task 구조체를 담는 배열
int task_count = 0;    // 스케줄러에 등록된 태스크 수
// volatile uint64_t ticks = 0;          // 1ms가 몇 번 지났는가? = tick
static int remaining_time[MAX_TASKS]; // 각 태스크별 남은 실행 시간(ms)
extern volatile int yield_flag;       // 선점 요청 플래그

// 스케쥴러 초기화 함수 == 타이머 생성하기
// int init_scheduler(void)
// {
//     struct itimerspec its = {
//         .it_value = {0, 1 * 1000000},    // 첫 만료: 1 ms
//         .it_interval = {0, 1 * 1000000}, // 첫 만료 이후 만료되는 주기: 1 ms
//     };

//     /*
//     타이머 디스크립터가 잘 생성되었다면 non-negative value를 return
//     즉, 음수의 값을 return 하면 에러 메시지 출력하도록 설정
//     */
//     if ((timer_fd = timerfd_create(CLOCK_MONOTONIC, 0)) < 0)
//     {
//         perror("timerfd_create");
//         return -1;
//     }

//     /*
//     timerfd_settime 함수를 통해서 timer_fd의 시간을 its 구조체에서 선언한 값들로 설정해주기
//     */
//     if (timerfd_settime(timer_fd, 0, &its, NULL) < 0)
//     {
//         perror("timerfd_settime");
//         close(timer_fd);
//         return -1;
//     }

//     printf("Scheduler initialized: 1 ms tick\n");
//     return 0;
// }

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

    // 일반 태스크 (정해진 실행 시간)
    remaining_time[task_count] = period_ms;
    printf("Normal task %d registered (period_ms=%d)\n", task_count, period_ms);

    task_count++;
}

// 스케줄링 메인 함수
void rtos_start(void)
{
    if (task_count == 0)
    {
        fprintf(stderr, "Error: No tasks registered. Cannot start scheduler.\n");
        return;
    }

    printf("RTOS started. Round-Robin scheduling (initial slice_ms = %d)\n", slice_ms);

    // 태스크 인덱스
    int idx = 0;

    while (1)
    {
        uint64_t expirations;

        // slice_ms 업데이트
        task_config();
        printf("Updated slice_ms = %d\n", slice_ms);

        /*
        모든 일반 태스크 완료 여부 검사
        1 -> 모든 태스크 수행 완료
        0 -> 아직 남은 태스크가 존재
        */
        int all_done = 1;
        for (int i = 0; i < task_count; i++)
        {
            if (remaining_time[i] > 0)
            {
                all_done = 0;
                break;
            }
        }

        if (all_done)
        {
            printf("All normal tasks completed. Exiting RTOS.\n");
            break;
        }


        // 인터럽트(ISR) 처리 중이면 종료 전까지 대}

        // if (read(timer_fd, &expirations, sizeof(expirations)) < 0)
        // {
        //     perror("read(timer_fd)");
        //     break;
        // }

        // printf("Timer expired, expirations: %llu\n", (unsigned long long)expirations);

        // ticks += expirations;

        // 남은 시간이 0인 일반 태스크 건너뛰기
        if (remaining_time[idx] <= 0)
        {
            idx = (idx + 1) % task_count;
            continue;
        }

        // CPU 할당 시간 계산
        int quantum_ms = DEFAULT_CPU_QUANTA_MS * slice_ms;
        int alloc_ms = (remaining_time[idx] < quantum_ms ? remaining_time[idx] : quantum_ms);

        // 태스크 실행
        printf("Starting task %d for up to %d ms\n", idx, alloc_ms);
        tasks[idx].func();
        int slept = 0;
        while (slept < alloc_ms && !yield_flag)
        {
            usleep(1000);
            // ticks++;
            slept++;
        }

        remaining_time[idx] -= slept;
        if (remaining_time[idx] <= 0)
            printf(" → Task %d completed!\n", idx);

        if (yield_flag)
        {
            while (yield_flag)
                usleep(1000);
            idx = (idx + 1) % task_count;
            continue;
        }

        // 다음 태스크로 업데이트
        idx = (idx + 1) % task_count;
        printf("→ Switching to task %d (remaining_time=%d ms)\n", idx, remaining_time[idx]);
    }

    
    printf("RTOS stopped.\n");
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "rtos.h"
#include "task.h"
#include "isr_push.h"   // interrupt_processing 접근을 위해 추가

#define DEFAULT_CPU_QUANTA_MS 1000 // CPU 점유 시간의 default 값

Task tasks[MAX_TASKS];                // Task 구조체를 담는 배열
int task_count = 0;                   // 스케줄러에 등록된 태스크 수
static int remaining_time[MAX_TASKS]; // 각 태스크별 남은 실행 시간(ms)
extern int slice_ms;                  // DIP switch의 값에 따라서 1 또는 5

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

    if (period_ms == -1)
    {
        // 백그라운드 태스크 (무한 실행)
        remaining_time[task_count] = -1;
        printf("Background task %d registered (infinite)\n", task_count);
    }
    else
    {
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
        // 일반 태스크가 모두 완료되었는지 확인
        int normal_tasks_completed = 1;
        for (int i = 0; i < task_count; i++)
        {
            if (tasks[i].period_ms != -1 && remaining_time[i] > 0)
            {
                normal_tasks_completed = 0;
                break;
            }
        }
        // 다 확인했는데도 만약에 일반 태스크가 모두 완료되었다면 while loop를 탈출
        if (normal_tasks_completed)
        {
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

        // 백그라운드 태스크는 항상 실행. 현재 while문이 백그라운드 태스크면 is_background가 1이됨
        int is_background = (tasks[idx].period_ms == -1);

        // 현재 CPU 점유 시간(quantum) 계산 (초단위로 변환하기 위해 앞의 parameter를 곱해줌)
        int quantum_ms = DEFAULT_CPU_QUANTA_MS * slice_ms;

        // 이 태스크가 실제로 실행할 최대 시간
        int alloc_ms;
        if (is_background)
        {
            // 백그라운드 태스크는 slice_ms만큼 실행
            alloc_ms = slice_ms;
        }
        else
        {
            // 일반 태스크는 min(남은 실행 시간, 할당된 quantum)
            alloc_ms = remaining_time[idx] < quantum_ms
                           ? remaining_time[idx]
                           : quantum_ms;
        }

        // alloc_ms 만큼 실행하되, 인터럽트 처리 중이면 일시정지할 예정
        printf("Starting task %d (%s) for up to %d ms\n",
               idx, is_background ? "background" : "normal", alloc_ms);

        // 태스크 함수 호출 -> 결국 while loop에서 이 줄 다음부터 해당 task가 실행되는 것
        tasks[idx].func();

        // 할당된 시간만큼 실행하되, 인터럽트 처리 중이면 일시정지
        int slept = 0;
        while (slept < alloc_ms)
        {
            // 인터럽트 처리 중이면 현재 태스크 일시정지 (slept 값 유지)
            if (interrupt_processing) {
                printf("Task %d paused at %d ms - interrupt processing\n", idx, slept);
                while (interrupt_processing) {
                    usleep(1000); // 인터럽트 처리 완료까지 대기
                }
                printf("Task %d resumed at %d ms - interrupt processing done\n", idx, slept);
            }
            
            usleep(1000); // 1 ms 지연
            slept++;
        }

        printf(" → Task %d ran for %d ms\n", idx, slept);

        // 남은 실행 시간 갱신 (백그라운드 태스크는 제외)
        if (!is_background)
        {
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

    printf("RTOS stopped.\n");
}

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "rtos.h"

// task 정보를 저장하는 배열
Task tasks[MAX_TASKS];
int task_count = 0;

// system 시작 후 발생한 1ms tick의 총 수를 저장
volatile uint32_t ticks = 0;

// task 선점 요청 flag
volatile bool yield_flag = false;

static HANDLE timer_handle;
static volatile int current_task_index = 0;

// 스케줄러 초기화 함수
int init_scheduler(void)
{
    LARGE_INTEGER due_time;

    // Waitable Timer 생성
    timer_handle = CreateWaitableTimer(NULL, FALSE, NULL);
    if (timer_handle == NULL)
    {
        perror("CreateWaitableTimer");
        return -1;
    }

    // 타이머 간격 설정 1ms
    due_time.QuadPart = -10000LL;
    if (!SetWaitableTimer(timer_handle, &due_time, 1, NULL, NULL, FALSE))
    {
        perror("SetWaitableTimer");
        CloseHandle(timer_handle);
        return -1;
    }

    printf("Scheduler initialized with 1ms tick.\n");
    return 0;
}

// 새로운 task 등록
void register_task(int period, void (*f)(void))
{
    if (task_count < MAX_TASKS)
    {
        tasks[task_count].period_ms = period;
        tasks[task_count].func = f;
        task_count++;
        printf("Task registered at index %d, period: %d ms\n", task_count - 1, period);
    }
    else
    {
        fprintf(stderr, "Error: Maximum number of tasks reached.\n");
    }
}

void rtos_start(void)
{
    if (task_count == 0)
    {
        fprintf(stderr, "Error: No tasks registered. Cannot start scheduler.\n");
        return;
    }

    printf("RTOS started. Running tasks...\n");

    int idx = 0;

    while (1)
    {
        // Wait for the next timer tick (1ms)
        if (WaitForSingleObject(timer_handle, INFINITE) == WAIT_OBJECT_0)
        {
            ticks++; // Increment global tick counter

            // Handle yield flag for preemptive context switching
            if (yield_flag)
            {
                yield_flag = false;           // Reset yield flag
                idx = (idx + 1) % task_count; // Move to the next task
            }

            // Get the current task
            Task *t = &tasks[idx];

            // Check if the task's period matches the current ticks
            if (ticks % t->period_ms == 0)
            {
                printf("Running task at index %d (period: %d ms, ticks: %u)\n", idx, t->period_ms, ticks);
                t->func(); // Execute the task function
            }

            // Move to the next task for the next tick (Round-Robin)
            idx = (idx + 1) % task_count;
        }
        else
        {
            perror("WaitForSingleObject");
            break; // Exit loop on error
        }
        Sleep(0); // Give other threads a chance to run (optional)
    }

    CloseHandle(timer_handle);
    printf("RTOS stopped.\n");
}
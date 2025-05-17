#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include "rtos.h" // 공통 자료형 및 상수 포함

static int timer_fd;
static Task tasks[MAX_TASKS];
static int task_count = 0;

int init_scheduler(void)
{
    struct sigevent sev;
    struct itimerspec its;

    // 타이머 생성 (timerfd 사용)
    timer_fd = timerfd_create(CLOCK_MONOTONIC_RAW, 0);
    if (timer_fd == -1)
    {
        perror("timerfd_create");
        return -1;
    }

    // 타이머 간격 설정 (1ms)
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = 1000000; // 1 ms
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 1000000; // 1 ms

    if (timerfd_settime(timer_fd, 0, &its, NULL) == -1)
    {
        perror("timerfd_settime");
        close(timer_fd);
        return -1;
    }

    printf("Scheduler initialized with 1ms tick (using timerfd).\n");
    return 0;
}

void register_task(int period_ms, void (*func)(void))
{
    if (task_count < MAX_TASKS)
    {
        tasks[task_count].period_ms = period_ms;
        tasks[task_count].func = func;
        task_count++;
        printf("Task registered at index %d, period: %d ms\n", task_count - 1, period_ms);
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
    uint64_t expirations;

    while (1)
    {
        // Wait for the next timer tick
        if (read(timer_fd, &expirations, sizeof(expirations)) > 0)
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
            perror("read(timer_fd)");
            break; // Exit loop on error
        }
        usleep(10); // Small delay to reduce busy-waiting (optional, but can be helpful)
    }

    close(timer_fd);
    printf("RTOS stopped.\n");
}
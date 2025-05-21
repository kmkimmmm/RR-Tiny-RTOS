#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include "rtos.h"

static int timer_fd;          // 타이머 파일 디스크립터
static struct itimerspec its; // 타이머의 초기 만료 시간, 만료 반복 주기를 저장하기 위한 구조체 선언
static Task tasks[MAX_TASKS]; // Task 구조체를 담는 배열
static int task_count = 0;    // 배열에 저장된 task의 수 = 스케쥴러에 등록된 task의 수
static uint64_t ticks = 0;    // 1ms가 몇 번 지났는가? = tick
static int yield_flag = 0;    // 선점 요청 정보를 담는 변수

// 스케쥴러 초기화 함수 == 타이머 생성하기
int init_scheduler(void)
{
    // 타이머 간격 설정 (1ms)
    its.it_value.tv_sec = 0;                              // 타이머 초기 만료 시간 (s)
    its.it_value.tv_nsec = DEFAULT_SLICE_MS * 1000000;    // 타이머 초기 만료 시간 (ns) = 1 ms (default)
    its.it_interval.tv_sec = 0;                           // 타이머의 반복 간격 시간 (s)
    its.it_interval.tv_nsec = DEFAULT_SLICE_MS * 1000000; // 타이머의 반복 간격 시간(ns) = 1ms (default)

    // timerfd_create -> 타이머 생성 함수
    // 성공적이라면 non-negative 정수를 반환
    // 생성에 실패 했다면 -1을 반환
    timer_fd = timerfd_create(CLOCK_MONOTONIC_RAW, 0);
    if (timer_fd == -1)
    {
        perror("timerfd_create");
        return -1;
    }

    // 타이머 속성 설정
    // 위에서 정의한 초기 만료 시간, 반복 간격 시간을 timer에 적용하는 것
    if (timerfd_settime(timer_fd, 0, &its, NULL) == -1)
    {
        perror("timerfd_settime");
        close(timer_fd);
        return -1;
    }

    printf("Scheduler initialized with 1ms tick (using timerfd).\n");
    return 0;
}

// task 배열에 새로운 구조체 추가 함수
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

// 스케쥴링 main 함수
void rtos_start(void)
{
    // 등록 된 task가 없다면
    if (task_count == 0)
    {
        fprintf(stderr, "Error: No tasks registered. Cannot start scheduler.\n");
        return;
    }

    printf("RTOS started. Running tasks...\n");

    int idx = 0; // 실행할 task의 index를 가리키는 변수
    unit64_t start_ticks = ticks;

    while (1)
    {
        // 성공적으로 타이머 만료 횟수 정보를 읽어 왔다면
        // == 1ms의 시간이 흘렀다면
        if (read(timer_fd, &expirations, sizeof(expirations)) > 0)
        {
            ticks++; // 여기서 tick은 1ms가 몇 번 흘렀는지를 나타냄

            // 선점 요청 플래그 == True일 경우
            if (yield_flag)
            {
                yield_flag = false;
                idx++;
                if (idx > task_count)
                {
                    break;
                }
                start_ticks = ticks;
            }

            // 현재의 task 가져오기
            Task *t = &tasks[idx];

            if ((ticks - start_ticks) < (t->period_ms) * slice_ms)
            {
                printf("Running task %d for %d ms (ticks=%llu)\n",
                       idx, t->period_ms, (unsigned long long)ticks);
                t->func();
            }

            else
            {
                // 할당된 시간이 끝났으니 선점 요청
                yield_flag = 1;
            }
        }

        else
        {
            perror("read(timer_fd)");
            break;
        }
        usleep(10); // optional
    }

    close(timer_fd);
    printf("RTOS stopped.\n");
}
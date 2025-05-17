#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include "rtos.h"

static int timer_fd;          // 타이머 파일 디스크립터
static Task tasks[MAX_TASKS]; // task 구조체를 저장하는 array
static int task_count = 0;    // 스케줄러에 등록된 태스크 수

// 스케쥴러 초기화 함수
int init_scheduler(void)
{
    // struct sigevent sev;
    struct itimerspec its;

    // 타이머 생성 (timerfd 사용)
    timer_fd = timerfd_create(CLOCK_MONOTONIC_RAW, 0);
    if (timer_fd == -1)
    {
        perror("timerfd_create");
        return -1;
    }

    // 타이머 간격 설정 (1ms)
    its.it_value.tv_sec = 0;           // 타이머 초기 만료 시간 (s)
    its.it_value.tv_nsec = 1000000;    // 타이머 초기 만료 시간 (ns) = 1 ms
    its.it_interval.tv_sec = 0;        // 타이머의 반복 간격 시간 (s)
    its.it_interval.tv_nsec = 1000000; // 타이머의 반복 간격 시간(ns) = 1ms

    // 타이머 속성 설정
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
    if (task_count == 0)
    {
        fprintf(stderr, "Error: No tasks registered. Cannot start scheduler.\n");
        return;
    }

    printf("RTOS started. Running tasks...\n");

    int idx = 0; // 실행할 task의 index를 가리키는 변수
    uint64_t expirations;

    while (1)
    {
        // 성공적으로 타이머 만료 횟수 정보를 읽어 왔다면
        // == 1ms의 시간이 흘렀다면
        if (read(timer_fd, &expirations, sizeof(expirations)) > 0)
        {
            ticks++; // tick 카운터 ++

            // 선점 요청 플래그 == True일 경우
            if (yield_flag)
            {
                yield_flag = false;
                idx = (idx + 1) % task_count; // 다음 태스크 이동 + Round Robin 방식
            }

            // 현재의 task 가져오기
            Task *t = &tasks[idx];

            if (ticks % t->period_ms == 0) // 태그크 실행 주기인지 확인
            {
                printf("Running task at index %d (period: %d ms, ticks: %u)\n", idx, t->period_ms, ticks);
                t->func(); // 태스크 함수를 실행
            }

            idx = (idx + 1) % task_count; // 다음 태스크 이동 + Round Robin 방식
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
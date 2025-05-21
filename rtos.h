// rtos.h
#ifndef RTOS_H
#define RTOS_H

#include <stdint.h>

// 최대 task 수 설정
#define MAX_TASKS 16
#define DEFAULT_SLICE_MS 1

// task의 정보를 담는 구조체
typedef struct
{
    int period_ms;      // 실행 주기
    void (*func)(void); // task가 실행할 함수
} Task;

// 전역 변수 (rtos_scheduler.c에서 정의)
extern Task tasks[MAX_TASKS];   // Task 구조체를 담는 배열
extern int task_count;          // 배열에 저장된 task의 수
extern volatile uint64_t ticks; // 1ms가 몇 번 지났는가? = tick
extern volatile int yield_flag; // 선점 요청 정보를 담는 변수

// 스케쥴러에서 사용할 함수
void register_task(int period_ms, void (*f)(void));
void rtos_start(void);
void update_timeslice(int new_slice_ms);

#endif

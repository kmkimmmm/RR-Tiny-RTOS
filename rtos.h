#ifndef RTOS_H
#define RTOS_H

#include <stdint.h>
#include <stdbool.h>

// 최대 task 개수 정의 -> 조정 가능
#define MAX_TASKS 10
// 기본 타임 슬라이스 (현재는 Round-Robin 이므로 직접 사용하지 않음)
#define DEFAULT_SLICE_MS 10

// Task Structure
typedef struct
{
    int period_ms;      // task 실행 주기 (ms)
    void (*func)(void); // task 실행 함수 포인터
} Task;

// 태스크 배열 (rtos_scheduler.c에서 정의)
extern Task tasks[MAX_TASKS];
extern int task_count;

// 시스템 틱 (rtos_scheduler.c에서 관리)
extern volatile uint32_t ticks;

// 선점 요청 플래그 (isr_push.c에서 설정, rtos_scheduler.c에서 처리)
extern volatile bool yield_flag;

// 함수 원형 선언
void register_task(int period, void (*f)(void));
void rtos_start(void);

#endif 
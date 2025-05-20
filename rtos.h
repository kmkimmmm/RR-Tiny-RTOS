//rtos.h

#ifndef RTOS_H
#define RTOS_H

#include <stdint.h>

/* 스케줄러 설정 */
#define MAX_TASKS        16    /* 최대 태스크 수 */
#define DEFAULT_SLICE_MS 1     /* 기본 타임 슬라이스 (ms) */

/* 태스크 정보 구조체 */
typedef struct {
    int            period_ms;  /* 실행 주기 (ms) */
    void (*func)(void);        /* 태스크 함수 포인터 */
} Task;

/* 스케줄러 전역 변수 (rtos_scheduler.c에 정의) */
extern Task           tasks[MAX_TASKS];
extern int            task_count;
extern volatile uint64_t ticks;
extern volatile int      yield_flag;
extern int            slice_ms;

/* 스케줄러 API */
void register_task(int period_ms, void (*f)(void));
void rtos_start(void);

#endif /* RTOS_H */

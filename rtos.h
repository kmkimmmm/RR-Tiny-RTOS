#ifndef RTOS_H
#define RTOS_H

#include <stdint.h>

// 최대 태스크 개수 정의
#define MAX_TASKS 10 

// 기본 타임 슬라이스 (ms) 정의
#define DEFAULT_SLICE_MS 10 

// 태스크 구조체 정의
typedef struct Task
{
    int period_ms;      // 태스크 주기 (ms)
    void (*func)(void); // 태스크가 수행할 함수 포인터
    // 필요한 다른 태스크 관련 정보 (예: 상태, 우선순위 등)를 여기에 추가할 수 있습니다.
} Task;

// 시스템 틱 카운터 (volatile로 선언하여 인터럽트 핸들러에서도 안전하게 접근 가능하도록 함)
extern volatile uint32_t ticks;

// 선점 요청 플래그 (volatile로 선언하여 인터럽트 핸들러에서도 안전하게 접근 가능하도록 함)
extern volatile uint8_t yield_flag;

// 태스크 등록 함수 원형
int register_task(int period, void (*f)(void));

// RTOS 시작 함수 원형
void rtos_start(void);

#endif // RTOS_H
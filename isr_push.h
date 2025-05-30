#ifndef ISR_PUSH_H
#define ISR_PUSH_H

// 전역 변수 선언
extern volatile int interrupt_processing;  // 인터럽트 처리 중 플래그

// 함수 선언
void* push_monitor(void* arg);

void led_toggle(void); // 만약 외부에서 LED 토글이 필요하다면

#endif // ISR_PUSH_H
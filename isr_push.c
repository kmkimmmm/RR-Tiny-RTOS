// isr push

#include <pthread.h>
#include <unistd.h>
#include "device_io.h"
#include "rtos.h"
#include <stdio.h>
#include <stdint.h>

extern int fd_push; // device_io.c (재훈) 에서 정의한 push 관련 파일 디스크립터
volatile int yield_flag = 0;  // 스케쥴러 선점 요청 플래그 (규민이에게 전달)

static uint8_t led_state = 0; // LED 토글 상태를 관리하기 위한 변수
static int interrupt_processing = 0; // 인터럽트 처리 중 플래그

// LED 상태를 반전시키는 함수
void led_toggle(void) {
    led_state = ~led_state;
    led_write(led_state);
}

// Push Swtich를 모니터링하는 스레드 선언
void* push_monitor(void* arg) {
    int previous_state = 0;  // 이전 버튼 상태 저장 (0: 안눌림, 1: 눌림)

    while (1) {
        // 50ms 주기적 체크 (기존 poll 타임아웃과 동일)
        usleep(50000); // 50ms = 50,000 마이크로초
        
        // 현재 버튼 상태 읽기
        int current_state = push_read();
        
        // 버튼이 눌렸는지 확인 (0 → 1 변화) && 인터럽트 처리 중이 아닐 때만
        if (current_state && !previous_state && !interrupt_processing) {
            // 인터럽트 처리 시작
            interrupt_processing = 1;
            
            // 푸시 인터럽트 처리: 한 번만 실행
            printf("[ISR] Push button interrupt detected!\n");
            
            // LED 토글
            led_toggle();
            
            // LCD에 인터럽트 메시지 표시
            lcd_write("INTERRUPT! Check LED, Motor and Buzzer");
            
            // 모터 즉시 정지
            motor_stop();
            printf("[ISR] Motor stopped\n");
            
            // 버저 켜기
            buzzer_on();
            printf("[ISR] Buzzer ON\n");
            
            // 스케줄러 선점 요청
            yield_flag = 1;
            
            // 5초 후 버저 끄기
            usleep(5000000); // 5000ms (5초) 대기
            buzzer_off();
            printf("[ISR] Buzzer OFF (interrupt processing completed)\n");
            
            // 인터럽트 처리 완료
            interrupt_processing = 0;
        }
        
        // 버튼을 뗐을 때 previous_state 업데이트 (다음 누름을 위해)
        if (!current_state && previous_state) {
            printf("[ISR] Push button released - ready for next interrupt\n");
        }
        
        // 이전 상태 업데이트
        previous_state = current_state;
        
        // 디바운스를 위한 짧은 대기
        usleep(10000); // 10ms
    }
    return NULL;
}
// isr push

#include <pthread.h>
#include <poll.h>
#include <unistd.h>
#include "device_io.h"
#include "rtos.h"
#include <stdio.h>
#include <stdint.h>

extern int fd_push; // device_io.c에서 open됨
extern volatile int yield_flag;
extern volatile int buzzer_flag; // task_buzzer와 연동

static uint8_t led_state = 0; // LED 토글 상태 관리

void led_toggle(void) {
    led_state = ~led_state;
    led_write(led_state);
}

void* push_monitor(void* arg) {
    struct pollfd pfd;
    pfd.fd = fd_push;
    pfd.events = POLLIN;

    while (1) {
        int ret = poll(&pfd, 1, -1); // 무한 대기
        if (ret > 0 && (pfd.revents & POLLIN)) {
            int btn = push_read();
            if (btn) { // 버튼 눌림 감지
                led_toggle();
                lcd_write("INT!");
                buzzer_flag = 1; // task_buzzer에서 처리
                yield_flag = 1;  // 스케줄러 선점 요청
            }
            // 잔여 입력 버퍼 비우기(필요시)
            usleep(10000); // 디바운스 (10ms)
        }
    }
    return NULL;
}
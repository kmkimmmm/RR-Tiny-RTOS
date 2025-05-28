// isr push

#include <pthread.h>
#include <poll.h>
#include <unistd.h>
#include "device_io.h"
#include "rtos.h"
#include <stdio.h>
#include <stdint.h>
#include "task.h"

extern int fd_push; // device_io.c (재훈) 에서 정의한 push 관련 파일 디스크립터
volatile int yield_flag = 0;  // 스케쥴러 선점 요청 플래그 (규민이에게 전달)
volatile int buzzer_flag = 0; // 버저 요청 플래그 (성빈이에게 전달)

static uint8_t led_state = 0; // LED 토글 상태를 관리하기 위한 변수

// LED 상태를 반전시키는 함수
void led_toggle(void) {
    led_state = ~led_state;
    led_write(led_state);
}

// Push Swtich를 모니터링하는 스레드 선언
void* push_monitor(void* arg) {
    struct pollfd pfd;
    pfd.fd = fd_push;
    pfd.events = POLLIN;

    while (1) {
        int ret = poll(&pfd, 1, -1); // poll()을 사용해서 Push Switch 입력을 비동기적으로 감지
        if (ret > 0 && (pfd.revents & POLLIN)) {
            int btn = push_read();
            if (btn) { // 버튼 눌림이 감지된다면 눌림 감지
                led_toggle(); // LED 상태를 반전시키고
                lcd_write("INT!"); // LCD에 "INT!"를 표시하고
                buzzer_flag = 1; // 버저 요청 플래그를 설정해서 성빈이 task에 전달
                task_buzzer();   // 버저 요청 플래그를 설정해서 성빈이 task에 전달
                yield_flag = 1;  // 스케쥴러 선점 요청 플래그를 설정해서 규민이 scheduler에 전달
            }
            
            usleep(10000); // 디바운스 (10ms)
        }
    }
    return NULL;
}
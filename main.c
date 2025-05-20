// main

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "rtos.h"
#include "device_io.h"
#include "task.h"
#include "config.h"
#include "isr_push.h" // push_monitor 함수 원형 선언

int main() {
    // (선택) 드라이버 모듈/노드 초기화
    // init_drivers();

    // 1. 디바이스 초기화
    device_init();

    // 2. 태스크 등록
    register_task(1000, task_led);
    register_task(500,  task_fnd);
    register_task(200,  task_dot);
    register_task(1000, task_lcd);
    register_task(20,   task_motor);
    register_task(100,  config_task);

    // 3. Push ISR 스레드 생성
    pthread_t push_thread;
    pthread_create(&push_thread, NULL, push_monitor, NULL);

    // 4. RTOS 메인 루프 시작
    rtos_start();

    // (선택) 종료 시 리소스 정리
    // pthread_join(push_thread, NULL);

    return 0;
}
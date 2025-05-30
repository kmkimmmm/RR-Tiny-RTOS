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

    // 1. 디바이스 초기화
    device_init();

    // 2. 태스크 등록
    // === 일반 태스크 (정해진 실행 시간, 완료 후 종료) ===
    register_task(30000,  task_led);    // 30초  - LED 패턴 변경
    register_task(30000,  task_fnd);    // 30초  - FND 숫자 증가  
    register_task(30000,  task_dot);    // 30초  - DOT 패턴 변경
    register_task(30000, task_lcd);    //  30초 - LCD 틱 표시
    register_task(30000,  task_motor);  // 30초  - 모터 제어 (새로 추가)
    
    // === 백그라운드 태스크 (무한 실행, 종료 조건에서 제외) ===
    register_task(-1,    task_config); // 무한 - DIP 스위치 설정 읽기

    // 3. Push ISR 스레드 생성
    pthread_t push_thread;
    pthread_create(&push_thread, NULL, push_monitor, NULL);

    // 4. RTOS 메인 루프 시작
    rtos_start();

    // (선택) 종료 시 리소스 정리
    // pthread_join(push_thread, NULL);

    return 0;
}
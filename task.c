// task

#ifndef TASKS_C
#define TASKS_C

/*
 * tasks.c
 * 역할: FPGA 8개 장치 제어 태스크 구현 및 드라이버 자동 초기화
 *
 * - init_drivers(): 프로그램 시작 시 한 번 호출하여 모든 드라이버 모듈 로드 및 디바이스 노드 생성
 * - 각 task_* 함수는 rtos_scheduler에서 주기적으로 호출되며,
 *   device_io 모듈의 래퍼 함수를 통해 하드웨어와 상호작용합니다.
 */

#include "rtos.h"         // RTOS 공통 구조체, register_task, rtos_start
#include "device_io.h"    // led_write, fnd_write, dot_write, lcd_write_fmt, buzzer_beep, set_motor_pwm
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

extern volatile uint32_t ticks;    // 시스템 틱 (rtos_scheduler.c)
extern volatile int buzzer_flag;   // ISR에서 설정되는 버저 플래그

/**
 * init_drivers()
 * - 동작: 셸 명령(system) 호출로 각 FPGA 드라이버 모듈 insmod 및 mknod 실행
 * - 위치: main() 시작 직후 호출
 */
void init_drivers(void) {
    // LED Driver
    system("cd ~/fpga_example/fpga_led && make clean && make");
    system("sudo insmod fpga_led_driver.ko");
    system("sudo mknod /dev/fpga_led c 260 0");

    // FND Driver
    system("cd ~/fpga_example/fpga_fnd && make clean && make");
    system("sudo insmod fpga_fnd_driver.ko");
    system("sudo mknod /dev/fpga_fnd c 261 0");

    // DOT Matrix Driver
    system("cd ~/fpga_example/fpga_dot && make clean && make");
    system("sudo insmod fpga_dot_driver.ko");
    system("sudo mknod /dev/fpga_dot c 262 0");

    // Text LCD Driver
    system("cd ~/fpga_example/fpga_text_lcd && make clean && make");
    system("sudo insmod fpga_text_lcd_driver.ko");
    system("sudo mknod /dev/fpga_text_lcd c 263 0");

    // Buzzer Driver
    system("cd ~/fpga_example/fpga_buzzer && make clean && make");
    system("sudo insmod fpga_buzzer_driver.ko");
    system("sudo mknod /dev/fpga_buzzer c 264 0");

    // Push Switch Driver
    system("cd ~/fpga_example/fpga_push_switch && make clean && make");
    system("sudo insmod fpga_push_switch_driver.ko");
    system("sudo mknod /dev/fpga_push_switch c 265 0");

    // DIP Switch Driver
    system("cd ~/fpga_example/fpga_dip_switch && make clean && make");
    system("sudo insmod fpga_dip_switch_driver.ko");
    system("sudo mknod /dev/fpga_dip_switch c 266 0");

    // Step Motor Driver
    system("cd ~/fpga_example/fpga_step_motor && make clean && make");
    system("sudo insmod fpga_step_motor_driver.ko");
    system("sudo mknod /dev/fpga_step_motor c 267 0");
}

/**
 * task_led()
 * - 주기: 1000 ms
 * - 장치: LED (/dev/fpga_led)
 * - 동작: 8비트 패턴을 좌측 시프트하며 순환 점등
 */
void task_led(void) {
    static uint8_t pattern = 1;
    led_write(pattern);
    pattern <<= 1;
    if (pattern == 0) pattern = 1;
}

/**
 * task_fnd()
 * - 주기: 500 ms
 * - 장치: FND (/dev/fpga_fnd)
 * - 동작: 내부 카운터 값을 7-세그먼트에 표시
 */
void task_fnd(void) {
    static uint16_t counter = 0;
    fnd_write(counter);
    counter++;
}

// 도트 매트릭스 애니메이션 프레임 배열 (3프레임 예시)
static const uint8_t dot_patterns[][8] = {
    {0x18,0x24,0x42,0x81,0x81,0x42,0x24,0x18},
    {0x00,0x66,0xFF,0x7E,0x3C,0x18,0x18,0x00},
    {0x00,0x18,0x3C,0x7E,0x7E,0x3C,0x18,0x00}
};
#define DOT_FRAMES (sizeof(dot_patterns)/sizeof(dot_patterns[0]))

/**
 * task_dot()
 * - 주기: 200 ms
 * - 장치: DOT Matrix (/dev/fpga_dot)
 * - 동작: dot_patterns 순환 출력
 */
void task_dot(void) {
    static size_t idx = 0;
    dot_write(dot_patterns[idx]);
    idx = (idx + 1) % DOT_FRAMES;
}

/**
 * task_lcd()
 * - 주기: 1000 ms
 * - 장치: Text LCD (/dev/fpga_text_lcd)
 * - 동작: "Tick:<ticks>" 문자열 출력
 */
void task_lcd(void) {
    char buf[32];
    snprintf(buf, sizeof(buf), "Tick:%u", ticks);
    lcd_write_fmt(buf);
}

/**
 * task_buzzer()
 * - 주기: 100 ms 혹은 필요시
 * - 장치: Buzzer (/dev/fpga_buzzer)
 * - 동작: ISR에서 set된 buzzer_flag 확인 후 50 ms 비프
 */
void task_buzzer(void) {
    if (buzzer_flag) {
        buzzer_beep(50);
        buzzer_flag = 0;
    }
}

/**
 * task_motor()
 * - 주기: 20 ms
 * - 장치: Step Motor (/dev/fpga_step_motor)
 * - 동작: config_task에서 설정된 PWM 유지
 */
void task_motor(void) {
    // set_motor_pwm()은 config_task에서 주기적 호출됨
}

#endif // TASKS_C

/*
 * tasks.c
 * 역할: FPGA 8개 장치 제어 태스크 구현 및 드라이버 자동 초기화
 *
 * - init_drivers(): 프로그램 시작 시 한 번 호출하여 모든 드라이버 모듈 로드 및 디바이스 노드 생성
 * - 각 task_* 함수는 rtos_scheduler에서 주기적으로 호출되며,
 *   device_io 모듈의 래퍼 함수를 통해 하드웨어와 상호작용합니다.
 */

#include "rtos.h"      // RTOS 공통 구조체, register_task, rtos_start
#include "device_io.h" // led_write, fnd_write, dot_write, lcd_write_fmt, buzzer_beep, motor_set_pwm
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define DOT_FRAMES 1
#define DOT_ROWS 10

extern volatile uint64_t ticks; // 시스템 틱 (rtos_scheduler.c)

uint8_t motor_speed = 0;

/**
 * task_led()
 * - 주기: 1000 ms
 * - 장치: LED (/dev/fpga_led)
 * - 동작: 8비트 패턴을 좌측 시프트하며 순환 점등
 */
void task_led(void)
{
    static uint8_t pattern = 1;
    led_write(pattern);
    pattern <<= 1;
    if (pattern == 0)
        pattern = 1;
}

/**
 * task_fnd()
 * - 주기: 500 ms
 * - 장치: FND (/dev/fpga_fnd)
 * - 동작: 내부 카운터 값을 7-세그먼트에 표시
 */
void task_fnd(void)
{
    static uint16_t counter = 0;
    char digits[4];

    // 네 자리 숫자로 분리
    digits[0] = (counter / 1000) % 10; // 천의 자리
    digits[1] = (counter / 100) % 10;  // 백의 자리
    digits[2] = (counter / 10) % 10;   // 십의 자리
    digits[3] = (counter) % 10;        // 일의 자리

    // 분리된 네 자리 값을 한 번에 전송
    // device_io.c 쪽에서 void fnd_write(const char digits[4])로 구현되어야 함
    fnd_write(digits);

    // counter를 0~9999 범위로 순환
    counter = (counter + 1) % 10000;
}

// 10행×7열 하트 모양 프레임 (한 프레임 정의)
static const uint8_t dot_patterns[1][10] = {
    {
        0b0110110, // ▓██▓██▓
        0b1111111, // ███████
        0b1111111, // ███████
        0b1111111, // ███████
        0b1111111, // ███████
        0b0111110, // ▓█████▓
        0b0011100, // ▓▓███▓▓
        0b0001000, // ▓▓▓█▓▓▓
        0b0000000, // ▓▓▓▓▓▓▓
        0b0000000  // ▓▓▓▓▓▓▓
    }};

/**
 * task_dot()
 * - 주기: 200 ms
 * - 장치: DOT Matrix (/dev/fpga_dot)
 * - 동작: dot_patterns 순환 출력
 */
void task_dot(void)
{
    static size_t row_off = 0;
    uint8_t buf[DOT_ROWS];

    // row_off부터 시작해 10행을 모듈러 연산으로 읽어 버퍼에 채움
    for (size_t i = 0; i < DOT_ROWS; i++)
    {
        buf[i] = dot_patterns[0][(row_off + i) % DOT_ROWS];
    }

    // 매트릭스에 스크롤된 데이터를 전송
    dot_write(buf);

    // 다음 호출 때 한 줄 더 스크롤
    row_off = (row_off + 1) % DOT_ROWS;
}

/**
 * task_lcd()
 * - 주기: 1000 ms
 * - 장치: Text LCD (/dev/fpga_text_lcd)
 * - 동작: "Tick:<ticks>" 문자열 출력
 */
void task_lcd(void)
{
    char buf[32];

    for (int i = 0; i < 32; ++i)
        buf[i] = ' ';

    snprintf(buf, sizeof(buf), "Motor_speed:%u", motor_speed);
    lcd_write(buf);
}

/**
 * task_buzzer()
 * - 주기: 100 ms 혹은 필요시
 * - 장치: Buzzer (/dev/fpga_buzzer)
 * - 동작: ISR에서 set된 buzzer_flag 확인 후 50 ms 비프
 */
// void task_buzzer(void) {
//     if (buzzer_flag) {
//         buzzer_on();
//         buzzer_flag = 0;
//     }
// }

/**
 * task_motor()
 * - 주기: 5초
 * - 장치: Step Motor (/dev/fpga_step_motor)
 * - 동작: DIP 스위치 값에 따라 2단계 속도 제어
 */
void task_motor(void)
{
    uint8_t v = dip_read(); // DIP 스위치 값 읽기

    // DIP 스위치 하위 2비트에 따라 모터 속도 결정
    if (v == 0x00)
    {
        motor_speed = 250;
    }
    else
    {
        motor_speed = 10;
    }

    motor_set_pwm(motor_speed);
    printf("Motor speed set to %d%% (DIP: 0x%02X)\n", motor_speed, v);
}

// device_io.c

#include "device_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

/* 전역 파일 디스크립터 - 각 장치 파일의 핸들을 저장 */
int fd_led;    // LED 제어용
int fd_fnd;    // 7-세그먼트 FND 제어용
int fd_dot;    // 도트 매트릭스 제어용
int fd_lcd;    // LCD 제어용
int fd_buz;    // 버저 제어용
int fd_push;   // 푸시 스위치 읽기용
int fd_dip;    // DIP 스위치 읽기용
int fd_motor;  // 모터 제어용

/**
 * 장치 파일을 열고 에러 체크하는 헬퍼 함수
 * path: 열고자 하는 장치 파일 경로 (/dev/fpga_*)
 * flags: open() 플래그 (O_RDONLY: 읽기 전용, O_WRONLY: 쓰기 전용)
 * return: 성공 시 파일 디스크립터, 실패 시 에러 메시지 출력 후 프로그램 종료
 */
static int open_device(const char *path, int flags)
{
    int fd = open(path, flags);
    if (fd < 0)
    {
        char errmsg[64];
        snprintf(errmsg, sizeof(errmsg), "open %s", path);
        perror(errmsg);
        exit(EXIT_FAILURE);
    }
    return fd;
}

/**
 * device_init()
 * 모든 FPGA 장치 파일을 열고 초기화
 * 쓰기 전용 장치: LED, FND, DOT, LCD, 버저, 모터
 * 읽기 전용 장치: 푸시 스위치, DIP 스위치
 */
void device_init(void)
{
    fd_led = open_device("/dev/fpga_led", O_WRONLY);
    fd_fnd = open_device("/dev/fpga_fnd", O_WRONLY);
    fd_dot = open_device("/dev/fpga_dot", O_WRONLY);
    fd_lcd = open_device("/dev/fpga_text_lcd", O_WRONLY);
    fd_buz = open_device("/dev/fpga_buzzer", O_WRONLY);
    fd_push = open_device("/dev/fpga_push_switch", O_RDONLY);
    fd_dip = open_device("/dev/fpga_dip_switch", O_RDONLY);
    fd_motor = open_device("/dev/fpga_step_motor", O_WRONLY);
}

/**
 * LED 출력 함수
 * v: 8비트 (1byte) LED 패턴 (0: 꺼짐, 1: 켜짐)
 */
void led_write(uint8_t v)
{
    write(fd_led, &v, 1);
}

/**
 * 7-세그먼트 FND 출력 함수
 * n: 표시할 16비트 숫자 (2byte)
 */
void fnd_write(uint16_t n)
{
    write(fd_fnd, &n, 2);
}

/**
 * 도트 매트릭스 출력 함수
 * pattern: 8x8 도트 매트릭스 패턴 (8바이트)
 */
void dot_write(const uint8_t pattern[8])
{
    write(fd_dot, pattern, 8);
}

/**
 * LCD 문자열 출력 함수
 * str: 출력할 문자열
 */
void lcd_write(const char *str)
{
    write(fd_lcd, str, strlen(str));
}

/**
 * LCD 포맷 출력 함수 (printf 스타일)
 * fmt: 포맷 문자열
 * ...: 가변 인자
 */
void lcd_write_fmt(const char *fmt, ...)
{
    char buf[64];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    write(fd_lcd, buf, strlen(buf));
}

/**
 * 버저 켜기 함수
 * duration_ms: 비프음 지속 시간 (밀리초)
 */
void buzzer_on(uint32_t duration_ms)
{
    write(fd_buz, &duration_ms, sizeof(duration_ms));
}

/**
 * 버저 끄기 함수
 */
void buzzer_off(void)
{
    uint32_t zero = 0;
    write(fd_buz, &zero, sizeof(zero));
}

/**
 * 푸시 스위치 상태 읽기 함수
 * return: 스위치 상태 (0: 안눌림, 1: 눌림)
 */
int push_read(void)
{
    uint8_t v;
    read(fd_push, &v, 1);
    return v;
}

/**
 * DIP 스위치 상태 읽기 함수
 * return: DIP 스위치 8비트 값
 */
uint8_t dip_read(void)
{
    uint8_t v;
    read(fd_dip, &v, 1);
    return v;
}

/**
 * 모터 PWM 듀티 설정 함수
 * duty: PWM 듀티비 (0-100)
 */
void motor_set_pwm(uint8_t duty)
{
    write(fd_motor, &duty, 1);
}

/**
 * 모터 정지 함수
 */
void motor_stop(void)
{
    uint8_t zero = 0;
    write(fd_motor, &zero, 1);
}

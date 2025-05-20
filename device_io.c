//device\_io.c

// device_io.c
#include "device_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

/* 전역 파일 디스크립터 */
int fd_led;
int fd_fnd;
int fd_dot;
int fd_lcd;
int fd_buz;
int fd_push;
int fd_dip;
int fd_motor;

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
 * 모든 FPGA 디바이스 파일을 open()하고, 각 호출 직후 에러 체크
 */
void device_init(void)
{
    fd_led = open_device("/dev/fpga_led", O_WRONLY);
    fd_fnd = open_device("/dev/fpga_fnd", O_WRONLY);
    fd_dot = open_device("/dev/fpga_dot", O_WRONLY);
    fd_lcd = open_device("/dev/fpga_text", O_WRONLY);
    fd_buz = open_device("/dev/fpga_buzzer", O_WRONLY);
    fd_push = open_device("/dev/fpga_push", O_RDONLY);
    fd_dip = open_device("/dev/fpga_dip", O_RDONLY);
    fd_motor = open_device("/dev/fpga_motor", O_WRONLY);
}

/**
 * LED 출력 (1 byte)
 */
void led_write(uint8_t v)
{
    write(fd_led, &v, 1);
}

/**
 * 7-세그먼트 FND 출력 (2 bytes)
 */
void fnd_write(uint16_t n)
{
    write(fd_fnd, &n, 2);
}

/**
 * Dot 매트릭스 출력 (8 bytes)
 */
void dot_write(uint8_t pattern[8])
{
    write(fd_dot, pattern, 8);
}

/**
 * LCD 문자열 출력
 */
void lcd_write(const char *str)
{
    write(fd_lcd, str, strlen(str));
}

/**
 * LCD 포맷 출력
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
 * 버저 켜기: duration_ms 만큼 비프
 */
void buzzer_on(uint32_t duration_ms)
{
    write(fd_buz, &duration_ms, sizeof(duration_ms));
}

/**
 * 버저 끄기
 */
void buzzer_off(void)
{
    uint32_t zero = 0;
    write(fd_buz, &zero, sizeof(zero));
}

/**
 * 푸시 스위치 읽기 (1 byte)
 */
int push_read(void)
{
    uint8_t v;
    read(fd_push, &v, 1);
    return v;
}

/**
 * 딥 스위치 읽기 (1 byte)
 */
uint8_t dip_read(void)
{
    uint8_t v;
    read(fd_dip, &v, 1);
    return v;
}

/**
 * 모터 PWM 듀티 설정 (1 byte, 0–100)
 */
void motor_set_pwm(uint8_t duty)
{
    write(fd_motor, &duty, 1);
}

/**
 * 모터 정지
 */
void motor_stop(void)
{
    uint8_t zero = 0;
    write(fd_motor, &zero, 1);
}

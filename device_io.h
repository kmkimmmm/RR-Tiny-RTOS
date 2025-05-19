#ifndef DEVICE_IO_H
#define DEVICE_IO_H

#include <stdint.h>
#include <stddef.h>

// 파일 디스크립터 extern 선언
extern int fd_led;
extern int fd_fnd;
extern int fd_dot;
extern int fd_lcd;
extern int fd_buz;
extern int fd_push;
extern int fd_dip;
extern int fd_motor;

// 초기화 함수
void device_init(void);

// LED
void led_write(uint8_t v);

// FND
void fnd_write(uint16_t n);

// DOT 매트릭스
void dot_write(const uint8_t *pattern, size_t len);

// Text LCD
void lcd_write(const char *s);

// Buzzer
void buzzer_beep(uint8_t t_ms);

// Push Switch
uint8_t push_read(void);

// Dip Switch
uint8_t dip_read(void);

// Step Motor
void motor_set_pwm(uint8_t duty);

#endif // DEVICE_IO_H

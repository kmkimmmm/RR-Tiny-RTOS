#ifndef DEVICE_IO_H
#define DEVICE_IO_H

#include <stdint.h>

/* 전역 파일 디스크립터 extern 선언 */
extern int fd_led;
extern int fd_fnd;
extern int fd_dot;
extern int fd_lcd;
extern int fd_buz;
extern int fd_push;
extern int fd_dip;
extern int fd_motor;

/* 디바이스 초기화 */
void device_init(void);

/* 장치별 래퍼 함수 */
void led_write(uint8_t v);
void fnd_write(uint16_t n);
void dot_write(uint8_t pattern[8]);
void lcd_write(const char *str);
void lcd_write_fmt(const char *fmt, ...);
void buzzer_on(uint32_t duration_ms);
void buzzer_off(void);
int  push_read(void);
uint8_t dip_read(void);
void motor_set_pwm(uint8_t duty);
void motor_stop(void);

#endif /* DEVICE_IO_H */
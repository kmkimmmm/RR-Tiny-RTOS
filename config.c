//config

#include "device_io.h"
#include "rtos.h"
#include <stdint.h>

int slice_ms = DEFAULT_SLICE_MS; // rtos.h에 extern 선언
uint8_t motor_pwm = 0;           // 필요시 extern 선언

void config_task(void) {
    uint8_t v = dip_read();

    // 하위 2비트: 타임 슬라이스(ms)
    if ((v & 0x03) == 0)
        slice_ms = 1;
    else
        slice_ms = 5;

    // 상위 6비트: 모터 PWM 듀티비 (0~63 → 0~100)
    motor_pwm = ((v >> 2) & 0x3F) * 100 / 63;
    motor_set_pwm(motor_pwm);
}
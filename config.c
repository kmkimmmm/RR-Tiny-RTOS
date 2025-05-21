// config

#include "device_io.h"
#include "rtos.h"
#include <stdint.h>

int slice_ms = DEFAULT_SLICE_MS; // rtos.h에 extern 선언
uint8_t motor_pwm = 0;           // 필요시 extern 선언

void config_task(void)
{
    uint8_t v = dip_read();
    int new_slice = ((v & 0x03) == 0) ? 1 : 5;

    // 변경된 경우에만 호출
    if (new_slice != slice_ms)
    {
        slice_ms = new_slice;
    }

    // 모터 PWM 설정(기존 로직)
    uint8_t motor_pwm = ((v >> 2) & 0x3F) * 100 / 63;
    motor_set_pwm(motor_pwm);
}
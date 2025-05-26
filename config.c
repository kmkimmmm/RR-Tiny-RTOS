// config

#include "device_io.h"
#include "rtos.h"
#include <stdint.h>

int slice_ms = DEFAULT_SLICE_MS; // RTOS의 타임 슬라이스 설정 (기본값은 rtos.h에 정의)
uint8_t motor_pwm = 0;           // 모터 PWM 값 (0-100 범위)

// 시스템 설정을 관리하는 태스크 (scheduler의 time slice 설정 및 모터 PWM 설정)
void config_task(void)
{
    uint8_t v = dip_read();  // DIP 스위치의 현재 상태를 읽어옴
    int new_slice = ((v & 0x03) == 0) ? 1 : 5;  // 하위 2비트로 타임 슬라이스 설정 (0: 1ms, 1-3: 5ms)

    // 타임 슬라이스가 변경된 경우에만 업데이트
    if (new_slice != slice_ms)
    {
        slice_ms = new_slice;
    }

    // 상위 6비트로 모터 PWM 값을 설정 (0-63 범위를 0-100 범위로 변환)
    uint8_t motor_pwm = ((v >> 2) & 0x3F) * 100 / 63;
    motor_set_pwm(motor_pwm);  // 계산된 PWM 값을 모터에 적용
}
// config

#include "device_io.h"
#include "rtos.h"
#include <stdint.h>
#include "config.h"
#include <stdio.h>

int slice_ms = DEFAULT_SLICE_MS; // RTOS의 타임 슬라이스 설정

/**
 * task_config()
 * - 주기: -1 (백그라운드 태스크)
 * - 장치: DIP Switch (/dev/fpga_dip_switch)
 * - 동작: DIP 스위치 값에 따라 타임슬라이스 설정
 */
void task_config(void)
{
    uint8_t v = dip_read();

    // 하위 2비트로 타임슬라이스 설정
    if (v == 0)
    {
        slice_ms = 1; // 빠른 멀티태스킹
    }
    else
    {
        slice_ms = 5; // 긴 시간 단위 실행
    }

    printf("Config: slice_ms=%d (DIP: 0x%02X)\n", slice_ms, v);
}

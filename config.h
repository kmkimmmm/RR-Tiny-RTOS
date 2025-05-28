#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

// 기본 설정값
#define DEFAULT_SLICE_MS 5

// 전역 변수 선언
extern int slice_ms;

// 함수 선언
void task_config(void);

#endif // CONFIG_H
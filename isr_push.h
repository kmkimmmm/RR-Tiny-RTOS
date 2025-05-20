#ifndef ISR_PUSH_H
#define ISR_PUSH_H

void* push_monitor(void* arg);

void led_toggle(void); // 만약 외부에서 LED 토글이 필요하다면

#endif // ISR_PUSH_H
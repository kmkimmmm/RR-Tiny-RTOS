#ifndef TASKS_H
#define TASKS_H

void task_led(void);
void task_fnd(void);
void task_dot(void);
void task_lcd(void);
void task_buzzer(void);
void task_motor(void);

// (선택) 드라이버 초기화 함수가 필요하다면
void init_drivers(void);

#endif // TASKS_H
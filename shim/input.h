#ifndef INPUT_H
#define INPUT_H

void input_init(void);
void input_poll(void);
int input_x(void);
int input_y(void);
int input_btn(void);
int input_scancode(void); /* -1 if queue empty */

#endif

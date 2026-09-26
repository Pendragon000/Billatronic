//
// Created by isaaf on 9/25/2026.
//

#ifndef KEYPAD_H
#define KEYPAD_H
#include "driver/gpio.h"
typedef struct {
    gpio_num_t rows[4];
    gpio_num_t cols[4];
} keypad_t;

void keypad_init(keypad_t *keypad);

char keypad_read(keypad_t *keypad);
#endif //KEYPAD_H

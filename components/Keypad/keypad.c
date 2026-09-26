//
// Created by isaaf on 9/25/2026.
//

#include "include/keypad.h"
char keypad_chars[][4] = {
    {'D', '#', '0', '*'},
    {'C', '9', '8', '7'},
    {'B', '6', '5', '4'},
    {'A', '3', '2', '1'},
};
void keypad_init(keypad_t *keypad) {
    for (uint8_t i = 0; i < 4; i++) {
        gpio_reset_pin(keypad->cols[i]);
        gpio_set_direction(keypad->cols[i], GPIO_MODE_OUTPUT);
        gpio_set_level(keypad->cols[i], 0);
    }
    for (uint8_t i = 0; i < 4; i++) {
        gpio_reset_pin(keypad->rows[i]);
        gpio_set_direction(keypad->rows[i], GPIO_MODE_INPUT);
        gpio_set_pull_mode(keypad->rows[i], GPIO_PULLDOWN_ONLY);
    }
}
char keypad_read(keypad_t *keypad) {
    for (uint8_t col = 0; col < 4; col++) {
        gpio_set_level(keypad->cols[col], 1);
        for (uint8_t row = 0; row < 4; row++) {
            if (gpio_get_level(keypad->rows[row])) {
                gpio_set_level(keypad->cols[col], 0);
                return keypad_chars[row][col];
            }
        }
        gpio_set_level(keypad->cols[col], 0);
    }
    return '\0';
}
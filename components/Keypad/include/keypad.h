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

///
/// Initialise les cols en output et row en input
///
/// @param keypad Le struct keypad avec tous les row et cols rempli avec leurs GPIO
void keypad_init(keypad_t *keypad);

///
/// Lit le caractère actuellement pesé sur le keypad.
///
/// @param keypad Le keypad initialisée avec qui on veut lire la valeur
///
/// @return La lettre pesé ou retourne un null char.
char keypad_read(keypad_t *keypad);
#endif //KEYPAD_H

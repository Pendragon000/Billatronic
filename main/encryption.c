//
// Created by isaaf on 10/9/2026.
//

#include "encryption.h"
seed = 123456;

uint32_t clk_syncronization() {
    return seed;
}

char* encrypt(char msg[]) {
    char* result = msg ^ seed;
    return result;
}

char* decrypt(char msg[]) {
    char* result = msg ^ seed;
    return result;
}

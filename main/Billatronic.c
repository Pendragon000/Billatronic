#include "ism3.h"
#include "keypad.h"
#include "esp_log.h"
#define TAG "billatronic"
static ism3_t ism3;
static keypad_t keypad;
void keypad_config(keypad_t *init);


void app_main(void)
{
    keypad_config(&keypad);

    while ( 1 )
    {
        char c = keypad_read(&keypad);
        ESP_LOGI( TAG, "Char detecter sur le keypad : %c", c );
    }
}

void keypad_config(keypad_t *init) {
    init->rows[0] = GPIO_NUM_4;
    init->rows[1] = GPIO_NUM_16;
    init->rows[2] = GPIO_NUM_17;
    init->rows[3] = GPIO_NUM_21;

    init->cols[0] = GPIO_NUM_26;
    init->cols[1] = GPIO_NUM_25;
    init->cols[2] = GPIO_NUM_33;
    init->cols[3] = GPIO_NUM_32;
    keypad_init(&keypad);
}

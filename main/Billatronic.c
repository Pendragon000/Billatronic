#include "keypad.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "led.h"
#define TAG "billatronic"
static keypad_t keypad;
void keypad_config(keypad_t *init);

#define MDP "1234"
void app_main(void)
{
    keypad_config(&keypad);
    char buff[50];
    buff[49] = '\0';
    uint8_t currIndex = 0;
    char lastC = '\0';
    while ( 1 )
    {
        char c = keypad_read(&keypad);
        if ( c != '\0' && c != lastC ) {
            switch ( c ) {
                // Delete kept string
                case '*':
                    ESP_LOGI( TAG, "Reseting string");
                    currIndex = 0;
                    break;
                    // Sends string for verif
                case '#':
                    buff[currIndex] = '\0';
                    ESP_LOGI( TAG, "Verif string : %s", buff );
                    if (strcmp(MDP, buff) == 0) {
                        // Light up green led
                    }
                    else {
                        // Light up red led
                    }
                    currIndex = 0;
                    break;
                default:
                    ESP_LOGI( TAG, "Adding char '%c' to string", c);
                    if (currIndex >= 49) {
                        buff[currIndex] = '\0';
                        break;
                    }
                    buff[currIndex] = c;
                    currIndex++;
                    break;
            }
        }
        lastC = c;
        vTaskDelay(pdMS_TO_TICKS(50));
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

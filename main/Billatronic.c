#include "keypad.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "led.h"
#define TAG "billatronic"
static keypad_t keypad;
void keypad_config(keypad_t *init);

#define MDP "45131914"

void app_main(void)
{
    keypad_config(&keypad);

    led_t green = led_init(GPIO_NUM_15);
    led_t red = led_init(GPIO_NUM_5);

    char buff[50];
    // Met un dernier null terminator
    // au cas ou pour quand je compare strcmp.
    buff[49] = '\0';

    uint8_t currIndex = 0;
    char lastC = '\0';

    while ( 1 )
    {
        const char c = keypad_read(&keypad);
        if ( c != '\0' && c != lastC ) {
            switch ( c ) {

                // Delete la string buff
                case '*':
                    ESP_LOGI( TAG, "Reseting string");
                    currIndex = 0;
                    break;

                // vérifie la string avec le mot de passe
                case '#':
                    buff[currIndex] = '\0';
                    ESP_LOGI( TAG, "Verif string : %s", buff );
                    if (strcmp(MDP, buff) == 0) {
                        led_set(&green, true);
                        vTaskDelay(pdMS_TO_TICKS(5000));
                        led_set(&green, false);
                    }
                    else {

                        led_set(&red, true);
                        vTaskDelay(pdMS_TO_TICKS(5000));
                        led_set(&red, false);
                    }
                    currIndex = 0;
                    break;
                    
                // Ajoute le caractère à la string buff
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

///
/// Configure le struct de keyboard avec les bon GPIO pour l'initialisation
///
/// @param init Le keypad à configurer
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

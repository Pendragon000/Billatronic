#include "ism3.h"
#include "keypad.h"
#include "esp_log.h"
#define TAG "billatronic"
static ism3_t ism3;
static keypad_t keypad;
void keypad_config(keypad_t *init);

void app_main(void)
{
    /*ism3_cfg_t cfg;
    ism3_cfg_setup(&cfg);

    if ( ISM3_OK != ism3_init( &ism3, &cfg ) ) { printf("init failed\n"); return; }
    if ( ISM3_OK != ism3_default_cfg( &ism3 ) ) { printf("radio config failed\n"); return; }

    uint8_t msg[ ISM3_PACKET_LEN ] = "hello";
    ism3_transmit_packet( &ism3, msg, sizeof(msg) );*/
    keypad_config(&keypad);
    while (1) {
        char c = keypad_read(&keypad);
        ESP_LOGI(TAG,"%c",c);
        vTaskDelay(10);
    }
}

void keypad_config(keypad_t *init) {
    init->rows[0] = GPIO_NUM_22;
    init->rows[1] = GPIO_NUM_23;
    init->rows[2] = GPIO_NUM_25;
    init->rows[3] = GPIO_NUM_26;

    init->cols[0] = GPIO_NUM_5;
    init->cols[1] = GPIO_NUM_18;
    init->cols[2] = GPIO_NUM_19;
    init->cols[3] = GPIO_NUM_21;
    keypad_init(&keypad);
}

#include "ism3.h"
#include "keypad.h"
#include "esp_log.h"
#define TAG "billatronic"
static ism3_t ism3;
static keypad_t keypad;
void keypad_config(keypad_t *init);
void ism3_config(char type[]);

void app_main_wrover(void) {
    ism3_config("WROVER");
    // Transmit loop
    uint8_t msg[ ISM3_PACKET_LEN ] = "hello";
    while (1) {
        ESP_LOGI(TAG, "Transmitting msg");
        ism3_transmit_packet( &ism3, msg, sizeof(msg) );
        vTaskDelay( 1000 / portTICK_PERIOD_MS );
    }
}
void app_main_wroom(void) {
    ism3_config("WROOM");
    // Recieve loop
    ESP_LOGI( TAG, "Listening..." );

    uint8_t rx_buf[ ISM3_PACKET_LEN + 1 ] = { 0 };   // +1 for a guaranteed terminator
    uint8_t rx_len = 0;

    while ( 1 )
    {
        err_t result = ism3_receive_packet( &ism3, rx_buf, &rx_len );

        if ( ISM3_OK == result )
        {
            rx_buf[ rx_len ] = '\0';               // force-terminate at the actual received length
            ESP_LOGI( TAG, "Received: %s", (char *) rx_buf );
        }
        else
        {
            ESP_LOGI( TAG, "result=%d", result );   // ESP_LOGD stays quiet unless you raise log level
        }
    }
}

void start_esp(char type[]) {
    if (strcmp(type, "WROVER") == 0) {
        app_main_wrover();
    }
    else if (strcmp(type, "WROOM") == 0) {
        app_main_wroom();
    }
}

void app_main(void)
{
    start_esp("WROOM");
}
void ism3_config(char type[]) {
    ism3_cfg_t cfg;
    ism3_cfg_setup(&cfg, type);

    if ( ISM3_OK != ism3_init( &ism3, &cfg ) ) { ESP_LOGE( TAG, "init failed\n"); return; }
    if ( ISM3_OK != ism3_default_cfg( &ism3 ) ) { ESP_LOGE( TAG, "radio config failed\n"); return; }
    if ( ISM3_OK != ism3_check_communication( &ism3 ) ) { ESP_LOGE( TAG, "Chip not responding over SPI!" ); return;}
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

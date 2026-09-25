#include <stdio.h>
#include <driver/spi_master.h>

#include "soc/gpio_num.h"

const gpio_num_t miso_spi = GPIO_NUM_19;
const gpio_num_t mosi_spi = GPIO_NUM_23;
const gpio_num_t clk_spi = GPIO_NUM_18;
const gpio_num_t cs_spi = GPIO_NUM_5;

const spi_bus_config_t bus_cfg = {
    .miso_io_num = miso_spi,
    .mosi_io_num = mosi_spi,
    .sclk_io_num = clk_spi,
    .quadhd_io_num = -1,
    .quadwp_io_num = -1,
    .max_transfer_sz = 4096
};

const spi_device_interface_config_t dev_cfg = {
    .command_bits = 0,
    .address_bits = 0,
    .dummy_bits = 0,
    .clock_speed_hz = 2000000,
    .duty_cycle_pos = 128,      //50% duty cycle
    .mode = 0,
    .spics_io_num = cs_spi,
    .queue_size = 3
};

static spi_device_handle_t spi_handle;

///
///Set up the spi connection with the Mikroe-6066 transciever.
///
static void setup_spi() {
    esp_err_t ret = spi_bus_initialize(SPI3_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);
    // Adds the device to the spi host
    ret = spi_bus_add_device(SPI3_HOST, &dev_cfg, &spi_handle);
    ESP_ERROR_CHECK(ret);
}

void app_main(void)
{
    setup_spi();
}

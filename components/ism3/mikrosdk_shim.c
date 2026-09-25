#include "include/mikrosdk_shim.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_rom_sys.h"   /* esp_rom_delay_us() */

/*
 * Which SPI peripheral/bus to use. ESP32 classic has SPI2_HOST (aka HSPI)
 * and SPI3_HOST (aka VSPI) available for general use. Change this if you
 * need the other bus, or if you're on an ESP32 variant (S2/S3/C3) with
 * different bus naming.
 */
#define SHIM_SPI_HOST   SPI2_HOST

/* Fixed SPI clock. The S2-LP supports much faster SPI, but 1 MHz is a
 * safe starting point -- raise this once basic comms are confirmed. */
#define SHIM_SPI_CLOCK_HZ   1000000

static bool s_bus_initialized = false;

void digital_out_init( digital_out_t *out, pin_name_t pin )
{
    out->pin = pin;
    gpio_reset_pin( (gpio_num_t) pin );
    gpio_set_direction( (gpio_num_t) pin, GPIO_MODE_OUTPUT );
}

void digital_out_high( digital_out_t *out )
{
    gpio_set_level( (gpio_num_t) out->pin, 1 );
}

void digital_out_low( digital_out_t *out )
{
    gpio_set_level( (gpio_num_t) out->pin, 0 );
}

void digital_in_init( digital_in_t *in, pin_name_t pin )
{
    in->pin = pin;
    gpio_reset_pin( (gpio_num_t) pin );
    gpio_set_direction( (gpio_num_t) pin, GPIO_MODE_INPUT );
}

uint8_t digital_in_read( digital_in_t *in )
{
    return (uint8_t) gpio_get_level( (gpio_num_t) in->pin );
}

void spi_master_configure_default( spi_master_config_t *cfg )
{
    cfg->sck  = HAL_PIN_NC;
    cfg->miso = HAL_PIN_NC;
    cfg->mosi = HAL_PIN_NC;
}

err_t spi_master_open( spi_master_t *spi, spi_master_config_t *cfg )
{
    esp_err_t ret;

    if ( !s_bus_initialized )
    {
        spi_bus_config_t bus_cfg = {
            .sclk_io_num = (gpio_num_t) cfg->sck,
            .miso_io_num = (gpio_num_t) cfg->miso,
            .mosi_io_num = (gpio_num_t) cfg->mosi,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .max_transfer_sz = 128,
        };
        ret = spi_bus_initialize( SHIM_SPI_HOST, &bus_cfg, SPI_DMA_CH_AUTO );
        if ( ( ESP_OK != ret ) && ( ESP_ERR_INVALID_STATE != ret ) )
        {
            return SPI_MASTER_ERROR;
        }
        s_bus_initialized = true;
    }

    /*
     * spics_io_num = -1: no hardware-managed CS. ism3.c drives its own CS
     * pin manually via ctx->cs (digital_out_low/high), so the SPI
     * peripheral must NOT toggle any CS line itself.
     */
    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = SHIM_SPI_CLOCK_HZ,
        .mode = 0,
        .spics_io_num = -1,
        .queue_size = 1,
    };

    ret = spi_bus_add_device( SHIM_SPI_HOST, &dev_cfg, &spi->handle );
    if ( ESP_OK != ret )
    {
        return SPI_MASTER_ERROR;
    }

    spi->default_write_data = 0x00;
    return SPI_MASTER_SUCCESS;
}

err_t spi_master_set_default_write_data( spi_master_t *spi, uint8_t data )
{
    spi->default_write_data = data;
    return SPI_MASTER_SUCCESS;
}

err_t spi_master_set_mode( spi_master_t *spi, spi_master_mode_t mode )
{
    /* ism3_cfg_setup always requests SPI_MASTER_MODE_0, which is the only
     * mode the S2-LP needs; mode is already fixed to 0 in spi_master_open
     * above, so nothing further to do here. */
    (void) spi;
    (void) mode;
    return SPI_MASTER_SUCCESS;
}

err_t spi_master_set_speed( spi_master_t *spi, uint32_t speed )
{
    /* Speed is fixed at SHIM_SPI_CLOCK_HZ when the device was added in
     * spi_master_open. Change that #define directly if you need a
     * different speed -- this function intentionally does nothing. */
    (void) spi;
    (void) speed;
    return SPI_MASTER_SUCCESS;
}

void spi_master_set_chip_select_polarity( spi_master_chip_select_polarity_t polarity )
{
    /* CS is driven manually by ism3.c via digital_out_low/high(&ctx->cs),
     * so there is no hardware CS polarity to configure here. */
    (void) polarity;
}

err_t spi_master_read( spi_master_t *spi, uint8_t *data_out, uint16_t len )
{
    if ( 0 == len )
    {
        return SPI_MASTER_SUCCESS;
    }

    uint8_t tx_buf[ 128 ];
    memset( tx_buf, spi->default_write_data, len );

    spi_transaction_t trans = { 0 };
    trans.length    = (size_t) len * 8;   /* length is in BITS */
    trans.tx_buffer = tx_buf;
    trans.rx_buffer = data_out;

    if ( ESP_OK != spi_device_polling_transmit( spi->handle, &trans ) )
    {
        return SPI_MASTER_ERROR;
    }
    return SPI_MASTER_SUCCESS;
}

err_t spi_master_write( spi_master_t *spi, uint8_t *data_in, uint16_t len )
{
    if ( 0 == len )
    {
        return SPI_MASTER_SUCCESS;
    }

    spi_transaction_t trans = { 0 };
    trans.length    = (size_t) len * 8;   /* length is in BITS */
    trans.tx_buffer = data_in;
    trans.rx_buffer = NULL;

    if ( ESP_OK != spi_device_polling_transmit( spi->handle, &trans ) )
    {
        return SPI_MASTER_ERROR;
    }
    return SPI_MASTER_SUCCESS;
}

void Delay_50us( void )  { esp_rom_delay_us( 50 ); }
void Delay_1ms( void )   { vTaskDelay( pdMS_TO_TICKS( 1 ) ); }
void Delay_100ms( void ) { vTaskDelay( pdMS_TO_TICKS( 100 ) ); }
void Delay_1sec( void )  { vTaskDelay( pdMS_TO_TICKS( 1000 ) ); }

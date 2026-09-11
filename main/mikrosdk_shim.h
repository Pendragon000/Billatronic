#ifndef MIKROSDK_SHIM_H
#define MIKROSDK_SHIM_H

/*
 * Minimal replacement for mikroSDK's HAL layer -- just enough to compile
 * MikroE's ism3.c / ism3.h (ISM 3 Click / S2-LP driver) unmodified on an
 * ESP32 using ESP-IDF's native SPI master driver. Pure C, no Arduino.
 *
 * Usage: drop this file + mikrosdk_shim.c into your ESP-IDF component
 * alongside the untouched ism3.c/ism3.h from MikroE's repo.
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Basic types ---- */
typedef int8_t  err_t;
typedef int     pin_name_t;   /* holds a gpio_num_t value */

#define HAL_PIN_NC   (-1)

/* ---- Return codes (0 = success, nonzero = error, matches ISM3_OK/ISM3_ERROR) ---- */
#define SPI_MASTER_SUCCESS   0
#define SPI_MASTER_ERROR    -1

/* ---- SPI mode / CS polarity enums (only MODE_0 / ACTIVE_LOW are actually
 *      used by ism3_cfg_setup, but defined fully for completeness) ---- */
typedef enum {
    SPI_MASTER_MODE_0 = 0,
    SPI_MASTER_MODE_1,
    SPI_MASTER_MODE_2,
    SPI_MASTER_MODE_3
} spi_master_mode_t;

typedef enum {
    SPI_MASTER_CHIP_SELECT_POLARITY_ACTIVE_LOW = 0,
    SPI_MASTER_CHIP_SELECT_POLARITY_ACTIVE_HIGH
} spi_master_chip_select_polarity_t;

/* ---- Digital I/O objects: just remember which GPIO pin they wrap ---- */
typedef struct {
    pin_name_t pin;
} digital_out_t;

typedef struct {
    pin_name_t pin;
} digital_in_t;

/* ---- SPI master object: holds the ESP-IDF device handle plus the
 *      "default write byte" sent out during register reads (ism3.c
 *      relies on this dummy-byte-clocking pattern heavily) ---- */
typedef struct {
    spi_device_handle_t handle;
    uint8_t             default_write_data;
} spi_master_t;

/* ---- SPI config object passed into spi_master_open ---- */
typedef struct {
    pin_name_t sck;
    pin_name_t miso;
    pin_name_t mosi;
} spi_master_config_t;

/* ---- Function prototypes (implemented in mikrosdk_shim.c) ---- */
void  digital_out_init( digital_out_t *out, pin_name_t pin );
void  digital_out_high( digital_out_t *out );
void  digital_out_low( digital_out_t *out );

void    digital_in_init( digital_in_t *in, pin_name_t pin );
uint8_t digital_in_read( digital_in_t *in );

void  spi_master_configure_default( spi_master_config_t *cfg );
err_t spi_master_open( spi_master_t *spi, spi_master_config_t *cfg );
err_t spi_master_set_default_write_data( spi_master_t *spi, uint8_t data );
err_t spi_master_set_mode( spi_master_t *spi, spi_master_mode_t mode );
err_t spi_master_set_speed( spi_master_t *spi, uint32_t speed );
void  spi_master_set_chip_select_polarity( spi_master_chip_select_polarity_t polarity );
err_t spi_master_read( spi_master_t *spi, uint8_t *data_out, uint16_t len );
err_t spi_master_write( spi_master_t *spi, uint8_t *data_in, uint16_t len );

/* ---- Delay helpers called directly by ism3.c ---- */
void Delay_50us( void );
void Delay_1ms( void );
void Delay_100ms( void );
void Delay_1sec( void );

#ifdef __cplusplus
}
#endif

#endif /* MIKROSDK_SHIM_H */

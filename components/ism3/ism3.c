/****************************************************************************
** Copyright (C) 2026 MikroElektronika d.o.o.
** Contact: https://www.mikroe.com/contact
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
** The above copyright notice and this permission notice shall be
** included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
** OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
** DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
** OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
**  USE OR OTHER DEALINGS IN THE SOFTWARE.
****************************************************************************/

/*!
 * @file ism3.c
 * @brief ISM 3 Click Driver.
 */

#include "include/ism3.h"

/**
 * @brief Dummy data.
 * @details Definition of dummy data.
 */
#define DUMMY  0x00
#define ISM3_MAX_XFER 130
static err_t ism3_transfer ( ism3_t *ctx, uint8_t hdr, uint8_t arg,
                             const uint8_t *tx, uint8_t *rx, size_t len )
{
    uint8_t txb[ 2 + ISM3_MAX_XFER ] = { hdr, arg };
    uint8_t rxb[ 2 + ISM3_MAX_XFER ] = { 0 };
    if ( len > ISM3_MAX_XFER ) return ISM3_ERROR;
    if ( tx ) memcpy( &txb[ 2 ], tx, len );

    spi_transaction_t t = {
        .length    = ( 2 + len ) * 8,
        .tx_buffer = txb,
        .rx_buffer = rxb,
    };
    if ( ESP_OK != spi_device_transmit( ctx->spi, &t ) ) return ISM3_ERROR;

    ctx->status = ( ( uint16_t ) rxb[ 0 ] << 8 ) | rxb[ 1 ];
    if ( rx ) memcpy( rx, &rxb[ 2 ], len );
    return ISM3_OK;
}
void ism3_cfg_setup ( ism3_cfg_t *cfg )
{
    cfg->sck = GPIO_NUM_18;  cfg->miso = GPIO_NUM_19;
    cfg->mosi = GPIO_NUM_23; cfg->cs = GPIO_NUM_5;
    cfg->rst = GPIO_NUM_21;  cfg->gp0 = GPIO_NUM_22;   // pick your own free pins
    cfg->gp1 = GPIO_NUM_25;  cfg->gp2 = GPIO_NUM_26;
    cfg->spi_speed = 1000000;
}

err_t ism3_init ( ism3_t *ctx, ism3_cfg_t *cfg )
{
    spi_bus_config_t bus = {
        .sclk_io_num = cfg->sck, .miso_io_num = cfg->miso, .mosi_io_num = cfg->mosi,
        .quadwp_io_num = -1, .quadhd_io_num = -1, .max_transfer_sz = 256,
    };
    spi_device_interface_config_t dev = {
        .clock_speed_hz = cfg->spi_speed, .mode = 0,
        .spics_io_num = cfg->cs, .queue_size = 3,
    };
    if ( ESP_OK != spi_bus_initialize( SPI3_HOST, &bus, SPI_DMA_CH_AUTO ) ) return ISM3_ERROR;
    if ( ESP_OK != spi_bus_add_device( SPI3_HOST, &dev, &ctx->spi ) )       return ISM3_ERROR;

    ctx->rst = cfg->rst; ctx->gp0 = cfg->gp0; ctx->gp1 = cfg->gp1; ctx->gp2 = cfg->gp2;

    gpio_set_direction( ctx->rst, GPIO_MODE_OUTPUT );
    gpio_set_direction( ctx->gp0, GPIO_MODE_INPUT );
    gpio_set_direction( ctx->gp1, GPIO_MODE_INPUT );
    gpio_set_direction( ctx->gp2, GPIO_MODE_INPUT );
    return ISM3_OK;
}

///
/// @param ctx The ism3 you want to configure
/// @return A initialised to a ook modulation at a 433.92 frequency band
err_t ism3_default_cfg ( ism3_t *ctx )
{
    uint8_t reg_data[ 6 ] = { 0 };
    err_t error_flag = ISM3_OK;
    ism3_disable_device ( ctx );
    Delay_100ms ( );
    ism3_enable_device ( ctx );
    Delay_100ms ( );

    error_flag |= ism3_write_cmd ( ctx, ISM3_CMD_SRES );
    Delay_100ms ( );

    if ( ISM3_ERROR == ism3_check_communication ( ctx ) )
    {
        return ISM3_ERROR;
    }

    // Set GPIO1 pin as IRQ
    reg_data[ 0 ] = ISM3_GPIO1_CONF_GPIO_SELECT_NIRQ | ISM3_GPIO1_CONF_GPIO_MODE_DIG_OUTPUT_LP;
    error_flag |= ism3_write_reg ( ctx, ISM3_REG_GPIO1_CONF, reg_data[ 0 ] );

    // radio config
    error_flag |= ism3_read_reg ( ctx, ISM3_REG_XO_RCO_CONF1, reg_data );
    if ( ISM3_XO_RCO_CONF1_PD_CLKDIV != ( reg_data[ 0 ] & ISM3_XO_RCO_CONF1_PD_CLKDIV ) )
    {
        error_flag |= ism3_write_cmd ( ctx, ISM3_CMD_STANDBY );
        error_flag |= ism3_wait_mc_state ( ctx, ISM3_MC_STATE_STANDBY, ISM3_DEFAULT_TIMEOUT_MS );

        error_flag |= ism3_read_reg ( ctx, ISM3_REG_XO_RCO_CONF1, reg_data );
        reg_data[ 0 ] |= ISM3_XO_RCO_CONF1_PD_CLKDIV;
        error_flag |= ism3_write_reg ( ctx, ISM3_REG_XO_RCO_CONF1, reg_data[ 0 ] );

        error_flag |= ism3_write_cmd ( ctx, ISM3_CMD_READY );
        error_flag |= ism3_wait_mc_state ( ctx, ISM3_MC_STATE_READY, ISM3_DEFAULT_TIMEOUT_MS );
    }

    // Set intermediate frequency to 300 kHz
    reg_data[ 0 ] = ISM3_IF_OFFSET_ANA_DEFAULT;
    reg_data[ 1 ] = ISM3_IF_OFFSET_DIG_DEFAULT;
    error_flag |= ism3_write_regs ( ctx, ISM3_REG_IF_OFFSET_ANA, reg_data, 2 );

    // --- MODULATION: OOK instead of 2-FSK ---
    // Lower the datarate a bit for a cleaner, easier-to-read raw capture on Flipper.
    // Keep the same DATARATE_M/E bytes if you just want to try OOK at the same rate first.
    reg_data[ 0 ] = ISM3_MOD4_DATARATE_M_15_8_DEFAULT;
    reg_data[ 1 ] = ISM3_MOD3_DATARATE_M_7_0_DEFAULT;
    reg_data[ 2 ] = ISM3_MOD2_MOD_TYPE_ASK_OOK | ISM3_MOD2_DATARATE_E_DEFAULT;   // <-- was ISM3_MOD2_MOD_TYPE_2FSK

    // Deviation fields are meaningless for OOK, but harmless to leave programmed.
    error_flag |= ism3_read_reg ( ctx, ISM3_REG_MOD1, &reg_data[ 3 ] );
    reg_data[ 3 ] &= ~( ISM3_MOD1_FDEV_E_MASK );
    reg_data[ 3 ] |= ISM3_MOD1_FDEV_E_DEFAULT;
    reg_data[ 4 ] = ISM3_MOD0_FDEV_M_DEFAULT;

    // Set bandwidth to 100 kHz
    reg_data[ 5 ] = ISM3_CHFLT_M_DEFAULT | ISM3_CHFLT_E_DEFAULT;
    error_flag |= ism3_write_regs ( ctx, ISM3_REG_MOD4, reg_data, 6 );

    // --- PA config for pure OOK ---
    // PA_POWER[0] must be 0 (not just "smoothing off") so bit '0' truly means PA off.
    // Disable FIR shaping AND ramping so the on/off edges are abrupt, which is what
    // gives Flipper's raw capture clean, easy-to-see pulses.
    error_flag |= ism3_read_regs ( ctx, ISM3_REG_PA_POWER0, reg_data, 3 );
    reg_data[ 0 ] = 0x00;                                    // PA_POWER[0] = 0 -> OOK, not ASK
    reg_data[ 1 ] &= ~( ISM3_PA_CONFIG1_FIR_EN );
    reg_data[ 2 ] &= ~( ISM3_PA_CONFIG0_PA_FC_MASK );
    reg_data[ 2 ] |= ISM3_PA_CONFIG0_PA_FC_50_KHZ;
    error_flag |= ism3_write_regs ( ctx, ISM3_REG_PA_POWER0, reg_data, 3 );

    // Enable AFC freeze on sync
    error_flag |= ism3_read_reg ( ctx, ISM3_REG_AFC2, reg_data );
    reg_data[ 0 ] |= ISM3_AFC2_AFC_FREEZE_ON_SYNC;
    error_flag |= ism3_write_reg ( ctx, ISM3_REG_AFC2, reg_data[ 0 ] );

    // Set synthesizer for frequency base of 433 MHz
    error_flag |= ism3_read_reg ( ctx, ISM3_REG_SYNTH_CONFIG2, reg_data );
    reg_data[ 0 ] |= ISM3_SYNTH_CONFIG2_PLL_PFD_SPLIT_EN;
    error_flag |= ism3_write_reg ( ctx, ISM3_REG_SYNTH_CONFIG2, reg_data[ 0 ] );
    reg_data[ 0 ] = ISM3_SYNT3_PLL_CP_ISEL_DEFAULT | ISM3_SYNT3_BS_8_MIDDLE_BAND | ISM3_SYNT3_SYNT_27_24_DEFAULT;
    reg_data[ 1 ] = ISM3_SYNT2_SYNT_23_16_DEFAULT;
    reg_data[ 2 ] = ISM3_SYNT1_SYNT_15_8_DEFAULT;
    reg_data[ 3 ] = ISM3_SYNT0_SYNT_7_0_DEFAULT;
    error_flag |= ism3_write_regs ( ctx, ISM3_REG_SYNT3, reg_data, 4 );

    // Set max PA level
    error_flag |= ism3_read_reg ( ctx, ISM3_REG_PA_POWER0, reg_data );
    reg_data[ 0 ] &= ~( ISM3_PA_POWER0_PA_MAXDBM );
    error_flag |= ism3_write_reg ( ctx, ISM3_REG_PA_POWER0, reg_data[ 0 ] );

    // Set PA level -- this is now the power used for bit '1' (PA_LEVEL8)
    error_flag |= ism3_write_reg ( ctx, ISM3_REG_PA_POWER8, ISM3_PA_POWER8_PA_LEVEL8_DEFAULT );

    // Set PA level max index -- must point at index 8, matching PA_POWER8 above,
    // since that's the level S2-LP uses for OOK's "on" bit
    error_flag |= ism3_read_reg ( ctx, ISM3_REG_PA_POWER0, reg_data );
    reg_data[ 0 ] &= ~( ISM3_PA_POWER0_PA_LEVEL_MAX_IDX_MASK );
    reg_data[ 0 ] |= ISM3_PA_POWER0_PA_LEVEL_MAX_IDX_DEFAULT;
    error_flag |= ism3_write_reg ( ctx, ISM3_REG_PA_POWER0, reg_data[ 0 ] );

    // Set auto packet filtering
    error_flag |= ism3_read_reg ( ctx, ISM3_REG_PROTOCOL1, reg_data );
    reg_data[ 0 ] |= ISM3_PROTOCOL1_AUTO_PCKT_FLT;
    error_flag |= ism3_write_reg ( ctx, ISM3_REG_PROTOCOL1, reg_data[ 0 ] );

    // Basic packet config
    reg_data[ 0 ] = ISM3_PCKTCTRL6_SYNC_LEN_DEFAULT | ISM3_PCKTCTRL6_PREAMBLE_LEN_DEFAULT;
    reg_data[ 1 ] = ISM3_PCKTCTRL5_PREAMBLE_LEN_DEFAULT;
    reg_data[ 2 ] = ISM3_PCKTCTRL4_LEN_WID_1BYTE | ISM3_PCKTCTRL4_ADDRESS_LEN_NOT_INCLUDED;
    error_flag |= ism3_read_regs ( ctx, ISM3_REG_PCKTCTRL3, &reg_data[ 3 ], 3 );
    reg_data[ 3 ] &= ~( ISM3_PCKTCTRL3_PCKT_FRMT_MASK | ISM3_PCKTCTRL3_RX_MODE_MASK );
    reg_data[ 3 ] |= ( ISM3_PCKTCTRL3_PCKT_FRMT_BASIC | ISM3_PCKTCTRL3_RX_MODE_NORMAL );
    reg_data[ 4 ] &= ~( ISM3_PCKTCTRL2_MBUS_3OF6_EN | ISM3_PCKTCTRL2_MANCHESTER_EN );
    reg_data[ 4 ] |= ISM3_PCKTCTRL2_FIX_VAR_LEN;
    reg_data[ 5 ] &= ~( ISM3_PCKTCTRL1_CRC_MODE_MASK | ISM3_PCKTCTRL1_TXSOURCE_MASK | ISM3_PCKTCTRL1_FEC_EN );
    reg_data[ 5 ] |= ( ISM3_PCKTCTRL1_CRC_MODE_POLY_07 | ISM3_PCKTCTRL1_WHIT_EN );
    error_flag |= ism3_write_regs ( ctx, ISM3_REG_PCKTCTRL6, reg_data, 6 );

    // Set CRC check
    error_flag |= ism3_read_reg ( ctx, ISM3_REG_PCKT_FLT_OPTIONS, reg_data );
    reg_data[ 0 ] |= ISM3_PCKT_FLT_OPTIONS_CRC_FLT;
    error_flag |= ism3_write_reg ( ctx, ISM3_REG_PCKT_FLT_OPTIONS, reg_data[ 0 ] );

    // Config IRQ
    error_flag |= ism3_set_irq_mask ( ctx, ISM3_IRQ_NONE );

    // Set packet length
    reg_data[ 0 ] = ( uint8_t ) ( ( ISM3_PACKET_LEN >> 8 ) & 0xFF );
    reg_data[ 1 ] = ( uint8_t ) ( ISM3_PACKET_LEN & 0xFF );
    error_flag |= ism3_write_regs ( ctx, ISM3_REG_PCKTLEN1, reg_data, 2 );
    
    // Set RX timer to 2000ms
    reg_data[ 0 ] = ISM3_RX_TIMER_CNT_2000MS;
    reg_data[ 1 ] = ISM3_RX_TIMER_PSC_2000MS;
    error_flag |= ism3_write_regs ( ctx, ISM3_REG_TIMERS5, reg_data, 2 );

    // Go to RX as initial state
    error_flag |= ism3_go_to_rx ( ctx );
    Delay_1sec ( );
    return error_flag;
}

err_t ism3_write_regs ( ism3_t *ctx, uint8_t reg, uint8_t *data_in, uint8_t len )
{
    return ism3_transfer( ctx, ISM3_HEADER_WRITE_REG, reg, data_in, NULL, len );
}

err_t ism3_write_reg ( ism3_t *ctx, uint8_t reg, uint8_t data_in )
{
    return ism3_write_regs ( ctx, reg, &data_in, 1 );
}

err_t ism3_read_regs ( ism3_t *ctx, uint8_t reg, uint8_t *data_out, uint8_t len )
{
    return ism3_transfer( ctx, ISM3_HEADER_READ_REG, reg, NULL, data_out, len );
}

err_t ism3_read_reg ( ism3_t *ctx, uint8_t reg, uint8_t *data_out )
{
    return ism3_read_regs ( ctx, reg, data_out, 1 );
}

err_t ism3_write_cmd ( ism3_t *ctx, uint8_t cmd )
{
    return ism3_transfer( ctx, ISM3_HEADER_WRITE_CMD, cmd, NULL, NULL, 0 );
}


void ism3_enable_device  ( ism3_t *ctx ) { gpio_set_level( ctx->rst, 1 ); }

void ism3_disable_device ( ism3_t *ctx ) { gpio_set_level( ctx->rst, 0 ); }

uint8_t ism3_get_gp0_pin ( ism3_t *ctx )
{
    digital_in_t in = { ctx->gp0 };
    return digital_in_read ( &in );
}

uint8_t ism3_get_gp1_pin ( ism3_t *ctx )
{
    digital_in_t in = { ctx->gp1 };
    return digital_in_read ( &in );
}

uint8_t ism3_get_gp2_pin ( ism3_t *ctx )
{
    digital_in_t in = { ctx->gp2 };
    return digital_in_read ( &in );
}

err_t ism3_check_communication ( ism3_t *ctx )
{
    uint8_t part_number = 0;
    if ( ISM3_OK == ism3_read_reg ( ctx, ISM3_REG_DEVICE_INFO1, &part_number ) )
    {
        if ( ISM3_PART_NUMBER == part_number )
        {
            return ISM3_OK;
        }
    }
    return ISM3_ERROR;
}

err_t ism3_wait_mc_state ( ism3_t *ctx, uint8_t mc_state, uint16_t timeout_ms )
{
    uint32_t timeout_cnt = 0;
    uint8_t reg_data = 0;
    err_t error_flag = ISM3_OK;
    do
    {
        Delay_50us ( );
        error_flag |= ism3_read_reg ( ctx, ISM3_REG_MC_STATE0, &reg_data );
        timeout_cnt++;
        if ( ( timeout_cnt / 20 ) > timeout_ms )
        {
            error_flag = ISM3_TIMEOUT;
        }
    }
    while ( ( ( ( reg_data >> 1 ) & ISM3_MC_STATE_MASK ) != mc_state ) && ( ISM3_OK == error_flag ) );

    return error_flag;
}

err_t ism3_go_to_ready ( ism3_t *ctx )
{
    uint8_t reg_data = 0;
    err_t error_flag = ism3_read_reg ( ctx, ISM3_REG_MC_STATE0, &reg_data );
    reg_data = ( uint8_t ) ( ( reg_data >> 1 ) & ISM3_MC_STATE_MASK );
    if ( ISM3_MC_STATE_READY != reg_data )
    {
        if ( ( ISM3_MC_STATE_STANDBY == reg_data ) || 
             ( ISM3_MC_STATE_SLEEP_NOFIFO == reg_data ) || 
             ( ISM3_MC_STATE_SLEEP == reg_data ) || 
             ( ISM3_MC_STATE_LOCKON == reg_data ) )
        {
            error_flag |= ism3_write_cmd ( ctx, ISM3_CMD_RX );
        }
        else if ( ( ISM3_MC_STATE_TX == reg_data ) || 
                  ( ISM3_MC_STATE_RX == reg_data ) || 
                  ( ISM3_MC_STATE_LOCKST == reg_data ) )
        {
            error_flag |= ism3_write_cmd ( ctx, ISM3_CMD_SABORT );
        }
        error_flag |= ism3_wait_mc_state ( ctx, ISM3_MC_STATE_READY, ISM3_DEFAULT_TIMEOUT_MS );
    }
    return error_flag;
}

err_t ism3_go_to_rx ( ism3_t *ctx )
{
    err_t error_flag = ISM3_OK;
    error_flag |= ism3_go_to_ready ( ctx );
    error_flag |= ism3_write_cmd ( ctx, ISM3_CMD_RX );
    error_flag |= ism3_clear_irq_status ( ctx );
    return error_flag;
}

err_t ism3_set_irq_mask ( ism3_t *ctx, uint32_t irq_mask )
{
    uint8_t reg_data[ 4 ] = { 0 };
    reg_data[ 0 ] = ( uint8_t ) ( ( irq_mask >> 24 ) & 0xFF );
    reg_data[ 1 ] = ( uint8_t ) ( ( irq_mask >> 16 ) & 0xFF );
    reg_data[ 2 ] = ( uint8_t ) ( ( irq_mask >> 8 ) & 0xFF );
    reg_data[ 3 ] = ( uint8_t ) ( irq_mask & 0xFF );
    return ism3_write_regs ( ctx, ISM3_REG_IRQ_MASK3, reg_data, 4 );
}

err_t ism3_read_irq_mask ( ism3_t *ctx, uint32_t *irq_mask )
{
    uint8_t reg_data[ 4 ] = { 0 };
    err_t error_flag = ism3_read_regs ( ctx, ISM3_REG_IRQ_MASK3, reg_data, 4 );
    *irq_mask = ( ( uint32_t ) reg_data[ 0 ] << 24 ) | ( ( uint32_t ) reg_data[ 1 ] << 16 ) | 
                ( ( uint16_t ) reg_data[ 2 ] << 8 ) | reg_data[ 3 ];
    return error_flag;
}

err_t ism3_clear_irq_status ( ism3_t *ctx )
{
    uint8_t reg_data[ 4 ] = { 0 };
    return ism3_read_regs ( ctx, ISM3_REG_IRQ_STATUS3, reg_data, 4 );
}

err_t ism3_read_irq_status ( ism3_t *ctx, uint32_t *irq_status )
{
    uint8_t reg_data[ 4 ] = { 0 };
    err_t error_flag = ism3_read_regs ( ctx, ISM3_REG_IRQ_STATUS3, reg_data, 4 );
    *irq_status = ( ( uint32_t ) reg_data[ 0 ] << 24 ) | ( ( uint32_t ) reg_data[ 1 ] << 16 ) | 
                  ( ( uint16_t ) reg_data[ 2 ] << 8 ) | reg_data[ 3 ];
    return error_flag;
}

err_t ism3_transmit_packet ( ism3_t *ctx, uint8_t *data_in, uint8_t len )
{
    uint32_t irq_status = 0;
    uint32_t timeout_cnt = 0;
    uint8_t data_buf[ ISM3_PACKET_LEN ] = { 0 };
    err_t error_flag = ISM3_OK;

    if ( ( NULL == data_in ) || ( len > ISM3_PACKET_LEN ) )
    {
        return ISM3_ERROR;
    }
    memcpy ( data_buf, data_in, len );

    error_flag |= ism3_set_irq_mask ( ctx, ISM3_IRQ_TX_DATA_SENT );
    error_flag |= ism3_clear_irq_status ( ctx );

    error_flag |= ism3_go_to_ready ( ctx );

    error_flag |= ism3_write_cmd ( ctx, ISM3_CMD_FLUSHTXFIFO );
    error_flag |= ism3_write_regs ( ctx, ISM3_REG_LINEAR_FIFO, data_buf, ISM3_PACKET_LEN );
    error_flag |= ism3_write_reg ( ctx, ISM3_REG_PM_CONF3, ISM3_PM_CONF3_TX );
    error_flag |= ism3_write_cmd ( ctx, ISM3_CMD_TX );

    while ( ism3_get_gp1_pin ( ctx ) )
    {
        Delay_1ms ( );
        if ( ++timeout_cnt >= ISM3_DEFAULT_TIMEOUT_MS )
        {
            error_flag = ISM3_TIMEOUT;
            break;
        }
    }

    if ( ISM3_OK == error_flag )
    {
        error_flag |= ism3_read_irq_status ( ctx, &irq_status );
        error_flag |= ism3_write_cmd ( ctx, ISM3_CMD_RX );
        if ( ISM3_IRQ_TX_DATA_SENT != ( irq_status & ISM3_IRQ_TX_DATA_SENT ) )
        {
            error_flag = ISM3_ERROR;
        }
    }
    return error_flag;
}

err_t ism3_receive_packet ( ism3_t *ctx, uint8_t *data_out, uint8_t *len )
{
    uint32_t irq_status = 0;
    uint32_t timeout_cnt = 0;
    uint8_t fifo_size = 0;
    err_t error_flag = ISM3_OK;

    if ( NULL == data_out )
    {
        return ISM3_ERROR;
    }
    error_flag |= ism3_set_irq_mask ( ctx, ISM3_IRQ_RX_DATA_READY | ISM3_IRQ_RX_DATA_DISC );
    error_flag |= ism3_clear_irq_status ( ctx );

    error_flag |= ism3_wait_mc_state ( ctx, ISM3_MC_STATE_READY, ISM3_DEFAULT_TIMEOUT_MS );
    error_flag |= ism3_write_reg ( ctx, ISM3_REG_PM_CONF3, ISM3_PM_CONF3_RX );
    error_flag |= ism3_write_cmd ( ctx, ISM3_CMD_RX );

    while ( ism3_get_gp1_pin ( ctx ) )
    {
        Delay_1ms ( );
        if ( ++timeout_cnt >= ISM3_DEFAULT_RX_TIMEOUT_MS )
        {
            ism3_go_to_rx ( ctx );
            error_flag = ISM3_TIMEOUT;
            break;
        }
    }

    if ( ISM3_OK == error_flag )
    {
        error_flag |= ism3_read_irq_status ( ctx, &irq_status );

        if ( irq_status & ISM3_IRQ_RX_DATA_READY )
        {
            error_flag |= ism3_read_reg ( ctx, ISM3_REG_RX_FIFO_STATUS, &fifo_size );
            if ( fifo_size > ISM3_PACKET_LEN )
            {
                fifo_size = ISM3_PACKET_LEN;
            }
            error_flag |= ism3_read_regs ( ctx, ISM3_REG_LINEAR_FIFO, data_out, fifo_size );
            error_flag |= ism3_write_cmd ( ctx, ISM3_CMD_FLUSHRXFIFO );
            if ( NULL != len )
            {
                *len = fifo_size;
            }
        }
        else if ( irq_status & ISM3_IRQ_RX_DATA_DISC )
        {
            error_flag = ISM3_ERROR;
        }
    }
    return error_flag;
}


// ------------------------------------------------------------------------- END

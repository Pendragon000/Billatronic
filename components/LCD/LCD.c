/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#if defined(__XC)
    #include <xc.h>         /* XC8 General Include File */
#elif defined(HI_TECH_C)
    #include <htc.h>        /* HiTech General Include File */
#endif

#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>       /* For true/false definition */

#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp */
#include "sci.h"
#include "LCD.h"
#include "user_prof.h"
/******************************************************************************/
/* User Global Variable Declaration                                           */
/******************************************************************************/

/******************************************************************************/
/* Main Program                                                               */
/******************************************************************************/


void LcdOn(void)
{
    uint8_t buffer[] = {0xFE,0x41};
    I2C_Master_Write(LCD_SLAVE_ADDRESS,buffer, 2);
    __delay_us(110);
}
void LcdOff(void)
{
    uint8_t buffer[] = {0xFE,0x42};
    I2C_Master_Write(LCD_SLAVE_ADDRESS,buffer, 2);
    __delay_us(110);
    
}

void SetBackLight(uint8_t BackLightLevel)
{
    uint8_t buffer[] = {0xFE,0x53,BackLightLevel};
    I2C_Master_Write(LCD_SLAVE_ADDRESS,buffer,3);
    __delay_us(110);
}

void SetCursorhome(void)
{
    uint8_t buffer[] = {0xFE,0x46};
    I2C_Master_Write(LCD_SLAVE_ADDRESS,buffer, 2);
    __delay_us(110);
}

void ClearScreen(void)
{
    uint8_t buffer[] = {0xFE,0x51};
    I2C_Master_Write(LCD_SLAVE_ADDRESS,buffer, 2);
    __delay_us(110);
}

void SetCursor(line_e line, uint8_t col)
{
    uint8_t pos = line + col;
    uint8_t buffer[] = {0xFE,0x45, pos};
    I2C_Master_Write(LCD_SLAVE_ADDRESS,buffer, 3);
    __delay_us(110);
    
}

void LcdPrintStr(uint8_t *Str)
{
    static uint8_t i = 0;
    
    for(i = 0; Str[i] != 0; i++)
    {
        I2C_Master_Write(LCD_SLAVE_ADDRESS,&Str[i], 1);
        __delay_us(110);
    }
}

uint8_t DecToAscii(uint8_t dec)
{
    uint8_t ascii = 0x00;
    
    switch(dec)
    {
        case 0:
            ascii = 0x30;
            break;
        case 1:
            ascii = 0x31;
            break;
        case 2:
            ascii = 0x32;
            break;
        case 3:
            ascii = 0x33;
            break;
        case 4:
            ascii = 0x34;
            break;
        case 5:
            ascii = 0x35;
            break;
        case 6:
            ascii = 0x36;
            break;
        case 7:
            ascii = 0x37;
            break;
        case 8:
            ascii = 0x38;
            break;
        case 9:
            ascii = 0x39;
            break;      
    }
    return ascii;
}

void LcdPrintNbr(uint16_t nbr)
{
    uint8_t mill;
    uint8_t cent;
    uint8_t dix;
    uint8_t unit;
    uint8_t changeDigit = 0;
    
    mill = nbr/1000;
    cent = (nbr%1000)/100;
    dix  = (nbr%100)/10;
    unit = nbr%10;

    while(1)
    {
        if(changeDigit == 0)
        {
            if(mill != 0)
            {
                mill = DecToAscii(mill);
                I2C_Master_Write(LCD_SLAVE_ADDRESS,&mill, 1);
            }
            changeDigit = 1;
        }
        else if(changeDigit == 1)
        {
            if(cent != 0 || mill != 0 )
            {
                cent = DecToAscii(cent);
                I2C_Master_Write(LCD_SLAVE_ADDRESS,&cent, 1);
            }
            changeDigit = 2;
        }
        else if(changeDigit == 2)
        {
            if(dix != 0 || cent != 0 || mill != 0)
            {
                dix = DecToAscii(dix);
                I2C_Master_Write(LCD_SLAVE_ADDRESS,&dix, 1);
            }
            changeDigit = 3;
        }
        else if(changeDigit == 3)
        {
            unit = DecToAscii(unit);
            I2C_Master_Write(LCD_SLAVE_ADDRESS,&unit, 1);
            changeDigit = 0;
            break;
        }
    }
}

void PrintByte(uint8_t byte)
{
    uint8_t buffer[] = {byte};
    I2C_Master_Write(LCD_SLAVE_ADDRESS, buffer,1);
    __delay_us(100);
}

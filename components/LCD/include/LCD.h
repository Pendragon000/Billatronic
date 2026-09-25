/* 
 * File:   LCD_Lib.h
 * Author: 2387811
 *
 * Created on 26 septembre 2025, 11:13
 */

#ifndef LCD_LIB_H
#define	LCD_LIB_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* LDC_LIB_H */

typedef enum line_e
{
    LINE1 = 0x00,
    LINE2 = 0x40,
    LINE3 = 0x14,
    LINE4 = 0x54
}line_e;

void LcdOn(void);
void LcdOff(void);
void SetBackLight(uint8_t BackLightLevel); // entre 1 et 8
void SetCursorhome(void);
void ClearScreen(void);
void SetCursor(line_e line, uint8_t col);
void LcdPrintStr(uint8_t *Str);
void LcdPrintNbr(uint16_t nbr);
void PrintByte(uint8_t byte);
uint8_t DecToAscii(uint8_t dec);

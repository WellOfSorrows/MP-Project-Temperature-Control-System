#include <avr/io.h>
#include <util/delay.h>
void LCD_cmd(unsigned char cmd);
void init_LCD(void);
void LCD_write(unsigned char data);

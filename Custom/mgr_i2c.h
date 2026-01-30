#ifndef MGR_I2C
#define MGR_I2C
#include "main.h"
#include "appl_main.h"


#define LCD_MODE_TUNER 0
#define LCD_MODE_NORMAL 1

void lcd_init (void);   // initialize lcd

void lcd_send_cmd (char cmd);  // send command to the lcd

void lcd_send_data (char data);  // send data to the lcd

void lcd_send_string (const char *str);  // send string to the lcd

void lcd_put_cur(int row, int col);  // put cursor at the entered position row (0 or 1), col (0-15);

void lcd_clear (void);

void lcd_update(void);

#endif
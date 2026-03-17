
/** Put this in the src folder **/

#include "mgr_i2c.h"
#include "lib_i2c.h"
#include "mgr_uart.h"
#include "mgr_gpio.h"
#include "mgr_can.h"

i2c_queue_t i2c_lcd = 
{
	.device = I2C1
};

// extern I2C_HandleTypeDef hi2c1;  // change your handler here accordingly

#define SLAVE_ADDRESS_LCD 0x4E // change this according to ur setup

void lcd_send_cmd_init (char cmd)
{
  	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (cmd&0xf0);
	data_l = ((cmd<<4)&0xf0);
	data_t[0] = data_u|0x0C;  //en=1, rs=0 -> bxxxx1100
	data_t[1] = data_u|0x08;  //en=0, rs=0 -> bxxxx1000
	data_t[2] = data_l|0x0C;  //en=1, rs=0 -> bxxxx1100
	data_t[3] = data_l|0x08;  //en=0, rs=0 -> bxxxx1000
	i2c_force_send_msg(&i2c_lcd,SLAVE_ADDRESS_LCD,data_t,4);
	// HAL_I2C_Master_Transmit_DMA (&hi2c1, SLAVE_ADDRESS_LCD,(uint8_t *) data_t, 4);
	// HAL_I2C_Master_Transmit (&hi2c1, SLAVE_ADDRESS_LCD,(uint8_t *) data_t, 4,10);
}
void lcd_send_cmd (char cmd)
{
  	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (cmd&0xf0);
	data_l = ((cmd<<4)&0xf0);
	data_t[0] = data_u|0x0C;  //en=1, rs=0 -> bxxxx1100
	data_t[1] = data_u|0x08;  //en=0, rs=0 -> bxxxx1000
	data_t[2] = data_l|0x0C;  //en=1, rs=0 -> bxxxx1100
	data_t[3] = data_l|0x08;  //en=0, rs=0 -> bxxxx1000
	// HAL_I2C_Master_Transmit_DMA (&hi2c1, SLAVE_ADDRESS_LCD,(uint8_t *) data_t, 4);
	// HAL_I2C_Master_Transmit (&hi2c1, SLAVE_ADDRESS_LCD,(uint8_t *) data_t, 4,10);
	i2c_queue_append(&i2c_lcd,SLAVE_ADDRESS_LCD,data_t,4);
}

void lcd_send_data (char data)
{
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (data&0xf0);
	data_l = ((data<<4)&0xf0);
	data_t[0] = data_u|0x0D;  //en=1, rs=0 -> bxxxx1101
	data_t[1] = data_u|0x09;  //en=0, rs=0 -> bxxxx1001
	data_t[2] = data_l|0x0D;  //en=1, rs=0 -> bxxxx1101
	data_t[3] = data_l|0x09;  //en=0, rs=0 -> bxxxx1001
	i2c_queue_append(&i2c_lcd,SLAVE_ADDRESS_LCD,data_t,4);

	// HAL_I2C_Master_Transmit (&hi2c1, SLAVE_ADDRESS_LCD,(uint8_t *) data_t, 4, 10);
	// HAL_I2C_Master_Transmit_IT (&hi2c1, SLAVE_ADDRESS_LCD,(uint8_t *) data_t, 4);
	// HAL_Delay(1);
}

void lcd_clear (void)
{
	lcd_send_cmd (0x80);
	for (int i=0; i<70; i++)
	{
		lcd_send_data (' ');
	}
}

void lcd_put_cur(int row, int col)
{
    switch (row)
    {
        case 0:
            col |= 0x80;
            break;
        case 1:
            col |= 0xC0;
            break;
    }

    lcd_send_cmd (col);
}

void lcd_update(void)
{
	i2c_svc(&i2c_lcd);
}

void lcd_init (void)
{
	i2c_queue_init(&i2c_lcd,4);
	// 4 bit initialisation
	HAL_Delay(50);  // wait for >40ms
	lcd_send_cmd_init (0x30);
	HAL_Delay(50);  // wait for >4.1ms
	lcd_send_cmd_init (0x30);
	HAL_Delay(1);  // wait for >100us
	lcd_send_cmd_init (0x30);
	HAL_Delay(10);
	lcd_send_cmd_init (0x20);  // 4bit mode
	HAL_Delay(10);

  // dislay initialisation
	lcd_send_cmd_init (0x28); // Function set --> DL=0 (4 bit mode), N = 1 (2 line display) F = 0 (5x8 characters)
	HAL_Delay(5);
	lcd_send_cmd_init (0x08); //Display on/off control --> D=0,C=0, B=0  ---> display off
	HAL_Delay(5);
	lcd_send_cmd_init (0x01);  // clear display
	HAL_Delay(5);
	HAL_Delay(5);
	lcd_send_cmd_init (0x06); //Entry mode set --> I/D = 1 (increment cursor) & S = 0 (no shift)
	HAL_Delay(5);
	lcd_send_cmd_init (0x0C); //Display on/off control --> D = 1, C and B = 0. (Cursor and blink, last two bits)
}

void lcd_send_string (const char *str)
{
	while (*str) lcd_send_data (*str++);
}

void I2C1_EV_IRQHandler(void)
{
	i2c_isr(&i2c_lcd);
}


uint32_t lcd_timer = 0;
#define lcd_update_interval 2500
bool lcd_send = 0;

void i2c_timingloop(void)
{
    i2c_timing();

    lcd_timer++;
    if(lcd_timer > lcd_update_interval)
    {
        lcd_timer = 0;
        lcd_send = 1;
    }
}

//4 quadrants of the 1602 lcd
uint8_t lcd_q1[8];
uint8_t lcd_q2[8];
uint8_t lcd_q3[8];
uint8_t lcd_q4[8];
uint8_t lcd_tmp_buf[64];

bool lcd_mode;
void i2c_mainloop(void)
{
	lcd_mode = gpio_getLcdMode();

    lcd_update();
    if(lcd_send)
    {
        lcd_send = 0;

		switch(lcd_mode)
		{
			case LCD_MODE_TUNER:
				sprintf(lcd_q1,"AFR:");
				sprintf(lcd_tmp_buf,"%f",14.7);
				memcpy(&lcd_q1[4],lcd_tmp_buf,can_getActualAFR());
				sprintf(lcd_q2,"TGT:");
				sprintf(lcd_tmp_buf,"%f",can_getTargetAFR());
				memcpy(&lcd_q2[4],lcd_tmp_buf,4);
				sprintf(lcd_q3,"IGN:");
				sprintf(lcd_tmp_buf,"%f",can_GetIgnAdvance());
				memcpy(&lcd_q3[4],lcd_tmp_buf,4);
				sprintf(lcd_q4,"IAT:");
				sprintf(lcd_tmp_buf,"%f",can_GetIntakeAirTemp());
				memcpy(&lcd_q4[4],lcd_tmp_buf,4);
				break;
			case LCD_MODE_NORMAL: //specific to mazda for now
				sprintf(lcd_q1,"AFR:");
				sprintf(lcd_tmp_buf,"%f",can_getActualAFR());
				memcpy(&lcd_q1[4],lcd_tmp_buf,4);
				sprintf(lcd_q2,"TFT:");
				sprintf(lcd_tmp_buf,"%f",can_getTransFluidTemp());
				memcpy(&lcd_q2[4],lcd_tmp_buf,4);
				sprintf(lcd_q3,"CLT:");
				sprintf(lcd_tmp_buf,"%f",can_getCoolantTemp());
				memcpy(&lcd_q3[4],lcd_tmp_buf,4);
				sprintf(lcd_q4,"EOT:");
				sprintf(lcd_tmp_buf,"%f",can_getEngineOilTemp());
				memcpy(&lcd_q4[4],lcd_tmp_buf,4);

				break;
		}
        lcd_put_cur(0,0);
        lcd_send_string(lcd_q1);
        lcd_put_cur(0,8);
        lcd_send_string(lcd_q2);
        lcd_put_cur(1,0);
        lcd_send_string(lcd_q3);
        lcd_put_cur(1,8);
        lcd_send_string(lcd_q4);
    }
}

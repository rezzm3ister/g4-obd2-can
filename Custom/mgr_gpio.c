#include "mgr_gpio.h"
#include "gpio.h"

static bool lcd_mode = 0;
uint32_t lcd_mode_validation = 0;
//100ms
#define LCD_MODE_VALIDATION_TIME 1000 

bool gpio_getRawLcdPin(void)
{
    return(GPIOC->IDR & 1);
}

bool gpio_getLcdMode(void)
{
    return(lcd_mode);
}

void gpio_timingloop(void)
{
    bool temp_state = gpio_getRawLcdPin();
    if(temp_state != lcd_mode)
    {
        lcd_mode_validation++;
        if(lcd_mode_validation >= LCD_MODE_VALIDATION_TIME)
        {
            lcd_mode = temp_state;
            lcd_mode_validation = 0;

        }

    }
    else
    {
        lcd_mode_validation = 0;
    }

}
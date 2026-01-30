#include "appl_main.h"
// #include "mgr_led.h"
// #include "mgr_i2c.h"
#include "mgr_uart.h"
#include "mgr_can.h"
#include "mgr_i2c.h"


extern TIM_HandleTypeDef htim1;


uint32_t t1=0;
uint8_t one_sec_flag=0;
uint32_t uptime_timer=0;

uint32_t main_loop_dur=0;
uint32_t main_loop_timer=0;


void custom_init(void)
{
    // HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_NVIC_SetPriority(I2C1_EV_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
    HAL_TIM_Base_Start_IT(&htim1);
    uart_init();
    can_init();
    lcd_init();
    lcd_clear();
}

//does things every 0.1ms
void timing_loop(void) //10khz
{
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,1);
    main_loop_timer++;
    if(t1<10000)
    {
        t1++;
    }
    else
    {
        t1=0;
        one_sec_flag=1;
    }
    can_timingloop();
    i2c_timingloop();
    // relay_1k();
    uart_1msloop(); 
    gpio_timingloop();
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,0);

}

//does things every 1s
uint8_t led_en=0;

void sec_loop(void) //operates every second
{
    // led_toggle_c13();
    // update_i2c();
    if(led_en==1)
    {
        led_en = 0;
    }
    else 
    {
        led_en = 1;
    }
    

    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,led_en);
}

//does things whenever it can
void main_loop(void)
{
    main_loop_dur=main_loop_timer;
    main_loop_timer=0;
    if(one_sec_flag)
    {
        sec_loop();

        one_sec_flag = 0;
        uptime_timer++;
    }
    uart_mainloop();
    can_mainloop();
    i2c_mainloop();
    // update_led();
    // relay_update();
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_11);
}


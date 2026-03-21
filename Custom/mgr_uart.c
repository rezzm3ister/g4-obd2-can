#include "mgr_uart.h"
// #include "mgr_i2c.h"
#include "mgr_adc.h"
extern UART_HandleTypeDef huart1;


uint16_t modb_db[MODB_SIZE];
uint8_t uart_rx_buffer[500];
uint8_t uart_tx_buffer[500];
uint16_t uart_rx_size;
uint16_t uart_tx_size;

uint8_t rx_cplt=0;

uart_state_t uart_state = UART_IDLE;

void uart_ondatarecv(void);


void uart_testwrite(void)
{
    uint8_t testmsg[5]={1,2,3,4,5};
    // HAL_UART_Transmit(&huart1,testmsg,5,100);
    HAL_UART_Transmit_IT(&huart1,testmsg,5);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    // if(Size<6) return;
    uart_rx_size=Size;
    rx_cplt=1;
    // uart_ondatarecv();

    // HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,0);
}

void uart_ondatarecv(void)
{
    if(uart_rx_buffer[0]!=MODB_ADDR) return;

    //add crc check here
    uint16_t reg_addr = (uart_rx_buffer[2]<<8) | uart_rx_buffer[3];
    uint16_t payload_len = (uart_rx_buffer[4]<<8) | uart_rx_buffer[5];
    uint8_t r_buf[300];
    uint16_t rsp_len = 3 + payload_len*2;
    // memcpy(w_payload,uart_rx_buffer,(payload_len*2));
    uart_tx_buffer[0]=MODB_ADDR;
    uart_tx_buffer[1]=uart_rx_buffer[1];
    // uart_testwrite();
    //do r/w functions
    switch(uart_rx_buffer[1])
    {
        case MODB_READ:
        case MODB_READ2:
            modbus_read(&modb_db[reg_addr],r_buf,payload_len);
            // HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,0);
            uart_tx_buffer[2]=payload_len*2;
            memcpy(&uart_tx_buffer[3],r_buf,payload_len*2);

            break;
        case MODB_WRITE:
            memcpy(&uart_tx_buffer[0],&uart_rx_buffer[0],6);
            modbus_write(&modb_db[reg_addr],&uart_rx_buffer[4],1);
            rsp_len=6;
            break;
        case MODB_WRITEMULT:
            // memcpy(w_payload,&uart_rx_buffer[7],uart_rx_buffer[6]);
            modbus_write(&modb_db[reg_addr],&uart_rx_buffer[7],payload_len);
            memcpy(&uart_tx_buffer[2],&uart_rx_buffer[2],4);
            rsp_len=6;
            break;
        default:
            break;
    }
    
    uint16_t crc=0;

    crc=crc_modbus(uart_tx_buffer,rsp_len);
    uart_tx_buffer[rsp_len]=crc&0xff;
    uart_tx_buffer[rsp_len+1]=crc>>8;
    HAL_UART_Transmit_DMA(&huart1,uart_tx_buffer,rsp_len+2);
    // HAL_UART_Transmit_IT(&huart1,rsp_msg,rsp_len+2);
    // HAL_UART_Transmit(&huart1,rsp_msg,rsp_len+2,10);
    // HAL_UARTEx_ReceiveToIdle_DMA(&huart1,uart_rx_buffer,300);

}

    //post-write functions


void uart_1msloop(void)
{
    // HAL_UARTEx_ReceiveToIdle_DMA(&huart1,uart_rx_buffer,300);

}

void uart_init(void)
{
    memset(modb_db,0,MODB_SIZE*2);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1,uart_rx_buffer,300);

}


void uart_mainloop(void)
{
    // HAL_UARTEx_ReceiveToIdle_IT(&huart1,uart_rx_buffer,300);
    // HAL_UART_Receive_IT(&huart1,uart_rx_buffer,1);
    if(rx_cplt)
    {
        rx_cplt=0;
        uart_ondatarecv();
        // uart_state = UART_TRANSMITTING;
    }
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1,uart_rx_buffer,300);

    modb_db[0x500] = adc_GetRawAdcValue(0);
    modb_db[0x501] = adc_GetRawAdcValue(1);
    modb_db[0x502] = adc_GetRawAdcValue(2);
    modb_db[0x503] = adc_GetRawAdcValue(3);
    modb_db[0x504] = adc_GetRawAdcValue(4);

    // uart_testwrite();

}

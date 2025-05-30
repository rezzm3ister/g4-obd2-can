#ifndef MGR_UART
#define MGR_UART
#include "appl_main.h"
#include "lib_modbus.h"

#define MODB_SIZE 0x1000
extern uint16_t modb_db[MODB_SIZE];

#define MODB_ADDR           0x69

#define CURRENT_PID         modb_db[0x100]
#define SUPPORTED_PID_1H    modb_db[0x101]
#define SUPPORTED_PID_1L    modb_db[0x102]
#define SUPPORTED_PID_2H    modb_db[0x103]
#define SUPPORTED_PID_2L    modb_db[0x104]
#define SUPPORTED_PID_3H    modb_db[0x105]
#define SUPPORTED_PID_3L    modb_db[0x106]
#define SUPPORTED_PID_4H    modb_db[0x107]
#define SUPPORTED_PID_4L    modb_db[0x108]

typedef enum
{
    UART_IDLE=0,
    UART_RECEIVING,
    UART_TRANSMITTING
} uart_state_t;



void uart_1msloop(void);
void uart_init(void);

void uart_mainloop(void);
#endif
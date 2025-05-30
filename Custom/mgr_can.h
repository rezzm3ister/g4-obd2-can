#ifndef MGR_CAN_H
#define MGR_CAN_H
#include "appl_main.h"

#define CAN_WAKEUP_TIME 50000
#define CAN_TIMEOUT 500
#define CAN_STARTUP_TIMEOUT 500
#define CAN_MAX_PID 0xBF
// #define CAN_START_PID 0x04
// #define CAN_DEBUG

typedef enum
{
    CAN_INIT=0,
    CAN_TURNON,
    CAN_TURNON_WAIT_RSP,
    CAN_TURNON_PROCESSING,
    CAN_FINISH_TURNON,
    CAN_WRITE,
    CAN_WAIT_RSP,
    CAN_PROCESSING,
    CAN_OFF
} can_state_t;

// bool can_is_pid_supported(uint8_t pid);
void can_onDataReceivedTurnon();
void can_onDataReceived();
void can_sendRequest(void);
void can_timingloop(void);
void can_init(void);
void can_mainloop(void);

#endif
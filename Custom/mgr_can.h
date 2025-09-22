#ifndef MGR_CAN_H
#define MGR_CAN_H
#include "appl_main.h"

#define CAN_WAKEUP_TIME 30000
#define CAN_TIMEOUT 125
// #define CAN_TIMEOUT_FAST 250
#define CAN_STARTUP_TIMEOUT 1000
// #define CAN_MAX_PID 0x80
#define CAN_MAX_PID 0x60
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
    CAN_WRITE_SLOW,
    CAN_WAIT_RSP_SLOW,
    CAN_PROCESSING_SLOW,
    CAN_OFF
} can_state_t;

#define FAST_PID_COUNT 18

// bool can_is_pid_supported(uint8_t pid);
void can_onDataReceivedTurnon();
void can_onDataReceived();
void can_sendRequest(void);
void can_timingloop(void);
void can_init(void);
void can_mainloop(void);

#endif
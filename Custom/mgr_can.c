#include "mgr_can.h"
#include "mgr_uart.h"

extern FDCAN_HandleTypeDef hfdcan1;
can_state_t can_state = CAN_INIT; // Initial state of the CAN manager

uint16_t requests=0;
uint16_t responses=0;
uint8_t target_pid = 0x00; // Default PID to request
uint8_t turnon_target_pid = 0x00; // Default PID to request
uint16_t can_time=0;


FDCAN_TxHeaderTypeDef can_tx_header;
uint8_t can_tx_data[8]; // Data buffer for CAN transmission
FDCAN_RxHeaderTypeDef can_rx_header;
uint8_t can_rx_data[8]; // Data buffer for CAN reception

// uint32_t can_supported_headers[7] = {
//     #ifdef CAN_DEBUG
//     0xFFFFFFFF, // 0x00-0x1F
//     0xFFFFFFFF, // 0x20-0x3F
//     0xFFFFFFFF, // 0x40-0x5F
//     0xFFFFFFFF, // 0x60-0x7F
//     0xFFFFFFFF, // 0x80-0x9F
//     0xFFFFFFFF, // 0xA0-0xBF
//     0xFFFFFFFF  // 0xC0-0xDF
//     #else
//     0x00000000, // 0x00-0x1F
//     0x00000000, // 0x20-0x3F
//     0x00000000, // 0x40-0x5F
//     0x00000000, // 0x60-0x7F
//     0x00000000, // 0x80-0x9F
//     0x00000000, // 0xA0-0xBF
//     0x00000000  // 0xC0-0xDF
// #endif
// };
uint8_t supported_pid[0x100];
uint8_t valid_headers=0;
uint8_t start_pid=0;

void can_forceUnsupportedPid(void)
{
    supported_pid[0x00] = 0; // Mark PID 0x00 as unsupported
    supported_pid[0x01] = 0; // Mark PID 0x01 as unsupported
    supported_pid[0x02] = 0; // Mark PID 0x02 as unsupported
}

bool can_isBusAlive(void)
{
    for(int i=0;i<0xFF;i++)
    {
        if(supported_pid[i])
        {
            start_pid=i; // Store the first supported PID   
            return true; // If any PID is supported, the bus is considered alive
        }
    }
    return false; // If no PIDs are supported, the bus is considered dead
}

bool can_isFastPID(uint8_t pid)
{
    switch(pid)
    {
        case 0x04:
        case 0x05:
        case 0x0B:
        case 0x0C:
        case 0x0D:
        case 0x0E:
        case 0x0F:
        case 0x10:
        case 0x11:
        case 0x44:
        case 0x45:
        case 0x47:
        case 0x48:
        case 0x49:
        case 0x4A:
        case 0x4C:
            return true; // These PIDs are considered fast
        default:
            return false; // Other PIDs are not considered fast
    }
}

bool can_isPidValid(uint8_t pid)
{
    if(pid > CAN_MAX_PID) return false; // Check if PID is within valid range
    // bool is_valid = false;
    //always ignore these PIDs
    switch(pid)
    {
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x20:
        case 0x40:
        case 0x60:
        case 0x80:
        case 0xA0:
        case 0xC0:
            return false; // These PIDs are valid
        default:
            return true;
        break;
    }}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
    {
        /* Retreive Rx messages from RX FIFO0 */
        if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &can_rx_header, can_rx_data) != HAL_OK)
        {
        /* Reception Error */
        Error_Handler();
        }

        if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
        {
        /* Notification Error */
        Error_Handler();
        }
    }

    if(can_state == CAN_TURNON_WAIT_RSP && can_rx_header.Identifier >0x7D0 && can_rx_data[2]==turnon_target_pid)
    {
        can_state=CAN_TURNON_PROCESSING;
        // can_onDataReceivedTurnon(); // Process the received CAN message for turn-on requests
    }
    else if (can_state == CAN_WAIT_RSP && can_rx_header.Identifier >0x7D0 && can_rx_data[2]==target_pid)
    {
        can_state = CAN_PROCESSING; // Set state to processing after receiving data
        // can_onDataReceived(); // Process the received CAN message
    }
}

void can_DataProcessing(uint8_t pid, uint8_t* data, uint8_t len)
{
    //follow wikipedia list of PIDs for OBD-II
    //convert raw obd2 to value that can be shown on modbus
    uint32_t val = 0;

    switch(len)
    {
        case 1:
            val = data[0];
            break;
        case 2:
            val = (data[0] << 8) | data[1];
            break;
        case 3:
            val = (data[0] << 16) | (data[1] << 8) | data[2];
            break;
        case 4:
            val = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
            break;
        default:
            // Handle unexpected length
            return;
    }


    //ignore for now, processing done in python
    switch(pid)
    {
        case 0x01:
            // Process PID 0x01
            break;
        case 0x02:
            // Process PID 0x02
            break;
        // Add more cases as needed
        default:
            // Handle unknown PID
            break;
    }

    //temporary for now
    modb_db[0x100+pid] = val & 0xFFFF; // Store the processed value in the modbus database
    modb_db[0x200+pid] = val >> 16; // Store the processed value in the modbus database
}




void can_onDataReceivedTurnon()
{
    
    uint32_t can_rx_u32=0;
    // can_rx_u32 = (can_rx_data[4] << 24) | (can_rx_data[5] << 16) | (can_rx_data[6] << 8) | can_rx_data[7];
    // can_supported_headers[turnon_target_pid/0x20] = can_rx_u32; // Mark the PID as supported
    supported_pid[turnon_target_pid] = 1; // Mark the PID as supported
    turnon_target_pid++;
    can_state = CAN_TURNON; // Reset state to turn on next PID
    if(turnon_target_pid > CAN_MAX_PID) // Wrap around if PID exceeds CAN_MAX_PID
    {
        turnon_target_pid = 0x00; // Reset to 0x00
        can_state = CAN_FINISH_TURNON; // Reset state to write next request
        // if(!can_isBusAlive()) // Check if the bus is alive
        // {
        //     can_state = CAN_OFF; // If no PIDs are supported, set state to OFF
        // }
    }
    
}

void can_onDataReceived()
{

    // Process the received data
    // For example, you can log it or store it for further processing
    // Here we just print the received data for demonstration
    // modb_db[can_rx_data[2]] = (can_rx_data[3] << 8) | can_rx_data[4]; // Store the received data in modbus database

    can_DataProcessing(target_pid,&can_rx_data[3],can_rx_data[0]-2); // Process the received data
    if(target_pid< CAN_MAX_PID) // Increment target PID for next request
    {
        target_pid++;
    }
    else
    {
        target_pid = start_pid; // Reset to 0x04 if exceeded
    }
    can_state = CAN_WRITE; // Reset state to write next request
    
}

void can_sendRequest(void)
{

    bool is_valid_pid = 0;
    do
    {
        if(FAST_MODE)
        {
            is_valid_pid = can_isFastPID(target_pid) && supported_pid[target_pid];
        }
        else
        {
            is_valid_pid = supported_pid[target_pid]; // Check if the target PID is supported
        }
        if(!is_valid_pid) // If the PID is not supported, increment it
        {
            target_pid++;
            if(target_pid > CAN_MAX_PID) // Wrap around if PID exceeds 0xDF
            {
                target_pid = start_pid; // Reset to 0x04
                break;
            }
        }
    }while(!is_valid_pid);

    can_tx_header.Identifier = 0x7DF; // Standard ID for OBD-II requests
    can_tx_header.IdType = FDCAN_STANDARD_ID;
    // can_tx_header.TxFrameType = FDCAN_REMOTE_FRAME;
    can_tx_header.TxFrameType = FDCAN_DATA_FRAME;
    can_tx_header.DataLength = FDCAN_DLC_BYTES_8; // OBD-II requests typically use 8 bytes
    can_tx_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    can_tx_header.BitRateSwitch = FDCAN_BRS_OFF;
    can_tx_header.FDFormat = FDCAN_CLASSIC_CAN;
    can_tx_header.TxEventFifoControl = FDCAN_NO_TX_EVENTS; // No Tx event FIFO control
    can_tx_header.MessageMarker = 0; // Not used in this context

    memset(can_tx_data, 0xFF, 8); // Clear the data buffer
    can_tx_data[0]=2;
    can_tx_data[1]=0x01; // OBD-II request
    can_tx_data[2]=target_pid; // PID to request
    HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &can_tx_header, can_tx_data);
    // Send a CAN request message
}

void can_sendTurnOnRequest(void)
{
    // Prepare the CAN message to turn on the target PID
    can_tx_header.Identifier = 0x7DF; // Standard ID for OBD-II requests
    can_tx_header.IdType = FDCAN_STANDARD_ID;
    can_tx_header.TxFrameType = FDCAN_DATA_FRAME;
    can_tx_header.DataLength = FDCAN_DLC_BYTES_8; // OBD-II requests typically use 8 bytes
    can_tx_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    can_tx_header.BitRateSwitch = FDCAN_BRS_OFF;
    can_tx_header.FDFormat = FDCAN_CLASSIC_CAN;
    can_tx_header.TxEventFifoControl = FDCAN_NO_TX_EVENTS; // No Tx event FIFO control
    can_tx_header.MessageMarker = 0; // Not used in this context

    while(!can_isPidValid(turnon_target_pid)) // Ensure the target PID is valid
    {
        turnon_target_pid++;
        if(turnon_target_pid > CAN_MAX_PID) // Wrap around if PID exceeds 0xDF
        {
            turnon_target_pid = 0x00; // Reset to 0x00
            break;
        }
    }
    memset(can_tx_data, 0xFF, 8); // Clear the data buffer
    can_tx_data[0] = 2; // Length of the request
    can_tx_data[1] = 0x01; // OBD-II request
    can_tx_data[2] = turnon_target_pid; // PID to turn on

    HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &can_tx_header, can_tx_data);
}

void can_init(void)
{
    memset(supported_pid, 0, 0xFF); // Clear the supported PID array
    if(HAL_FDCAN_Start(&hfdcan1)!= HAL_OK)
    {
        Error_Handler();
    }
    // Initialize CAN hardware and configure filters
    if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
    {
        /* Notification Error */
        Error_Handler();
    }
}

uint16_t can_init_timer=CAN_WAKEUP_TIME;
uint32_t can_timeout=CAN_TIMEOUT;
void can_timingloop(void)
{
    can_time++;
    if(can_time > 0xFFFF) // Reset can_time to prevent overflow
    {
        can_time = 0;
    }
    switch(can_state)
    {
        case CAN_INIT:
            if(can_init_timer>0)
            {
                can_init_timer--;
            }
            else
            {
                #ifdef CAN_DEBUG
                can_state = CAN_FINISH_TURNON;
                for(int i=0;i<0x100;i++)
                {
                    supported_pid[i] = 1; // Mark all PIDs as supported for debugging
                }
                #else
                can_state = CAN_TURNON;
                #endif
            }
            break;
        case CAN_TURNON_WAIT_RSP:
            if (can_timeout>0)
            {
                can_timeout--;
            }
            else
            {
                can_timeout=CAN_STARTUP_TIMEOUT; // Reset timeout for next turn-on request
                can_state = CAN_TURNON;

                // can_supported_headers[turnon_target_pid/0x20] = 0; // Mark the PID as supported
                supported_pid[turnon_target_pid] = 0; // Mark the PID as not supported
                turnon_target_pid ++; // Reset target PID to the next one
                if(turnon_target_pid > CAN_MAX_PID) // Wrap around if PID exceeds 0xDF
                {
                    turnon_target_pid = 0; // Reset to 0x00
                    can_state = CAN_FINISH_TURNON; // Reset state to write next request
                    can_timeout=CAN_TIMEOUT;
                    // if(!can_isBusAlive()) // Check if the bus is alive
                    // {
                    //     can_state = CAN_OFF; // If no PIDs are supported, set state to OFF
                    // }
                }
            }
            break;
        case CAN_WAIT_RSP:
            if(can_timeout>0)
            {
                can_timeout--;
            }
            else
            {
                can_timeout=CAN_TIMEOUT;
                can_state = CAN_WRITE;
                target_pid++;
            }
            break;
    }
}


void can_mainloop(void)
{
    // Main loop for CAN message processing
//    CURRENT_PID = target_pid; // Update the current PID in the modbus database
    // SUPPORTED_PID_1H = can_supported_headers[0] >> 16; // Update supported PIDs in modbus database
    // SUPPORTED_PID_1L = can_supported_headers[0] & 0xFFFF; // Update supported PIDs in modbus database
    // SUPPORTED_PID_2H = can_supported_headers[1] >> 16; // Update supported PIDs in modbus database
    // SUPPORTED_PID_2L = can_supported_headers[1] & 0xFFFF; // Update supported PIDs in modbus database
    // SUPPORTED_PID_3H = can_supported_headers[2] >> 16; // Update supported PIDs in modbus database
    // SUPPORTED_PID_3L = can_supported_headers[2] & 0xFFFF; // Update supported PIDs in modbus database
    // SUPPORTED_PID_4H = can_supported_headers[3] >> 16; // Update supported PIDs in modbus database
    // SUPPORTED_PID_4L = can_supported_headers[3] & 0xFFFF; // Update supported PIDs in modbus database
    // Process the current state of the CAN manager
    can_forceUnsupportedPid();
    switch(can_state)
    {
        case CAN_INIT:
            break;
        case CAN_TURNON:
            can_sendTurnOnRequest();
            can_state = CAN_TURNON_WAIT_RSP; // Transition to write state after initialization
            break;
        case CAN_TURNON_PROCESSING:
            can_onDataReceivedTurnon();
            break;
            
        case CAN_FINISH_TURNON:
            if(!can_isBusAlive()) // Check if the bus is alive
            {
                can_state = CAN_OFF; // If no PIDs are supported, set state to OFF
                break;
            }
            for(int i=0;i<0x100;i++)
            {
                modb_db[i] = supported_pid[i]; // Store the supported PIDs in the modbus database
            }
            can_state=CAN_WRITE;
            break;
        case CAN_WRITE:
            // Check for incoming messages or send requests
            if(target_pid == start_pid)
            {
                CAN_LOOP_TIME = can_time;
                can_time=0;
            }
            can_sendRequest();
            can_state = CAN_WAIT_RSP; // Transition to waiting for response state
            break;
        case CAN_PROCESSING:
            // Process the received data
            can_onDataReceived(); // Process the received CAN message
            
            // This state can be used for further processing of received messages
            break;
        default:
            // Handle unexpected states
            break;
    }

    // modb_db[0x300] = can_state; // Update the modbus database with the current CAN state
    // modb_db[0x301] = requests;
    // modb_db[0x302] = responses; // Update the modbus database with the number of requests and responses
}

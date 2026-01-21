#include "mgr_can.h"
#include "mgr_uart.h"

extern FDCAN_HandleTypeDef hfdcan1;
can_state_t can_state = CAN_INIT; // Initial state of the CAN manager

uint16_t requests=0;
uint16_t responses=0;
uint8_t target_pid = 0x00; // Default PID to request
uint8_t turnon_target_pid = 0x00; // Default PID to request
uint16_t can_time=0;

uint16_t target_pid_slow = 0x00; // Default PID to request in slow mode
uint16_t target_pid_fast_idx = 0x00; // Default PID to request in fast mode

bool support_all=true;

FDCAN_TxHeaderTypeDef can_tx_header;
uint8_t can_tx_data[8]; // Data buffer for CAN transmission
FDCAN_RxHeaderTypeDef can_rx_header;
uint8_t can_rx_data[8]; // Data buffer for CAN reception
uint8_t can_rx_data_buf[8]; // Data buffer for CAN reception


// target ecu, mode, pid, modb_addr, prio (lower = higher)
uint8_t pid_idx=0;
uint8_t pid_idx_slow=0;

uint8_t pid_max_prio[2]; // 2 prio levels: high/low (0/1)

uint16_t pid_table[PID_TABLE_ROWS * PID_TABLE_COLS] = {
// uint16_t pid_table[] = {
    0x7DF, 0x01, 0x06, 0,
    0x7DF, 0x01, 0x0B, 0,
    0x7DF, 0x01, 0x0C, 0,
    0x7DF, 0x01, 0x0D, 0,
    0x7DF, 0x01, 0x0E, 0,
    0x7DF, 0x01, 0x11, 0,
    0x7DF, 0x01, 0x14, 0,
    0x7DF, 0x01, 0x34, 0,
    0x7DF, 0x01, 0x3C, 0,
    0x7DF, 0x01, 0x44, 0,
    0x7DF, 0x01, 0x45, 0,
};

uint16_t pid_table_slow[PID_TABLE_ROWS * PID_TABLE_COLS] = {
    0x7DF, 0x01, 0x05, 0,
    0x7DF, 0x01, 0x0F, 0,
    0x7E0, 0x22, 0x1310, 0x401, //Oil Temp, 16b
    0x7E1, 0x22, 0x1E1C, 0x402, //ATF Temp, 16b

};



uint8_t fast_pids[FAST_PID_COUNT] = {
    0x04, // PID 0x04
    0x05, // PID 0x05
    0x0B, // PID 0x0B
    0x0C, // PID 0x0C
    0x0D, // PID 0x0D
    0x0E, // PID 0x0E
    // 0x0F, // PID 0x0F (commented out)
    0x10, // PID 0x10
    0x11, // PID 0x11
    0x34,
    0x42, // PID 0x42
    0x43, // PID 0x43
    0x44, // PID 0x44
    0x45, // PID 0x45
    0x47, // PID 0x47
    // 0x48, // PID 0x48
    0x49, // PID 0x49
    0x4A, // PID 0x4A
    // 0x4B, // PID 0x4B
    0x4C, // PID 0x4C
};



uint32_t can_supported_headers[7] = {
    #ifdef CAN_DEBUG
    0xFFFFFFFF, // 0x00-0x1F
    0xAAAAAAAA, // 0x20-0x3F
    0x55555555, // 0x40-0x5F
    0xF0F00F0F, // 0x60-0x7F
    0xF0FF0F0, // 0x80-0x9F
    0xAAAA5555, // 0xA0-0xBF
    0x00000000  // 0xC0-0xDF
    #else
    0x00000000, // 0x00-0x1F
    0x00000000, // 0x20-0x3F
    0x00000000, // 0x40-0x5F
    0x00000000, // 0x60-0x7F
    0x00000000, // 0x80-0x9F
    0x00000000, // 0xA0-0xBF
    0x00000000  // 0xC0-0xDF
#endif
};
uint8_t supported_pid[0x100];
uint8_t valid_headers=0;
uint8_t start_pid=0;

void can_forceUnsupportedPid(void)
{
    //dont support rereading support list
    supported_pid[0x00] = 0; // Mark PID 0x00 as unsupported
    supported_pid[0x01] = 0; // Mark PID 0x01 as unsupported
    supported_pid[0x02] = 0; // Mark PID 0x02 as unsupported
    supported_pid[0x20] = 0; // Mark PID 0x20 as unsupported
    supported_pid[0x40] = 0; // Mark PID 0x40 as unsupported
    supported_pid[0x60] = 0; // Mark PID 0x60 as unsupported
    supported_pid[0x80] = 0; // Mark PID 0x80 as unsupported
    supported_pid[0xA0] = 0; // Mark PID 0xA0 as unsupported
    supported_pid[0xC0] = 0; // Mark PID 0xC0 as unsupported
    
    //dont support o2 sensors
    supported_pid[0x14] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x15] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x16] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x17] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x18] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x19] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x1A] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x1B] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x1D] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x1E] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x1F] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x24] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x25] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x26] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x27] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x28] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x29] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x2A] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x2B] = 0; // Mark PID 0xC0 as unsupported
    // supported_pid[0x34] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x35] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x36] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x37] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x38] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x39] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x3A] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x3B] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x55] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x56] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x57] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x58] = 0; // Mark PID 0xC0 as unsupported

    //EGR
    supported_pid[0x12] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x2C] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x2D] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x2E] = 0; // Mark PID 0xC0 as unsupported

    //fuel rail pressure relative to manifold vacuum
    supported_pid[0x22] = 0; // Mark PID 0xC0 as unsupported
    
    //error related
    supported_pid[0x41] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x4D] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x4E] = 0; // Mark PID 0xC0 as unsupported
    supported_pid[0x4F] = 0; // Mark PID 0xC0 as unsupported



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
        case 0x42:
        // case 0x44:
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

//unused
// bool can_isPidValid(uint8_t pid)
// {
//     if(pid > CAN_MAX_PID) return false; // Check if PID is within valid range
//     // bool is_valid = false;
//     //always ignore these PIDs
//     switch(pid)
//     {
//         case 0x00:
//         case 0x01:
//         case 0x02:
//         case 0x20:
//         case 0x40:
//         case 0x60:
//         case 0x80:
//         case 0xA0:
//         case 0xC0:
//             return false; // These PIDs are valid
//         default:
//             return true;
//         break;
//     }
// }

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

    if(can_rx_header.Identifier > 0x7D0)
    {
        
        switch(can_state)
        {
            case CAN_WAIT_RSP:
                can_state = CAN_PROCESSING; // Set state to processing after receiving data

            break;
            case CAN_WAIT_RSP_SLOW:
                can_state = CAN_PROCESSING_SLOW; // Set state to processing after receiving data

            break;
            default:
            return;
            break;
        }
        memcpy(can_rx_data_buf,can_rx_data,8);
    }

    // if (can_state == CAN_WAIT_RSP && can_rx_header.Identifier >0x7D0)
    // {
    //     memcpy(can_rx_data_buf,can_rx_data,8);
    //     can_state = CAN_PROCESSING; // Set state to processing after receiving data
    //     // can_onDataReceived(); // Process the received CAN message
    // }

    // if(can_state == CAN_TURNON_WAIT_RSP && can_rx_header.Identifier >0x7D0 && can_rx_data[2]==turnon_target_pid)
    // {
    //     can_state=CAN_TURNON_PROCESSING;
    //     // can_onDataReceivedTurnon(); // Process the received CAN message for turn-on requests
    // }
    // else if (can_state == CAN_WAIT_RSP && can_rx_header.Identifier >0x7D0 && can_rx_data[2]==target_pid)
    // {
    //     memcpy(can_rx_data_buf,can_rx_data,8);
    //     can_state = CAN_PROCESSING; // Set state to processing after receiving data
    //     // can_onDataReceived(); // Process the received CAN message
    // }
    // else if (can_state == CAN_WAIT_RSP_SLOW && can_rx_header.Identifier >0x7D0 && can_rx_data[2]==target_pid)
    // {
    //     memcpy(can_rx_data_buf,can_rx_data,8);
    //     can_state = CAN_PROCESSING_SLOW; // Set state to processing after receiving data
        
    //     // can_onDataReceived(); // Process the received CAN message
    // }
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
    // switch(pid)
    // {
    //     case 0x01:
    //         // Process PID 0x01
    //         break;
    //     case 0x02:
    //         // Process PID 0x02
    //         break;
    //     // Add more cases as needed
    //     default:
    //         // Handle unknown PID
    //         break;
    // }

    //temporary for now
    modb_db[0x100+pid] = val & 0xFFFF; // Store the processed value in the modbus database
    modb_db[0x200+pid] = val >> 16; // Store the processed value in the modbus database
}




void can_onDataReceivedTurnon()
{
    
    uint32_t can_rx_u32=0;
    can_rx_u32 = (can_rx_data[4] << 24) | (can_rx_data[5] << 16) | (can_rx_data[6] << 8) | can_rx_data[7];
    can_supported_headers[turnon_target_pid/0x20] = can_rx_u32; // Mark the PID as supported
    // supported_pid[turnon_target_pid] = 1; // Mark the PID as supported
    turnon_target_pid+=0x20;
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

void can_processSupportedPIDs(void)
{
    // for(uint8_t i=0;i<8;i++)
    // {
    //     for(uint8_t j=0;j<0x20;j++)
    //     {
    //         uint8_t pid = (i*0x20) + j;
    //         if((can_supported_headers[i] & (1 << (31-j))) != 0) // Check if the PID is supported
    //         {
    //             supported_pid[pid] = 1; // Mark the PID as supported
    //         }
    //         else
    //         {
    //             supported_pid[pid] = 0; // Mark the PID as not supported
    //         }
    //     }
    // }
    for(uint8_t i=0;i<PID_TABLE_ROWS;i++)
    {
        uint16_t temp = pid_table[i*PID_TABLE_COLS + 2];
        if(temp == 0) break;
        if(temp<=0xFF)
        {
            supported_pid[temp] = 1;
        }
        
    }
    for(uint8_t i=0;i<PID_TABLE_ROWS;i++)
    {
        uint16_t temp = pid_table_slow[i*PID_TABLE_COLS + 2];
        if(temp == 0) break;

        if(temp<=0xFF)
        {
            supported_pid[temp] = 1;
        }
        
    }
    // if(support_all)
    // {
    //     for(uint8_t i=0;i<CAN_MAX_PID;i++)
    //     {
    //         supported_pid[i]=1;
    //     }
    //     for(uint16_t i=CAN_MAX_PID;i<=0xFF;i++)
    //     {
    //         supported_pid[i]=0;
    //     }
        
    // }
    // can_forceUnsupportedPid();//force to not use the 0x_0 PIDs
}


void can_onDataReceived(void)
{
    uint32_t val = 0;
    uint8_t bytes_following = 0;
    uint8_t len = 0;
    uint8_t data[4];
    if (can_rx_data_buf[1] > 1)
    {
        len = can_rx_data_buf[1]-3;
        memcpy(data, &can_rx_data_buf[4], 4);
    }
    else
    {
        len = can_rx_data_buf[1]-2;
        memcpy(data, &can_rx_data_buf[3], 4);

    }

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

    switch(can_state)
    {
        case CAN_PROCESSING:
        pid_idx++;
        can_state=CAN_WRITE;
        if(pid_idx==PID_TABLE_ROWS || (pid_table[pid_idx * PID_TABLE_COLS] < 0x700))
        {
            pid_idx=0;
        }
        case CAN_PROCESSING_SLOW:
        pid_idx_slow++;
        can_state=CAN_WRITE;
        if(pid_idx_slow==PID_TABLE_ROWS || (pid_table_slow[pid_idx * PID_TABLE_COLS] < 0x700))
        {
            pid_idx_slow=0;
        }
        break;
        default:
            break;
    }

    if(pid_table[pid_idx * PID_TABLE_COLS + 3] > 0)
    {
        modb_db[pid_table[pid_idx * PID_TABLE_COLS + 3]] = val;
    }
    else
    {
        modb_db[0x100+can_rx_data_buf[2]] = val & 0xFFFF; // Store the processed value in the modbus database
        modb_db[0x200+can_rx_data_buf[2]] = val >> 16;
    }
}


void can_onDataReceived_Old()
{

    // Process the received data
    // For example, you can log it or store it for further processing
    // Here we just print the received data for demonstration
    // modb_db[can_rx_data[2]] = (can_rx_data[3] << 8) | can_rx_data[4]; // Store the received data in modbus database

    // uint8_t process_pid = 0;
    // switch(can_state)
    // {
    //     case CAN_PROCESSING:
    //         process_pid = target_pid; // Use the current target PID for processing
    //     break;
    //     case CAN_PROCESSING_SLOW:
    //         process_pid = target_pid_slow; // Use the current target PID for processing
    //     break;
    //     default:
    //         break;
    // }

    can_DataProcessing(target_pid,&can_rx_data_buf[3],can_rx_data_buf[0]-2); // Process the received data

    // switch(can_state)
    // {
    //     case CAN_PROCESSING:
    //         target_pid_fast_idx++;
    //     break;
    //     case CAN_PROCESSING_SLOW:
    //         target_pid_slow++;
    //     break;
    //     default:
    //         break;
    // }
    
    switch(can_state)
    {
        case CAN_PROCESSING:
        target_pid_fast_idx++;
        can_state=CAN_WRITE;
        if(target_pid_fast_idx==FAST_PID_COUNT)
        {
            target_pid_fast_idx=0;
            can_state=CAN_WRITE_SLOW;
        }
        break;
        case CAN_PROCESSING_SLOW:
            target_pid_slow++;
            
            if(target_pid_slow==CAN_MAX_PID)
            {
                target_pid_slow=0;
            }
            can_state=CAN_WRITE;
            // if(target_pid_fast_idx==FAST_PID_COUNT)
            // {
            //     target_pid_fast_idx=CAN_MAX_PID;
            // }
        break;
        default:
            break;
    }
    


    // if(target_pid< CAN_MAX_PID) // Increment target PID for next request
    // {
    //     target_pid++;
    // }
    // else
    // {
    //     target_pid = start_pid; // Reset to 0x04 if exceeded
    // }
    // can_state = CAN_WRITE; // Reset state to write next request
    
}

bool can_isPIDValid(uint8_t pid)
{
    // bool isvalid=0;
    // if(can_state == CAN_WRITE)
    // {
	// 	if(supported_pid[pid])
	// 	{
	// 		return 1;
    //     }
    // }
    // else if (can_state == CAN_WRITE_SLOW)
    // {

	// 	if(supported_pid[pid])
	// 	{
	// 		for(uint8_t j=0;j<FAST_PID_COUNT;j++)
	// 		{
	// 			if(pid == fast_pids[j])
	// 			return 0;
	// 		}
	// 		return 1;
	// 	}

    // }

    // return 0;

}


void can_sendRequest(void)
{
    // switch(can_state)
    // {
    //     case CAN_WRITE:
    //         target_pid = fast_pids[target_pid_fast_idx]; // Use the current target PID for processing
    //         while(!can_isPIDValid(target_pid))
    //         {
    //             target_pid_fast_idx++;
    //             target_pid = fast_pids[target_pid_fast_idx];
    //             if(target_pid_fast_idx>FAST_PID_COUNT)
    //             {
    //                 target_pid_fast_idx=0;
    //                 break;
    //             }
    //         }
    //         target_pid = fast_pids[target_pid_fast_idx];
            
    //     break;
    //     default:
    //     return;
    //         break;
    // }


    can_tx_header.Identifier = pid_table[PID_TABLE_COLS * pid_idx]; // Standard ID for OBD-II requests
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

    uint16_t * target_table;
    uint8_t target_pid_idx;

    switch(can_state)
    {
        case CAN_WRITE:
            target_table = pid_table;
            target_pid_idx = pid_idx;
            break;
            case CAN_WRITE_SLOW:
            target_table = pid_table_slow;
            target_pid_idx = pid_idx_slow;
            break;
        default:
            return;
            break;
    }


    // can_tx_data[0]=2;
    can_tx_data[1]=target_table[PID_TABLE_COLS * target_pid_idx + 1]; // OBD-II request
    // can_tx_data[2]=target_pid; // PID to request
    if(target_table[PID_TABLE_COLS * target_pid_idx + 2] > 0xFF)
    {
        can_tx_data[0] = 3;

        can_tx_data[2] = target_table[PID_TABLE_COLS * target_pid_idx + 2] >> 8;
        can_tx_data[3] = target_table[PID_TABLE_COLS * target_pid_idx + 2] & 0xFF;
    }
    else
    {
        can_tx_data[0] = 2;
        can_tx_data[2] = target_table[PID_TABLE_COLS * target_pid_idx + 2] & 0xFF;
        
    }
    if(HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &can_tx_header, can_tx_data) !=0)
    {
        Error_Handler();
    }
}



void can_sendRequest_Old(void)
{

    // bool is_valid_pid = 0;
    // do
    // {
    //     if(FAST_MODE)
    //     {
    //         is_valid_pid = can_isFastPID(target_pid) && supported_pid[target_pid];
    //     }
    //     else
    //     {
    //         is_valid_pid = supported_pid[target_pid]; // Check if the target PID is supported
    //     }
    //     if(!is_valid_pid) // If the PID is not supported, increment it
    //     {
    //         target_pid++;
    //         if(target_pid > CAN_MAX_PID) // Wrap around if PID exceeds 0xDF
    //         {
    //             target_pid = start_pid; // Reset to 0x04
    //             break;
    //         }
    //     }
    // }while(!is_valid_pid);

//     switch(can_state)
//     {
//         case CAN_WRITE:
//             target_pid = fast_pids[target_pid_fast_idx]; // Use the current target PID for processing
//             while(!can_isPIDValid(target_pid))
//             {
//                 target_pid_fast_idx++;
//                 target_pid = fast_pids[target_pid_fast_idx];
//                 if(target_pid_fast_idx>FAST_PID_COUNT)
//                 {
//                     target_pid_fast_idx=0;
//                     break;
//                 }
//             }
//             target_pid = fast_pids[target_pid_fast_idx];
            
//         break;
//         case CAN_WRITE_SLOW:
// //            target_pid = target_pid_slow;
// //            if(!can_isPIDValid(target_pid))
//             while(!can_isPIDValid(target_pid_slow))
//             {
//                 target_pid_slow++;
//                 target_pid = target_pid_slow;
//                 if(target_pid_slow == CAN_MAX_PID)
//                 {
//                 	target_pid_slow = 4;
//                 	break;
//                 }
//             }
//             target_pid = target_pid_slow;
//         break;
//         default:
//             break;
//     }

//     can_tx_header.Identifier = 0x7DF; // Standard ID for OBD-II requests
//     can_tx_header.IdType = FDCAN_STANDARD_ID;
//     // can_tx_header.TxFrameType = FDCAN_REMOTE_FRAME;
//     can_tx_header.TxFrameType = FDCAN_DATA_FRAME;
//     can_tx_header.DataLength = FDCAN_DLC_BYTES_8; // OBD-II requests typically use 8 bytes
//     can_tx_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
//     can_tx_header.BitRateSwitch = FDCAN_BRS_OFF;
//     can_tx_header.FDFormat = FDCAN_CLASSIC_CAN;
//     can_tx_header.TxEventFifoControl = FDCAN_NO_TX_EVENTS; // No Tx event FIFO control
//     can_tx_header.MessageMarker = 0; // Not used in this context

//     memset(can_tx_data, 0xFF, 8); // Clear the data buffer
//     can_tx_data[0]=2;
//     can_tx_data[1]=0x01; // OBD-II request
//     can_tx_data[2]=target_pid; // PID to request
//     if(HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &can_tx_header, can_tx_data) !=0)
//     {
//         Error_Handler();
//     }
    
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

    // while(!can_isPidValid(turnon_target_pid)) // Ensure the target PID is valid
    // {
    //     turnon_target_pid+=0x20;
    //     if(turnon_target_pid > CAN_MAX_PID) // Wrap around if PID exceeds 0xDF
    //     {
    //         turnon_target_pid = 0x00; // Reset to 0x00
    //         break;
    //     }
    // }
    memset(can_tx_data, 0xFF, 8); // Clear the data buffer
    can_tx_data[0] = 2; // Length of the request
    can_tx_data[1] = 0x01; // OBD-II request
    can_tx_data[2] = turnon_target_pid; // PID to turn on

    HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &can_tx_header, can_tx_data);
}

void can_init(void)
{
    turnon_target_pid= 0x00; // Reset target PID for turn-on requests
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
bool can_timedout=0;
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
                // can_state = CAN_FINISH_TURNON;
                can_state = CAN_TURNON;
                #else
                can_state=CAN_FINISH_TURNON;
                // can_state = CAN_TURNON;
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
                can_timedout=1;
            }
            break;
        case CAN_WAIT_RSP:
            if(can_timeout>0)
            {
                can_timeout--;
            }
            else
            {
                can_timedout=1;
            }
            break;
        case CAN_WAIT_RSP_SLOW:
            if(can_timeout>0)
            {
                can_timeout--;
            }
            else
            {
                can_timedout=1;
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
    // can_forceUnsupportedPid();
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
        
        case CAN_TURNON_WAIT_RSP:
            if(can_timedout)
            {
                can_timedout=0;
                can_timeout=CAN_STARTUP_TIMEOUT; // Reset timeout for next turn-on request
                can_state = CAN_TURNON;

                // can_supported_headers[turnon_target_pid/0x20] = 0; // Mark the PID as supported
                // supported_pid[turnon_target_pid] = 0; // Mark the PID as not supported
                turnon_target_pid += 0x20; // Reset target PID to the next one
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
                

        case CAN_FINISH_TURNON:
            can_processSupportedPIDs();
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
        case CAN_WRITE: // writes fast
            // Check for incoming messages or send requests
            // if(target_pid_fast_idx == 0)
            // {
            //     CAN_LOOP_TIME = can_time;
            //     can_time=0;
            // }
            can_sendRequest();
            can_state = CAN_WAIT_RSP; // Transition to waiting for response state
            break;
        case CAN_WAIT_RSP:
            if(can_timedout)
            {
                can_timedout=0;
                can_timeout=CAN_TIMEOUT;
                can_state = CAN_WRITE;
                pid_idx++;
                if(pid_table[pid_idx * PID_TABLE_COLS] == 0 || pid_idx > PID_TABLE_ROWS)
                {
                    pid_idx = 0;
                    can_state = CAN_WRITE_SLOW;

                }
                // target_pid_fast_idx++;
                // if(target_pid_fast_idx==FAST_PID_COUNT)
                // {
                //     target_pid_fast_idx=0;
                //     can_state = CAN_WRITE_SLOW;
                //     can_timeout=CAN_TIMEOUT;
                // }
            }
            break;
            
        case CAN_PROCESSING:
            // Process the received data
            can_onDataReceived(); // Process the received CAN message
            
            // This state can be used for further processing of received messages
            break;
        case CAN_WRITE_SLOW: // writes slow
            CAN_LOOP_TIME = can_time;
            can_time=0;
            can_sendRequest();
            can_state = CAN_WAIT_RSP_SLOW;
        break;
        case CAN_WAIT_RSP_SLOW:
            if(can_timedout)
            {
                can_timedout=0;
                modb_db[target_pid + 0x400]++;
                if(modb_db[target_pid + 0x400]==0xFFFF)  modb_db[target_pid + 0x400]=0;
                can_timeout=CAN_TIMEOUT;
                can_state = CAN_WRITE;
                pid_idx_slow++;
                if(pid_idx_slow==PID_TABLE_ROWS || (pid_table_slow[pid_idx_slow * PID_TABLE_COLS] < 0x700))
                {
                    pid_idx_slow=0;
                }
            }
            break;
        case CAN_PROCESSING_SLOW:
        can_onDataReceived();
        //processes slow data
        break;
        default:
            // Handle unexpected states
            break;
    }

    // modb_db[0x302]=target_pid_slow;
    // modb_db[0x300] = can_state; // Update the modbus database with the current CAN state
    // modb_db[0x301] = requests;
    // modb_db[0x302] = responses; // Update the modbus database with the number of requests and responses
}

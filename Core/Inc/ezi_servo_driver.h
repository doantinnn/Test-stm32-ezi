/**
  ******************************************************************************
  * @file    ezi_servo_driver.h
  * @brief   Header file for FASTECH Ezi-SERVOII Plus-E UDP Driver Communication
  *          Grounded in Ezi-SERVOII Plus-E Communication Manual (FASTECH)
  *          Optimized for STM32F767 IO Extension Board with LwIP Raw UDP API
  ******************************************************************************
  */

#ifndef INC_EZI_SERVO_DRIVER_H_
#define INC_EZI_SERVO_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "eziservo_config.h"
#include "lwip/ip_addr.h"
#include "lwip/udp.h"
#include "lwip/pbuf.h"

/* Protocol Constants */
#define EZI_FRAME_HEADER        0xAA    // Beginning of Frame marker
#define EZI_MAX_FRAME_DATA      250U

#define CMD_SET_PARAMETER       0x12    // 18: FAS_SetParameter
#define CMD_GET_PARAMETER       0x13    // 19: FAS_GetParameter
#define CMD_GET_IO_INPUT        0x24    // 36: FAS_GetIOInput
#define CMD_GET_IO_OUTPUT       0x25    // 37: FAS_GetIOOutput
#define CMD_SET_IO_OUTPUT       0x26    // 38: FAS_SetIOOutput
#define CMD_SERVO_ENABLE        0x2A    // 42: FAS_ServoEnable
#define CMD_ALARM_RESET         0x2B    // 43: FAS_ServoAlarmReset
#define CMD_GET_ALARM_TYPE      0x2C    // 44: FAS_GetAlarmType
#define CMD_MOVE_STOP           0x31    // 49: FAS_MoveStop
#define CMD_EMERGENCY_STOP      0x32    // 50: FAS_EmergencyStop
#define CMD_MOVE_ORIGIN         0x33    // 51: FAS_MoveOriginSingleAxis
#define CMD_MOVE_ABS            0x34    // 52: FAS_MoveSingleAxisAbsPos
#define CMD_MOVE_INC            0x35    // 53: FAS_MoveSingleAxisIncPos
#define CMD_MOVE_VELOCITY       0x37    // 55: FAS_MoveVelocity
#define CMD_GET_IO_AXIS_STATUS  0x41    // 65: FAS_GetIOAxisStatus
#define CMD_GET_MOTION_STATUS   0x42    // 66: FAS_GetMotionStatus
#define CMD_GET_ALL_STATUS      0x43    // 67: FAS_GetAllStatus
#define CMD_GET_CMD_POS         0x51    // 81: FAS_GetCommandPos
#define CMD_GET_ACT_POS         0x53    // 83: FAS_GetActualPos
#define CMD_GET_POS_ERR         0x54    // 84: FAS_GetPosError
#define CMD_GET_ACT_VEL         0x55    // 85: FAS_GetActualVel
#define CMD_CLEAR_POSITION      0x56    // 86: FAS_ClearPosition

/* Communication Return Codes */
#define EZI_OK                  0       // Operation successful
#define EZI_ERR_SOCKET          -1      // Socket / PCB error
#define EZI_ERR_TIMEOUT         -2      // Response timeout
#define EZI_ERR_HEADER          -3      // Invalid frame header
#define EZI_ERR_STATUS          -4      // Driver returned communication error
#define EZI_ERR_PARAM           -5      // Invalid argument
#define EZI_ERR_LINK            -6      // Ethernet Link Down

/* Motion Status Flag Bits (Axis Status Bitmask) */
#define FLAG_ERROR_ALL          (1UL << 0)   // Any error occurs
#define FLAG_HW_POS_LIMIT       (1UL << 1)   // +Limit sensor ON
#define FLAG_HW_NEG_LIMIT       (1UL << 2)   // -Limit sensor ON
#define FLAG_SW_POS_LIMIT       (1UL << 3)   // +Software Limit reached
#define FLAG_SW_NEG_LIMIT       (1UL << 4)   // -Software Limit reached
#define FLAG_POS_OVERFLOW       (1UL << 7)   // Position Error overflow
#define FLAG_SERVO_ON           (1UL << 8)   // Servo Motor is ON
#define FLAG_ALARM_RESET        (1UL << 9)   // Alarm Reset signal active
#define FLAG_IN_POSITION        (1UL << 10)  // Motor reached target position
#define FLAG_DRIVE_FAULT        (1UL << 11)  // Driver Fault / Alarm
#define FLAG_MOTION_MOVING      (1UL << 13)  // Motor is currently moving
#define FLAG_MOTION_ACCEL       (1UL << 14)  // Motor in Acceleration phase
#define FLAG_MOTION_DECEL       (1UL << 15)  // Motor in Deceleration phase
#define FLAG_MOTION_CONST       (1UL << 16)  // Motor running at Constant speed
#define FLAG_ORIGIN_OK          (1UL << 21)  // Origin Search Completed

#pragma pack(push, 1)

/* Tx Frame Structure (STM32 -> Driver) */
typedef struct {
    uint8_t header;         // 0xAA
    uint8_t length;         // Length after Length field (SyncNo + Reserved + FrameType + Data)
    uint8_t sync_no;        // Packet Sync Number (increments per command)
    uint8_t reserved;       // Fixed 0x00
    uint8_t frame_type;     // Command ID
    uint8_t data[EZI_MAX_FRAME_DATA];      // Command Payload
} EziTxFrame_t;

/* Rx Frame Structure (Driver -> STM32) */
typedef struct {
    uint8_t header;         // 0xAA
    uint8_t length;         // Length after Length field
    uint8_t sync_no;        // Sync Number matching Tx
    uint8_t reserved;       // Fixed 0x00
    uint8_t frame_type;     // Command ID
    uint8_t comm_status;    // 0x00 = Success, >0x80 = Driver Error Code
    uint8_t data[EZI_MAX_FRAME_DATA];      // Response Payload
} EziRxFrame_t;

#pragma pack(pop)

/* Motion Status Data Structure (Frame Type 0x42 Response - naturally 4-byte aligned) */
typedef struct {
    int32_t  cmd_pos;       // Command Position [pulse]
    int32_t  act_pos;       // Actual Position [pulse]
    int32_t  pos_err;       // Position Error [pulse]
    uint32_t act_vel;       // Actual Velocity [pps]
    uint32_t pt_num;        // Current Position Table Item Number (4 bytes on wire)
} EziServo_MotionStatus_t;

/* All Status Data Structure (Frame Type 0x43 Response - naturally 4-byte aligned) */
typedef struct {
    uint32_t input_status;  // Digital Input Pin Status
    uint32_t output_status; // Digital Output Pin Status
    uint32_t axis_status;   // Motion Flag Bits (FLAG_SERVO_ON, FLAG_MOTION_MOVING, etc.)
    int32_t  cmd_pos;       // Command Position [pulse]
    int32_t  act_pos;       // Actual Position [pulse]
    int32_t  pos_err;       // Position Error [pulse]
    uint32_t act_vel;       // Actual Velocity [pps]
    uint32_t pt_num;        // Position Table Item Number (4 bytes on wire)
} EziServo_AllStatus_t;


/* Driver Handle Structure */
typedef struct {
    struct udp_pcb *upcb;           // LwIP UDP Control Block
    ip_addr_t dest_ip;              // Target Driver IP Address
    uint16_t dest_port;             // Target Driver Port (3002)
    uint8_t sync_no;                // Sync Number Counter
    volatile bool is_initialized;   // Driver Connection State
    volatile bool is_connected;     // Active UDP Response Received
    volatile bool rx_ready;         // Response Flag
    volatile uint8_t rx_expected_sync;
    volatile uint8_t rx_expected_frame_type;
    EziRxFrame_t rx_frame;          // Last received frame buffer
    uint16_t rx_len;                // Received payload byte count
} EziServo_Handle_t;

/* Function Prototypes */
int  EziServo_Init(EziServo_Handle_t *handle, const char *driver_ip, uint16_t port);
void EziServo_DeInit(EziServo_Handle_t *handle);
int  EziServo_SendAndReceive(EziServo_Handle_t *handle, uint8_t frame_type, const uint8_t *payload, uint8_t payload_len, uint8_t expected_data_len, EziRxFrame_t *rx_out);

int  EziServo_SetEnable(EziServo_Handle_t *handle, bool enable);
int  EziServo_AlarmReset(EziServo_Handle_t *handle);

int  EziServo_MoveAbs(EziServo_Handle_t *handle, int32_t target_pos_pulse, uint32_t speed_pps);
int  EziServo_MoveInc(EziServo_Handle_t *handle, int32_t inc_pos_pulse, uint32_t speed_pps);
int  EziServo_MoveVelocity(EziServo_Handle_t *handle, uint32_t speed_pps, uint8_t dir_forward);
int  EziServo_MoveStop(EziServo_Handle_t *handle);
int  EziServo_EmergencyStop(EziServo_Handle_t *handle);
int  EziServo_MoveOrigin(EziServo_Handle_t *handle);

int  EziServo_GetActualPos(EziServo_Handle_t *handle, int32_t *act_pos);
int  EziServo_GetCommandPos(EziServo_Handle_t *handle, int32_t *cmd_pos);
int  EziServo_GetMotionStatus(EziServo_Handle_t *handle, EziServo_MotionStatus_t *status);
int  EziServo_GetAllStatus(EziServo_Handle_t *handle, EziServo_AllStatus_t *status);
int  EziServo_ClearPosition(EziServo_Handle_t *handle);
int  EziServo_GetAlarmType(EziServo_Handle_t *handle, uint8_t *alarm_code);
const char* EziServo_GetAlarmString(uint8_t alarm_code);

/* Diagnostic & Parameter Access Functions (Safe to read without motor) */
int  EziServo_GetPosError(EziServo_Handle_t *handle, int32_t *pos_err);
int  EziServo_GetActualVel(EziServo_Handle_t *handle, uint32_t *act_vel);
int  EziServo_GetParameter(EziServo_Handle_t *handle, uint8_t param_num, int32_t *val);
int  EziServo_GetIOInput(EziServo_Handle_t *handle, uint32_t *u_in);
int  EziServo_GetIOOutput(EziServo_Handle_t *handle, uint32_t *u_out);

/* Driver Diagnostic & Parameter Status Structure (Safe to read without motor) */
typedef struct {
    uint32_t udp_packet_count;      // Total valid UDP packets exchanged
    int32_t  last_res;              // Last command return code (0 = EZI_OK)
    uint32_t alarm_code;            // Driver Alarm Code (0 = No alarm, 7/8 = Motor/Encoder fault)
    int32_t  act_pos;               // Actual Position counter [pulse] (CMD 0x53)
    int32_t  cmd_pos;               // Command Position counter [pulse] (CMD 0x51)
    int32_t  pos_err;               // Position Error [pulse] (CMD 0x54)
    uint32_t act_vel;               // Actual Velocity [pps] (CMD 0x55)
    uint32_t io_inputs;             // CN1 Digital Input Bits (Limit+, Limit-, Org, etc.) (CMD 0x24)
    uint32_t io_outputs;            // CN1 Digital Output Bits (InPos, Alarm, Out1..9) (CMD 0x25)
    int32_t  param_resolution;      // Param 0: Microstep Resolution (Pulses/Rev, e.g. 10000)
    int32_t  param_max_speed;       // Param 1: Max Speed setting [pps]
    int32_t  param_start_speed;     // Param 2: Starting Speed setting [pps]
    int32_t  param_acc_time;        // Param 3: Acceleration Time [ms]
    int32_t  param_dec_time;        // Param 4: Deceleration Time [ms]
} EziDriverDiag_t;

int  EziServo_UpdateDiagnostics(EziServo_Handle_t *handle, EziDriverDiag_t *diag);

#ifdef __cplusplus
}
#endif

#endif /* INC_EZI_SERVO_DRIVER_H_ */

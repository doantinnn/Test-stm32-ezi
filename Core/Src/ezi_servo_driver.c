/**
  ******************************************************************************
  * @file    ezi_servo_driver.c
  * @brief   Implementation of FASTECH Ezi-SERVOII Plus-E UDP Driver Communication
  *          Grounded in Ezi-SERVOII Plus-E Communication Manual (FASTECH)
  *          Optimized for STM32F767 IO Extension Board with LwIP Raw UDP API
  ******************************************************************************
  */

#include "ezi_servo_driver.h"
#include "ethernetif.h"
#include "lwip/lwip_timers.h"
#include "lwip/netif.h"
#include "stm32f7xx_hal.h"
#include "IO_Board.h"

/* External Network Interface */
extern struct netif gnetif;

/**
  * @brief LwIP UDP Receive Callback for Ezi-SERVO Response Packets
  */
static void EziServo_RxCallback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    EziServo_Handle_t *handle = (EziServo_Handle_t *)arg;
    (void)upcb;

    if (!handle || !p || !handle->is_initialized) {
        if (p) pbuf_free(p);
        return;
    }

    /* A valid response needs at least 2 bytes (Header + Length) */
    if (p->tot_len < 2U) {
        pbuf_free(p);
        return;
    }

    uint16_t copy_len = (p->tot_len < sizeof(EziRxFrame_t)) ? p->tot_len : sizeof(EziRxFrame_t);
    pbuf_copy_partial(p, &handle->rx_frame, copy_len, 0);

    /* Verify FASTECH Header (0xAA) */
    if (handle->rx_frame.header != EZI_FRAME_HEADER) {
        pbuf_free(p);
        return;
    }

    /* Adopt responding IP and Port so communication is self-healing */
    if (addr != NULL) {
        handle->dest_ip = *addr;
        handle->dest_port = port;
    }

    /* Total expected frame size = Header(1) + Length(1) + length */
    uint16_t frame_total_len = (uint16_t)(handle->rx_frame.length + 2U);
    handle->rx_len = (copy_len < frame_total_len) ? copy_len : frame_total_len;
    handle->rx_ready = true;
    handle->is_connected = true;

    pbuf_free(p);
}

/**
  * @brief Helper function to send command and wait for response frame from Driver
  * @param expected_data_len Number of response Data bytes after communication-status byte.
  */
int EziServo_SendAndReceive(EziServo_Handle_t *handle,
                            uint8_t frame_type,
                            const uint8_t *payload,
                            uint8_t payload_len,
                            uint8_t expected_data_len,
                            EziRxFrame_t *rx_out)
{
    (void)expected_data_len;

    if (!handle || !handle->is_initialized || !handle->upcb) {
        return EZI_ERR_SOCKET;
    }

    if (!netif_is_up(&gnetif)) {
        return EZI_ERR_LINK;
    }

    if ((payload_len > EZI_MAX_FRAME_DATA) || (payload_len > 0U && payload == NULL)) {
        return EZI_ERR_PARAM;
    }

    EziTxFrame_t tx;
    memset(&tx, 0, sizeof(tx));

    /* Increment Sync Number for each new transaction. */
    handle->sync_no++;
    if (handle->sync_no == 0U) handle->sync_no = 1U;

    tx.header     = EZI_FRAME_HEADER;
    tx.sync_no    = handle->sync_no;
    tx.reserved   = 0x00;
    tx.frame_type = frame_type;
    tx.length     = (uint8_t)(3U + payload_len);

    if (payload_len > 0U) {
        memcpy(tx.data, payload, payload_len);
    }

    const uint16_t tx_bytes = (uint16_t)(2U + tx.length);

    handle->rx_ready = false;
    handle->rx_len = 0U;
    handle->rx_expected_sync = tx.sync_no;
    handle->rx_expected_frame_type = frame_type;

    /* Allocate pbuf and transmit over LwIP UDP. */
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, tx_bytes, PBUF_RAM);
    if (!p) {
        return EZI_ERR_SOCKET;
    }

    if (pbuf_take(p, &tx, tx_bytes) != ERR_OK) {
        pbuf_free(p);
        return EZI_ERR_SOCKET;
    }

    err_t err = udp_sendto(handle->upcb, p, &handle->dest_ip, handle->dest_port);
    pbuf_free(p);

    if (err != ERR_OK) {
        return EZI_ERR_SOCKET;
    }

    /* Wait for response packet with timeout (polling Ethernet input). */
    uint32_t start_time = HAL_GetTick();
    while (!handle->rx_ready && (HAL_GetTick() - start_time < EZI_SERVO_RX_TIMEOUT_MS)) {
        ethernetif_input(&gnetif);
        sys_check_timeouts();
        IOBoard_ProcessJ101Pulse();
    }

    if (!handle->rx_ready) {
        return EZI_ERR_TIMEOUT;
    }

    if (rx_out != NULL) {
        memset(rx_out, 0, sizeof(*rx_out));
        memcpy(rx_out, &handle->rx_frame,
               (handle->rx_len <= sizeof(*rx_out)) ? handle->rx_len : sizeof(*rx_out));
    }

    handle->rx_ready = false;

    /* Check Header */
    if (handle->rx_frame.header != EZI_FRAME_HEADER) {
        return EZI_ERR_HEADER;
    }

    /* Check Driver Communication Status Byte if frame has status (Byte 5) */
    if ((handle->rx_len >= 6U) && (handle->rx_frame.comm_status != 0x00U)) {
        return EZI_ERR_STATUS;
    }

    return EZI_OK;
}

/**
  * @brief Initialize UDP Communication for Ezi-SERVOII Plus-E
  */
int EziServo_Init(EziServo_Handle_t *handle, const char *driver_ip, uint16_t port)
{
    if (!handle || !driver_ip) return EZI_ERR_PARAM;

    memset(handle, 0, sizeof(EziServo_Handle_t));

    /* Create LwIP UDP control block */
    handle->upcb = udp_new();
    if (!handle->upcb) {
        return EZI_ERR_SOCKET;
    }

    /* Bind to local ephemeral port */
    err_t err = udp_bind(handle->upcb, IP_ADDR_ANY, 0);
    if (err != ERR_OK) {
        udp_remove(handle->upcb);
        handle->upcb = NULL;
        return EZI_ERR_SOCKET;
    }

    /* Set target IP and Port */
    if (!ipaddr_aton(driver_ip, &handle->dest_ip)) {
        udp_remove(handle->upcb);
        handle->upcb = NULL;
        return EZI_ERR_PARAM;
    }
    handle->dest_port = port;

    /* Register UDP Receive Callback */
    udp_recv(handle->upcb, EziServo_RxCallback, handle);

    handle->sync_no = 0;
    handle->is_initialized = true;

    return EZI_OK;
}

/**
  * @brief Close Socket and Release Resources
  */
void EziServo_DeInit(EziServo_Handle_t *handle)
{
    if (handle && handle->is_initialized) {
        if (handle->upcb) {
            udp_remove(handle->upcb);
            handle->upcb = NULL;
        }
        handle->is_initialized = false;
    }
}

/**
  * @brief Enable/Disable Servo Motor (CMD 0x2A)
  * @param enable: true = Servo ON, false = Servo OFF
  */
int EziServo_SetEnable(EziServo_Handle_t *handle, bool enable)
{
    uint8_t payload[1] = { enable ? 1 : 0 };
    return EziServo_SendAndReceive(handle, CMD_SERVO_ENABLE, payload, 1, 0, NULL);
}

/**
  * @brief Reset Servo Driver Alarm (CMD 0x2B)
  */
int EziServo_AlarmReset(EziServo_Handle_t *handle)
{
    return EziServo_SendAndReceive(handle, CMD_ALARM_RESET, NULL, 0, 0, NULL);
}

/**
  * @brief Single Axis Absolute Position Move (CMD 0x34)
  * @param target_pos_pulse: Absolute Target Position in pulses (-134,217,728 ~ +134,217,727)
  * @param speed_pps: Speed in pulses per second [pps] (1 ~ 500,000)
  */
int EziServo_MoveAbs(EziServo_Handle_t *handle, int32_t target_pos_pulse, uint32_t speed_pps)
{
    uint8_t payload[8];
    memcpy(&payload[0], &target_pos_pulse, 4);
    memcpy(&payload[4], &speed_pps, 4);

    return EziServo_SendAndReceive(handle, CMD_MOVE_ABS, payload, 8, 0, NULL);
}

/**
  * @brief Single Axis Incremental Position Move (CMD 0x35)
  * @param inc_pos_pulse: Incremental Distance in pulses
  * @param speed_pps: Speed in pulses per second [pps]
  */
int EziServo_MoveInc(EziServo_Handle_t *handle, int32_t inc_pos_pulse, uint32_t speed_pps)
{
    uint8_t payload[8];
    memcpy(&payload[0], &inc_pos_pulse, 4);
    memcpy(&payload[4], &speed_pps, 4);

    return EziServo_SendAndReceive(handle, CMD_MOVE_INC, payload, 8, 0, NULL);
}

/**
  * @brief Jog/Velocity Motion (CMD 0x37)
  * @param speed_pps: Speed in pulses per second [pps]
  * @param dir_forward: 1 = Positive Jog (+), 0 = Negative Jog (-)
  */
int EziServo_MoveVelocity(EziServo_Handle_t *handle, uint32_t speed_pps, uint8_t dir_forward)
{
    uint8_t payload[5];
    memcpy(&payload[0], &speed_pps, 4);
    payload[4] = dir_forward ? 1 : 0;

    return EziServo_SendAndReceive(handle, CMD_MOVE_VELOCITY, payload, 5, 0, NULL);
}

/**
  * @brief Stop Running Motor (Deceleration Stop) (CMD 0x31)
  */
int EziServo_MoveStop(EziServo_Handle_t *handle)
{
    return EziServo_SendAndReceive(handle, CMD_MOVE_STOP, NULL, 0, 0, NULL);
}

/**
  * @brief Emergency Stop Motor (CMD 0x32)
  */
int EziServo_EmergencyStop(EziServo_Handle_t *handle)
{
    return EziServo_SendAndReceive(handle, CMD_EMERGENCY_STOP, NULL, 0, 0, NULL);
}

/**
  * @brief Start Origin Search Return Movement (CMD 0x33)
  */
int EziServo_MoveOrigin(EziServo_Handle_t *handle)
{
    return EziServo_SendAndReceive(handle, CMD_MOVE_ORIGIN, NULL, 0, 0, NULL);
}

/**
  * @brief Query Actual Position [pulse] (CMD 0x53)
  */
int EziServo_GetActualPos(EziServo_Handle_t *handle, int32_t *act_pos)
{
    if (!act_pos) return EZI_ERR_PARAM;

    EziRxFrame_t rx;
    int res = EziServo_SendAndReceive(handle, CMD_GET_ACT_POS, NULL, 0, 4, &rx);
    if (res == EZI_OK || res == EZI_ERR_STATUS) {
        memcpy(act_pos, rx.data, 4);
    }
    return res;
}

/**
  * @brief Query Command Position [pulse] (CMD 0x51)
  */
int EziServo_GetCommandPos(EziServo_Handle_t *handle, int32_t *cmd_pos)
{
    if (!cmd_pos) return EZI_ERR_PARAM;

    EziRxFrame_t rx;
    int res = EziServo_SendAndReceive(handle, CMD_GET_CMD_POS, NULL, 0, 4, &rx);
    if (res == EZI_OK || res == EZI_ERR_STATUS) {
        memcpy(cmd_pos, rx.data, 4);
    }
    return res;
}

/**
  * @brief Read Packed Motion Status (CMD 0x42: CmdPos, ActPos, PosErr, ActVel, PT_Num)
  */
int EziServo_GetMotionStatus(EziServo_Handle_t *handle, EziServo_MotionStatus_t *status)
{
    if (!status) return EZI_ERR_PARAM;

    EziRxFrame_t rx;
    int res = EziServo_SendAndReceive(handle, CMD_GET_MOTION_STATUS, NULL, 0, 20, &rx);
    if (res == EZI_OK) {
        memcpy(status, &rx.data[0], sizeof(EziServo_MotionStatus_t));
    }
    return res;
}

/**
  * @brief Read All Driver Status Flags & Positions (CMD 0x43)
  */
int EziServo_GetAllStatus(EziServo_Handle_t *handle, EziServo_AllStatus_t *status)
{
    if (!status) return EZI_ERR_PARAM;

    EziRxFrame_t rx;
    int res = EziServo_SendAndReceive(handle, CMD_GET_ALL_STATUS, NULL, 0, 32, &rx);
    if (res == EZI_OK || res == EZI_ERR_STATUS) {
        memcpy(status, &rx.data[0], sizeof(EziServo_AllStatus_t));
    }
    return res;
}

/**
  * @brief Clear Command and Actual Positions to 0 (CMD 0x56)
  */
int EziServo_ClearPosition(EziServo_Handle_t *handle)
{
    return EziServo_SendAndReceive(handle, CMD_CLEAR_POSITION, NULL, 0, 0, NULL);
}

/**
  * @brief Query Driver Active Alarm Code (CMD 0x2C - FAS_GetAlarmType)
  * @param handle Pointer to driver handle
  * @param alarm_code Pointer to store returned 1-byte alarm code
  */
int EziServo_GetAlarmType(EziServo_Handle_t *handle, uint8_t *alarm_code)
{
    if (!alarm_code) return EZI_ERR_PARAM;

    EziRxFrame_t rx;
    int res = EziServo_SendAndReceive(handle, CMD_GET_ALARM_TYPE, NULL, 0, 1, &rx);
    if (res == EZI_OK || res == EZI_ERR_STATUS) {
        *alarm_code = rx.data[0];
    }
    return res;
}

/**
  * @brief Get Human-Readable string for FASTECH Alarm Code
  */
const char* EziServo_GetAlarmString(uint8_t alarm_code)
{
    switch (alarm_code) {
        case 0:  return "No Alarm (Normal)";
        case 1:  return "Over Current";
        case 2:  return "Over Speed";
        case 3:  return "Position Tracking Error";
        case 4:  return "Over Load";
        case 5:  return "Over Temperature";
        case 6:  return "Over Voltage";
        case 7:  return "Motor Disconnected / Bad Connection";
        case 8:  return "Encoder Disconnected / Bad Connection";
        case 9:  return "Motor Power Low Voltage";
        case 10: return "In-Position Error";
        case 11: return "Regeneration Overload";
        case 12: return "ROM Error";
        default: return "Unknown Alarm";
    }
}

/**
  * @brief Query Position Error [pulse] (CMD 0x54)
  */
int EziServo_GetPosError(EziServo_Handle_t *handle, int32_t *pos_err)
{
    if (!pos_err) return EZI_ERR_PARAM;
    EziRxFrame_t rx;
    int res = EziServo_SendAndReceive(handle, CMD_GET_POS_ERR, NULL, 0, 4, &rx);
    if (res == EZI_OK || res == EZI_ERR_STATUS) {
        memcpy(pos_err, rx.data, 4);
    }
    return res;
}

/**
  * @brief Query Actual Velocity [pps] (CMD 0x55)
  */
int EziServo_GetActualVel(EziServo_Handle_t *handle, uint32_t *act_vel)
{
    if (!act_vel) return EZI_ERR_PARAM;
    EziRxFrame_t rx;
    int res = EziServo_SendAndReceive(handle, CMD_GET_ACT_VEL, NULL, 0, 4, &rx);
    if (res == EZI_OK || res == EZI_ERR_STATUS) {
        memcpy(act_vel, rx.data, 4);
    }
    return res;
}

/**
  * @brief Read RAM Parameter from Driver (CMD 0x13 - FAS_GetParameter)
  * @param param_num Parameter index (0: Pulses/Rev, 1: Max Speed, 2: Start Speed, 3: Acc, 4: Dec...)
  */
int EziServo_GetParameter(EziServo_Handle_t *handle, uint8_t param_num, int32_t *val)
{
    if (!val) return EZI_ERR_PARAM;
    EziRxFrame_t rx;
    uint8_t payload[1] = { param_num };
    int res = EziServo_SendAndReceive(handle, CMD_GET_PARAMETER, payload, 1, 4, &rx);
    if (res == EZI_OK || res == EZI_ERR_STATUS) {
        memcpy(val, rx.data, 4);
    }
    return res;
}

/**
  * @brief Read Digital Input Status on CN1 (CMD 0x24 - FAS_GetIOInput)
  *        Returns 32-bit mask of sensors: Limit+, Limit-, Origin, E-Stop...
  */
int EziServo_GetIOInput(EziServo_Handle_t *handle, uint32_t *u_in)
{
    if (!u_in) return EZI_ERR_PARAM;
    EziRxFrame_t rx;
    int res = EziServo_SendAndReceive(handle, CMD_GET_IO_INPUT, NULL, 0, 4, &rx);
    if (res == EZI_OK || res == EZI_ERR_STATUS) {
        memcpy(u_in, rx.data, 4);
    }
    return res;
}

/**
  * @brief Read Digital Output Status on CN1 (CMD 0x25 - FAS_GetIOOutput)
  */
int EziServo_GetIOOutput(EziServo_Handle_t *handle, uint32_t *u_out)
{
    if (!u_out) return EZI_ERR_PARAM;
    EziRxFrame_t rx;
    int res = EziServo_SendAndReceive(handle, CMD_GET_IO_OUTPUT, NULL, 0, 4, &rx);
    if (res == EZI_OK || res == EZI_ERR_STATUS) {
        memcpy(u_out, rx.data, 4);
    }
    return res;
}

/**
  * @brief Read All Safe Diagnostics & Parameters while motor is not connected
  */
int EziServo_UpdateDiagnostics(EziServo_Handle_t *handle, EziDriverDiag_t *diag)
{
    if (!handle || !diag) return EZI_ERR_PARAM;

    /* 1. Read Active Alarm Code (CMD 0x2C) */
    uint8_t alm = 0;
    if (EziServo_GetAlarmType(handle, &alm) == EZI_OK || alm != 0) {
        diag->alarm_code = alm;
    }
    HAL_Delay(2);

    /* 2. Read Actual Position (CMD 0x53) */
    diag->last_res = EziServo_GetActualPos(handle, &diag->act_pos);
    HAL_Delay(2);

    /* 3. Read Command Position (CMD 0x51) */
    EziServo_GetCommandPos(handle, &diag->cmd_pos);
    HAL_Delay(2);

    /* 4. Read Position Error (CMD 0x54) & Actual Velocity (CMD 0x55) */
    EziServo_GetPosError(handle, &diag->pos_err);
    HAL_Delay(2);
    EziServo_GetActualVel(handle, &diag->act_vel);
    HAL_Delay(2);

    /* 5. Read CN1 Digital Inputs (Sensors) & Outputs */
    EziServo_GetIOInput(handle, &diag->io_inputs);
    HAL_Delay(2);
    EziServo_GetIOOutput(handle, &diag->io_outputs);
    HAL_Delay(2);

    /* 6. Read Basic Configuration Parameters from Driver ROM/RAM (CMD 0x13) */
    EziServo_GetParameter(handle, 0, &diag->param_resolution);  // Param 0: Resolution
    HAL_Delay(2);
    EziServo_GetParameter(handle, 1, &diag->param_max_speed);   // Param 1: Max Speed
    HAL_Delay(2);
    EziServo_GetParameter(handle, 2, &diag->param_start_speed); // Param 2: Start Speed
    HAL_Delay(2);
    EziServo_GetParameter(handle, 3, &diag->param_acc_time);    // Param 3: Acc Time
    HAL_Delay(2);
    EziServo_GetParameter(handle, 4, &diag->param_dec_time);    // Param 4: Dec Time

    return EZI_OK;
}

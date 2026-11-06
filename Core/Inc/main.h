/**
  ******************************************************************************
  * @file    main.h
  * @brief   Header for main.c - Ezi-SERVOII Plus-E Motion Controller
  *          E7502 IO Extension Board / STM32F767ZI
  ******************************************************************************
  */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f7xx_hal.h"
#include "IO_Board.h"
#include "ezi_servo_driver.h"
#include "lwip/netif.h"
#include "lwip/init.h"
#include "lwip/lwip_timers.h"
#include "netif/etharp.h"
#include "ethernetif.h"

/* Global Network Interface Handle */
extern struct netif gnetif;

/* Operating Mode Definitions */
#define OP_MODE_MANUAL          0
#define OP_MODE_AUTO            1

#define MODE_SWITCH_NONE        0
#define MODE_SWITCH_TO_MANUAL   1
#define MODE_SWITCH_TO_AUTO     2

/* Multi-Axis Motion Definitions (3-Axis Glue Dispensing System) */
#define NUM_AXES                3
#define AXIS_X                  0   /* Trục X: Đưa hàng từ ngoài vào máy (Infeed) */
#define AXIS_Y                  1   /* Trục Y: Lên/xuống tra keo (Dispensing Head) */
#define AXIS_Z                  2   /* Trục Z: Đưa hàng ra bên ngoài (Outfeed) */
#define AXIS_ALL                3   /* Cả 3 trục đồng thời */

/* Global Handles and Status Arrays for 3 Axes */
extern EziServo_Handle_t g_ezi_servo[NUM_AXES];
extern volatile EziServo_AllStatus_t g_driver_st[NUM_AXES];
extern volatile int32_t  g_driver_act_pos[NUM_AXES];
extern volatile int32_t  g_driver_cmd_pos[NUM_AXES];
extern volatile uint32_t g_driver_axis_status[NUM_AXES];
extern volatile uint32_t g_driver_alarm_code[NUM_AXES];
extern volatile uint32_t g_driver_io_inputs[NUM_AXES];
extern volatile uint32_t g_driver_io_outputs[NUM_AXES];
extern volatile uint32_t g_jog_state[NUM_AXES];
extern volatile uint32_t g_jog_speed_pps[NUM_AXES];

/* Automated 3-Axis Glue Dispensing Machine Coordinates & Timing */
extern volatile int32_t  g_pos_x_load;      /* Vị trí trục X chờ nhận hàng ngoài máy (pulse) */
extern volatile int32_t  g_pos_x_work;      /* Vị trí trục X đưa hàng vào trạm tra keo (pulse) */
extern volatile int32_t  g_pos_y_safe;      /* Vị trí trục Y nâng lên an toàn (pulse) */
extern volatile int32_t  g_pos_y_disp;      /* Vị trí trục Y hạ xuống tra keo (pulse) */
extern volatile int32_t  g_pos_z_wait;      /* Vị trí trục Z chờ trong máy (pulse) */
extern volatile int32_t  g_pos_z_out;       /* Vị trí trục Z đẩy hàng ra ngoài máy (pulse) */

extern volatile uint32_t g_speed_x_pps;     /* Tốc độ chạy trục X (pps) */
extern volatile uint32_t g_speed_y_pps;     /* Tốc độ chạy trục Y (pps) */
extern volatile uint32_t g_speed_z_pps;     /* Tốc độ chạy trục Z (pps) */
extern volatile uint32_t g_dispense_time_ms;/* Thời gian dừng tra keo (ms) */

extern volatile uint32_t g_auto_cycle_trigger;
extern volatile uint32_t g_auto_cycle_count;
extern volatile uint32_t g_auto_step;

extern volatile uint32_t g_operation_mode;
extern volatile uint32_t g_mode_switch_pending;

void Request_OperationModeSwitch(uint32_t target_mode);

void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

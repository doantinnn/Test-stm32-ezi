/**
  ******************************************************************************
  * @file    eziservo_config.h
  * @brief   System configuration parameters for Ezi-SERVOII Plus-E driver
  ******************************************************************************
  */

#ifndef INC_EZISERVO_CONFIG_H_
#define INC_EZISERVO_CONFIG_H_

/* Network Settings for STM32F767 Board */
#define STM32_IP_ADDR           "192.168.0.100"
#define STM32_NETMASK           "255.255.255.0"
#define STM32_GATEWAY           "192.168.0.1"

/* Target Ezi-SERVOII Plus-E 3-Axis Driver Settings */
#define DRIVER_IP_ADDR          "192.168.0.2"   // Default / Axis-X IP
#define DRIVER_X_IP_ADDR        "192.168.0.2"   // Driver X (Infeed): SW1/SW2 = 2
#define DRIVER_Y_IP_ADDR        "192.168.0.3"   // Driver Y (Dispense): SW1/SW2 = 3
#define DRIVER_Z_IP_ADDR        "192.168.0.4"   // Driver Z (Outfeed): SW1/SW2 = 4
#define DRIVER_UDP_PORT         3002            // UDP Command Port for all Drivers
#define DRIVER_TCP_PORT         2002            // TCP Command Port (Optional)

/* Communication Timeouts */
#define EZI_SERVO_RX_TIMEOUT_MS 80              // Responsive Socket Receive Timeout in ms (Driver typically responds in <1ms)
#define EZI_SERVO_MAX_RETRIES   2               // Maximum retry count on comm fail

/* PC UI Communication Server Settings */
#define UI_SERVER_UDP_PORT      5000            // UDP Port for PC UI Application to STM32

#endif /* INC_EZISERVO_CONFIG_H_ */

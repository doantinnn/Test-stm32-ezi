/**
  ******************************************************************************
  * @file    IO_Board.h
  * @brief   Hardware definition and board support package for
  *          E7502 IO Extension Board (TOP ENGINEERING) - STM32F767ZI
  ******************************************************************************
  */

#ifndef __IO_BOARD_H__
#define __IO_BOARD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f7xx_hal.h"
#include "UserDefine.h"
#include <stdbool.h>

/* ==============================================================================
 * NETWORK & SYSTEM CONFIGURATION
 * ============================================================================== */
typedef struct {
    u8  macAdrs[6];
    u8  ipAdrs[4];
    u8  nmAdrs[4];
    u8  gwAdrs[4];
    u16 tcpPort;
    u16 udpPort;
} sysConfig_t;

extern sysConfig_t sysConfig;
extern uint8_t networkRxFlag;

/* ==============================================================================
 * DIGITAL OUTPUTS J100, J101, J102, J103 (Connector J100..J103 via U23 MC1413D)
 * Schematics:
 *   Page 9 (OUTPUT):
 *     U23 (MC1413D Darlington Driver):
 *       IN1: GPIO_OUTPUT0 -> OUT1 -> R64 (100R) -> OUT_0 -> J100 (5267-02 Pin 2, Pin 1=POUT1 24V)
 *       IN2: GPIO_OUTPUT1 -> OUT2 -> R66 (100R) -> OUT_1 -> J101 (5267-02 Pin 2, Pin 1=POUT1 24V)
 *       IN3: GPIO_OUTPUT2 -> OUT3 -> R67 (100R) -> OUT_2 -> J102 (5267-02 Pin 2, Pin 1=POUT1 24V)
 *       IN4: GPIO_OUTPUT3 -> OUT4 -> R68 (100R) -> OUT_3 -> J103 (5267-02 Pin 2, Pin 1=POUT1 24V)
 *   Page 4 & 5 (MCU):
 *     GPIO_OUTPUT0: PG11 (Pin 126 on Sheet 5) -> U23 IN1 -> J100
 *     GPIO_OUTPUT1: PG10 (Pin 125 on Sheet 5) -> U23 IN2 -> J101
 *     GPIO_OUTPUT2: PG9  (Pin 124 on Sheet 5) -> U23 IN3 -> J102
 *     GPIO_OUTPUT3: PD7  (Pin 123 on Sheet 4) -> U23 IN4 -> J103
 *
 * MAPPING THAY THẾ CHO ĐÈN BÁO & CÒI (Do board không có đèn tháp):
 *   - J100 (GPIO_OUTPUT0 / PG11): ĐÈN BÁO HỆ THỐNG / LINK READY (Solid ON)
 *   - J101 (GPIO_OUTPUT1 / PG10): ĐÈN BÁO GIAO TIẾP DRIVER UDP (Solid ON khi kết nối OK)
 *   - J102 (GPIO_OUTPUT2 / PG9):  ĐÈN BÁO ALARM DRIVER (Solid ON khi Driver có lỗi/chưa gắn motor)
 *   - J103 (GPIO_OUTPUT3 / PD7):  ĐÈN BÁO HEARTBEAT STM32 (Blink 1Hz)
 *
 * Note: MC1413D là IC đệm Darlington đảo cực. MCU xuất mức HIGH (1) sẽ kéo OUT xuống GND,
 * tạo dòng điện 24V chạy qua tải nối giữa Pin 1 (+24V) và Pin 2 (GND) -> Output BẬT (ON).
 * ============================================================================== */
#define OUT_J100_PIN            GPIO_PIN_9
#define OUT_J100_PORT           GPIOG

#define OUT_J101_PIN            GPIO_PIN_10
#define OUT_J101_PORT           GPIOG

#define OUT_J102_PIN            GPIO_PIN_11
#define OUT_J102_PORT           GPIOG

#define OUT_J103_PIN            GPIO_PIN_7
#define OUT_J103_PORT           GPIOD

/* Backward Compatibility Aliases */
#define LAMP_RED_PIN            OUT_J100_PIN
#define LAMP_RED_PORT           OUT_J100_PORT

#define LAMP_YELLOW_PIN         OUT_J101_PIN
#define LAMP_YELLOW_PORT        OUT_J101_PORT

#define LAMP_GREEN_PIN          OUT_J102_PIN
#define LAMP_GREEN_PORT         OUT_J102_PORT

#define BUZZER_PIN              OUT_J103_PIN
#define BUZZER_PORT             OUT_J103_PORT

/* ==============================================================================
 * DIGITAL INPUTS J10, J11 (Connector J10, J11 via Optocoupler PS2801-1)
 * Schematics:
 *   Page 6 (SENSOR INPUT CONNECTORS):
 *     J10: Pin 1: P24V, Pin 2: INPUT_P_0, Pin 3: GND
 *     J11: Pin 1: P24V, Pin 2: INPUT_P_1, Pin 3: GND
 *   Page 7 (OPTOCOUPLER ISOLATION):
 *     INPUT_P_0 -> U5 (PS2801_1) -> R27 (1k pull-up to 3.3V) -> GPIO_INPUT0
 *     INPUT_P_1 -> U7 (PS2801_1) -> R31 (1k pull-up to 3.3V) -> GPIO_INPUT1
 *   Page 4 (MCU STM32F767ZI U2A):
 *     GPIO_INPUT0 -> PA4 (Pin 40) -> JOG+ (Positive Jog Pushbutton)
 *     GPIO_INPUT0 -> PB5 (Pin 135) -> JOG+ (Positive Jog Pushbutton)
 *     GPIO_INPUT1 -> PB6 (Pin 136) -> JOG- (Negative Jog Pushbutton)
 *
 * NOTE VỀ MỨC LOGIC QUANG:
 *   - Khi bấm nút (chập Pin 1 +24V vào Pin 2 INPUT_P_x):
 *     Dòng 24V đi qua LED quang -> Transistor quang dẫn kéo PB5/PB6 về GND (LOW / 0).
 *     => Nút nhấn BẤM (ACTIVE) = GPIO_PIN_RESET (0).
 *   - Khi nhả nút: Pull-up kéo PB5/PB6 lên 3.3V (HIGH / 1).
 *     => Nút nhấn NHẢ (INACTIVE) = GPIO_PIN_SET (1).
 * ============================================================================== */
#define INPUT_J10_PIN           GPIO_PIN_5
#define INPUT_J10_PORT          GPIOB

#define INPUT_J11_PIN           GPIO_PIN_6
#define INPUT_J11_PORT          GPIOB

/* Aliases for Jog Pushbuttons */
#define BTN_JOG_POS_PIN         INPUT_J10_PIN
#define BTN_JOG_POS_PORT        INPUT_J10_PORT

#define BTN_JOG_NEG_PIN         INPUT_J11_PIN
#define BTN_JOG_NEG_PORT        INPUT_J11_PORT

/* ==============================================================================
 * BOARD ID CONTROL DIP SWITCH (SW1 - DHNF-02F)
 * Schematics Page 3 & 4:
 *   SW1.1 -> Board_ID_Control_0 -> PD14 (10k Pull-Up R421)
 *   SW1.2 -> Board_ID_Control_1 -> PD15 (10k Pull-Up R437)
 * ============================================================================== */
#define BOARD_ID_0_PIN          GPIO_PIN_14
#define BOARD_ID_0_PORT         GPIOD

#define BOARD_ID_1_PIN          GPIO_PIN_15
#define BOARD_ID_1_PORT         GPIOD

/* ==============================================================================
 * RS-232 DEBUG UART (U3 / MAX3232 - Connector J2)
 * Schematics Page 4:
 *   UART1_TX -> PA9  (AF7)
 *   UART1_RX -> PA10 (AF7)
 * ============================================================================== */
#define DEBUG_UART              USART1
#define DEBUG_UART_CLK_ENABLE() __HAL_RCC_USART1_CLK_ENABLE()
#define DEBUG_UART_TX_PIN       GPIO_PIN_9
#define DEBUG_UART_TX_PORT      GPIOA
#define DEBUG_UART_RX_PIN       GPIO_PIN_10
#define DEBUG_UART_RX_PORT      GPIOA

/* ==============================================================================
 * ETHERNET MII HARDWARE MAPPING (U113 / DP83848C)
 * Schematics Page 3 & 4:
 *   MII_CRS:     PA0  (AF11)
 *   MII_RX_CLK:  PA1  (AF11)
 *   MII_MDIO:    PA2  (AF11)
 *   MII_COL:     PA3  (AF11)
 *   MII_RX_DV:   PA7  (AF11)
 *   MII_RX_ER:   PB10 (AF11)
 *   MII_MDC:     PC1  (AF11)
 *   MII_TXD2:    PC2  (AF11)
 *   MII_TX_CLK:  PC3  (AF11)
 *   MII_RXD0:    PC4  (AF11)
 *   MII_RXD1:    PC5  (AF11)
 *   MII_TXD3:    PE2  (AF11)
 *   MII_TX_EN:   PB11 (AF11)
 *   MII_TXD0:    PB12 (AF11)
 *   MII_TXD1:    PB13 (AF11)
 *   MII_RXD2:    PB0  (AF11)
 *   MII_RXD3:    PB1  (AF11)
 *   ETH_INT:     PB2  (Input / EXTI2)
 * ============================================================================== */
#define DP83848_PHY_ADDRESS     0x01U

/* ==============================================================================
 * FUNCTION PROTOTYPES
 * ============================================================================== */
void IOBoard_Init(void);
void IOBoard_UART_Init(uint32_t baudrate);
void IOBoard_UART_TransmitBytes(const uint8_t *buf, uint16_t len);
bool IOBoard_UART_GetChar(uint8_t *ch);
UART_HandleTypeDef *IOBoard_GetUartHandle(void);

uint8_t IOBoard_GetBoardID(void);

/* J100..J103 Output Direct Controls */
void IOBoard_SetOutput_J100(bool on);
void IOBoard_SetOutput_J101(bool on);
void IOBoard_SetOutput_J102(bool on);
void IOBoard_SetOutput_J103(bool on);
void IOBoard_ToggleOutput_J100(void);
void IOBoard_ToggleOutput_J101(void);
void IOBoard_ToggleOutput_J102(void);
void IOBoard_ToggleOutput_J103(void);

void IOBoard_ResetAllOutputs(void);

/* Pulse Output for Activity Indicator (Pulse Stretcher) */
void IOBoard_PulseOutput_J101(uint32_t pulse_ms);
void IOBoard_ProcessJ101Pulse(void);

/* Status Indicator Stubs */
void IOBoard_SetLampRed(bool on);
void IOBoard_SetLampYellow(bool on);
void IOBoard_SetLampGreen(bool on);
void IOBoard_SetBuzzer(bool on);

void IOBoard_SetOutput(uint8_t channel, bool on);
bool IOBoard_GetOutput(uint8_t channel);
bool IOBoard_GetInput(uint8_t channel);

/* Digital Input Functions (J10 & J11 Pushbuttons) */
void IOBoard_UpdateInputs(void);
void IOBoard_UpdateInputsWithLevel(uint32_t active_high);
uint32_t IOBoard_GetJogPosRaw(void);
uint32_t IOBoard_GetJogNegRaw(void);
uint32_t IOBoard_GetJogPosPressed(void);
uint32_t IOBoard_GetJogNegPressed(void);

#ifdef __cplusplus
}
#endif

#endif /* __IO_BOARD_H__ */

/**
  ******************************************************************************
  * @file    IO_Board.c
  * @brief   Board Support Package implementation for
  *          E7502 IO Extension Board (TOP ENGINEERING) - STM32F767ZI
  ******************************************************************************
  */

#include "IO_Board.h"
#include <stdio.h>

/* Global Network Configuration */
sysConfig_t sysConfig = {
    .macAdrs = {0x00, 0x80, 0xE1, 0x02, 0x00, 0x64}, // Default MAC: 00:80:E1:02:00:64
    .ipAdrs  = {192, 168, 0, 100},                   // Default STM32 IP: 192.168.0.100
    .nmAdrs  = {255, 255, 255, 0},                   // Netmask: 255.255.255.0
    .gwAdrs  = {192, 168, 0, 1},                     // Gateway: 192.168.0.1
    .tcpPort = 5000,
    .udpPort = 3000
};

uint8_t networkRxFlag = 0;
static UART_HandleTypeDef s_UartHandle;
static volatile uint32_t s_j101_pulse_end_tick = 0;

/**
  * @brief Initialize Board Peripherals (Tower Lamp, Board ID DIP switch, etc.)
  */
void IOBoard_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable all GPIO Port Clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();

    /* Configure J100, J101, J102 Output Pins (PG9, PG10, PG11 via U23 MC1413D) */
    HAL_GPIO_WritePin(GPIOG, OUT_J100_PIN | OUT_J101_PIN | OUT_J102_PIN, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin   = OUT_J100_PIN | OUT_J101_PIN | OUT_J102_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    /* Configure J103 Output Pin (PD7 via U23 MC1413D) */
    HAL_GPIO_WritePin(GPIOD, OUT_J103_PIN, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin   = OUT_J103_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* Configure Board ID DIP Switch Pins (PD14, PD15) as Inputs */
    GPIO_InitStruct.Pin   = BOARD_ID_0_PIN | BOARD_ID_1_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* Configure J10 (PB5 / JOG+) and J11 (PB6 / JOG-) Optocoupler Inputs with Pull-Up */
    GPIO_InitStruct.Pin   = BTN_JOG_POS_PIN | BTN_JOG_NEG_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Read DIP switch and adjust last octet of IP address */
    uint8_t board_id = IOBoard_GetBoardID();
    sysConfig.ipAdrs[3] = 100 + board_id; // e.g. ID 0 -> .100, ID 1 -> .101 ...
    sysConfig.macAdrs[5] = 0x64 + board_id;
}

/**
  * @brief Read Board ID from SW1 DIP Switch (0 to 3)
  */
uint8_t IOBoard_GetBoardID(void)
{
    uint8_t id = 0;
    // DIP switch closes to GND when ON (active LOW)
    if (HAL_GPIO_ReadPin(BOARD_ID_0_PORT, BOARD_ID_0_PIN) == GPIO_PIN_RESET) {
        id |= 0x01;
    }
    if (HAL_GPIO_ReadPin(BOARD_ID_1_PORT, BOARD_ID_1_PIN) == GPIO_PIN_RESET) {
        id |= 0x02;
    }
    return id;
}

/**
  * @brief Digital Output J100 Control (GPIO_OUTPUT0 / PG9 via U23 MC1413D)
  *        Replaces Red Lamp (Error Indicator)
  */
void IOBoard_SetOutput_J100(bool on)
{
    HAL_GPIO_WritePin(OUT_J100_PORT, OUT_J100_PIN, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**
  * @brief Toggle Digital Output J100
  */
void IOBoard_ToggleOutput_J100(void)
{
    HAL_GPIO_TogglePin(OUT_J100_PORT, OUT_J100_PIN);
}

/**
  * @brief Digital Output J101 Control (GPIO_OUTPUT1 / PG10 via U23 MC1413D)
  *        Replaces Yellow Lamp (Busy / Motion Moving Indicator)
  */
void IOBoard_SetOutput_J101(bool on)
{
    s_j101_pulse_end_tick = 0;
    HAL_GPIO_WritePin(OUT_J101_PORT, OUT_J101_PIN, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**
  * @brief Toggle Digital Output J101 (for Driver UDP Activity Indicator)
  */
void IOBoard_ToggleOutput_J101(void)
{
    HAL_GPIO_TogglePin(OUT_J101_PORT, OUT_J101_PIN);
}

/**
  * @brief Pulse Digital Output J101 (Pulse Stretcher for Traffic/Activity Indicator)
  *        Turns J101 ON and schedules it to turn OFF after pulse_ms.
  */
void IOBoard_PulseOutput_J101(uint32_t pulse_ms)
{
    HAL_GPIO_WritePin(OUT_J101_PORT, OUT_J101_PIN, GPIO_PIN_SET);
    s_j101_pulse_end_tick = HAL_GetTick() + pulse_ms;
    if (s_j101_pulse_end_tick == 0) {
        s_j101_pulse_end_tick = 1;
    }
}

/**
  * @brief Process J101 Pulse Expiration (Must be called in main loop)
  *        Turns J101 OFF as soon as pulse_ms has elapsed.
  */
void IOBoard_ProcessJ101Pulse(void)
{
    if (s_j101_pulse_end_tick != 0) {
        if ((int32_t)(HAL_GetTick() - s_j101_pulse_end_tick) >= 0) {
            HAL_GPIO_WritePin(OUT_J101_PORT, OUT_J101_PIN, GPIO_PIN_RESET);
            s_j101_pulse_end_tick = 0;
        }
    }
}

/**
  * @brief Digital Output J102 Control (GPIO_OUTPUT2 / PG11 via U23 MC1413D)
  *        Replaces Green Lamp (Ready / Done / Heartbeat Indicator)
  */
void IOBoard_SetOutput_J102(bool on)
{
    HAL_GPIO_WritePin(OUT_J102_PORT, OUT_J102_PIN, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**
  * @brief Toggle Digital Output J102 (for Heartbeat blink)
  */
void IOBoard_ToggleOutput_J102(void)
{
    HAL_GPIO_TogglePin(OUT_J102_PORT, OUT_J102_PIN);
}

/**
  * @brief Digital Output J103 Control (GPIO_OUTPUT3 / PD7 via U23 MC1413D)
  *        Replaces Buzzer (Alarm / Error Output)
  */
void IOBoard_SetOutput_J103(bool on)
{
    HAL_GPIO_WritePin(OUT_J103_PORT, OUT_J103_PIN, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**
  * @brief Toggle Digital Output J103 (for System Heartbeat blink)
  */
void IOBoard_ToggleOutput_J103(void)
{
    HAL_GPIO_TogglePin(OUT_J103_PORT, OUT_J103_PIN);
}

/**
  * @brief Reset All Actuator Outputs (J100, J101, J102, J103) to Safe Inactive State (OFF)
  */
void IOBoard_ResetAllOutputs(void)
{
    s_j101_pulse_end_tick = 0;
    HAL_GPIO_WritePin(OUT_J100_PORT, OUT_J100_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(OUT_J101_PORT, OUT_J101_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(OUT_J102_PORT, OUT_J102_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(OUT_J103_PORT, OUT_J103_PIN, GPIO_PIN_RESET);
}

/**
  * @brief Status Lamp & Buzzer Stubs (Disabled to prevent unintended ACT firing)
  */
void IOBoard_SetLampRed(bool on)
{
    /* Disabled: ACT test mode OFF */
    (void)on;
}

void IOBoard_SetLampYellow(bool on)
{
    /* Disabled: ACT test mode OFF */
    (void)on;
}

void IOBoard_SetLampGreen(bool on)
{
    /* Disabled: ACT test mode OFF */
    (void)on;
}

void IOBoard_SetBuzzer(bool on)
{
    /* Disabled: ACT test mode OFF */
    (void)on;
}

/**
  * @brief Generic Output Control by channel index (0..3 -> J100..J103)
  */
void IOBoard_SetOutput(uint8_t channel, bool on)
{
    switch (channel) {
        case 0: IOBoard_SetOutput_J100(on); break;
        case 1: IOBoard_SetOutput_J101(on); break;
        case 2: IOBoard_SetOutput_J102(on); break;
        case 3: IOBoard_SetOutput_J103(on); break;
        default: break;
    }
}

/**
  * @brief Generic Output Read by channel index (0..3 -> J100..J103)
  */
bool IOBoard_GetOutput(uint8_t channel)
{
    switch (channel) {
        case 0: return (HAL_GPIO_ReadPin(OUT_J100_PORT, OUT_J100_PIN) == GPIO_PIN_SET);
        case 1: return (HAL_GPIO_ReadPin(OUT_J101_PORT, OUT_J101_PIN) == GPIO_PIN_SET);
        case 2: return (HAL_GPIO_ReadPin(OUT_J102_PORT, OUT_J102_PIN) == GPIO_PIN_SET);
        case 3: return (HAL_GPIO_ReadPin(OUT_J103_PORT, OUT_J103_PIN) == GPIO_PIN_SET);
        default: return false;
    }
}

/* ==============================================================================
 * DIGITAL INPUTS J10 & J11 IMPLEMENTATION (Optocoupler with 20ms Software Debounce)
 * ============================================================================== */
static bool s_btn_jog_pos_debounced = false;
static bool s_btn_jog_neg_debounced = false;

#define DEBOUNCE_DELAY_MS 20

/**
  * @brief Read raw hardware pin state for J10 (PA4)
  * @return 1 if pin is HIGH (3.3V), 0 if pin is LOW (GND)
  */
uint32_t IOBoard_GetJogPosRaw(void)
{
    return (HAL_GPIO_ReadPin(BTN_JOG_POS_PORT, BTN_JOG_POS_PIN) == GPIO_PIN_SET) ? 1U : 0U;
}

/**
  * @brief Read raw hardware pin state for J11 (PA5)
  * @return 1 if pin is HIGH (3.3V), 0 if pin is LOW (GND)
  */
uint32_t IOBoard_GetJogNegRaw(void)
{
    return (HAL_GPIO_ReadPin(BTN_JOG_NEG_PORT, BTN_JOG_NEG_PIN) == GPIO_PIN_SET) ? 1U : 0U;
}

/**
  * @brief Periodic update of digital input debouncing (called in main loop)
  * @param active_high: 1 = Active HIGH (Pin HIGH means Pressed)
  *                     0 = Active LOW  (Pin LOW means Pressed)
  */
void IOBoard_UpdateInputsWithLevel(uint32_t active_high)
{
    static bool s_last_raw_pos = false;
    static bool s_last_raw_neg = false;
    static uint32_t s_pos_change_tick = 0;
    static uint32_t s_neg_change_tick = 0;

    uint32_t now = HAL_GetTick();

    GPIO_PinState pressed_state = active_high ? GPIO_PIN_SET : GPIO_PIN_RESET;

    bool raw_pos_pressed = (HAL_GPIO_ReadPin(BTN_JOG_POS_PORT, BTN_JOG_POS_PIN) == pressed_state);
    bool raw_neg_pressed = (HAL_GPIO_ReadPin(BTN_JOG_NEG_PORT, BTN_JOG_NEG_PIN) == pressed_state);

    /* Debounce JOG+ (J10 / PA4) */
    if (raw_pos_pressed != s_last_raw_pos) {
        s_last_raw_pos = raw_pos_pressed;
        s_pos_change_tick = now;
    }
    if ((now - s_pos_change_tick) >= DEBOUNCE_DELAY_MS) {
        s_btn_jog_pos_debounced = raw_pos_pressed;
    }

    /* Debounce JOG- (J11 / PA5) */
    if (raw_neg_pressed != s_last_raw_neg) {
        s_last_raw_neg = raw_neg_pressed;
        s_neg_change_tick = now;
    }
    if ((now - s_neg_change_tick) >= DEBOUNCE_DELAY_MS) {
        s_btn_jog_neg_debounced = raw_neg_pressed;
    }
}

/**
  * @brief Default periodic update (Active HIGH)
  */
void IOBoard_UpdateInputs(void)
{
    IOBoard_UpdateInputsWithLevel(0);
}

/**
  * @brief Check if JOG+ (J10) button is pressed (debounced)
  * @return 1 if pressed, 0 if released
  */
uint32_t IOBoard_GetJogPosPressed(void)
{
    return s_btn_jog_pos_debounced ? 1U : 0U;
}

/**
  * @brief Check if JOG- (J11) button is pressed (debounced)
  * @return 1 if pressed, 0 if released
  */
uint32_t IOBoard_GetJogNegPressed(void)
{
    return s_btn_jog_neg_debounced ? 1U : 0U;
}

/**
  * @brief Generic Input reading by channel
  *        Channel 0 -> J10 (JOG+)
  *        Channel 1 -> J11 (JOG-)
  */
bool IOBoard_GetInput(uint8_t channel)
{
    switch (channel) {
        case 0: return (IOBoard_GetJogPosPressed() != 0);
        case 1: return (IOBoard_GetJogNegPressed() != 0);
        default: return false;
    }
}

/**
  * @brief Initialize RS-232 Debug UART (USART1 on PA9/PA10)
  */
void IOBoard_UART_Init(uint32_t baudrate)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable Clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    DEBUG_UART_CLK_ENABLE();

    /* Configure PA9 (TX) and PA10 (RX) in Alternate Function AF7 */
    GPIO_InitStruct.Pin       = DEBUG_UART_TX_PIN | DEBUG_UART_RX_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* Configure USART1 */
    s_UartHandle.Instance          = DEBUG_UART;
    s_UartHandle.Init.BaudRate     = baudrate;
    s_UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
    s_UartHandle.Init.StopBits     = UART_STOPBITS_1;
    s_UartHandle.Init.Parity       = UART_PARITY_NONE;
    s_UartHandle.Init.Mode         = UART_MODE_TX_RX;
    s_UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    s_UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;

    HAL_UART_Init(&s_UartHandle);
}

/**
  * @brief Transmit byte buffer over RS-232 UART
  */
void IOBoard_UART_TransmitBytes(const uint8_t *buf, uint16_t len)
{
    if (buf != NULL && len > 0) {
        HAL_UART_Transmit(&s_UartHandle, (uint8_t *)buf, len, 1000);
    }
}

/**
  * @brief Non-blocking receive single byte from RS-232 UART
  * @return true if byte received, false if no data
  */
bool IOBoard_UART_GetChar(uint8_t *ch)
{
    if (__HAL_UART_GET_FLAG(&s_UartHandle, UART_FLAG_RXNE)) {
        *ch = (uint8_t)(s_UartHandle.Instance->RDR & 0xFF);
        return true;
    }
    return false;
}

/**
  * @brief Get UART handle pointer
  */
UART_HandleTypeDef *IOBoard_GetUartHandle(void)
{
    return &s_UartHandle;
}

/**
  * @brief Retarget printf to RS-232 UART
  */
#if defined(__ICCARM__)
size_t __write(int handle, const unsigned char *buf, size_t bufSize)
{
    HAL_UART_Transmit(&s_UartHandle, (uint8_t *)buf, bufSize, 0xFFFF);
    return bufSize;
}
#elif defined(__GNUC__)
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&s_UartHandle, (uint8_t *)&ch, 1, 0xFFFF);
    return ch;
}
int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&s_UartHandle, (uint8_t *)ptr, len, 0xFFFF);
    return len;
}
#else
int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&s_UartHandle, (uint8_t *)&ch, 1, 0xFFFF);
    return ch;
}
#endif

/**
  ******************************************************************************
  * @file      startup_stm32f767xx.s
  * @author    MCD Application Team
  * @brief     STM32F767xx Devices vector table for GCC based toolchains.
  ******************************************************************************
  */

  .syntax unified
  .cpu cortex-m7
  .fpu fpv5-d16
  .thumb

.global  g_pfnVectors
.global  Default_Handler

/* start address for the initialization values of the .data section.
defined in linker script */
.word  _sidata
/* start address for the .data section. defined in linker script */
.word  _sdata
/* end address for the .data section. defined in linker script */
.word  _edata
/* start address for the .bss section. defined in linker script */
.word  _sbss
/* end address for the .bss section. defined in linker script */
.word  _ebss

.equ  BootRAM, 0xF1E0F85F
/**
 * @brief  This is the code that gets called when the processor first
 *          starts execution following a reset event. Only the absolutely
 *          necessary set is performed, after which the application
 *          supplied main() routine is called.
 * @param  None
 * @retval : None
*/

    .section  .text.Reset_Handler
  .weak  Reset_Handler
  .type  Reset_Handler, %function
Reset_Handler:
  ldr   sp, =_estack      /* set stack pointer */

/* Copy the data segment initializers from flash to SRAM */
  movs  r1, #0
  b  LoopCopyDataInit

CopyDataInit:
  ldr  r3, =_sidata
  ldr  r3, [r3, r1]
  str  r3, [r0, r1]
  adds  r1, r1, #4

LoopCopyDataInit:
  ldr  r0, =_sdata
  ldr  r3, =_edata
  adds  r2, r0, r1
  cmp  r2, r3
  bcc  CopyDataInit
  ldr  r2, =_sbss
  b  LoopFillZerobss
/* Zero fill the bss segment. */
FillZerobss:
  movs  r3, #0
  str  r3, [r2], #4

LoopFillZerobss:
  ldr  r3, = _ebss
  cmp  r2, r3
  bcc  FillZerobss

/* Call the clock system initialization function.*/
  bl  SystemInit
/* Call static constructors */
  bl __libc_init_array
/* Call the application's entry point.*/
  bl  main
  bx  lr
.size  Reset_Handler, .-Reset_Handler

/**
 * @brief  This is the code that gets called when the processor receives an
 *         unexpected interrupt.  This simply enters an infinite loop, preserving
 *         the system state for examination by a debugger.
 * @param  None
 * @retval None
*/
    .section  .text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
  b  Infinite_Loop
  .size  Default_Handler, .-Default_Handler
/******************************************************************************
*
* The minimal vector table for a Cortex-M7. Note that the proper constructs
* must be placed on this to ensure that it ends up at physical address
* 0x0000.0000.
*
*******************************************************************************/
   .section  .isr_vector,"a",%progbits
  .type  g_pfnVectors, %object
  .size  g_pfnVectors, .-g_pfnVectors

g_pfnVectors:
  .word  _estack
  .word  Reset_Handler
  .word  NMI_Handler
  .word  HardFault_Handler
  .word  MemManage_Handler
  .word  BusFault_Handler
  .word  UsageFault_Handler
  .word  0
  .word  0
  .word  0
  .word  0
  .word  SVC_Handler
  .word  DebugMon_Handler
  .word  0
  .word  PendSV_Handler
  .word  SysTick_Handler

  /* External Interrupts */
  .word     WWDG_IRQHandler
  .word     PVD_IRQHandler
  .word     TAMP_STAMP_IRQHandler
  .word     RTC_WKUP_IRQHandler
  .word     FLASH_IRQHandler
  .word     RCC_IRQHandler
  .word     EXTI0_IRQHandler
  .word     EXTI1_IRQHandler
  .word     EXTI2_IRQHandler
  .word     EXTI3_IRQHandler
  .word     EXTI4_IRQHandler
  .word     DMA1_Stream0_IRQHandler
  .word     DMA1_Stream1_IRQHandler
  .word     DMA1_Stream2_IRQHandler
  .word     DMA1_Stream3_IRQHandler
  .word     DMA1_Stream4_IRQHandler
  .word     DMA1_Stream5_IRQHandler
  .word     DMA1_Stream6_IRQHandler
  .word     ADC_IRQHandler
  .word     CAN1_TX_IRQHandler
  .word     CAN1_RX0_IRQHandler
  .word     CAN1_RX1_IRQHandler
  .word     CAN1_SCE_IRQHandler
  .word     EXTI9_5_IRQHandler
  .word     TIM1_BRK_TIM9_IRQHandler
  .word     TIM1_UP_TIM10_IRQHandler
  .word     TIM1_TRG_COM_TIM11_IRQHandler
  .word     TIM1_CC_IRQHandler
  .word     TIM2_IRQHandler
  .word     TIM3_IRQHandler
  .word     TIM4_IRQHandler
  .word     I2C1_EV_IRQHandler
  .word     I2C1_ER_IRQHandler
  .word     I2C2_EV_IRQHandler
  .word     I2C2_ER_IRQHandler
  .word     SPI1_IRQHandler
  .word     SPI2_IRQHandler
  .word     USART1_IRQHandler
  .word     USART2_IRQHandler
  .word     USART3_IRQHandler
  .word     EXTI15_10_IRQHandler
  .word     RTC_Alarm_IRQHandler
  .word     OTG_FS_WKUP_IRQHandler
  .word     TIM8_BRK_TIM12_IRQHandler
  .word     TIM8_UP_TIM13_IRQHandler
  .word     TIM8_TRG_COM_TIM14_IRQHandler
  .word     TIM8_CC_IRQHandler
  .word     DMA1_Stream7_IRQHandler
  .word     FMC_IRQHandler
  .word     SDMMC1_IRQHandler
  .word     TIM5_IRQHandler
  .word     SPI3_IRQHandler
  .word     UART4_IRQHandler
  .word     UART5_IRQHandler
  .word     TIM6_DAC_IRQHandler
  .word     TIM7_IRQHandler
  .word     DMA2_Stream0_IRQHandler
  .word     DMA2_Stream1_IRQHandler
  .word     DMA2_Stream2_IRQHandler
  .word     DMA2_Stream3_IRQHandler
  .word     DMA2_Stream4_IRQHandler
  .word     ETH_IRQHandler
  .word     ETH_WKUP_IRQHandler
  .word     CAN2_TX_IRQHandler
  .word     CAN2_RX0_IRQHandler
  .word     CAN2_RX1_IRQHandler
  .word     CAN2_SCE_IRQHandler
  .word     OTG_FS_IRQHandler
  .word     DMA2_Stream5_IRQHandler
  .word     DMA2_Stream6_IRQHandler
  .word     DMA2_Stream7_IRQHandler
  .word     USART6_IRQHandler
  .word     I2C3_EV_IRQHandler
  .word     I2C3_ER_IRQHandler
  .word     OTG_HS_EP1_OUT_IRQHandler
  .word     OTG_HS_EP1_IN_IRQHandler
  .word     OTG_HS_WKUP_IRQHandler
  .word     OTG_HS_IRQHandler
  .word     DCMI_IRQHandler
  .word     0
  .word     RNG_IRQHandler
  .word     FPU_IRQHandler
  .word     UART7_IRQHandler
  .word     UART8_IRQHandler
  .word     SPI4_IRQHandler
  .word     SPI5_IRQHandler
  .word     SPI6_IRQHandler
  .word     SAI1_IRQHandler
  .word     LTDC_IRQHandler
  .word     LTDC_ER_IRQHandler
  .word     DMA2D_IRQHandler
  .word     SAI2_IRQHandler
  .word     QUADSPI_IRQHandler
  .word     LPTIM1_IRQHandler
  .word     CEC_IRQHandler
  .word     I2C4_EV_IRQHandler
  .word     I2C4_ER_IRQHandler
  .word     SPDIF_RX_IRQHandler
  .word     0
  .word     DFSDM1_FLT0_IRQHandler
  .word     DFSDM1_FLT1_IRQHandler
  .word     DFSDM1_FLT2_IRQHandler
  .word     DFSDM1_FLT3_IRQHandler
  .word     SDMMC2_IRQHandler
  .word     CAN3_TX_IRQHandler
  .word     CAN3_RX0_IRQHandler
  .word     CAN3_RX1_IRQHandler
  .word     CAN3_SCE_IRQHandler
  .word     JPEG_IRQHandler
  .word     MDIOS_IRQHandler

/*******************************************************************************
* Provide weak aliases for each Exception handler to the Default_Handler.
*******************************************************************************/
   .weak      NMI_Handler
   .thumb_set NMI_Handler,Default_Handler

   .weak      HardFault_Handler
   .thumb_set HardFault_Handler,Default_Handler

   .weak      MemManage_Handler
   .thumb_set MemManage_Handler,Default_Handler

   .weak      BusFault_Handler
   .thumb_set BusFault_Handler,Default_Handler

   .weak      UsageFault_Handler
   .thumb_set UsageFault_Handler,Default_Handler

   .weak      SVC_Handler
   .thumb_set SVC_Handler,Default_Handler

   .weak      DebugMon_Handler
   .thumb_set DebugMon_Handler,Default_Handler

   .weak      PendSV_Handler
   .thumb_set PendSV_Handler,Default_Handler

   .weak      SysTick_Handler
   .thumb_set SysTick_Handler,Default_Handler

   .weak      ETH_IRQHandler
   .thumb_set ETH_IRQHandler,Default_Handler

   .weak      ETH_WKUP_IRQHandler
   .thumb_set ETH_WKUP_IRQHandler,Default_Handler

   .weak      USART1_IRQHandler
   .thumb_set USART1_IRQHandler,Default_Handler

   .weak      EXTI0_IRQHandler
   .thumb_set EXTI0_IRQHandler,Default_Handler

   .weak      EXTI1_IRQHandler
   .thumb_set EXTI1_IRQHandler,Default_Handler

   .weak      EXTI2_IRQHandler
   .thumb_set EXTI2_IRQHandler,Default_Handler

   .weak      EXTI3_IRQHandler
   .thumb_set EXTI3_IRQHandler,Default_Handler

   .weak      EXTI4_IRQHandler
   .thumb_set EXTI4_IRQHandler,Default_Handler

   .weak      EXTI9_5_IRQHandler
   .thumb_set EXTI9_5_IRQHandler,Default_Handler

   .weak      EXTI15_10_IRQHandler
   .thumb_set EXTI15_10_IRQHandler,Default_Handler

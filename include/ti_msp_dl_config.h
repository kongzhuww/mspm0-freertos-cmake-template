/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  This file is manually managed. SysConfig is no longer used.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X
#define CONFIG_MSPM0G3507

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)


#define GPIO_HFXT_PORT                                                     GPIOA
#define GPIO_HFXIN_PIN                                             DL_GPIO_PIN_5
#define GPIO_HFXIN_IOMUX                                         (IOMUX_PINCM10)
#define GPIO_HFXOUT_PIN                                            DL_GPIO_PIN_6
#define GPIO_HFXOUT_IOMUX                                        (IOMUX_PINCM11)
#define CPUCLK_FREQ                                                     80000000



/* Defines for TIM_delay_us */
#define TIM_delay_us_INST                                                (TIMA0)
#define TIM_delay_us_INST_IRQHandler                            TIMA0_IRQHandler
#define TIM_delay_us_INST_INT_IRQN                              (TIMA0_INT_IRQn)
#define TIM_delay_us_INST_LOAD_VALUE                                        (0U)



/* Defines for UART_DEBUG */
#define UART_DEBUG_INST                                                    UART0
#define UART_DEBUG_INST_FREQUENCY                                       40000000
#define UART_DEBUG_INST_IRQHandler                              UART0_IRQHandler
#define UART_DEBUG_INST_INT_IRQN                                  UART0_INT_IRQn
#define GPIO_UART_DEBUG_RX_PORT                                            GPIOA
#define GPIO_UART_DEBUG_TX_PORT                                            GPIOA
#define GPIO_UART_DEBUG_RX_PIN                                    DL_GPIO_PIN_11
#define GPIO_UART_DEBUG_TX_PIN                                    DL_GPIO_PIN_10
#define GPIO_UART_DEBUG_IOMUX_RX                                 (IOMUX_PINCM22)
#define GPIO_UART_DEBUG_IOMUX_TX                                 (IOMUX_PINCM21)
#define GPIO_UART_DEBUG_IOMUX_RX_FUNC                  IOMUX_PINCM22_PF_UART0_RX
#define GPIO_UART_DEBUG_IOMUX_TX_FUNC                  IOMUX_PINCM21_PF_UART0_TX
#define UART_DEBUG_BAUD_RATE                                            (115200)
#define UART_DEBUG_IBRD_40_MHZ_115200_BAUD                                  (21)
#define UART_DEBUG_FBRD_40_MHZ_115200_BAUD                                  (45)





/* Port definition for Pin Group KEY */
#define KEY_PORT                                                         (GPIOB)

/* Defines for B21: GPIOB.21 with pinCMx 49 on package pin 20 */
#define KEY_B21_PIN                                             (DL_GPIO_PIN_21)
#define KEY_B21_IOMUX                                            (IOMUX_PINCM49)

#define KEY_A_PORT                                                       (GPIOA)
#define KEY_A26_PIN                                             (DL_GPIO_PIN_26)
#define KEY_A26_IOMUX                                            (IOMUX_PINCM59)
#define KEY_A27_PIN                                             (DL_GPIO_PIN_27)
#define KEY_A27_IOMUX                                            (IOMUX_PINCM60)

#define KEY_B23_PIN                                             (DL_GPIO_PIN_23)
#define KEY_B23_IOMUX                                            (IOMUX_PINCM51)
/* Port definition for Pin Group LED */
#define LED_PORT                                                         (GPIOB)
#define LED_A_PORT                                                       (GPIOA)

/* Defines for B14: GPIOB.14 with pinCMx 31 on package pin 2 */
#define LED_B14_PIN                                             (DL_GPIO_PIN_14)
#define LED_B14_IOMUX                                            (IOMUX_PINCM31)

#define LED_A28_PIN                                             (DL_GPIO_PIN_28)
#define LED_A28_IOMUX                                            (IOMUX_PINCM3)

#define LED_A29_PIN                                             (DL_GPIO_PIN_29)
#define LED_A29_IOMUX                                            (IOMUX_PINCM4)


/* Port definition for Pin Group OLED (7-Pin SPI 模拟) */
/* SCL (D0): GPIOB.9 with pinCMx 26 */
#define OLED_SCL_PORT                                                    (GPIOB)
#define OLED_SCL_PIN                                             (DL_GPIO_PIN_9)
#define OLED_SCL_IOMUX                                           (IOMUX_PINCM26)
/* SDA (D1): GPIOB.8 with pinCMx 25 */
#define OLED_SDA_PORT                                                    (GPIOB)
#define OLED_SDA_PIN                                             (DL_GPIO_PIN_8)
#define OLED_SDA_IOMUX                                           (IOMUX_PINCM25)
/* RES: GPIOB.10 with pinCMx 27 */
#define OLED_RES_PORT                                                    (GPIOB)
#define OLED_RES_PIN                                            (DL_GPIO_PIN_10)
#define OLED_RES_IOMUX                                           (IOMUX_PINCM27)
/* DC: GPIOB.11 with pinCMx 28 */
#define OLED_DC_PORT                                                     (GPIOB)
#define OLED_DC_PIN                                             (DL_GPIO_PIN_11)
#define OLED_DC_IOMUX                                            (IOMUX_PINCM28)
/* CS: GPIOA.13 with pinCMx 35 */
#define OLED_CS_PORT                                                     (GPIOA)
#define OLED_CS_PIN                                             (DL_GPIO_PIN_13)
#define OLED_CS_IOMUX                                            (IOMUX_PINCM35)

/* Defines for UART_ZDT */
#define UART_ZDT_INST                                                    UART2
#define UART_ZDT_INST_FREQUENCY                                       40000000
#define UART_ZDT_INST_IRQHandler                              UART2_IRQHandler
#define UART_ZDT_INST_INT_IRQN                                  UART2_INT_IRQn
#define GPIO_UART_ZDT_RX_PORT                                            GPIOB
#define GPIO_UART_ZDT_TX_PORT                                            GPIOB
#define GPIO_UART_ZDT_RX_PIN                                    DL_GPIO_PIN_16
#define GPIO_UART_ZDT_TX_PIN                                    DL_GPIO_PIN_15
#define GPIO_UART_ZDT_IOMUX_RX                                 (IOMUX_PINCM33)
#define GPIO_UART_ZDT_IOMUX_TX                                 (IOMUX_PINCM32)
#define GPIO_UART_ZDT_IOMUX_RX_FUNC                  IOMUX_PINCM33_PF_UART2_RX
#define GPIO_UART_ZDT_IOMUX_TX_FUNC                  IOMUX_PINCM32_PF_UART2_TX
#define UART_ZDT_BAUD_RATE                                            (115200)
#define UART_ZDT_IBRD_40_MHZ_115200_BAUD                                  (21)
#define UART_ZDT_FBRD_40_MHZ_115200_BAUD                                  (45)

/* Defines for UART_ZIGBEE */
#define UART_ZIGBEE_INST                                           UART1
#define GPIO_UART_ZIGBEE_TX_PORT                                         GPIOA
#define GPIO_UART_ZIGBEE_RX_PORT                                         GPIOA
#define GPIO_UART_ZIGBEE_TX_PIN                                 DL_GPIO_PIN_8
#define GPIO_UART_ZIGBEE_RX_PIN                                 DL_GPIO_PIN_9
#define GPIO_UART_ZIGBEE_IOMUX_TX                              (IOMUX_PINCM19)
#define GPIO_UART_ZIGBEE_IOMUX_RX                              (IOMUX_PINCM20)
#define GPIO_UART_ZIGBEE_IOMUX_TX_FUNC               IOMUX_PINCM19_PF_UART1_TX
#define GPIO_UART_ZIGBEE_IOMUX_RX_FUNC               IOMUX_PINCM20_PF_UART1_RX
#define UART_ZIGBEE_BAUD_RATE                                         (115200)
#define UART_ZIGBEE_IBRD_40_MHZ_115200_BAUD                               (21)
#define UART_ZIGBEE_FBRD_40_MHZ_115200_BAUD                               (45)
#define UART_ZIGBEE_INST_INT_IRQN                                 UART1_INT_IRQn
#define UART_ZIGBEE_INST_IRQHandler                             UART1_IRQHandler

/* Defines for GRAY_ADC: OUT=PA21/ADC1_CH7, AD0=PB19, AD1=PB24, AD2=PB21 */
#define GRAY_ADC_INST                                                    ADC1
#define GRAY_ADC_MEM_IDX                                      DL_ADC12_MEM_IDX_0
#define GRAY_ADC_INPUT_CHAN                              DL_ADC12_INPUT_CHAN_7
#define GRAY_ADC_OUT_PORT                                               GPIOA
#define GRAY_ADC_OUT_PIN                                       DL_GPIO_PIN_21
#define GRAY_ADC_OUT_IOMUX                                      IOMUX_PINCM46
#define GRAY_ADC_AD0_PORT                                               GPIOB
#define GRAY_ADC_AD0_PIN                                      DL_GPIO_PIN_19
#define GRAY_ADC_AD0_IOMUX                                     IOMUX_PINCM45
#define GRAY_ADC_AD1_PORT                                               GPIOB
#define GRAY_ADC_AD1_PIN                                      DL_GPIO_PIN_24
#define GRAY_ADC_AD1_IOMUX                                     IOMUX_PINCM52
#define GRAY_ADC_AD2_PORT                                               GPIOB
#define GRAY_ADC_AD2_PIN                                      DL_GPIO_PIN_21
#define GRAY_ADC_AD2_IOMUX                                     IOMUX_PINCM49
#define GRAY_ADC_ADDR_INVERTED                                           (1)

/* Defines for TB6612: PWMA=PA17/TIMA1_CCP0, PWMB=PA16/TIMA1_CCP1 */
#define TB6612_PWM_INST                                                TIMA1
#define TB6612_PWM_PERIOD                                             (2000U)
#define TB6612_PWMA_PORT                                               GPIOA
#define TB6612_PWMA_PIN                                      DL_GPIO_PIN_17
#define TB6612_PWMA_IOMUX                                     IOMUX_PINCM39
#define TB6612_PWMA_IOMUX_FUNC                        IOMUX_PINCM39_PF_TIMA1_CCP0
#define TB6612_PWMB_PORT                                               GPIOA
#define TB6612_PWMB_PIN                                      DL_GPIO_PIN_16
#define TB6612_PWMB_IOMUX                                     IOMUX_PINCM38
#define TB6612_PWMB_IOMUX_FUNC                        IOMUX_PINCM38_PF_TIMA1_CCP1
#define TB6612_AIN1_PORT                                               GPIOA
#define TB6612_AIN1_PIN                                      DL_GPIO_PIN_18
#define TB6612_AIN1_IOMUX                                     IOMUX_PINCM40
#define TB6612_AIN2_PORT                                               GPIOA
#define TB6612_AIN2_PIN                                      DL_GPIO_PIN_15
#define TB6612_AIN2_IOMUX                                     IOMUX_PINCM37
#define TB6612_BIN1_PORT                                               GPIOB
#define TB6612_BIN1_PIN                                      DL_GPIO_PIN_22
#define TB6612_BIN1_IOMUX                                     IOMUX_PINCM50
#define TB6612_BIN2_PORT                                               GPIOA
#define TB6612_BIN2_PIN                                      DL_GPIO_PIN_22
#define TB6612_BIN2_IOMUX                                     IOMUX_PINCM47


/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_TIM_delay_us_init(void);
void SYSCFG_DL_UART_DEBUG_init(void);
void SYSCFG_DL_UART_ZDT_init(void);
void SYSCFG_DL_UART_ZIGBEE_init(void);
void SYSCFG_DL_GRAY_ADC_init(void);
void SYSCFG_DL_TB6612_PWM_init(void);
void SYSCFG_DL_TB6612_setPWMDuty(uint16_t dutyA, uint16_t dutyB);


bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */

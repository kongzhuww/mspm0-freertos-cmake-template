/*
 * Copyright (c) 2023, Texas Instruments Incorporated
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
 *  ============ ti_msp_dl_config.c =============
 *  Configured MSPM0 DriverLib module definitions
 *
 *  This file is manually managed. SysConfig is no longer used.
 */

#include "ti_msp_dl_config.h"

DL_TimerA_backupConfig gTIM_delay_usBackup;

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform any initialization needed before using any board APIs
 */
SYSCONFIG_WEAK void SYSCFG_DL_init(void)
{
    SYSCFG_DL_initPower();
    SYSCFG_DL_GPIO_init();
    /* Module-Specific Initializations*/
    SYSCFG_DL_SYSCTL_init();
    SYSCFG_DL_TIM_delay_us_init();
    SYSCFG_DL_GRAY_ADC_init();
    SYSCFG_DL_TB6612_PWM_init();
    SYSCFG_DL_UART_DEBUG_init();
    SYSCFG_DL_UART_ZDT_init();
    SYSCFG_DL_UART_ZIGBEE_init();
    /* Ensure backup structures have no valid state */
	gTIM_delay_usBackup.backupRdy 	= false;


}
/*
 * User should take care to save and restore register configuration in application.
 * See Retention Configuration section for more details.
 */
SYSCONFIG_WEAK bool SYSCFG_DL_saveConfiguration(void)
{
    bool retStatus = true;

	retStatus &= DL_TimerA_saveConfiguration(TIM_delay_us_INST, &gTIM_delay_usBackup);

    return retStatus;
}


SYSCONFIG_WEAK bool SYSCFG_DL_restoreConfiguration(void)
{
    bool retStatus = true;

	retStatus &= DL_TimerA_restoreConfiguration(TIM_delay_us_INST, &gTIM_delay_usBackup, false);

    return retStatus;
}

SYSCONFIG_WEAK void SYSCFG_DL_initPower(void)
{
    DL_GPIO_reset(GPIOA);
    DL_GPIO_reset(GPIOB);
    DL_TimerA_reset(TIM_delay_us_INST);
    DL_TimerA_reset(TB6612_PWM_INST);
    DL_ADC12_reset(GRAY_ADC_INST);
    DL_UART_Main_reset(UART_DEBUG_INST);
    DL_UART_Main_reset(UART_ZDT_INST);
    DL_UART_Main_reset(UART_ZIGBEE_INST);

    DL_GPIO_enablePower(GPIOA);
    DL_GPIO_enablePower(GPIOB);
    DL_TimerA_enablePower(TIM_delay_us_INST);
    DL_TimerA_enablePower(TB6612_PWM_INST);
    DL_ADC12_enablePower(GRAY_ADC_INST);
    DL_UART_Main_enablePower(UART_DEBUG_INST);
    DL_UART_Main_enablePower(UART_ZDT_INST);
    DL_UART_Main_enablePower(UART_ZIGBEE_INST);
    delay_cycles(POWER_STARTUP_DELAY);
}

SYSCONFIG_WEAK void SYSCFG_DL_GPIO_init(void)
{

    DL_GPIO_initPeripheralAnalogFunction(GPIO_HFXIN_IOMUX);
    DL_GPIO_initPeripheralAnalogFunction(GPIO_HFXOUT_IOMUX);

    DL_GPIO_initPeripheralOutputFunction(
        GPIO_UART_DEBUG_IOMUX_TX, GPIO_UART_DEBUG_IOMUX_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_UART_DEBUG_IOMUX_RX, GPIO_UART_DEBUG_IOMUX_RX_FUNC);

    DL_GPIO_initPeripheralOutputFunction(
        GPIO_UART_ZDT_IOMUX_TX, GPIO_UART_ZDT_IOMUX_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_UART_ZDT_IOMUX_RX, GPIO_UART_ZDT_IOMUX_RX_FUNC);

    DL_GPIO_initPeripheralOutputFunction(
        GPIO_UART_ZIGBEE_IOMUX_TX, GPIO_UART_ZIGBEE_IOMUX_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_UART_ZIGBEE_IOMUX_RX, GPIO_UART_ZIGBEE_IOMUX_RX_FUNC);

    // ---- 灰度传感器：OUT=PA21/ADC1_CH7，AD0=PB19，AD1=PB24，AD2=PB21 ----
    DL_GPIO_initPeripheralAnalogFunction(GRAY_ADC_OUT_IOMUX);
    DL_GPIO_initDigitalOutputFeatures(GRAY_ADC_AD0_IOMUX,
         DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN,
         DL_GPIO_DRIVE_STRENGTH_LOW, DL_GPIO_HIZ_DISABLE);
    DL_GPIO_initDigitalOutputFeatures(GRAY_ADC_AD1_IOMUX,
         DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN,
         DL_GPIO_DRIVE_STRENGTH_LOW, DL_GPIO_HIZ_DISABLE);
    DL_GPIO_initDigitalOutputFeatures(GRAY_ADC_AD2_IOMUX,
         DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN,
         DL_GPIO_DRIVE_STRENGTH_LOW, DL_GPIO_HIZ_DISABLE);
    DL_GPIO_clearPins(GPIOB, GRAY_ADC_AD0_PIN | GRAY_ADC_AD1_PIN | GRAY_ADC_AD2_PIN);
    DL_GPIO_enableOutput(GPIOB, GRAY_ADC_AD0_PIN | GRAY_ADC_AD1_PIN | GRAY_ADC_AD2_PIN);

    // ---- TB6612：方向脚 + PWM 输出脚 ----
    DL_GPIO_initPeripheralOutputFunction(TB6612_PWMA_IOMUX, TB6612_PWMA_IOMUX_FUNC);
    DL_GPIO_initPeripheralOutputFunction(TB6612_PWMB_IOMUX, TB6612_PWMB_IOMUX_FUNC);
    DL_GPIO_initDigitalOutputFeatures(TB6612_AIN1_IOMUX,
         DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN,
         DL_GPIO_DRIVE_STRENGTH_LOW, DL_GPIO_HIZ_DISABLE);
    DL_GPIO_initDigitalOutputFeatures(TB6612_AIN2_IOMUX,
         DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN,
         DL_GPIO_DRIVE_STRENGTH_LOW, DL_GPIO_HIZ_DISABLE);
    DL_GPIO_initDigitalOutputFeatures(TB6612_BIN1_IOMUX,
         DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN,
         DL_GPIO_DRIVE_STRENGTH_LOW, DL_GPIO_HIZ_DISABLE);
    DL_GPIO_initDigitalOutputFeatures(TB6612_BIN2_IOMUX,
         DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN,
         DL_GPIO_DRIVE_STRENGTH_LOW, DL_GPIO_HIZ_DISABLE);
    DL_GPIO_clearPins(GPIOA, TB6612_AIN1_PIN | TB6612_AIN2_PIN | TB6612_BIN2_PIN);
    DL_GPIO_clearPins(GPIOB, TB6612_BIN1_PIN);
    DL_GPIO_enableOutput(GPIOA, TB6612_PWMA_PIN | TB6612_PWMB_PIN | TB6612_AIN1_PIN | TB6612_AIN2_PIN | TB6612_BIN2_PIN);
    DL_GPIO_enableOutput(GPIOB, TB6612_BIN1_PIN);

    DL_GPIO_initDigitalInputFeatures(KEY_B23_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalInputFeatures(KEY_A26_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalInputFeatures(KEY_A27_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);


    DL_GPIO_initDigitalOutputFeatures(LED_B14_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN,
		 DL_GPIO_DRIVE_STRENGTH_LOW, DL_GPIO_HIZ_DISABLE);
    DL_GPIO_initDigitalOutputFeatures(LED_A28_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN,
		 DL_GPIO_DRIVE_STRENGTH_LOW, DL_GPIO_HIZ_DISABLE);
    DL_GPIO_initDigitalOutputFeatures(LED_A29_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN,
		 DL_GPIO_DRIVE_STRENGTH_LOW, DL_GPIO_HIZ_DISABLE);

    DL_GPIO_clearPins(GPIOB, LED_B14_PIN);
    DL_GPIO_enableOutput(GPIOB, LED_B14_PIN);
    
    DL_GPIO_clearPins(GPIOA, LED_A28_PIN | LED_A29_PIN);
    DL_GPIO_enableOutput(GPIOA, LED_A28_PIN | LED_A29_PIN);
    // ---- OLED SPI 模拟引脚初始化 (推挽输出) ----
    // SCL (D0): PB9
    DL_GPIO_initDigitalOutputFeatures(OLED_SCL_IOMUX,
         DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN,
         DL_GPIO_DRIVE_STRENGTH_LOW, DL_GPIO_HIZ_DISABLE);
    // SDA (D1): PB8
    DL_GPIO_initDigitalOutputFeatures(OLED_SDA_IOMUX,
         DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN,
         DL_GPIO_DRIVE_STRENGTH_LOW, DL_GPIO_HIZ_DISABLE);
    // RES: PB10
    DL_GPIO_initDigitalOutputFeatures(OLED_RES_IOMUX,
         DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN,
         DL_GPIO_DRIVE_STRENGTH_LOW, DL_GPIO_HIZ_DISABLE);
    // DC: PB11
    DL_GPIO_initDigitalOutputFeatures(OLED_DC_IOMUX,
         DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN,
         DL_GPIO_DRIVE_STRENGTH_LOW, DL_GPIO_HIZ_DISABLE);
    // CS: PA13
    DL_GPIO_initDigitalOutputFeatures(OLED_CS_IOMUX,
         DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN,
         DL_GPIO_DRIVE_STRENGTH_LOW, DL_GPIO_HIZ_DISABLE);

    // OLED GPIOB 引脚使能输出（SCL=PB9, SDA=PB8, RES=PB10, DC=PB11）
    DL_GPIO_clearPins(GPIOB, OLED_SCL_PIN | OLED_SDA_PIN | OLED_RES_PIN | OLED_DC_PIN);
    DL_GPIO_enableOutput(GPIOB, OLED_SCL_PIN | OLED_SDA_PIN | OLED_RES_PIN | OLED_DC_PIN);
    // OLED GPIOA 引脚使能输出（CS=PA13，初始拉高表示片选无效）
    DL_GPIO_setPins(GPIOA, OLED_CS_PIN);
    DL_GPIO_enableOutput(GPIOA, OLED_CS_PIN);
}


static const DL_SYSCTL_SYSPLLConfig gSYSPLLConfig = {
    .inputFreq              = DL_SYSCTL_SYSPLL_INPUT_FREQ_32_48_MHZ,
	.rDivClk2x              = 1,
	.rDivClk1               = 0,
	.rDivClk0               = 0,
	.enableCLK2x            = DL_SYSCTL_SYSPLL_CLK2X_DISABLE,
	.enableCLK1             = DL_SYSCTL_SYSPLL_CLK1_DISABLE,
	.enableCLK0             = DL_SYSCTL_SYSPLL_CLK0_ENABLE,
	.sysPLLMCLK             = DL_SYSCTL_SYSPLL_MCLK_CLK0,
	.sysPLLRef              = DL_SYSCTL_SYSPLL_REF_HFCLK,
	.qDiv                   = 3,
	.pDiv                   = DL_SYSCTL_SYSPLL_PDIV_1
};
SYSCONFIG_WEAK void SYSCFG_DL_SYSCTL_init(void)
{

	//Low Power Mode is configured to be SLEEP0
    DL_SYSCTL_setBORThreshold(DL_SYSCTL_BOR_THRESHOLD_LEVEL_0);
    DL_SYSCTL_setFlashWaitState(DL_SYSCTL_FLASH_WAIT_STATE_2);

    
	DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
	/* Set default configuration */
	DL_SYSCTL_disableHFXT();
	DL_SYSCTL_disableSYSPLL();
    DL_SYSCTL_setHFCLKSourceHFXTParams(DL_SYSCTL_HFXT_RANGE_32_48_MHZ,10, true);
    DL_SYSCTL_configSYSPLL((DL_SYSCTL_SYSPLLConfig *) &gSYSPLLConfig);
    DL_SYSCTL_setULPCLKDivider(DL_SYSCTL_ULPCLK_DIV_2);
    DL_SYSCTL_enableMFCLK();
    DL_SYSCTL_setMCLKSource(SYSOSC, HSCLK, DL_SYSCTL_HSCLK_SOURCE_SYSPLL);

}



/*
 * Timer clock configuration to be sourced by BUSCLK /  (40000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   1000000 Hz = 40000000 Hz / (8 * (4 + 1))
 */
static const DL_TimerA_ClockConfig gTIM_delay_usClockConfig = {
    .clockSel    = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_8,
    .prescale    = 4U,
};

/*
 * Timer load value (where the counter starts from) is calculated as (timerPeriod * timerClockFreq) - 1
 * TIM_delay_us_INST_LOAD_VALUE = (1us * 1000000 Hz) - 1
 */
static const DL_TimerA_TimerConfig gTIM_delay_usTimerConfig = {
    .period     = TIM_delay_us_INST_LOAD_VALUE,
    .timerMode  = DL_TIMER_TIMER_MODE_ONE_SHOT,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_TIM_delay_us_init(void) {

    DL_TimerA_setClockConfig(TIM_delay_us_INST,
        (DL_TimerA_ClockConfig *) &gTIM_delay_usClockConfig);

    DL_TimerA_initTimerMode(TIM_delay_us_INST,
        (DL_TimerA_TimerConfig *) &gTIM_delay_usTimerConfig);
    DL_TimerA_enableClock(TIM_delay_us_INST);
}

SYSCONFIG_WEAK void SYSCFG_DL_GRAY_ADC_init(void)
{
    static const DL_ADC12_ClockConfig grayAdcClockConfig = {
        .clockSel = DL_ADC12_CLOCK_SYSOSC,
        .freqRange = DL_ADC12_CLOCK_FREQ_RANGE_24_TO_32,
        .divideRatio = DL_ADC12_CLOCK_DIVIDE_8,
    };

    DL_ADC12_setClockConfig(GRAY_ADC_INST, (DL_ADC12_ClockConfig *)&grayAdcClockConfig);
    DL_ADC12_disableConversions(GRAY_ADC_INST);
    DL_ADC12_clearInterruptStatus(GRAY_ADC_INST,
        DL_ADC12_INTERRUPT_MEM0_RESULT_LOADED |
        DL_ADC12_INTERRUPT_OVERFLOW |
        DL_ADC12_INTERRUPT_UNDERFLOW);
    DL_ADC12_initSingleSample(GRAY_ADC_INST,
        DL_ADC12_REPEAT_MODE_ENABLED,
        DL_ADC12_SAMPLING_SOURCE_AUTO,
        DL_ADC12_TRIG_SRC_SOFTWARE,
        DL_ADC12_SAMP_CONV_RES_12_BIT,
        DL_ADC12_SAMP_CONV_DATA_FORMAT_UNSIGNED);
    DL_ADC12_setStartAddress(GRAY_ADC_INST, DL_ADC12_SEQ_START_ADDR_00);
    DL_ADC12_setSampleTime0(GRAY_ADC_INST, 160U);
    DL_ADC12_configConversionMem(GRAY_ADC_INST,
        GRAY_ADC_MEM_IDX,
        GRAY_ADC_INPUT_CHAN,
        DL_ADC12_REFERENCE_VOLTAGE_VDDA,
        DL_ADC12_SAMPLE_TIMER_SOURCE_SCOMP0,
        DL_ADC12_AVERAGING_MODE_DISABLED,
        DL_ADC12_BURN_OUT_SOURCE_DISABLED,
        DL_ADC12_TRIGGER_MODE_AUTO_NEXT,
        DL_ADC12_WINDOWS_COMP_MODE_DISABLED);
    DL_ADC12_setPowerDownMode(GRAY_ADC_INST, DL_ADC12_POWER_DOWN_MODE_MANUAL);
    DL_ADC12_enableConversions(GRAY_ADC_INST);
}

SYSCONFIG_WEAK void SYSCFG_DL_TB6612_PWM_init(void)
{
    static const DL_TimerA_ClockConfig tb6612PwmClockConfig = {
        .clockSel = DL_TIMER_CLOCK_BUSCLK,
        .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
        .prescale = 0U,
    };

    DL_TimerA_setClockConfig(TB6612_PWM_INST, (DL_TimerA_ClockConfig *)&tb6612PwmClockConfig);
    DL_TimerA_setLoadValue(TB6612_PWM_INST, TB6612_PWM_PERIOD - 1U);

    DL_TimerA_setCaptureCompareAction(TB6612_PWM_INST,
        (DL_TIMER_CC_LACT_CCP_LOW | DL_TIMER_CC_CDACT_CCP_HIGH),
        DL_TIMER_CC_0_INDEX);
    DL_TimerA_setCaptureCompareAction(TB6612_PWM_INST,
        (DL_TIMER_CC_LACT_CCP_LOW | DL_TIMER_CC_CDACT_CCP_HIGH),
        DL_TIMER_CC_1_INDEX);
    DL_TimerA_setCaptureCompareCtl(TB6612_PWM_INST,
        DL_TIMER_CC_MODE_COMPARE, 0U, DL_TIMER_CC_0_INDEX);
    DL_TimerA_setCaptureCompareCtl(TB6612_PWM_INST,
        DL_TIMER_CC_MODE_COMPARE, 0U, DL_TIMER_CC_1_INDEX);
    DL_TimerA_setCaptureCompareInput(TB6612_PWM_INST,
        DL_TIMER_CC_INPUT_INV_NOINVERT, DL_TIMER_CC_IN_SEL_CCPX,
        DL_TIMER_CC_0_INDEX);
    DL_TimerA_setCaptureCompareInput(TB6612_PWM_INST,
        DL_TIMER_CC_INPUT_INV_NOINVERT, DL_TIMER_CC_IN_SEL_CCPX,
        DL_TIMER_CC_1_INDEX);
    DL_TimerA_setCaptureCompareOutCtl(TB6612_PWM_INST,
        DL_TIMER_CC_OCTL_INIT_VAL_LOW, DL_TIMER_CC_OCTL_INV_OUT_DISABLED,
        DL_TIMER_CC_OCTL_SRC_FUNCVAL, DL_TIMER_CC_0_INDEX);
    DL_TimerA_setCaptureCompareOutCtl(TB6612_PWM_INST,
        DL_TIMER_CC_OCTL_INIT_VAL_LOW, DL_TIMER_CC_OCTL_INV_OUT_DISABLED,
        DL_TIMER_CC_OCTL_SRC_FUNCVAL, DL_TIMER_CC_1_INDEX);
    DL_TimerA_setCaptCompUpdateMethod(TB6612_PWM_INST,
        DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMER_CC_0_INDEX);
    DL_TimerA_setCaptCompUpdateMethod(TB6612_PWM_INST,
        DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMER_CC_1_INDEX);

    DL_TimerA_setCounterRepeatMode(TB6612_PWM_INST, DL_TIMER_REPEAT_MODE_ENABLED);
    DL_TimerA_setCounterValueAfterEnable(TB6612_PWM_INST, DL_TIMER_COUNT_AFTER_EN_LOAD_VAL);
    DL_TimerA_setCounterControl(TB6612_PWM_INST,
        DL_TIMER_CZC_CCCTL0_ZCOND,
        DL_TIMER_CAC_CCCTL0_ACOND,
        DL_TIMER_CLC_CCCTL0_LCOND);
    DL_TimerA_setCCPDirection(TB6612_PWM_INST, (DL_TIMER_CC0_OUTPUT | DL_TIMER_CC1_OUTPUT));
    DL_TimerA_setCCPOutputDisabled(TB6612_PWM_INST,
        DL_TIMER_CCP_DIS_OUT_SET_BY_OCTL,
        DL_TIMER_CCP_DIS_OUT_SET_BY_OCTL);

    DL_TimerA_setTimerCount(TB6612_PWM_INST, 0U);
    SYSCFG_DL_TB6612_setPWMDuty(0U, 0U);
    DL_TimerA_enableClock(TB6612_PWM_INST);
    DL_TimerA_startCounter(TB6612_PWM_INST);
}

void SYSCFG_DL_TB6612_setPWMDuty(uint16_t dutyA, uint16_t dutyB)
{
    uint32_t period = DL_TimerA_getLoadValue(TB6612_PWM_INST) + 1UL;
    uint32_t compareA;
    uint32_t compareB;

    if (dutyA == 0U) {
        compareA = period;
    } else {
        if ((uint32_t)dutyA >= period) {
            dutyA = (uint16_t)(period - 1UL);
        }
        compareA = dutyA;
    }

    if (dutyB == 0U) {
        compareB = period;
    } else {
        if ((uint32_t)dutyB >= period) {
            dutyB = (uint16_t)(period - 1UL);
        }
        compareB = dutyB;
    }

    DL_TimerA_setCaptureCompareValue(TB6612_PWM_INST, compareA, DL_TIMER_CC_0_INDEX);
    DL_TimerA_setCaptureCompareValue(TB6612_PWM_INST, compareB, DL_TIMER_CC_1_INDEX);
}


static const DL_UART_Main_ClockConfig gUART_DEBUGClockConfig = {
    .clockSel    = DL_UART_MAIN_CLOCK_BUSCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

static const DL_UART_Main_Config gUART_DEBUGConfig = {
    .mode        = DL_UART_MAIN_MODE_NORMAL,
    .direction   = DL_UART_MAIN_DIRECTION_TX_RX,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .parity      = DL_UART_MAIN_PARITY_NONE,
    .wordLength  = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .stopBits    = DL_UART_MAIN_STOP_BITS_ONE
};

SYSCONFIG_WEAK void SYSCFG_DL_UART_DEBUG_init(void)
{
    DL_UART_Main_setClockConfig(UART_DEBUG_INST, (DL_UART_Main_ClockConfig *) &gUART_DEBUGClockConfig);

    DL_UART_Main_init(UART_DEBUG_INST, (DL_UART_Main_Config *) &gUART_DEBUGConfig);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 115200
     *  Actual baud rate: 115190.78
     */
    DL_UART_Main_setOversampling(UART_DEBUG_INST, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(UART_DEBUG_INST, UART_DEBUG_IBRD_40_MHZ_115200_BAUD, UART_DEBUG_FBRD_40_MHZ_115200_BAUD);


    /* Configure Interrupts */
    // 注意：RX 中断已禁用，因为当前工程无 RX 处理逻辑。如需接收串口数据，请打开此行并实现 RX 中断处理。
    // DL_UART_Main_enableInterrupt(UART_DEBUG_INST, DL_UART_MAIN_INTERRUPT_RX);
    /* Setting the Interrupt Priority */
    NVIC_SetPriority(UART_DEBUG_INST_INT_IRQN, 3);


    DL_UART_Main_enable(UART_DEBUG_INST);
}

static const DL_UART_Main_ClockConfig gUART_ZDTClockConfig = {
    .clockSel    = DL_UART_MAIN_CLOCK_BUSCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

static const DL_UART_Main_Config gUART_ZDTConfig = {
    .mode        = DL_UART_MAIN_MODE_NORMAL,
    .direction   = DL_UART_MAIN_DIRECTION_TX_RX,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .parity      = DL_UART_MAIN_PARITY_NONE,
    .wordLength  = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .stopBits    = DL_UART_MAIN_STOP_BITS_ONE
};

SYSCONFIG_WEAK void SYSCFG_DL_UART_ZDT_init(void)
{
    DL_UART_Main_setClockConfig(UART_ZDT_INST, (DL_UART_Main_ClockConfig *) &gUART_ZDTClockConfig);

    DL_UART_Main_init(UART_ZDT_INST, (DL_UART_Main_Config *) &gUART_ZDTConfig);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 115200
     *  Actual baud rate: 115190.78
     */
    DL_UART_Main_setOversampling(UART_ZDT_INST, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(UART_ZDT_INST, UART_ZDT_IBRD_40_MHZ_115200_BAUD, UART_ZDT_FBRD_40_MHZ_115200_BAUD);


    /* Configure Interrupts */
    // 注意：如果有需要接收串口数据（如查询电机位置），请打开此行并实现 RX 中断处理
    // DL_UART_Main_enableInterrupt(UART_ZDT_INST, DL_UART_MAIN_INTERRUPT_RX);
    /* Setting the Interrupt Priority */
    NVIC_SetPriority(UART_ZDT_INST_INT_IRQN, 3);


    DL_UART_Main_enable(UART_ZDT_INST);
}

static const DL_UART_Main_ClockConfig gUART_ZIGBEEClockConfig = {
    .clockSel    = DL_UART_MAIN_CLOCK_BUSCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

static const DL_UART_Main_Config gUART_ZIGBEEConfig = {
    .mode        = DL_UART_MAIN_MODE_NORMAL,
    .direction   = DL_UART_MAIN_DIRECTION_TX_RX,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .parity      = DL_UART_MAIN_PARITY_NONE,
    .wordLength  = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .stopBits    = DL_UART_MAIN_STOP_BITS_ONE
};

SYSCONFIG_WEAK void SYSCFG_DL_UART_ZIGBEE_init(void)
{
    DL_UART_Main_setClockConfig(UART_ZIGBEE_INST, (DL_UART_Main_ClockConfig *) &gUART_ZIGBEEClockConfig);

    DL_UART_Main_init(UART_ZIGBEE_INST, (DL_UART_Main_Config *) &gUART_ZIGBEEConfig);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 115200
     *  Actual baud rate: 115190.78
     */
    DL_UART_Main_setOversampling(UART_ZIGBEE_INST, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(UART_ZIGBEE_INST, UART_ZIGBEE_IBRD_40_MHZ_115200_BAUD, UART_ZIGBEE_FBRD_40_MHZ_115200_BAUD);


    /* Configure Interrupts */
    // DL_UART_Main_enableInterrupt(UART_ZIGBEE_INST, DL_UART_MAIN_INTERRUPT_RX);
    /* Setting the Interrupt Priority */
    NVIC_SetPriority(UART_ZIGBEE_INST_INT_IRQN, 3);


    DL_UART_Main_enable(UART_ZIGBEE_INST);
}


#include "TB6612.h"

#if defined(__GNUC__) && defined(__MSPM0G3507__)
#include "ti_msp_dl_config.h"
#endif

static uint16_t TB6612_AbsToDuty(int16_t value)
{
    uint32_t duty;

    if (value < 0) {
        duty = (uint32_t)(-value);
    } else {
        duty = (uint32_t)value;
    }

    if (duty > TB6612_MAX_DUTY) {
        duty = TB6612_MAX_DUTY;
    }

    return (uint16_t)duty;
}

void TB6612_Init(void)
{
#if defined(__GNUC__) && defined(__MSPM0G3507__)
    DL_GPIO_clearPins(TB6612_AIN1_PORT, TB6612_AIN1_PIN);
    DL_GPIO_clearPins(TB6612_AIN2_PORT, TB6612_AIN2_PIN);
    DL_GPIO_clearPins(TB6612_BIN1_PORT, TB6612_BIN1_PIN);
    DL_GPIO_clearPins(TB6612_BIN2_PORT, TB6612_BIN2_PIN);
    TB6612_SetSpeed(0, 0);
#endif
}

void TB6612_SetSpeed(int16_t speedA, int16_t speedB)
{
#if defined(__GNUC__) && defined(__MSPM0G3507__)
    uint16_t dutyA = TB6612_AbsToDuty(speedA);
    uint16_t dutyB = TB6612_AbsToDuty(speedB);

    if (speedA > 0) {
        DL_GPIO_clearPins(TB6612_AIN1_PORT, TB6612_AIN1_PIN);
        DL_GPIO_setPins(TB6612_AIN2_PORT, TB6612_AIN2_PIN);
    } else if (speedA < 0) {
        DL_GPIO_setPins(TB6612_AIN1_PORT, TB6612_AIN1_PIN);
        DL_GPIO_clearPins(TB6612_AIN2_PORT, TB6612_AIN2_PIN);
    } else {
        DL_GPIO_clearPins(TB6612_AIN1_PORT, TB6612_AIN1_PIN);
        DL_GPIO_clearPins(TB6612_AIN2_PORT, TB6612_AIN2_PIN);
    }

    if (speedB > 0) {
        DL_GPIO_clearPins(TB6612_BIN1_PORT, TB6612_BIN1_PIN);
        DL_GPIO_setPins(TB6612_BIN2_PORT, TB6612_BIN2_PIN);
    } else if (speedB < 0) {
        DL_GPIO_setPins(TB6612_BIN1_PORT, TB6612_BIN1_PIN);
        DL_GPIO_clearPins(TB6612_BIN2_PORT, TB6612_BIN2_PIN);
    } else {
        DL_GPIO_clearPins(TB6612_BIN1_PORT, TB6612_BIN1_PIN);
        DL_GPIO_clearPins(TB6612_BIN2_PORT, TB6612_BIN2_PIN);
    }

    SYSCFG_DL_TB6612_setPWMDuty(dutyA, dutyB);
#else
    (void)speedA;
    (void)speedB;
#endif
}

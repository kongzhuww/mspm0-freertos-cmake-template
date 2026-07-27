#include "gray_adc.h"

#include <stdio.h>

#if defined(__GNUC__) && defined(__MSPM0G3507__)
#include "ti_msp_dl_config.h"
#include "tim_delay.h"
#endif

#define GRAY_ADC_SAMPLES_PER_CH       2U
#define GRAY_ADC_SWITCH_DELAY_US      5U
#define GRAY_ADC_BLACK_CONFIRM_COUNT  2U
#define GRAY_ADC_READ_TIMEOUT         100000UL

static GrayADC_Sensor_t g_gray_sensor;

static const uint16_t s_default_white[8] = GRAY_ADC_WHITE_DEFAULT;
static const uint16_t s_default_black[8] = GRAY_ADC_BLACK_DEFAULT;

static uint16_t GrayADC_ReadOne(void)
{
#if defined(__GNUC__) && defined(__MSPM0G3507__)
    uint32_t timeout = GRAY_ADC_READ_TIMEOUT;

    DL_ADC12_clearInterruptStatus(GRAY_ADC_INST, DL_ADC12_INTERRUPT_MEM0_RESULT_LOADED);
    DL_ADC12_startConversion(GRAY_ADC_INST);

    while ((DL_ADC12_getRawInterruptStatus(GRAY_ADC_INST, DL_ADC12_INTERRUPT_MEM0_RESULT_LOADED) == 0U) &&
           (timeout > 0U)) {
        timeout--;
    }

    DL_ADC12_stopConversion(GRAY_ADC_INST);

    if (timeout == 0U) {
        return 0U;
    }

    return DL_ADC12_getMemResult(GRAY_ADC_INST, GRAY_ADC_MEM_IDX);
#else
    return 0U;
#endif
}

void GrayADC_Init(void)
{
    uint8_t i;

    GrayADC_SelectChannel(0U);

    g_gray_sensor.digital = 0xFFU;
    for (i = 0U; i < 8U; i++) {
        g_gray_sensor.digital_bits[i] = 1U;
    }
}

void GrayADC_SelectChannel(uint8_t channel)
{
#if defined(__GNUC__) && defined(__MSPM0G3507__)
    uint32_t ad0_level;
    uint32_t ad1_level;
    uint32_t ad2_level;

    channel &= 0x07U;

#if GRAY_ADC_ADDR_INVERTED
    ad0_level = (channel & 0x01U) ? 0U : 1U;
    ad1_level = (channel & 0x02U) ? 0U : 1U;
    ad2_level = (channel & 0x04U) ? 0U : 1U;
#else
    ad0_level = (channel & 0x01U) ? 1U : 0U;
    ad1_level = (channel & 0x02U) ? 1U : 0U;
    ad2_level = (channel & 0x04U) ? 1U : 0U;
#endif

    if (ad0_level) {
        DL_GPIO_setPins(GRAY_ADC_AD0_PORT, GRAY_ADC_AD0_PIN);
    } else {
        DL_GPIO_clearPins(GRAY_ADC_AD0_PORT, GRAY_ADC_AD0_PIN);
    }

    if (ad1_level) {
        DL_GPIO_setPins(GRAY_ADC_AD1_PORT, GRAY_ADC_AD1_PIN);
    } else {
        DL_GPIO_clearPins(GRAY_ADC_AD1_PORT, GRAY_ADC_AD1_PIN);
    }

    if (ad2_level) {
        DL_GPIO_setPins(GRAY_ADC_AD2_PORT, GRAY_ADC_AD2_PIN);
    } else {
        DL_GPIO_clearPins(GRAY_ADC_AD2_PORT, GRAY_ADC_AD2_PIN);
    }

    delay_us(GRAY_ADC_SWITCH_DELAY_US);
#else
    (void)channel;
#endif
}

void GrayADC_ReadAllRaw(GrayADC_Sensor_t *sensor)
{
    uint8_t ch;
    uint8_t sample;
    uint32_t sum;

    if (sensor == NULL) {
        return;
    }

    for (ch = 0U; ch < 8U; ch++) {
        GrayADC_SelectChannel(7U - ch);

        sum = 0UL;
        for (sample = 0U; sample < GRAY_ADC_SAMPLES_PER_CH; sample++) {
            sum += GrayADC_ReadOne();
        }
        sensor->raw_value[ch] = (uint16_t)(sum / GRAY_ADC_SAMPLES_PER_CH);
    }
}

void GrayADC_InitSensor(GrayADC_Sensor_t *sensor,
                        const uint16_t *calib_white,
                        const uint16_t *calib_black)
{
    uint8_t i;
    uint16_t temp;

    if (sensor == NULL) {
        return;
    }

    if (calib_white == NULL) {
        calib_white = s_default_white;
    }
    if (calib_black == NULL) {
        calib_black = s_default_black;
    }

    for (i = 0U; i < 8U; i++) {
        sensor->raw_value[i] = 0U;
        sensor->normalized[i] = 0U;
        sensor->digital_bits[i] = 1U;
        sensor->calib_white[i] = calib_white[i];
        sensor->calib_black[i] = calib_black[i];

        if (sensor->calib_black[i] >= sensor->calib_white[i]) {
            temp = sensor->calib_white[i];
            sensor->calib_white[i] = sensor->calib_black[i];
            sensor->calib_black[i] = temp;
        }

        sensor->threshold_white[i] =
            (uint16_t)(((uint32_t)sensor->calib_white[i] * 2UL +
                        (uint32_t)sensor->calib_black[i]) / 3UL);
        sensor->threshold_black[i] =
            (uint16_t)(((uint32_t)sensor->calib_white[i] +
                        (uint32_t)sensor->calib_black[i] * 2UL) / 3UL);
    }

    sensor->bits = 4096U;
    sensor->digital = 0xFFU;
    sensor->calib_ready = 1U;
}

GrayADC_Sensor_t *GrayADC_GetSensor(void)
{
    return &g_gray_sensor;
}

static void GrayADC_ConvertToDigital(GrayADC_Sensor_t *sensor)
{
    static uint8_t black_count[8];
    uint8_t i;
    uint16_t threshold;

    if (sensor == NULL) {
        return;
    }

    for (i = 0U; i < 8U; i++) {
        threshold = (uint16_t)(((uint32_t)sensor->calib_white[i] +
                                (uint32_t)sensor->calib_black[i]) / 2UL);

        if (sensor->raw_value[i] < threshold) {
            if (black_count[i] < GRAY_ADC_BLACK_CONFIRM_COUNT) {
                black_count[i]++;
            }
            if (black_count[i] >= GRAY_ADC_BLACK_CONFIRM_COUNT) {
                sensor->digital &= (uint8_t)(~(1U << i));
                sensor->digital_bits[i] = 0U;
            }
        } else {
            black_count[i] = 0U;
            sensor->digital |= (uint8_t)(1U << i);
            sensor->digital_bits[i] = 1U;
        }
    }
}

static void GrayADC_Normalize(GrayADC_Sensor_t *sensor)
{
    uint8_t i;
    uint32_t diff_calib;
    int32_t diff_raw;
    uint32_t value;

    if (sensor == NULL) {
        return;
    }

    for (i = 0U; i < 8U; i++) {
        diff_calib = (uint32_t)sensor->calib_white[i] - (uint32_t)sensor->calib_black[i];
        diff_raw = (int32_t)sensor->raw_value[i] - (int32_t)sensor->calib_black[i];

        if ((diff_calib == 0U) || (diff_raw <= 0)) {
            sensor->normalized[i] = 0U;
            continue;
        }

        value = ((uint32_t)diff_raw * sensor->bits) / diff_calib;
        if (value > sensor->bits) {
            value = sensor->bits;
        }
        sensor->normalized[i] = (uint16_t)value;
    }
}

void GrayADC_Task(GrayADC_Sensor_t *sensor)
{
    if (sensor == NULL) {
        return;
    }

    GrayADC_ReadAllRaw(sensor);

    if (sensor->calib_ready == 0U) {
        GrayADC_InitSensor(sensor, s_default_white, s_default_black);
    }

    GrayADC_ConvertToDigital(sensor);
    GrayADC_Normalize(sensor);
}

int32_t GrayADC_LinePosition(const GrayADC_Sensor_t *sensor)
{
    static int32_t s_filtered = -1;
    int32_t weighted = 0;
    int32_t total = 0;
    int32_t dark;
    int32_t raw_pos;
    const int32_t step = (int32_t)(GRAY_ADC_SENSOR_SPACING_MM * 100UL);
    const int32_t max_pos = 7 * step;
    const int32_t center_pos = max_pos / 2;
    uint8_t i;

    if ((sensor == NULL) || (sensor->calib_ready == 0U)) {
        s_filtered = -1;
        return -1;
    }

    if (s_filtered < 0) {
        s_filtered = center_pos;
    }

    for (i = 0U; i < 8U; i++) {
        dark = (int32_t)sensor->bits - (int32_t)sensor->normalized[i];
        if (dark < 0) {
            dark = 0;
        }

        weighted += dark * (int32_t)(7U - i) * step;
        total += dark;
    }

    if (total < 50) {
        return s_filtered;
    }

    raw_pos = weighted / total;

    if (raw_pos < 0) {
        raw_pos = 0;
    }
    if (raw_pos > max_pos) {
        raw_pos = max_pos;
    }

#if GRAY_ADC_POSITION_SMOOTHING > 0U
    s_filtered = s_filtered
               - s_filtered / (int32_t)GRAY_ADC_POSITION_SMOOTHING
               + raw_pos / (int32_t)GRAY_ADC_POSITION_SMOOTHING;
#else
    s_filtered = raw_pos;
#endif

    return s_filtered;
}

void GrayADC_PrintRaw(const GrayADC_Sensor_t *sensor)
{
    if (sensor == NULL) {
        return;
    }

    printf("RAW:%u %u %u %u %u %u %u %u\n",
           sensor->raw_value[0], sensor->raw_value[1],
           sensor->raw_value[2], sensor->raw_value[3],
           sensor->raw_value[4], sensor->raw_value[5],
           sensor->raw_value[6], sensor->raw_value[7]);
}

void GrayADC_PrintBits(const GrayADC_Sensor_t *sensor)
{
    if (sensor == NULL) {
        return;
    }

    printf("D:%u%u%u%u%u%u%u%u\n",
           sensor->digital_bits[0], sensor->digital_bits[1],
           sensor->digital_bits[2], sensor->digital_bits[3],
           sensor->digital_bits[4], sensor->digital_bits[5],
           sensor->digital_bits[6], sensor->digital_bits[7]);
}

void GrayADC_PrintLinePos(const GrayADC_Sensor_t *sensor)
{
    int32_t pos;
    int32_t center;

    if (sensor == NULL) {
        return;
    }

    pos = GrayADC_LinePosition(sensor);
    center = (int32_t)(7U * GRAY_ADC_SENSOR_SPACING_MM * 100U / 2U);

    printf("POS:%ld E:%ld D:%u%u%u%u%u%u%u%u\n",
           (long)pos, (long)(pos - center),
           sensor->digital_bits[0], sensor->digital_bits[1],
           sensor->digital_bits[2], sensor->digital_bits[3],
           sensor->digital_bits[4], sensor->digital_bits[5],
           sensor->digital_bits[6], sensor->digital_bits[7]);
}

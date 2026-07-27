#ifndef GRAY_ADC_H
#define GRAY_ADC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 相邻两个灰度管中心距离，单位：mm */
#define GRAY_ADC_SENSOR_SPACING_MM  12U

/* 位置 EMA 低通滤波强度，0 表示关闭滤波 */
#define GRAY_ADC_POSITION_SMOOTHING  4U

/* OmniM0 默认校准值：sensor[0]=最右路，sensor[7]=最左路 */
#define GRAY_ADC_WHITE_DEFAULT  { 3428U, 3234U, 3337U, 2606U, \
                                  3436U, 3457U, 3409U, 2823U }
#define GRAY_ADC_BLACK_DEFAULT  {  234U,  228U,  225U,  225U, \
                                   245U,  349U,  236U,  226U }

typedef struct {
    uint16_t raw_value[8];
    uint16_t normalized[8];
    uint8_t digital;
    uint8_t digital_bits[8];

    uint16_t calib_white[8];
    uint16_t calib_black[8];
    uint16_t threshold_white[8];
    uint16_t threshold_black[8];
    uint16_t bits;
    uint8_t calib_ready;
} GrayADC_Sensor_t;

void GrayADC_Init(void);
void GrayADC_SelectChannel(uint8_t channel);
void GrayADC_ReadAllRaw(GrayADC_Sensor_t *sensor);
void GrayADC_InitSensor(GrayADC_Sensor_t *sensor,
                        const uint16_t *calib_white,
                        const uint16_t *calib_black);
GrayADC_Sensor_t *GrayADC_GetSensor(void);
void GrayADC_Task(GrayADC_Sensor_t *sensor);
int32_t GrayADC_LinePosition(const GrayADC_Sensor_t *sensor);

void GrayADC_PrintRaw(const GrayADC_Sensor_t *sensor);
void GrayADC_PrintBits(const GrayADC_Sensor_t *sensor);
void GrayADC_PrintLinePos(const GrayADC_Sensor_t *sensor);

#ifdef __cplusplus
}
#endif

#endif /* GRAY_ADC_H */

#ifndef TB6612_H
#define TB6612_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 20kHz @ 40MHz BUSCLK / 2000，速度值范围 0~2000 */
#define TB6612_MAX_DUTY  2000U

void TB6612_Init(void);
void TB6612_SetSpeed(int16_t speedA, int16_t speedB);

#ifdef __cplusplus
}
#endif

#endif /* TB6612_H */

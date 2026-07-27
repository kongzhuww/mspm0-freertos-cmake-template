#ifndef CONTROL_H
#define CONTROL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CONTROL_STEER_LIMIT  100

void Control_PID_Reset(void);
int32_t Control_PID_Update(int32_t error);
void Control_LineFollowTask(void);
void Control_ManualTurn(int8_t dir);
void Control_StartLineFollow(void);
void Control_Stop(void);
void Control_Service(void);
uint8_t Control_IsRunning(void);

#ifdef __cplusplus
}
#endif

#endif /* CONTROL_H */

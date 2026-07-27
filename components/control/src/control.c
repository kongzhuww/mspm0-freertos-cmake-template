#include "control.h"

#include "gray_adc.h"
#include "TB6612.h"

#define CONTROL_BASE_SPEED       700
#define CONTROL_TURN_SPEED       900
#define CONTROL_PRE_TURN_TICKS   30U
#define CONTROL_TURN_HOLD_TICKS  15U
#define CONTROL_LOST_SEARCH_SPEED 450
#define CONTROL_OLED_PERIOD      10U
#define CONTROL_SHOW_GRAY_ON_OLED 0
#define CONTROL_KP               0.12f
#define CONTROL_KD               0.8f

static int32_t g_pid_last_error;
static uint16_t g_pre_turn_ticks;
static uint16_t g_turn_hold_ticks;
static int8_t g_turn_dir;
static uint8_t g_need_white;
static uint8_t g_turn_show_tick;
static uint8_t g_line_follow_running;

static int16_t Control_ClampSpeed(int32_t speed)
{
    if (speed > (int32_t)TB6612_MAX_DUTY) {
        return (int16_t)TB6612_MAX_DUTY;
    }
    if (speed < 0) {
        return 0;
    }
    return (int16_t)speed;
}

static int32_t Control_ClampSteer(int32_t steer)
{
    if (steer > CONTROL_STEER_LIMIT) {
        return CONTROL_STEER_LIMIT;
    }
    if (steer < -CONTROL_STEER_LIMIT) {
        return -CONTROL_STEER_LIMIT;
    }
    return steer;
}

static uint8_t Control_CountBlack(const GrayADC_Sensor_t *sensor, uint8_t start, uint8_t end)
{
    uint8_t i;
    uint8_t count = 0U;

    for (i = start; i < end; i++) {
        if (sensor->digital_bits[i] == 0U) {
            count++;
        }
    }
    return count;
}

static int32_t Control_LineError(const GrayADC_Sensor_t *sensor)
{
    static const int16_t weight[8] = {3500, 2500, 1500, 500, -500, -1500, -2500, -3500};
    int32_t sum = 0;
    uint8_t count = 0U;
    uint8_t i;

    for (i = 0U; i < 8U; i++) {
        if (sensor->digital_bits[i] == 0U) {
            sum += weight[i];
            count++;
        }
    }

    if (count == 0U) {
        return 0;
    }
    return sum / (int32_t)count;
}

static void Control_ShowBlackBitsPeriod(const GrayADC_Sensor_t *sensor)
{
#if CONTROL_SHOW_GRAY_ON_OLED
    static uint8_t show_tick;
    char bits[16];

    show_tick++;
    if (show_tick < CONTROL_OLED_PERIOD) {
        return;
    }
    show_tick = 0U;

    bits[0] = 'D';
    bits[1] = ':';
    for (uint8_t i = 0U; i < 8U; i++) {
        bits[2U + i] = (char)('0' + sensor->digital_bits[i]);
    }
    bits[10] = '\0';

    oled_clear();
    oled_show_string(0, 0, bits, 16, 0);
#else
    // Start 页面要显示小猫动画，循迹任务不能再清屏抢占 OLED。
    (void)sensor;
#endif
}

static void Control_Turn(int8_t dir)
{
    /* A 通道=右轮，B 通道=左轮。 */
    if (dir < 0) {
        TB6612_SetSpeed(CONTROL_TURN_SPEED, -CONTROL_TURN_SPEED);
    } else {
        TB6612_SetSpeed(-CONTROL_TURN_SPEED, CONTROL_TURN_SPEED);
    }
}

static void Control_SearchLine(void)
{
    /* 转弯结束后灰度可能短暂全白，此时不能关闭运行标志。
     * 按刚才的转弯方向低速继续找线，重新看到黑线后再交给 PID 循迹。
     */
    if (g_turn_dir < 0) {
        TB6612_SetSpeed(CONTROL_LOST_SEARCH_SPEED, -CONTROL_LOST_SEARCH_SPEED);
    } else if (g_turn_dir > 0) {
        TB6612_SetSpeed(-CONTROL_LOST_SEARCH_SPEED, CONTROL_LOST_SEARCH_SPEED);
    } else {
        TB6612_SetSpeed(CONTROL_LOST_SEARCH_SPEED, CONTROL_LOST_SEARCH_SPEED);
    }
}

void Control_PID_Reset(void)
{
    g_pid_last_error = 0;
    g_pre_turn_ticks = 0U;
    g_turn_hold_ticks = 0U;
    g_turn_show_tick = 0U;
    g_need_white = 0U;
}

void Control_ManualTurn(int8_t dir)
{
    g_turn_dir = dir;
    g_turn_hold_ticks = CONTROL_TURN_HOLD_TICKS;
    g_turn_show_tick = CONTROL_OLED_PERIOD;
    g_need_white = 0U;
}

int32_t Control_PID_Update(int32_t error)
{
    int32_t derivative;
    int32_t steer;

    derivative = error - g_pid_last_error;
    g_pid_last_error = error;

    steer = (int32_t)((float)error * CONTROL_KP + (float)derivative * CONTROL_KD);
    return Control_ClampSteer(steer);
}

void Control_StartLineFollow(void)
{
    /* 进入 Start 页面时清掉旧的 PID/路口状态，保证每次启动都从确定状态开始。 */
    Control_PID_Reset();
    g_line_follow_running = 1U;
}

void Control_Stop(void)
{
    /* 退出 Start 页面或丢线停车时关闭运行标志，并立即把两路 PWM 置零。 */
    Control_PID_Reset();
    g_line_follow_running = 0U;
    TB6612_SetSpeed(0, 0);
}

void Control_Service(void)
{
    /* 控制任务每 20ms 调用一次；只有 Start 页面置位后才真正驱动电机。 */
    if (g_line_follow_running != 0U) {
        Control_LineFollowTask();
    } else {
        GrayADC_Task(GrayADC_GetSensor());
    }
}

uint8_t Control_IsRunning(void)
{
    return g_line_follow_running;
}

void Control_LineFollowTask(void)
{
    GrayADC_Sensor_t *sensor;
    int32_t error;
    int32_t steer;
    uint8_t total_black;
    uint8_t right_black;
    uint8_t left_black;

    if (g_turn_hold_ticks > 0U) {
        g_turn_hold_ticks--;
        Control_Turn(g_turn_dir);
        if (g_turn_hold_ticks == 0U) {
            g_need_white = 1U;
        }
        return;
    }

    if (g_pre_turn_ticks > 0U) {
        g_pre_turn_ticks--;
        TB6612_SetSpeed(CONTROL_BASE_SPEED, CONTROL_BASE_SPEED);
        if (g_pre_turn_ticks == 0U) {
            g_turn_hold_ticks = CONTROL_TURN_HOLD_TICKS;
            g_turn_show_tick = CONTROL_OLED_PERIOD;
            Control_Turn(g_turn_dir);
        }
        return;
    }

    sensor = GrayADC_GetSensor();
    GrayADC_Task(sensor);
    Control_ShowBlackBitsPeriod(sensor);

    total_black = Control_CountBlack(sensor, 0U, 8U);
    right_black = Control_CountBlack(sensor, 0U, 4U);
    left_black  = Control_CountBlack(sensor, 4U, 8U);

    if (total_black < 4U) {
        g_need_white = 0U;
    }

    if ((total_black >= 4U) && (g_need_white == 0U)) {
        Control_PID_Reset();
        if (left_black > right_black) {
            g_turn_dir = -1;
        } else if (right_black > left_black) {
            g_turn_dir = 1;
        } else {
            g_turn_dir = -1;
        }
        g_pre_turn_ticks = CONTROL_PRE_TURN_TICKS;
        g_turn_show_tick = CONTROL_OLED_PERIOD;
        TB6612_SetSpeed(CONTROL_BASE_SPEED, CONTROL_BASE_SPEED);
        return;
    }

    if (total_black == 0U) {
        Control_PID_Reset();
        Control_SearchLine();
        return;
    }

    error = Control_LineError(sensor);
    steer = Control_PID_Update(error);

    TB6612_SetSpeed(Control_ClampSpeed(CONTROL_BASE_SPEED + steer),
                   Control_ClampSpeed(CONTROL_BASE_SPEED - steer));
}

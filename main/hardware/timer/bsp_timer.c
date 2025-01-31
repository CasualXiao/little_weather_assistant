#include "bsp_timer.h"
#include "freertos/semphr.h"

volatile SemaphoreHandle_t sec_timersem; // timer fire sem

/**
 * @brief 定时器回调函数
 *
 * @param timer
 * @param edata
 * @param user_data
 * @return true
 * @return false
 */
static bool IRAM_ATTR TimerCallback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
    BaseType_t high_task_awoken = pdFALSE;
    xSemaphoreGiveFromISR(sec_timersem, NULL);
    return (high_task_awoken == pdTRUE);
}

/**
 * @brief 初始化定时器
 *
 */
void init_timer()
{
    // 创建一个二进制信号量用于秒计时器
    sec_timersem = xSemaphoreCreateBinary();
    // 定义一个通用定时器
    gptimer_handle_t gptimer = NULL;

    // 配置定时器参数
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT, // 定时器时钟来源 选择APB作为默认选项
        .direction = GPTIMER_COUNT_UP,      // 向上计数
        // 计数器分辨率(工作频率)以Hz为单位，因此，每个计数滴答的步长等于(1 / resolution_hz)秒
        // 假设 resolution_hz = 1000 000
        // 1 / resolution_hz = 1 / 1000000 = 0.000001(秒) = 1(微秒) （ 1 tick= 1us ）
        .resolution_hz = RESOLUTION_HZ,
    };
    // 将配置设置到定时器
    gptimer_new_timer(&timer_config, &gptimer);

    // 绑定一个回调函数
    gptimer_event_callbacks_t cbs = {
        .on_alarm = TimerCallback,
    };
    // 设置定时器gptimer的 回调函数为cbs  传入的参数为NULL
    gptimer_register_event_callbacks(gptimer, &cbs, NULL);

    // 使能定时器
    gptimer_enable(gptimer);

    // 通用定时器的报警值设置
    gptimer_alarm_config_t alarm_config = {
        .reload_count = 0,                  // 重载计数值为0
        .alarm_count = ALARM_COUNT,         // 报警目标计数值 1000000 = 1s
        .flags.auto_reload_on_alarm = true, // 开启重加载
    };
    // 设置触发报警动作
    gptimer_set_alarm_action(gptimer, &alarm_config);
    // 开始定时器开始工作
    gptimer_start(gptimer);
}

#ifdef __cplusplus
extern "C" {
#endif

#include "tc_iot_inc.h"

void tc_iot_hal_timer_init(tc_iot_timer* timer) {
    timer->xTicksToWait = 0;
    memset(&timer->xTimeOut, '\0', sizeof(timer->xTimeOut));
}

char tc_iot_hal_timer_is_expired(tc_iot_timer* timer) {
    return xTaskCheckForTimeOut(&timer->xTimeOut, &timer->xTicksToWait) == pdTRUE;
}

void tc_iot_hal_timer_countdown_ms(tc_iot_timer* timer, unsigned int timeout_ms) {
    timer->xTicksToWait = timeout_ms / portTICK_RATE_MS; /* convert milliseconds to ticks */
    vTaskSetTimeOutState(&timer->xTimeOut); /* Record the time at which this function was entered. */
}

void tc_iot_hal_timer_countdown_second(tc_iot_timer* timer,
                                       unsigned int timeout) {
    tc_iot_hal_timer_countdown_ms(timer, timeout * 1000);
}

int tc_iot_hal_timer_left_ms(tc_iot_timer* timer) {
    xTaskCheckForTimeOut(&timer->xTimeOut, &timer->xTicksToWait); /* updates xTicksToWait to the number left */
    return (timer->xTicksToWait < 0) ? 0 : (timer->xTicksToWait * portTICK_RATE_MS);
}

#ifdef __cplusplus
}
#endif

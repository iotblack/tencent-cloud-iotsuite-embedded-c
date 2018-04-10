#ifdef __cplusplus
extern "C" {
#endif

#include "tc_iot_inc.h"

int tc_iot_hal_thread_start(Thread* thread, void (*fn)(void*), void* arg)
{
    int rc = 0;
    uint16_t usTaskStackSize = (configMINIMAL_STACK_SIZE * 5);
    unsigned portBASE_TYPE uxTaskPriority = uxTaskPriorityGet(NULL); /* set the priority as the same as the calling task*/

    rc = xTaskCreate(fn,    /* The function that implements the task. */
            "MQTTTask",         /* Just a text name for the task to aid debugging. */
            usTaskStackSize,    /* The stack size is defined in FreeRTOSIPConfig.h. */
            arg,                /* The task parameter, not used in this case. */
            uxTaskPriority,     /* The priority assigned to the task is defined in FreeRTOSConfig.h. */
            &thread->task);     /* The task handle is not used. */

    return rc;
}


void tc_iot_hal_mutex_init(Mutex* mutex)
{
    mutex->sem = xSemaphoreCreateMutex();
}

int tc_iot_hal_mutex_lock(Mutex* mutex)
{
    return xSemaphoreTake(mutex->sem, portMAX_DELAY);
}

int tc_iot_hal_mutext_unlock(Mutex* mutex)
{
    return xSemaphoreGive(mutex->sem);
}

long tc_iot_hal_timestamp(void* zone) {
    return sntp_get_current_timestamp();
}

int tc_iot_hal_sleep_ms(long ms) {
    if ((ms > 0) && (ms < portTICK_RATE_MS)) {
        ms = portTICK_RATE_MS;
    }

    vTaskDelay(ms / portTICK_RATE_MS);
    return 0;
}

long tc_iot_hal_random() {
    return (unsigned int)os_random();
}

void tc_iot_hal_srandom(unsigned int seed) {
    srand(seed);
}

#ifdef __cplusplus
}
#endif

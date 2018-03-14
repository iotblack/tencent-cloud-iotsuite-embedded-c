#ifndef TC_IOT_PLATFORM_H
#define TC_IOT_PLATFORM_H

#include <ctype.h>
// #include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// esp8266
#include <espressif/c_types.h>
#include <espressif/esp_libc.h>
#include "MQTTFreeRTOS.h"
#include "lwip/netdb.h"
#include "esp_common.h"
#include "esp_wifi.h"
#include "uart.h"
#include "apps/sntp.h"

#define tc_iot_hal_malloc malloc
#define tc_iot_hal_free free
#define tc_iot_hal_printf printf
#define tc_iot_hal_snprintf snprintf

typedef struct tc_iot_timer 
{
    portTickType xTicksToWait;
    xTimeOutType xTimeOut;
} tc_iot_timer;


#endif /* end of include guard */

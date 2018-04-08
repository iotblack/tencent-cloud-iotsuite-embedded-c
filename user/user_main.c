/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2015 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

/* #include "freertos/FreeRTOS.h" */
/* #include "freertos/task.h" */
#include "esp_common.h"
/* #include "esp_wifi.h" */
#include "uart.h"
/* #include "apps/sntp.h" */

#include "tc_iot_device_config.h"
#include "tc_iot_device_logic.h"
#include "gpio.h"
#include "tc_iot_export.h"

#ifdef TM1638_TASK
#include "TM1638.h"
#endif

void user_uart_init_new(void);


#define HEAP_CHECK_TASK 1

#define TASK_CYCLE 2000
#define WIFI_SSID       "wifitest"       // type:string, your AP/router SSID to config your device networking
#define WIFI_PASSWORD   "wifitest12345"       // type:string, your AP/router password


void light_demo(void *pvParameter);

int got_ip_flag = 0;
static xTaskHandle xHandleTaskLight = NULL;

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
 *******************************************************************************/
uint32 user_rf_cal_sector_set(void)
{
    flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        case FLASH_SIZE_64M_MAP_1024_1024:
            rf_cal_sec = 2048 - 5;
            break;

        case FLASH_SIZE_128M_MAP_1024_1024:
            rf_cal_sec = 4096 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}

void sntpfn()
{
    tc_iot_hal_printf("Initializing SNTP\n");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);

    // for set more sntp server, plz modify macro SNTP_MAX_SERVERS in sntp_opts.h file
    sntp_setservername(0, "202.112.29.82");        // set sntp server after got ip address, you had better to adjust the sntp server to your area
    sntp_setservername(1, "time-a.nist.gov");
    /* sntp_setservername(2, "ntp.sjtu.edu.cn"); */
    //    sntp_setservername(3, "0.nettime.pool.ntp.org");
    //    sntp_setservername(4, "time-b.nist.gov");
    //    sntp_setservername(5, "time-a.timefreq.bldrdoc.gov");
    //    sntp_setservername(6, "time-b.timefreq.bldrdoc.gov");
    //    sntp_setservername(7, "time-c.timefreq.bldrdoc.gov");
    //    sntp_setservername(8, "utcnist.colorado.edu");
    //    sntp_setservername(9, "time.nist.gov");

    sntp_set_timezone(0);
    sntp_init();

    while (1) {
        u32_t ts = 0;
        ts = sntp_get_current_timestamp();
        tc_iot_hal_printf("current time : ts=%d, %s\n", ts, sntp_get_real_time(ts));

        if (ts == 0) {
            tc_iot_hal_printf("did not get a valid time from sntp server\n");
        } else {
            break;
        }

        vTaskDelay(TASK_CYCLE / portTICK_RATE_MS);
    }
}

// WiFi callback function
void event_handler(System_Event_t *event)
{
    switch (event->event_id) {
        case EVENT_STAMODE_GOT_IP:
            tc_iot_hal_printf("WiFi connected\n");
            sntpfn();
            got_ip_flag = 1;
            /* if (xHandleTaskLight == NULL) { */
                /* xTaskCreate(light_demo, "light_demo", 8192, NULL, 5, &xHandleTaskLight); */
                /* tc_iot_hal_printf("\nMQTT task started...\n"); */
            /* } else { */
                /* tc_iot_hal_printf("\nMQTT task already started...\n"); */
            /* } */
            break;

        case EVENT_STAMODE_DISCONNECTED:
            tc_iot_hal_printf("WiFi disconnected\n");
            got_ip_flag = 0;
            /* if (xHandleTaskLight != NULL) { */
                /* vTaskDelete(xHandleTaskLight); */
                /* xHandleTaskLight = NULL; */
                /* tc_iot_hal_printf("\nMQTT task deleted...\n"); */
            /* } */
            /* wifi_station_connect(); */
            break;

        default:
            break;
    }
}

void initialize_wifi(void)
{
    wifi_set_opmode(STATION_MODE);

    // set AP parameter
    struct station_config config;
    bzero(&config, sizeof(struct station_config));
    sprintf(config.ssid, WIFI_SSID);
    sprintf(config.password, WIFI_PASSWORD);
    wifi_station_set_config(&config);

    wifi_station_set_auto_connect(true);
    wifi_station_set_reconnect_policy(true);
    wifi_set_event_handler_cb(event_handler);
}

void command_callback(char * buffer) {
    const char * WIFI_COMMAND = "AT+WIFI";
    const char * MQTT_COMMAND = "AT+MQTT";

    if (0 == memcmp(buffer, WIFI_COMMAND, strlen(WIFI_COMMAND))) {
        printf("doing:%s\n",buffer);
        initialize_wifi();
        wifi_station_connect();
    } else if (0 == memcmp(buffer, MQTT_COMMAND, strlen(MQTT_COMMAND))) {
        printf("doing:%s\n",buffer);
        if (!got_ip_flag) {
            printf("ERROR %s\n", "network not ready");
            return;
        }
        if (xHandleTaskLight == NULL) {
            xTaskCreate(light_demo, "light_demo", 8192, NULL, 5, &xHandleTaskLight);
            tc_iot_hal_printf("\nMQTT task started...\n");
        } else {
            tc_iot_hal_printf("\nMQTT task already started...\n");
        }
    } else {
        printf("AT+COMMAND received:%s\n",buffer);
    }
    return;
}


void heap_check_task(void *para)
{
    while (1) {
        vTaskDelay(TASK_CYCLE / portTICK_RATE_MS);
        tc_iot_hal_printf("[heap check task] free heap size:%d\n", system_get_free_heap_size());
    }
}


#ifdef TM1638_TASK
void tm1638_task(void *para)
{
    TM_TM1638(4,14,5,true,7);
    TM_setDisplayToHexNumber(0x1234ABCD, 0xF0);

    while (1) {
        vTaskDelay(TASK_CYCLE / portTICK_RATE_MS);
        byte keys = TM_getButtons();
        tc_iot_hal_printf("[tm1638] buttons:%d,\n", (int)keys);

        TM_setLEDs(((keys & 0xF0) << 8) | (keys & 0xF));
    }
}
#endif

void user_init(void)
{
    // default baudrate: 74880, change it if necessary
    //    UART_SetBaudrate(0, 115200);
    //    UART_SetBaudrate(1, 115200);

    extern unsigned int max_content_len;    // maxium fragment length in bytes, more info see as RFC 6066: part 4
    max_content_len = 4 * 1024;

    /* hal_micros_set_default_time();  // startup millisecond timer */
    tc_iot_hal_printf("SDK version:%s \n", system_get_sdk_version());
    tc_iot_hal_printf("\n******************************************  \n  SDK compile time:%s %s\n******************************************\n\n", __DATE__, __TIME__);
    /* initialize_wifi(); */

    got_ip_flag = 0;

    user_uart_init_new();

#ifdef TM1638_TASK
    xTaskCreate(tm1638_task, "tm1638_task", 128, NULL, 5, NULL);
#endif

#if HEAP_CHECK_TASK
    /* xTaskCreate(heap_check_task, "heap_check_task", 128, NULL, 5, NULL); */
#endif
}



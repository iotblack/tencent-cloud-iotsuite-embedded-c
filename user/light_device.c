#include "tc_iot_device_config.h"
#include "tc_iot_device_logic.h"
#include "gpio.h"
#include "tc_iot_export.h"

#define TC_IOT_TROUBLE_SHOOTING_URL "https://git.io/vN9le"

extern tc_iot_shadow_config g_tc_iot_shadow_config;

/* 循环退出标识 */
static volatile int stop = 0;

/**
 * @brief operate_light 操作灯光控制开关
 *
 * @param light 灯状态数据
 */
void operate_device(tc_iot_shadow_local_data * light) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    int i = 0;
    int color = light->color % 3;
    const char * ansi_color_name = NULL;
    char brightness_bar[]      = "||||||||||||||||||||";
    int brightness_bar_len = strlen(brightness_bar);
    int brightness_bar_left_len = 0;
    int pinid = 2; /*4=blue, 0=green, 2=red*/
    int gpiolist[] = {};

    switch(light->color) {
        case TC_IOT_PROP_color_red:
            ansi_color_name = "RED";
            pinid = 4;
            break;
        case TC_IOT_PROP_color_green:
            ansi_color_name = "GREEN";
            pinid = 0;
            break;
        case TC_IOT_PROP_color_blue:
            ansi_color_name = "BLUE";
            pinid = 2;
            break;
        default:
            ansi_color_name = "UNKNOWN";
            break;
    }

    /* 灯光亮度显示条 */
    brightness_bar_len = light->brightness >= 100?brightness_bar_len:(int)((light->brightness * brightness_bar_len)/100);
    for (i = brightness_bar_len; i < strlen(brightness_bar); i++ ){
        if (brightness_bar[i] == '\0') {
            break;
        }
        brightness_bar[i] = '-';
    }

    if (light->device_switch) {
        /*GPIO_OUTPUT_SET(GPIO_ID_PIN(2),  0);*/
        /*GPIO_OUTPUT_SET(GPIO_ID_PIN(4),  0);*/
        /*GPIO_OUTPUT_SET(GPIO_ID_PIN(5),  0);*/
        /*GPIO_OUTPUT_SET(GPIO_ID_PIN(0),  0);*/
        /*GPIO_OUTPUT_SET(GPIO_ID_PIN(16), 0);*/
        /*GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 0);*/
        /*GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 0);*/
        /*GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0);*/
        /*GPIO_OUTPUT_SET(GPIO_ID_PIN(13), 0);*/

        GPIO_OUTPUT_SET(GPIO_ID_PIN(2),  1);
        GPIO_OUTPUT_SET(GPIO_ID_PIN(0),  1);
        GPIO_OUTPUT_SET(GPIO_ID_PIN(4),  1);

        GPIO_OUTPUT_SET(GPIO_ID_PIN(pinid), 0);

        /* 灯光开启式，按照控制参数展示 */
        tc_iot_hal_printf( "%04d-%02d-%02d %02d:%02d:%02d ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        tc_iot_hal_printf(
                "[  lighting  ]|[color:%s]|[brightness:%s]\n",
                ansi_color_name,
                brightness_bar
                );
    } else {
        GPIO_OUTPUT_SET(GPIO_ID_PIN(2),  1);
        GPIO_OUTPUT_SET(GPIO_ID_PIN(0),  1);
        GPIO_OUTPUT_SET(GPIO_ID_PIN(4),  1);
        /*GPIO_OUTPUT_SET(GPIO_ID_PIN(5),  1);*/
        /*GPIO_OUTPUT_SET(GPIO_ID_PIN(16), 1);*/
        /*GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 1);*/
        /*GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 1);*/
        /*GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1);*/
        /*GPIO_OUTPUT_SET(GPIO_ID_PIN(13), 1);*/
        /* 灯处于关闭状态时的展示 */
        tc_iot_hal_printf( "%04d-%02d-%02d %02d:%02d:%02d ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        tc_iot_hal_printf(
                "[  light off ]|[color:%s]|[brightness:%s]\n",
                ansi_color_name,
                brightness_bar
                );
    }
}

void light_demo(void *pvParameter) {
    tc_iot_mqtt_client_config * p_client_config;
    bool token_defined;
    int ret;
    long timestamp = 0;
    long nonce = 0; 

start:
    p_client_config = &(g_tc_iot_shadow_config.mqtt_client_config);
    timestamp = tc_iot_hal_timestamp(NULL);
    tc_iot_hal_srandom(timestamp);
    nonce = tc_iot_hal_random();

    /* 根据 product id 和device name 定义，生成发布和订阅的 Topic 名称。 */
    snprintf(g_tc_iot_shadow_config.sub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN, TC_IOT_SUB_TOPIC_FMT,
            p_client_config->device_info.product_id,p_client_config->device_info.device_name);
    snprintf(g_tc_iot_shadow_config.pub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN, TC_IOT_PUB_TOPIC_FMT,
            p_client_config->device_info.product_id,p_client_config->device_info.device_name);

    /* 判断是否需要获取动态 token */
    token_defined = strlen(p_client_config->device_info.username) && strlen(p_client_config->device_info.password);

    if (!token_defined) {
        /* 获取动态 token */
        tc_iot_hal_printf("requesting username and password for mqtt.\n");
        ret = http_refresh_auth_token_with_expire(
                TC_IOT_CONFIG_AUTH_API_URL, NULL,
                timestamp, nonce,
                &p_client_config->device_info, TC_IOT_TOKEN_MAX_EXPIRE_SECOND);
        if (ret != TC_IOT_SUCCESS) {
            tc_iot_hal_printf("refresh token failed, trouble shooting guide: " "%s#%d\n", TC_IOT_TROUBLE_SHOOTING_URL, ret);
            goto exit ;
        }
        tc_iot_hal_printf("request username and password for mqtt success.\n");
    } else {
        tc_iot_hal_printf("username & password using: %s %s\n", p_client_config->device_info.username, p_client_config->device_info.password);
    }

    ret = tc_iot_server_init(tc_iot_get_shadow_client(), &g_tc_iot_shadow_config);
    if (ret != TC_IOT_SUCCESS) {
        tc_iot_hal_printf("tc_iot_server_init failed, trouble shooting guide: " "%s#%d\n", TC_IOT_TROUBLE_SHOOTING_URL, ret);
        goto exit ;
    }

    while (!stop) {
        ret = tc_iot_server_loop(tc_iot_get_shadow_client(),2000);
        if (ret == TC_IOT_MQTT_RECONNECT_TIMEOUT) {
            TC_IOT_LOG_ERROR("reconnect timeout ret=%d, try restarting", ret);
            goto start;
        } else if ( ret != TC_IOT_SUCCESS) {
            TC_IOT_LOG_ERROR("ret=%d", ret);
        }
    }

exit:
    tc_iot_server_destroy(tc_iot_get_shadow_client());
    vTaskDelete(NULL);
}



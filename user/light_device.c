#include "tc_iot_device_config.h"
#include "tc_iot_device_logic.h"
#include "tc_iot_export.h"


/* anis color control codes */
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_256_FORMAT   "\x1b[38;5;%dm"

#define TC_IOT_TROUBLE_SHOOTING_URL "https://git.io/vN9le"


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
    int color = light->color % 3;
    const char * ansi_color = NULL;
    const char * ansi_color_name = NULL;
    static const char * brightness_bar      = "||||||||||||||||||||";
    static const char * brightness_bar_left = "--------------------";
    int brightness_bar_len = strlen(brightness_bar);
    int brightness_bar_left_len = 0;

    switch(light->color) {
        case TC_IOT_PROP_color_red:
            ansi_color = ANSI_COLOR_RED;
            ansi_color_name = "RED";
            break;
        case TC_IOT_PROP_color_green:
            ansi_color = ANSI_COLOR_GREEN;
            ansi_color_name = "GREEN";
            break;
        case TC_IOT_PROP_color_blue:
            ansi_color = ANSI_COLOR_BLUE;
            ansi_color_name = "BLUE";
            break;
        default:
            ansi_color = ANSI_COLOR_YELLOW;
            ansi_color_name = "UNKNOWN";
            break;
    }

    /* 灯光亮度显示条 */
    brightness_bar_len = light->brightness >= 100?brightness_bar_len:(int)((light->brightness * brightness_bar_len)/100);
    brightness_bar_left_len = strlen(brightness_bar) - brightness_bar_len;

    if (light->device_switch) {
        /* 灯光开启式，按照控制参数展示 */
        tc_iot_hal_printf( "%s%04d-%02d-%02d %02d:%02d:%02d " ANSI_COLOR_RESET, 
                ansi_color,
                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        tc_iot_hal_printf(
                "%s[  lighting  ]|[color:%s]|[brightness:%.*s%.*s]\n" 
                ANSI_COLOR_RESET, 
                ansi_color,
                ansi_color_name,
                brightness_bar_len, brightness_bar,
                brightness_bar_left_len,brightness_bar_left
                );
    } else {
        /* 灯处于关闭状态时的展示 */
        tc_iot_hal_printf( ANSI_COLOR_YELLOW "%04d-%02d-%02d %02d:%02d:%02d " ANSI_COLOR_RESET, 
                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        tc_iot_hal_printf(
                ANSI_COLOR_YELLOW "[" "light is off" "]|[color:%s]|[brightness:%.*s%.*s]\n" ANSI_COLOR_RESET , 
                ansi_color_name,
                brightness_bar_len, brightness_bar,
                brightness_bar_left_len, brightness_bar_left
                );
    }
}

void light_demo(void *pvParameter) {
    tc_iot_mqtt_client_config * p_client_config;
    bool token_defined;
    int ret;
    long timestamp = tc_iot_hal_timestamp(NULL);
    tc_iot_hal_srandom(timestamp);
    long nonce = tc_iot_hal_random();

    /* 设定 will ，当灯设备异常退出时，MQ 服务端自动往设备影子数据中，写入 abnormal_exit 的离线状态。 */
    static const char * will_message = "{\"method\":\"update\",\"state\":{\"reported\":{\"status\":\"abnormal_exit\"}}}";

    p_client_config = &(g_client_config.mqtt_client_config);


    /* 根据 product id 和device name 定义，生成发布和订阅的 Topic 名称。 */
    snprintf(g_client_config.sub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN, TC_IOT_SUB_TOPIC_FMT, 
            p_client_config->device_info.product_id,p_client_config->device_info.device_name);
    snprintf(g_client_config.pub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN, TC_IOT_PUB_TOPIC_FMT, 
            p_client_config->device_info.product_id,p_client_config->device_info.device_name);

    /* 判断是否需要获取动态 token */
    token_defined = strlen(p_client_config->device_info.username) && strlen(p_client_config->device_info.password);
    
    p_client_config->willFlag = 1;
    p_client_config->will.message.cstring = (char*)will_message;
    p_client_config->will.qos = 1;
    p_client_config->will.retained = 0;
    p_client_config->will.topicName.cstring = (char*)g_client_config.pub_topic;

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

    ret = tc_iot_server_init(&g_client_config);
    if (ret != TC_IOT_SUCCESS) {
        tc_iot_hal_printf("tc_iot_server_init failed, trouble shooting guide: " "%s#%d\n", TC_IOT_TROUBLE_SHOOTING_URL, ret);
        goto exit ;
    }

    while (!stop) {
        tc_iot_server_loop(200);
    }

exit:
    tc_iot_server_destroy();
    vTaskDelete(NULL);
}



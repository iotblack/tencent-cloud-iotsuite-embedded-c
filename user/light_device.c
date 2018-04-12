#include "tc_iot_device_config.h"
#include "tc_iot_device_logic.h"
#include "gpio.h"
#include "tc_iot_export.h"

#define TC_IOT_TROUBLE_SHOOTING_URL "https://git.io/vN9le"

/*D1	IO, SCL	GPIO5*/
/*D2	IO, SDA	GPIO4*/
/*D3	IO, 10k Pull-up	GPIO0*/
/*D4	IO, 10k Pull-up, BUILTIN_LED	GPIO2*/
/*D5	IO, SCK	GPIO14*/
/*D6	IO, MISO	GPIO12*/
/*D7	IO, MOSI	GPIO13*/
/*D8	IO, 10k Pull-down, SS	GPIO15*/

#define DP_D1  5
#define DP_D2  4
#define DP_D3  0

#define DP_D4  2
#define DP_D5  14
#define DP_D6  12
#define DP_D7  13
#define DP_D8  15

#define DEVICE_PIN_LED_RED     DP_D1
#define DEVICE_PIN_LED_GREEN   DP_D2
#define DEVICE_PIN_LED_BLUE    DP_D3

#define DEVICE_SWITCH_LED_ON    0
#define DEVICE_SWITCH_LED_OFF   1

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
    int pinid = DEVICE_PIN_LED_RED;
    int gpiolist[] = {};

    switch(light->color) {
        case TC_IOT_PROP_color_red:
            ansi_color_name = "RED";
            pinid = DEVICE_PIN_LED_RED;
            break;
        case TC_IOT_PROP_color_green:
            ansi_color_name = "GREEN";
            pinid = DEVICE_PIN_LED_GREEN;
            break;
        case TC_IOT_PROP_color_blue:
            ansi_color_name = "BLUE";
            pinid = DEVICE_PIN_LED_BLUE;
            break;
        default:
            ansi_color_name = "UNKNOWN";
            break;
    }

    if (light->device_switch) {

        GPIO_OUTPUT_SET(GPIO_ID_PIN(DEVICE_PIN_LED_RED),   DEVICE_SWITCH_LED_OFF);
        GPIO_OUTPUT_SET(GPIO_ID_PIN(DEVICE_PIN_LED_GREEN), DEVICE_SWITCH_LED_OFF);
        GPIO_OUTPUT_SET(GPIO_ID_PIN(DEVICE_PIN_LED_BLUE),  DEVICE_SWITCH_LED_OFF);

        if (pinid == DEVICE_PIN_LED_RED) {
            GPIO_OUTPUT_SET(GPIO_ID_PIN(DEVICE_PIN_LED_RED), DEVICE_SWITCH_LED_ON);
        } else if (pinid == DEVICE_PIN_LED_GREEN) {
            GPIO_OUTPUT_SET(GPIO_ID_PIN(DEVICE_PIN_LED_GREEN), DEVICE_SWITCH_LED_ON);
        } else if (pinid == DEVICE_PIN_LED_BLUE) {
            GPIO_OUTPUT_SET(GPIO_ID_PIN(DEVICE_PIN_LED_BLUE), DEVICE_SWITCH_LED_ON);
        } else {
            TC_IOT_LOG_ERROR("pin=%d unkown", pinid);
        }

        /* 灯光开启式，按照控制参数展示 */
        tc_iot_hal_printf( "%04d-%02d-%02d %02d:%02d:%02d ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        tc_iot_hal_printf(
                "[  lighting  ]|[color:%s]|[brightness:%s]\n",
                ansi_color_name,
                brightness_bar
                );
    } else {
        GPIO_OUTPUT_SET(GPIO_ID_PIN(DEVICE_PIN_LED_RED),   DEVICE_SWITCH_LED_OFF);
        GPIO_OUTPUT_SET(GPIO_ID_PIN(DEVICE_PIN_LED_GREEN), DEVICE_SWITCH_LED_OFF);
        GPIO_OUTPUT_SET(GPIO_ID_PIN(DEVICE_PIN_LED_BLUE),  DEVICE_SWITCH_LED_OFF);

        /* 灯处于关闭状态时的展示 */
        tc_iot_hal_printf( "%04d-%02d-%02d %02d:%02d:%02d ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        tc_iot_hal_printf(
                "[  light off ]|[color:%s]|[brightness:%s]\n",
                ansi_color_name,
                brightness_bar
                );
    }
}

void test_light(void) {
    int light_pins[] = {DEVICE_PIN_LED_RED,DEVICE_PIN_LED_GREEN,DEVICE_PIN_LED_BLUE};
    int i = 0;
    for(i = 0; i < 3; i++) {
        GPIO_OUTPUT_SET(GPIO_ID_PIN(DEVICE_PIN_LED_RED),   DEVICE_SWITCH_LED_OFF);
        GPIO_OUTPUT_SET(GPIO_ID_PIN(DEVICE_PIN_LED_GREEN), DEVICE_SWITCH_LED_OFF);
        GPIO_OUTPUT_SET(GPIO_ID_PIN(DEVICE_PIN_LED_BLUE),  DEVICE_SWITCH_LED_OFF);

        GPIO_OUTPUT_SET(GPIO_ID_PIN(light_pins[i]), DEVICE_SWITCH_LED_ON);
        tc_iot_hal_sleep_ms(1000);
    }

    GPIO_OUTPUT_SET(GPIO_ID_PIN(DEVICE_PIN_LED_RED), DEVICE_SWITCH_LED_ON);
    GPIO_OUTPUT_SET(GPIO_ID_PIN(DEVICE_PIN_LED_GREEN), DEVICE_SWITCH_LED_ON);
    GPIO_OUTPUT_SET(GPIO_ID_PIN(DEVICE_PIN_LED_BLUE), DEVICE_SWITCH_LED_ON);

    tc_iot_hal_sleep_ms(1000);
    GPIO_OUTPUT_SET(GPIO_ID_PIN(DEVICE_PIN_LED_RED),   DEVICE_SWITCH_LED_OFF);
    GPIO_OUTPUT_SET(GPIO_ID_PIN(DEVICE_PIN_LED_GREEN), DEVICE_SWITCH_LED_OFF);
    GPIO_OUTPUT_SET(GPIO_ID_PIN(DEVICE_PIN_LED_BLUE),  DEVICE_SWITCH_LED_OFF);
}

void light_demo(void *pvParameter) {
    tc_iot_mqtt_client_config * p_client_config;
    bool token_defined;
    int ret;
    long timestamp = 0;
    long nonce = 0;

        tc_iot_hal_printf("starting mqtt.\n");
start:
    p_client_config = &(g_tc_iot_shadow_config.mqtt_client_config);
    tc_iot_hal_printf("get timestamp\n");
    timestamp = tc_iot_hal_timestamp(NULL);
    tc_iot_hal_srandom(timestamp);
    tc_iot_hal_printf("get random\n");
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

    test_light();
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



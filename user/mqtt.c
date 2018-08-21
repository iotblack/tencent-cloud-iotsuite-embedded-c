#include "tc_iot_device_config.h"
#include "gpio.h"
#include "tc_iot_export.h"

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

#define DEVICE_SWITCH_LED_ON    1
#define DEVICE_SWITCH_LED_OFF   0

typedef unsigned long timestamp_t;
typedef struct _smartbox_data {
    bool door_switch;
    char status[6];
    char fault[20];
} smartbox_data;

smartbox_data g_tc_smartbox_data = {
    false,
    "full",
    "no",
};


#define TC_IOT_TROUBLE_SHOOTING_URL "https://git.io/vN9le"

extern void parse_command(tc_iot_mqtt_client_config * config, int argc, char ** argv);

bool use_static_token = false;

void my_default_msg_handler(tc_iot_message_data*);
void my_disconnect_handler(tc_iot_mqtt_client* c, void* ctx);
void _refresh_token();

int run_mqtt(tc_iot_mqtt_client_config* p_client_config);

tc_iot_mqtt_client_config g_client_config = {
    {
        /* device info*/
        TC_IOT_CONFIG_DEVICE_SECRET, TC_IOT_CONFIG_DEVICE_PRODUCT_ID,
        TC_IOT_CONFIG_DEVICE_NAME, TC_IOT_CONFIG_DEVICE_CLIENT_ID,
        TC_IOT_CONFIG_DEVICE_USER_NAME, TC_IOT_CONFIG_DEVICE_PASSWORD, 0,
        TC_IOT_CONFIG_AUTH_MODE, TC_IOT_CONFIG_REGION, TC_IOT_CONFIG_AUTH_API_URL,
    },
    TC_IOT_CONFIG_MQ_SERVER_HOST,
    TC_IOT_CONFIG_MQ_SERVER_PORT,
    TC_IOT_CONFIG_COMMAND_TIMEOUT_MS,
    TC_IOT_CONFIG_TLS_HANDSHAKE_TIMEOUT_MS,
    TC_IOT_CONFIG_KEEP_ALIVE_INTERVAL_SEC,
    TC_IOT_CONFIG_CLEAN_SESSION,
    TC_IOT_CONFIG_USE_TLS,
    TC_IOT_CONFIG_AUTO_RECONNECT,
    TC_IOT_CONFIG_ROOT_CA,
    TC_IOT_CONFIG_CLIENT_CRT,
    TC_IOT_CONFIG_CLIENT_KEY,
    my_disconnect_handler,
    my_default_msg_handler,
};

char sub_topic[TC_IOT_MAX_MQTT_TOPIC_LEN+1] = TC_IOT_CONFIG_DEVICE_PRODUCT_ID "/" TC_IOT_CONFIG_DEVICE_NAME "/cmd";
char pub_topic[TC_IOT_MAX_MQTT_TOPIC_LEN+1] = TC_IOT_CONFIG_DEVICE_PRODUCT_ID "/" TC_IOT_CONFIG_DEVICE_NAME "/update";

void app_main(void *pvParameter) {
    int ret;
    long timestamp = tc_iot_hal_timestamp(NULL);
    tc_iot_hal_srandom(timestamp);
    long nonce = tc_iot_hal_random();

    tc_iot_mqtt_client_config * p_client_config;

    p_client_config = &(g_client_config);
    use_static_token = strlen(p_client_config->device_info.username) && strlen(p_client_config->device_info.password);
    if (!use_static_token) {
        tc_iot_hal_printf("requesting username and password for mqtt by token interface.\n");
        ret = http_refresh_auth_token_with_expire(
                TC_IOT_CONFIG_AUTH_API_URL, //TC_IOT_CONFIG_AUTH_API_URL_DEBUG
                TC_IOT_CONFIG_ROOT_CA,
                timestamp, nonce,
                &p_client_config->device_info,
                TC_IOT_TOKEN_MAX_EXPIRE_SECOND
                );
        if (ret != TC_IOT_SUCCESS) {
            tc_iot_hal_printf("refresh token failed, trouble shooting guide: " "%s#%d\n", TC_IOT_TROUBLE_SHOOTING_URL, ret);
            return ;
        }
        tc_iot_hal_printf("request username and password for mqtt success.\n");
    } else {
        tc_iot_hal_printf("username & password using: %s %s\n", p_client_config->device_info.username, p_client_config->device_info.password);
    }

    tc_iot_hal_printf("sub topic: %s\n", sub_topic);
    tc_iot_hal_printf("pub topic: %s\n", pub_topic);
    run_mqtt(&g_client_config);
    tc_iot_hal_printf("Fatal error: mqtt loop exited.");
    vTaskDelete(NULL);
}

void report_status(tc_iot_mqtt_client * p_client)
{
    int ret = 0;
    char action_report[256];
    /* 具体消息格式可自行定义，注意保持为 JSON 格式*/
    tc_iot_hal_snprintf(action_report, sizeof(action_report),
            "{\"door_switch\":%s,\"status\":\"%s\",\"fault\":\"%s\"}",
            g_tc_smartbox_data.door_switch?"true":"false",
            g_tc_smartbox_data.status,
            g_tc_smartbox_data.fault);

    tc_iot_mqtt_message pubmsg;
    memset(&pubmsg, '\0', sizeof(pubmsg));
    pubmsg.payload = action_report;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = TC_IOT_QOS1;
    pubmsg.retained = 0;
    pubmsg.dup = 0;
    tc_iot_hal_printf("[c->s] publishing msg\n%s:%s\n",pub_topic, action_report);
    ret = tc_iot_mqtt_client_publish(p_client, pub_topic, &pubmsg);
    if (TC_IOT_SUCCESS != ret) {
        if (ret != TC_IOT_MQTT_RECONNECT_IN_PROGRESS) {
            tc_iot_hal_printf("publish to topic %s failed, trouble shooting guide: " "%s#%d\n", 
                    pub_topic, TC_IOT_TROUBLE_SHOOTING_URL, ret);
        } else {
            tc_iot_hal_printf("publish to topic %s failed, try reconnecting, "
                    "or visit trouble shooting guide: " "%s#%d\n", pub_topic, TC_IOT_TROUBLE_SHOOTING_URL, ret);
        }
    }
}

void led_flash(int pin, int delay_ms) {
    GPIO_OUTPUT_SET(GPIO_ID_PIN(pin),   DEVICE_SWITCH_LED_ON);
    tc_iot_hal_sleep_ms(delay_ms);
    GPIO_OUTPUT_SET(GPIO_ID_PIN(pin),   DEVICE_SWITCH_LED_OFF);
}

void show_some_response(smartbox_data * data) {
    int i = 0;
    if (data->door_switch) {
        for (i = 0; i < 3; i++) {
            led_flash(DEVICE_PIN_LED_RED, 1000);
            led_flash(DEVICE_PIN_LED_GREEN, 1000);
            led_flash(DEVICE_PIN_LED_BLUE, 1000);
        }
    }
}

void _on_message_received(tc_iot_message_data* md) {
    tc_iot_mqtt_message* message = md->message;
    jsmntok_t  json_token[TC_IOT_MAX_JSON_TOKEN_COUNT];
    char field_buf[32];
    int field_index = 0;
    int ret = 0;
    char * start = NULL;
    char * payload = NULL;
    /* int len = 0; */
    tc_iot_mqtt_client* p_client = NULL;

    tc_iot_hal_printf("[s->c] %s\n",  (char*)message->payload);

    p_client = md->mqtt_client;
    memset(field_buf, 0, sizeof(field_buf));

    /* 有效性检查 */
    ret = tc_iot_json_parse(message->payload, message->payloadlen, json_token, TC_IOT_ARRAY_LENGTH(json_token));
    if (ret <= 0) {
        TC_IOT_LOG_ERROR("parse payload failed:ret=%d", ret)
        return ;
    }

    payload = (char*)message->payload;
    field_index = tc_iot_json_find_token(payload, json_token, ret, "door_switch", field_buf, sizeof(field_buf));
    if (field_index > 0 ) {
        start = payload + json_token[field_index].start;
        /* len = json_token[field_index].end - json_token[field_index].start; */
        if (start[0] == 't') {
            g_tc_smartbox_data.door_switch = true;
            show_some_response(&g_tc_smartbox_data);
        } else {
            if (!g_tc_smartbox_data.door_switch) {
                tc_iot_hal_snprintf(g_tc_smartbox_data.fault, sizeof(g_tc_smartbox_data.fault), "bad state");
            } else {
                g_tc_smartbox_data.door_switch = false;
                tc_iot_hal_snprintf(g_tc_smartbox_data.status, sizeof(g_tc_smartbox_data.status), "free");
            }
        }
    }

    report_status(p_client);
}

void my_disconnect_handler(tc_iot_mqtt_client* c, void* ctx) {
    /* 自动刷新 password*/
    if (!use_static_token && tc_iot_mqtt_get_auto_reconnect(c)) {
        /* _refresh_token(); */
    }
}

void my_default_msg_handler(tc_iot_message_data * md) {
    tc_iot_mqtt_message* message = md->message;
    tc_iot_hal_printf("UNHANDLED [s->c] %s\n", (char*)message->payload);
}


int stop = 0;

int run_mqtt(tc_iot_mqtt_client_config* p_client_config) {
    int ret;
    int timeout = 200;
    int i = 0;
    const int MQTT_CONSTRUCT_MAX_RETRY = 5;

    tc_iot_mqtt_client client;
    tc_iot_mqtt_client* p_client = &client;
    do {
        ret = tc_iot_mqtt_client_construct(p_client, p_client_config);
        if (ret != TC_IOT_SUCCESS) {
            tc_iot_hal_printf("connect MQTT server failed, retrying...\n");
            tc_iot_hal_sleep_ms(1000);
        } else {
            break;
        }
    } while(true);
    

    ret = tc_iot_mqtt_client_subscribe(p_client, sub_topic, TC_IOT_QOS1,
                                           _on_message_received, NULL);
    if (ret != TC_IOT_SUCCESS) {
        tc_iot_hal_printf("subscribe topic %s failed, trouble shooting guide: " "%s#%d\n", 
                sub_topic, TC_IOT_TROUBLE_SHOOTING_URL, ret);
        return TC_IOT_FAILURE;
    }

    tc_iot_mqtt_client_yield(p_client, timeout);
    /* report_status(p_client); */

    while (!stop) {
        tc_iot_mqtt_client_yield(p_client, timeout);
    }

    tc_iot_mqtt_client_disconnect(p_client);
    return 0;
}

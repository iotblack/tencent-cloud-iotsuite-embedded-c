#ifndef TC_IOT_DEMO_LIGHT_H
#define TC_IOT_DEMO_LIGHT_H

#include "tc_iot_inc.h"

#define TC_IOT_LIGHT_NAME_LEN  25

typedef struct _tc_iot_demo_light {
    bool          light_switch; /* true means light on, false means light off. */
    char          name[TC_IOT_LIGHT_NAME_LEN];  /* name or id */
    unsigned int  color; /* ansi color index: 0~255, if greater use color=color%256 */
    float         brightness; /* light brightness*/
}tc_iot_demo_light;

#define HEAP_CHECK_TASK 0
#define TASK_CYCLE 2000
#define WIFI_SSID       "wifitest"       // type:string, your AP/router SSID to config your device networking
#define WIFI_PASSWORD   "wifitest$-123456"       // type:string, your AP/router password

#endif /* end of include guard */

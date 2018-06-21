#ifndef TC_IOT_DEVICE_CONFIG_H
#define TC_IOT_DEVICE_CONFIG_H

/* 设备激活及获取 secret 接口，地址格式为：<机房标识>.auth-device-iot.tencentcloudapi.com/secret */
/* Token接口，地址格式为：<机房标识>.auth-device-iot.tencentcloudapi.com/token */
/* 机房标识：
    广州机房=gz
    北京机房=bj
    ...
*/
#ifdef ENABLE_TLS
#define TC_IOT_CONFIG_AUTH_API_URL "https:///*${template_config.Region}*/.auth-device-iot.tencentcloudapi.com/token"
#define TC_IOT_CONFIG_ACTIVE_API_URL "https:///*${template_config.Region}*/.auth-device-iot.tencentcloudapi.com/secret"
#else
#define TC_IOT_CONFIG_AUTH_API_URL "http:///*${template_config.Region}*/.auth-device-iot.tencentcloudapi.com/token"
#define TC_IOT_CONFIG_ACTIVE_API_URL "http:///*${template_config.Region}*/.auth-device-iot.tencentcloudapi.com/secret"
#endif

#define TC_IOT_CONFIG_ACTIVE_API_URL_DEBUG   "http:///*${template_config.Region}*/.auth.iot.cloud.tencent.com/secret"
#define TC_IOT_CONFIG_AUTH_API_URL_DEBUG	 "http:///*${template_config.Region}*/.auth.iot.cloud.tencent.com/token"

/************************************************************************/
/**********************************必填项********************************/

#ifdef ENABLE_TLS
/* 是否启用TLS用于MQTT请求*/
#define TC_IOT_CONFIG_USE_TLS 1
#else
#define TC_IOT_CONFIG_USE_TLS 0
#endif

#if TC_IOT_CONFIG_USE_TLS
/* MQ服务的TLS端口一般为8883*/
#define TC_IOT_CONFIG_SERVER_PORT 8883
#else
/* MQ服务的默认端口一般为1883*/
#define TC_IOT_CONFIG_SERVER_PORT 1883
#endif


/* 以下配置需要先在官网创建产品和设备，然后获取相关信息更新*/
/* MQ服务地址，可以在产品“基本信息页”->“mqtt链接地址”位置找到。*/
#define TC_IOT_CONFIG_SERVER_HOST "/*${template_config.Domain}*/"
/*#define TC_IOT_CONFIG_SERVER_HOST "localhost"*/
/* 产品id，可以在产品“基本信息页”->“产品id”位置找到*/
#define TC_IOT_CONFIG_DEVICE_PRODUCT_ID "/*${template_config.ProductId}*/"
/* 产品id，可以在产品“基本信息页”->“产品key”位置找到*/
#define TC_IOT_CONFIG_DEVICE_PRODUCT_KEY "/*${template_config.ProductKey}*/"

/* 设备密钥，可以在产品“设备管理”->“设备证书”->“Device Secret”位置找到*/
#define TC_IOT_CONFIG_DEVICE_SECRET "00000000000000000000000000000000"

/* 设备名称，可以在产品“设备管理”->“设备名称”位置找到*/
#define TC_IOT_CONFIG_DEVICE_NAME "device_name"

/* client id 由两部分组成，组成形式为“ProductKey@DeviceName” */
#define TC_IOT_CONFIG_DEVICE_CLIENT_ID TC_IOT_CONFIG_DEVICE_PRODUCT_KEY "@" TC_IOT_CONFIG_DEVICE_NAME

/************************************************************************/
/**********************************选填项********************************/
/* 关于username和password：*/
/* 1)如果是通过TC_IOT_CONFIG_AUTH_API_URL接口，动态获取的，以下两个参数可不用填写*/
/* 2)如果有预先申请好的固定username和password，可以把获取到的固定参数填写到如下位置*/
#if 1==/*${template_config.AuthType}*/
/* Token 模式 */
#define TC_IOT_CONFIG_DEVICE_USER_NAME ""
#define TC_IOT_CONFIG_DEVICE_PASSWORD ""
#else
/* 直连模式 */
#define TC_IOT_CONFIG_DEVICE_USER_NAME "/*${template_config.Username}*/"
#define TC_IOT_CONFIG_DEVICE_PASSWORD "/*${template_config.Password}*/"
#endif
/************************************************************************/


/* connect、publish、subscribe、unsubscribe */
/* 等命令执行超时时长，单位是毫秒*/
#define TC_IOT_CONFIG_COMMAND_TIMEOUT_MS  10000
/* TLS 握手执行超时时长，单位是毫秒*/
#define TC_IOT_CONFIG_TLS_HANDSHAKE_TIMEOUT_MS  10000
/* keep alive 间隔时长，单位是秒*/
#define TC_IOT_CONFIG_KEEP_ALIVE_INTERVAL_SEC  60
/* 网络故障或服务端超时，是否自动重连*/
#define TC_IOT_CONFIG_AUTO_RECONNECT 1

#define TC_IOT_CONFIG_CLEAN_SESSION 1


/* shadow下行消息topic，mq服务端的响应和下行推送，*/
/* 都会发布到 "shadow/get/<product id>/<device name>" 这个topic*/
/* 客户端只需要订阅这个topic即可*/
#define TC_IOT_SUB_TOPIC_PREFIX "shadow/get/"
#define TC_IOT_SUB_TOPIC_FMT TC_IOT_SUB_TOPIC_PREFIX "%s/%s"
#define TC_IOT_SUB_TOPIC_DEF TC_IOT_SUB_TOPIC_PREFIX TC_IOT_CONFIG_DEVICE_PRODUCT_ID "/" TC_IOT_CONFIG_DEVICE_NAME
/* shadow上行消息topic，客户端请求服务端的消息，发到到这个topic即可*/
/* topic格式"shadow/update/<product id>/<device name>"*/
#define TC_IOT_PUB_TOPIC_PREFIX "shadow/update/"
#define TC_IOT_PUB_TOPIC_FMT TC_IOT_PUB_TOPIC_PREFIX "%s/%s"
#define TC_IOT_PUB_TOPIC_DEF TC_IOT_PUB_TOPIC_PREFIX TC_IOT_CONFIG_DEVICE_PRODUCT_ID "/" TC_IOT_CONFIG_DEVICE_NAME

/* tls 相关配置*/
/* 根证书路径*/
#define TC_IOT_CONFIG_ROOT_CA NULL
/* 客户端证书路径*/
#define TC_IOT_CONFIG_CLIENT_CRT NULL
/* 客户端私钥路径*/
#define TC_IOT_CONFIG_CLIENT_KEY NULL

#define TC_IOT_TROUBLE_SHOOTING_URL "https://git.io/vN9le"

#endif /* end of include guard */

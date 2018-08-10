##  开发准备

### SDK 获取

移植测试分支： [tencent-cloud-iotsuite-embedded-c-esp8266-test](https://github.com/iotblack/tencent-cloud-iotsuite-embedded-c/tree/dev/user)

```shell
git clone -b dev --recursive https://github.com/iotblack/tencent-cloud-iotsuite-embedded-c.git
```


### 开发环境

1. 安装esp open sdk，https://github.com/pfalcon/esp-open-sdk.git 。
2. 参见 [开发准备](https://github.com/tencentyun/tencent-cloud-iotsuite-embedded-c/blob/master/README.md) ，创建产品和设备，注意事项：创建产品时，“鉴权模式”建议选择“临时token模式”；
3. 点击导出按钮，导出 iot-xxxxx.json 数据模板描述文档，将 iot-xxxxx.json 文档放到 examples/linux/ 目录下覆盖 iot-product.json 文件。
4. 通过脚本自动生成配置文件。

```shell
# 进入工具脚本目录
cd tools
./tc_iot_code_generator.py -c ../examples/linux/iot-product.json code_templates/tc_iot_device_config.h
```

执行成功后会看到有如下提示信息：
```shell
加载 ../examples/linux/app/iot-product.json 文件成功
文件 ../examples/linux/app/tc_iot_device_config.h 生成成功
```

打开 tc_iot_device_config.h ，可以看到生成的如下产品相关信息：
```c
...
/* 设备激活及获取 secret 接口，地址格式为：<机房标识>.auth-device-iot.tencentcloudapi.com/secret */
/* Token接口，地址格式为：<机房标识>.auth-device-iot.tencentcloudapi.com/token */
/* 机房标识：
    广州机房=gz
    北京机房=bj
    ...
*/
#ifdef ENABLE_TLS
#define TC_IOT_CONFIG_AUTH_API_URL "https://gz.auth-device-iot.tencentcloudapi.com/token"
#define TC_IOT_CONFIG_ACTIVE_API_URL "https://gz.auth-device-iot.tencentcloudapi.com/secret"
#else
#define TC_IOT_CONFIG_AUTH_API_URL "http://gz.auth-device-iot.tencentcloudapi.com/token"
#define TC_IOT_CONFIG_ACTIVE_API_URL "http://gz.auth-device-iot.tencentcloudapi.com/secret"
#endif

#define TC_IOT_CONFIG_ACTIVE_API_URL_DEBUG   "http://gz.auth.iot.cloud.tencent.com/secret"
#define TC_IOT_CONFIG_AUTH_API_URL_DEBUG	 "http://gz.auth.iot.cloud.tencent.com/token"
...
#define TC_IOT_CONFIG_SERVER_HOST "mqtt-xxxx.ap-guangzhou.mqtt.tencentcloudmq.com"
#define TC_IOT_CONFIG_DEVICE_PRODUCT_ID "iot-yyyy"
#define TC_IOT_CONFIG_DEVICE_PRODUCT_KEY "mqtt-zzzz"
...
```

5. 修改 tc_iot_device_config.h 配置，设置 Device Name 和 Device Secret：
```c
/* 设备密钥，可以在产品“设备管理”->“设备证书”->“Device Secret”位置找到*/
#define TC_IOT_CONFIG_DEVICE_SECRET "00000000000000000000000000000000"

/* 设备名称，可以在产品“设备管理”->“设备名称”位置找到*/
#define TC_IOT_CONFIG_DEVICE_NAME "device_name"
```

6. 将examples/linux/tc_iot_device_config.h 拷贝到 user/ 目录下。
7. 打开 user/user_main.c 配置 wifi 信息：

```c
// 配置为当前可连接的wifi网络信息
#define WIFI_SSID       "wifitest"
#define WIFI_PASSWORD   "wifitest12345"
```


### 编译及运行
1. 执行下面的命令，编译示例程序：

```shell
./gen_misc.sh
```

2. 编译后，会生成 bin/upgrade/user1.2048.new.5.bin 。
3. 参考 factory.sh 脚本，将程序烧录到 flash 中。
4. 通过串口连接到固件。

### 参考开发板：
1. wemos d1 开发板： https://wiki.wemos.cc/products:d1:d1_mini


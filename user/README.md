##  开发准备

### SDK 获取

腾讯云 iotsuite C语言版 SDK的下载地址： [tencent-cloud-iotsuite-embedded-c.git](https://github.com/tencentyun/tencent-cloud-iotsuite-embedded-c.git)

```shell
git clone -b dev --recursive https://github.com/iotblack/tencent-cloud-iotsuite-embedded-c.git
```


### 开发环境

1. 安装esp open sdk，https://github.com/pfalcon/esp-open-sdk.git 。
2. 从控制台创建产品和设备，获取对应的 MQTT Server Host、Product ID、DeviceName、DeviceSecret，详情请登录[控制台](https://console.qcloud.com/iotsuite/product)。
3. 打开 examples/linux/tc_iot_device_config.h ，配置文件，配置设备参数：
```c
/************************************************************************/
/**********************************必填项********************************/
// 以下配置需要先在官网创建产品和设备，然后获取相关信息更新
// MQ服务地址，可以在产品“基本信息”->“mqtt链接地址”位置找到。
#define TC_IOT_CONFIG_SERVER_HOST "<mqtt-xxx.ap-guangzhou.mqtt.tencentcloudmq.com>"
// MQ服务端口，直连一般为1883，无需改动
#define TC_IOT_CONFIG_SERVER_PORT 1883
// 产品id，可以在产品“基本信息”->“产品id”位置找到
#define TC_IOT_CONFIG_DEVICE_PRODUCT_ID "<iot-xxx>"

// 设备密钥，可以在产品“设备管理”->“设备证书”->“Device Secret”位置找到
#define TC_IOT_CONFIG_DEVICE_SECRET "<0000000000000000>"
// 设备名称，可以在产品“设备管理”->“设备名称”位置找到
#define TC_IOT_CONFIG_DEVICE_NAME "<device001>"
// client id，
// 由两部分组成，组成形式为“Instanceid@DeviceID”，ClientID 的长度不超过 64个字符
// ，请不要使用不可见字符。其中
// Instanceid 为 IoT MQ 的实例 ID，可以在“基本信息”->“产品 key”位置找到。
// DeviceID 为每个设备独一无二的标识，由业务方自己指定，需保证全局唯一，例如，每个
// 传感器设备的序列号，或者设备名称等。
#define TC_IOT_CONFIG_DEVICE_CLIENT_ID "mqtt-xxx@" TC_IOT_CONFIG_DEVICE_NAME
/************************************************************************/
```
4. 将examples/linux/tc_iot_device_config.h 拷贝到 user/ 目录下。
5. 打开 user/user_main.c 配置 wifi 信息：

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
5. 参考 tools/light_controller.sh 发送控制指令。


### 参考开发板：
1. wemos d1 开发板： https://wiki.wemos.cc/products:d1:d1_mini
2. RGB LED 连线方式：
```shell
R  -> D1
G  -> D2
B  -> D3
正 -> 3V3
```



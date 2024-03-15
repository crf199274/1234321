#ifndef __mqtt_H
#define __mqtt_H

#include "arduino.h"
#include "string.h"
#include "stdlib.h"

// 协议包-封包
typedef struct mqtt_package_ {

  uint8_t  *_data; // 协议数据

  uint32_t data_ptr; // 数据指针

  uint32_t _size; // 数据包大小
  
}mqtt_package;

// 连接回包
typedef struct mqtt_ConnectACK_ {
  uint8_t _code; // 返回码
}mqtt_ConnectACK;

// 推送包
typedef struct mqtt_Publish_ {

  uint8_t Qos; // Qos等级

  uint16_t identifier; // 标识符

  char *topicName; // 主题名字

  uint32_t topicNameLen; // 主题长度

  char *payload; // 有效载荷

  uint32_t payloadLen; // 有效载荷长度
}mqtt_Publish;

typedef struct mqtt_SubscribeACK_ {

  uint16_t identifier; // 标识符

  uint8_t _code;
  
}mqtt_SubscribeACK;

typedef enum mqttPackageType_ {

  MQTT_PKT_CONNECT = 1, /**< 连接请求数据包 */
  MQTT_PKT_CONNACK,     /**< 连接确认数据包 */
  MQTT_PKT_PUBLISH,     /**< 发布数据数据包 */
  MQTT_PKT_PUBACK,      /**< 发布确认数据包 */
  MQTT_PKT_PUBREC,      /**< 发布数据已接收数据包，Qos 2时，回复MQTT_PKT_PUBLISH */
  MQTT_PKT_PUBREL,      /**< 发布数据释放数据包， Qos 2时，回复MQTT_PKT_PUBREC */
  MQTT_PKT_PUBCOMP,     /**< 发布完成数据包， Qos 2时，回复MQTT_PKT_PUBREL */
  MQTT_PKT_SUBSCRIBE,   /**< 订阅数据包 */
  MQTT_PKT_SUBACK,      /**< 订阅确认数据包 */
  MQTT_PKT_UNSUBSCRIBE, /**< 取消订阅数据包 */
  MQTT_PKT_UNSUBACK,    /**< 取消订阅确认数据包 */
  MQTT_PKT_PINGREQ,     /**< ping 数据包 */
  MQTT_PKT_PINGRESP,    /**< ping 响应数据包 */
  MQTT_PKT_DISCONNECT,  /**< 断开连接数据包 */
}mqttPackageType;

typedef enum mqttQosLevel_ {

  MQTT_QOS_LEVEL0,  /**< 最多发送一次 */
  MQTT_QOS_LEVEL1,  /**< 最少发送一次  */
  MQTT_QOS_LEVEL2   /**< 只发送一次 */
}mqttQosLevel;

typedef enum mqttConnectFlag_ {
  
    MQTT_CONNECT_CLEAN_SESSION  = 0x02,
    MQTT_CONNECT_WILL_FLAG      = 0x04,
    MQTT_CONNECT_WILL_QOS0      = 0x00,
    MQTT_CONNECT_WILL_QOS1      = 0x08,
    MQTT_CONNECT_WILL_QOS2      = 0x10,
    MQTT_CONNECT_WILL_RETAIN    = 0x20,
    MQTT_CONNECT_PASSORD        = 0x40,
    MQTT_CONNECT_USER_NAME      = 0x80
  
}mqttConnectFlag;

void mqttDeletePackage(mqtt_package *package);
void mqttDeletePublishPackage(mqtt_Publish *package);
uint8_t mqtt_UnpackageType(const char *data_ptr);
uint8_t mqtt_ConnectPackage(const char *user,const char *password,const char *devid,
                         uint16_t kaTime,bool cleanSession,bool willFlag,uint8_t willQos,bool willRetain,
                         const char *willTopic,const char *willMsg,mqtt_package *package);
void mqtt_UnConnectACK(const char *data_ptr,mqtt_ConnectACK *ack);
uint8_t mqtt_PublishPackage(bool DUP,uint8_t Qos,bool RETAIN,const char *TopicName,const char *Massage,mqtt_package *package);
uint8_t mqtt_PublishUnPackage(const char *data_ptr,mqtt_Publish *ack);
uint8_t mqtt_SubscribePackage(const char*topicFilterName,uint8_t Qos,uint16_t packageFlag,mqtt_package *package);
void mqtt_SubscribeUnPackage(const char *data_ptr,mqtt_SubscribeACK *package);
uint8_t mqtt_UnsubscribePackage(const char*topicFilterName,uint16_t packageFlag,mqtt_package *package);
uint8_t mqtt_PUBACKPackage(uint16_t packageFlag,mqtt_package *package);
uint8_t mqtt_PUBRECPackage(uint16_t packageFlag,mqtt_package *package);
uint8_t mqtt_PUBCOMPPackage(uint16_t packageFlag,mqtt_package *package);
uint8_t mqtt_PingPackage(mqtt_package *package);

#endif

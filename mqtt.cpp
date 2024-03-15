#include "mqtt.h"

bool mqttNewPackage(mqtt_package *package,uint32_t size)
{
  uint32_t i;

  if(package->_data == NULL)
  {
    package->_data = (uint8_t *)malloc(size);
    if(package->_data == NULL)
      return 1;
    else {
      package->data_ptr = 0;
      package->_size = size;
      memset(package->_data,0,package->_size);
    }
  }else {
    return 1;
  }
  return 0;
}

void mqttDeletePackage(mqtt_package *package)
{
  free(package->_data);
  package->_data = NULL;
  package->data_ptr = 0;
  package->_size = 0;
}

void mqttDeletePublishPackage(mqtt_Publish *package)
{
  free(package->topicName);
  package->topicName = NULL;
  free(package->payload);
  package->payload = NULL;
  package->Qos = 255;
  package->identifier = 255;
  package->topicNameLen = 0;
  package->payloadLen = 0;
}

void mqttFixHeaderFillRemainingLength(uint32_t len,uint8_t cnt,uint8_t *data_ptr)
{
  uint8_t i;

  for(i=0;i<cnt;i++)
  {
    if((i+1)<cnt)
      data_ptr[i] = (((len>>(i*7))%128)|0x80);
    else
      data_ptr[i] = ((len>>(i*7))%128);
  } 
}

void mqttFixHeaderReadRemainingLength(const char *data_ptr,uint32_t *packageLen,uint8_t *packageLenBytes)
{
  uint8_t byteLen=1,i=0;
  uint32_t len=0;
  
  for(i=0;i<4;i++)
  {
    if(data_ptr[i]&0x80)
    {
      byteLen++;
    }else {
      break;
    }
  }
  
  for(i=0;i<byteLen;i++)
  {
    len |= (data_ptr[i]<<(i*7));
  }

  *packageLen = len;
  *packageLenBytes = byteLen;
}

uint8_t mqtt_UnpackageType(const char *data_ptr)
{
  uint8_t _type=255;

  _type = data_ptr[0]>>4;

  if(_type < 0 || _type > 14)
    return 255;

  return _type;
}

uint8_t mqtt_ConnectPackage(const char *user,const char *password,const char *devid,
                         uint16_t kaTime,bool cleanSession,bool willFlag,uint8_t willQos,bool willRetain,
                         const char *willTopic,const char *willMsg,mqtt_package *package)
{
  uint32_t packageLen=0; // 报文长度
  uint8_t packageFlag=0; // 报文控制标志
  uint16_t userLen,passwordLen,devidLen,willTopicLen,willMsgLen; // 消息长度
  uint8_t packageLenBytes;
  
  if(devid == NULL)
    return 1;

  devidLen = strlen(devid);
  packageLen += devidLen + 2;

  if(cleanSession)
    packageFlag |= MQTT_CONNECT_CLEAN_SESSION;

  if(willFlag)
  {
    packageFlag |= MQTT_CONNECT_WILL_FLAG;
    willTopicLen = strlen(willTopic);
    packageLen += willTopicLen + 2;
    willMsgLen = strlen(willMsg);
    packageLen += willMsgLen + 2;
  }else {
    willRetain = false;
  }

  switch(willQos)
  {
    case MQTT_QOS_LEVEL0:
      packageFlag |= MQTT_CONNECT_WILL_QOS0;
      break;
    case MQTT_QOS_LEVEL1:
      packageFlag |= MQTT_CONNECT_WILL_QOS1;
      break;
    case MQTT_QOS_LEVEL2:
      packageFlag |= MQTT_CONNECT_WILL_QOS2;
      break;
    default:
      return 2;
  }

  if(willRetain)
    packageFlag |= MQTT_CONNECT_WILL_RETAIN;

  if(user == NULL || password == NULL)
    return 3;

  packageFlag |= (MQTT_CONNECT_PASSORD|MQTT_CONNECT_USER_NAME);

  userLen = strlen(user);
  packageLen += userLen + 2;
  passwordLen = strlen(password);
  packageLen += passwordLen + 2;

  packageLen += 10;
  
  if(packageLen > 0x0FFFFFFF)
    return 4;
  
  if(packageLen<128)
    packageLenBytes = 1;
  else if(packageLen<16384)
    packageLenBytes = 2;
  else if(packageLen<2097152)
    packageLenBytes = 3;
  else
    packageLenBytes = 4;
  
  packageLen += (packageLenBytes + 1);

  // +1 是结束符的空间,因为下面使用strncat函数,不申请多一个空间会出错。
  if(mqttNewPackage(package,packageLen+1))
    return 5;
  package->_size -= 1;

/**************************固定包头******************************/
  // 请求类型
  package->_data[package->data_ptr++] = MQTT_PKT_CONNECT << 4;
  // 剩余长度
  mqttFixHeaderFillRemainingLength((packageLen-(1+packageLenBytes)),packageLenBytes,(package->_data + package->data_ptr));
  package->data_ptr += packageLenBytes;

/*************************可变头部******************************/
  // 协议名字
  package->_data[package->data_ptr++] = 0;
  package->_data[package->data_ptr++] = 4;
  package->_data[package->data_ptr++] = 'M';
  package->_data[package->data_ptr++] = 'Q';
  package->_data[package->data_ptr++] = 'T';
  package->_data[package->data_ptr++] = 'T';

  // 协议版本
  package->_data[package->data_ptr++] = 4;

  // 连接标志
  package->_data[package->data_ptr++] = packageFlag;

  // 存活时间
  package->_data[package->data_ptr++] = (kaTime&0xFF00)>>8;
  package->_data[package->data_ptr++] = (kaTime&0x00FF);

/************************消息体********************************/
  // 客户标识符
  package->_data[package->data_ptr++] = (devidLen&0xFF00)>>8;
  package->_data[package->data_ptr++] = (devidLen&0x00FF);

  strncat((char *)package->_data + package->data_ptr,devid,devidLen);
  package->data_ptr += devidLen;

  // 遗嘱主题 遗嘱消息
  if(willFlag)
  {
    if(willMsg == NULL)
      willMsg = "";
    // 遗嘱主题
    package->_data[package->data_ptr++] = (willTopicLen&0xFF00)>>8;
    package->_data[package->data_ptr++] = (willTopicLen&0x00FF);

    strncat((char *)package->_data + package->data_ptr,willTopic,willTopicLen);
    package->data_ptr += willTopicLen;

    // 遗嘱消息
    package->_data[package->data_ptr++] = (willMsgLen&0xFF00)>>8;
    package->_data[package->data_ptr++] = (willMsgLen&0x00FF);

    strncat((char *)package->_data + package->data_ptr,willMsg,willMsgLen);
    package->data_ptr += willMsgLen;
  }

  // 用户名字
  package->_data[package->data_ptr++] = (userLen&0xFF00)>>8;
  package->_data[package->data_ptr++] = (userLen&0x00FF);

  strncat((char *)package->_data + package->data_ptr,user,userLen);
  package->data_ptr += userLen;

  // 用户密码
  package->_data[package->data_ptr++] = (passwordLen&0xFF00)>>8;
  package->_data[package->data_ptr++] = (passwordLen&0x00FF);

  strncat((char *)package->_data + package->data_ptr,password,passwordLen);
  package->data_ptr += passwordLen;

  return 0;
}

void mqtt_UnConnectACK(const char *data_ptr,mqtt_ConnectACK *package)
{
  package->_code = data_ptr[3];
}

uint8_t mqtt_PublishPackage(bool DUP,uint8_t Qos,bool RETAIN,const char *TopicName,const char *Massage,mqtt_package *package)
{
  uint32_t packageLen=0; // 报文长度
  uint8_t packageHeaderFlag=0; // 报文头部标志
  uint16_t TopicNameLen=0; // 主题名字
  uint16_t MassageLen=0;
  uint8_t packageLenBytes;

  packageHeaderFlag = MQTT_PKT_PUBLISH << 4;
  switch(Qos)
  {
    case 0:
      packageHeaderFlag |= (0<<1);
      break;
    case 1:
      packageHeaderFlag |= (1<<1);
      break;
    case 2:
      packageHeaderFlag |= (2<<1);
      break;
    default:
      return 1;
  }

  if(TopicName==NULL)
    return 2;
  
  TopicNameLen = strlen(TopicName);
  packageLen += TopicNameLen + 2;

  if(Qos!=0)
    packageLen += 2;

  if(Massage!=NULL)
  {
    MassageLen = strlen(Massage);
    packageLen += MassageLen;
  }

  if(packageLen > 0x0FFFFFFF)
    return 3;

  if(packageLen<128)
    packageLenBytes = 1;
  else if(packageLen<16384)
    packageLenBytes = 2;
  else if(packageLen<2097152)
    packageLenBytes = 3;
  else
    packageLenBytes = 4;
  
  packageLen += packageLenBytes;

  // 包头标志
  packageLen += 1;


  // +1 是结束符的空间,因为下面使用strncat函数,不申请多一个空间会出错。
  if(mqttNewPackage(package,(packageLen+1)))
    return 4;
  package->_size -= 1;
  
/*************************************固定包头************************************/
  // 包头标志
  package->_data[package->data_ptr++] = packageHeaderFlag;
  // 剩余长度
  mqttFixHeaderFillRemainingLength((packageLen-(1+packageLenBytes)),packageLenBytes,(package->_data + package->data_ptr));
  package->data_ptr += packageLenBytes;

/*************************************可变头部************************************/
  // 主题名字
  package->_data[package->data_ptr++] = (TopicNameLen&0xFF00)>>8;
  package->_data[package->data_ptr++] = (TopicNameLen&0x00FF);

  strncat((char *)package->_data + package->data_ptr,TopicName,TopicNameLen);
  package->data_ptr += TopicNameLen;

  if(Qos!=0)
  {
    // 报文标识符
    package->_data[package->data_ptr++] = 0;
    package->_data[package->data_ptr++] = 0x0A;
  }

/************************************消息体**************************************/
  // 内容
  strncat((char *)package->_data + package->data_ptr,Massage,MassageLen);
  package->data_ptr += MassageLen;

  return 0;
}

uint8_t mqtt_PublishUnPackage(const char *data_ptr,mqtt_Publish *package)
{
  uint8_t packageLenBytes=0;
  uint32_t packageLen=0;
  uint32_t position=0; // 记录位置

  position += 1; // 计算剩余长度位置
  mqttFixHeaderReadRemainingLength((data_ptr+position),&packageLen,&packageLenBytes); // 获取剩余长度和剩余长度占用的字节数

  package->Qos = data_ptr[0]&0x06; // 记录Qos

  position += packageLenBytes; // 计算主题长度位置
  package->topicNameLen = (data_ptr[position]<<8)|(data_ptr[position+1]); // 记录主题长度
  position += 2; // 计算主题位置

  // 申请内存
  package->topicName = (char *)malloc(package->topicNameLen+1);
  if(package->topicName==NULL)
    return 1;
    
  memset(package->topicName,0,(package->topicNameLen+1));
  strncat(package->topicName,(data_ptr+position),package->topicNameLen);

  position += package->topicNameLen; // 计算标识符位置或者有效载荷的位置
  
  if(package->Qos!=0)
  {
    package->identifier = (data_ptr[position]<<8)|(data_ptr[position+1]); // 记录标识符
    position += 2; // 计算有效载荷位置
  }
  
  // 计算有效载荷长度
  package->payloadLen = packageLen - package->topicNameLen - 2;
  package->payloadLen = (package->Qos == 0) ? (package->payloadLen) : (package->payloadLen - 2);

  // 申请内存
  package->payload = (char *)malloc(package->payloadLen+1);
  if(package->payload==NULL)
  {
    free(package->topicName); // 释放前面申请成功的内存
    return 2;
  }

  memset(package->payload,0,(package->payloadLen+1));
  strncat(package->payload,(data_ptr+position),package->payloadLen);

  return 0;
}

uint8_t mqtt_SubscribePackage(const char *topicFilterName,uint8_t Qos,uint16_t packageFlag,mqtt_package *package)
{
  uint32_t packageLen=0; // 报文长度
  uint8_t packageHeaderFlag=0; // 报文头部标志
  uint32_t topicFilterNameLen;
  uint8_t packageLenBytes;

  packageHeaderFlag = MQTT_PKT_SUBSCRIBE << 4;
  packageHeaderFlag |= 0x02;

  if(topicFilterName==NULL)
    return 1;

  packageLen += 2;

  topicFilterNameLen = strlen(topicFilterName);
  packageLen += topicFilterNameLen + 2;

  switch(Qos)
  {
    case 0:
      break;
    case 1:
      break;
    case 2:
      break;
    default:
      return 2;
  }

  packageLen += 1;

  if(packageLen > 0x0FFFFFFF)
    return 3;

  if(packageLen<128)
    packageLenBytes = 1;
  else if(packageLen<16384)
    packageLenBytes = 2;
  else if(packageLen<2097152)
    packageLenBytes = 3;
  else
    packageLenBytes = 4;

  packageLen += packageLenBytes;

  // 包头标志
  packageLen += 1;

  // +1 是结束符的空间,因为下面使用strncat函数,不申请多一个空间会出错。
  if(mqttNewPackage(package,(packageLen+1)))
    return 4;
  package->_size -= 1;

/*************************************固定包头************************************/
  // 包头标志
  package->_data[package->data_ptr++] = packageHeaderFlag;
  // 剩余长度
  mqttFixHeaderFillRemainingLength((packageLen-(1+packageLenBytes)),packageLenBytes,(package->_data + package->data_ptr));
  package->data_ptr += packageLenBytes;

/*************************************可变头部************************************/
  // 报文标识符
  package->_data[package->data_ptr++] = (packageFlag&0xFF00)>>8;
  package->_data[package->data_ptr++] = (packageFlag&0x00FF);

/************************************消息体**************************************/
  // 内容
  package->_data[package->data_ptr++] = (topicFilterNameLen&0xFF00)>>8;
  package->_data[package->data_ptr++] = (topicFilterNameLen&0x00FF);
  
  strncat((char *)package->_data + package->data_ptr,topicFilterName,topicFilterNameLen);
  package->data_ptr += topicFilterNameLen;

  // Qos
  package->_data[package->data_ptr++] = Qos;

  return 0;
}

void mqtt_SubscribeUnPackage(const char *data_ptr,mqtt_SubscribeACK *package)
{
  package->identifier = (data_ptr[2]<<8)|data_ptr[3];
  package->_code = data_ptr[4];
}

uint8_t mqtt_UnsubscribePackage(const char*topicFilterName,uint16_t packageFlag,mqtt_package *package)
{
  uint32_t packageLen=0; // 报文长度
  uint8_t packageHeaderFlag=0; // 报文头部标志
  uint32_t topicFilterNameLen;
  uint8_t packageLenBytes;

  packageHeaderFlag = MQTT_PKT_UNSUBSCRIBE << 4;
  packageHeaderFlag |= 0x02;

  if(topicFilterName==NULL)
    return 1;

  packageLen += 2;

  topicFilterNameLen = strlen(topicFilterName);
  packageLen += topicFilterNameLen + 2;

  if(packageLen > 0x0FFFFFFF)
    return 2;

  if(packageLen<128)
    packageLenBytes = 1;
  else if(packageLen<16384)
    packageLenBytes = 2;
  else if(packageLen<2097152)
    packageLenBytes = 3;
  else
    packageLenBytes = 4;

  packageLen += packageLenBytes;

  // 包头标志
  packageLen += 1;

  // +1 是结束符的空间,因为下面使用strncat函数,不申请多一个空间会出错。
  if(mqttNewPackage(package,(packageLen+1)))
    return 3;
  package->_size -= 1;

/*************************************固定包头************************************/
  // 包头标志
  package->_data[package->data_ptr++] = packageHeaderFlag;
  // 剩余长度
  mqttFixHeaderFillRemainingLength((packageLen-(1+packageLenBytes)),packageLenBytes,(package->_data + package->data_ptr));
  package->data_ptr += packageLenBytes;

/*************************************可变头部************************************/
  // 报文标识符
  package->_data[package->data_ptr++] = (packageFlag&0xFF00)>>8;
  package->_data[package->data_ptr++] = (packageFlag&0x00FF);

/************************************消息体**************************************/
  // 内容
  package->_data[package->data_ptr++] = (topicFilterNameLen&0xFF00)>>8;
  package->_data[package->data_ptr++] = (topicFilterNameLen&0x00FF);
  
  strncat((char *)package->_data + package->data_ptr,topicFilterName,topicFilterNameLen);
  package->data_ptr += topicFilterNameLen;

  return 0;
}

uint8_t mqtt_PUBACKPackage(uint16_t packageFlag,mqtt_package *package)
{
  uint32_t packageLen=4; // 报文长度
  uint8_t packageHeaderFlag=0; // 报文头部标志

  packageHeaderFlag = MQTT_PKT_PUBACK << 4;

  if(mqttNewPackage(package,packageLen))
    return 1;

/************************************固定包头*************************************/
  // 包头标志
  package->_data[package->data_ptr++] = packageHeaderFlag;
  // 剩余长度
  package->_data[package->data_ptr++] = 2;
  
/*************************************可变头部************************************/
  // 报文标识符
  package->_data[package->data_ptr++] = (packageFlag&0xFF00)>>8;
  package->_data[package->data_ptr++] = (packageFlag&0x00FF);
  
  return 0;
}

uint8_t mqtt_PUBRECPackage(uint16_t packageFlag,mqtt_package *package)
{
  uint32_t packageLen=4; // 报文长度
  uint8_t packageHeaderFlag=0; // 报文头部标志

  packageHeaderFlag = MQTT_PKT_PUBREC << 4;

  if(mqttNewPackage(package,packageLen))
    return 1;

/************************************固定包头*************************************/
  // 包头标志
  package->_data[package->data_ptr++] = packageHeaderFlag;
  // 剩余长度
  package->_data[package->data_ptr++] = 2;
  
/*************************************可变头部************************************/
  // 报文标识符
  package->_data[package->data_ptr++] = (packageFlag&0xFF00)>>8;
  package->_data[package->data_ptr++] = (packageFlag&0x00FF);
  
  return 0;
}

uint8_t mqtt_PUBCOMPPackage(uint16_t packageFlag,mqtt_package *package)
{
  uint32_t packageLen=4; // 报文长度
  uint8_t packageHeaderFlag=0; // 报文头部标志

  packageHeaderFlag = MQTT_PKT_PUBCOMP << 4;

  if(mqttNewPackage(package,packageLen))
    return 1;

/************************************固定包头*************************************/
  // 包头标志
  package->_data[package->data_ptr++] = packageHeaderFlag;
  // 剩余长度
  package->_data[package->data_ptr++] = 2;
  
/*************************************可变头部************************************/
  // 报文标识符
  package->_data[package->data_ptr++] = (packageFlag&0xFF00)>>8;
  package->_data[package->data_ptr++] = (packageFlag&0x00FF);
  
  return 0;
}

uint8_t mqtt_PingPackage(mqtt_package *package)
{
  uint32_t packageLen=2; // 报文长度
  uint8_t packageHeaderFlag=0; // 报文头部标志

  packageHeaderFlag = MQTT_PKT_PINGREQ << 4;

  if(mqttNewPackage(package,packageLen))
    return 1;

/************************************固定包头*************************************/
  // 包头标志
  package->_data[package->data_ptr++] = packageHeaderFlag;
  // 剩余长度
  package->_data[package->data_ptr++] = 0;
  
  return 0;
}

#include "Node.h"

node *node1 = new node;

const char *ssid = "Mi 10"; /* AP SSID */
const char *password = "ch1994360101"; /* AP 密码 */

const char *serverHost = "mqtts.heclouds.com"; /* 服务器域名 */

uint16_t serverPort = 1883; /* mqtt服务端口 */

const char *user = "oeI0gWmfaw";
const char *key = "version=2018-10-31&res=products%2FoeI0gWmfaw%2Fdevices%2FWHCB_001&et=4096027535&method=md5&sign=0m%2FtVhjNCy4p%2BRqtdVAHMg%3D%3D";
const char *devid = "WHCB_001";

void node::ap_connect(const char *ssid,const char *password)
{
  Serial.print("Connect AP");
  WiFi.begin(ssid,password);
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("\r\nConnected\r\n");
  Serial.printf("SSID:%s\r\n",WiFi.SSID());
  Serial.print("ipaddr:");
  Serial.println(WiFi.localIP());
}

bool node::tcp_connect(const char *host,uint16_t post,uint16_t cnt)
{ 
  uint16_t i=1;
  
  do{
    Serial.printf("尝试第%d次访问%s\r\n",i,serverHost);
    if(client.connect(host,post))
    {
      Serial.printf("正在访问%s\r\n",serverHost);
      return 0;
    }else {
      i++;
      client.stop();
    }
    delay(3000);
  }while(cnt--);

  return 1;
}

void node::tcp_stop(void)
{
  Serial.printf("结束访问\r\n");
  client.stop();
}

uint8_t node::serverConnect(const char *user,const char *key,const char *devid,uint16_t time)
{
  mqtt_package package={NULL,0,0};
  mqtt_ConnectACK ack = {0};
  char *data_ptr=NULL;
  int32_t i=60;

  alive = false;
  
  Serial.println("发起连接.");
  if(mqtt_ConnectPackage(user,key,devid,time,1,0,MQTT_QOS_LEVEL0,0,NULL,NULL,&package)==0)
  {
    client.write(package._data,package._size);
    
    mqttDeletePackage(&package);
    
    while((!client.available())&&(i--))
      delay(50);
    
    if(i==-1)
    {
      Serial.println("服务器无响应,请检查网络或者报文格式。");
    }else {

      data_ptr = (char *)malloc(client.available());

      if(data_ptr == NULL)
        return 1;
      
      memset(data_ptr,0,client.available());
      
      client.read((uint8_t*)data_ptr,client.available());
      
      mqtt_UnConnectACK((const char *)data_ptr,&ack);

      switch(ack._code)
      {
        case 0:
          Serial.println("连接已被服务端接受.");
          alive = true;
          break;
        case 1:
          Serial.println("服务端不支持客户端请求的MQTT协议级别.");
          break;
        case 2:
          Serial.println("客户端标识符是正确的UTF-8编码，但服务端不允许使用.");
          break;
        case 3:
          Serial.println("网络连接已建立，但MQTT服务不可用.");
          break;
        case 4:
          Serial.println("用户名或密码的数据格式无效.");
          break;
        case 5:
          Serial.println("客户端未被授权连接到此服务器.");
          break;
        default:
          Serial.println("未知响应.");
          break;
      }
      free(data_ptr);
    }
  }else {
    return 2;
  }

  return 0;
}

uint8_t node::commitPropertyToServer(void)
{
  mqtt_package package={NULL,0,0};
  StaticJsonDocument<512> propertyBuf;
  char Msg[512]={0};
  char *topicName = NULL;
  uint32_t topicNameLen=0;
  char buf[20];

  topicNameLen += strlen("$sys/");
  topicNameLen += strlen(user);
  topicNameLen += strlen("/");
  topicNameLen += strlen(devid);
  topicNameLen += strlen("/thing/property/post");

  topicName = (char *)malloc(topicNameLen+1);
  if(topicName == NULL)
    return 1;
  memset(topicName,0,topicNameLen);

  strncat(topicName,"$sys/",strlen("$sys/"));
  strncat(topicName,user,strlen(user));
  strncat(topicName,"/",strlen("/"));
  strncat(topicName,devid,strlen(devid));
  strncat(topicName,"/thing/property/post",strlen("/thing/property/post"));
  
  property_my.RT_M = config1.time_remaining_m;
  property_my.RT_S = config1.time_remaining_s;
  property_my.status = config1.heat;
  property_my.obj_T = config1.obj_time;
  property_my.startup_cnt = 0;

  propertyBuf["id"] = "1";
  JsonObject paramsObj = propertyBuf.createNestedObject("params");
  
  JsonObject RT_M_Obj = paramsObj.createNestedObject("RT_M");
  RT_M_Obj["value"] = property_my.RT_M;

  JsonObject RT_S_Obj = paramsObj.createNestedObject("RT_S");
  RT_S_Obj["value"] = property_my.RT_S;

  JsonObject obj_time_Obj = paramsObj.createNestedObject("obj_time");
  obj_time_Obj["value"] = property_my.obj_T;

  JsonObject statusObj = paramsObj.createNestedObject("status");
  statusObj["value"] = (uint8_t)property_my.status;

  JsonObject startup_cnt_Obj = paramsObj.createNestedObject("startup_cnt");
  startup_cnt_Obj["value"] = property_my.startup_cnt;

  serializeJson(propertyBuf, Msg);
  
  if(mqtt_PublishPackage(0,0,0,(const char *)topicName,(const char *)Msg,&package)==0)
  {
    client.write(package._data,package._size);
    
    mqttDeletePackage(&package);

    Serial.println("属性上传完成。");
  }else {
    return 2;
  }

  free(topicName);

  return 0;
}

void node::sendPingToServer(void)
{
  mqtt_package package={NULL,0,0};

  if(ping_PLR>=3)
  {
    ping_PLR=0;
    alive=false;
    return;
  }
  
  if(mqtt_PingPackage(&package)==0)
  {
    
    client.write(package._data,package._size);

    mqttDeletePackage(&package);

    ping_PLR++;
  }
}

uint8_t  node::subscribeToServer(char *topicName,uint8_t Qos)
{
  mqtt_package package={NULL,0,0};
  mqtt_SubscribeACK ack;
  uint32_t i=500;
  char *rdata=NULL;

  if(mqtt_SubscribePackage((const char*)topicName,Qos,(uint16_t)getSubscribePackageFlag(topicName),&package)==0)
  {
    client.write(package._data,package._size);
    
    mqttDeletePackage(&package);
    
    while((!client.available())&&(i--))
      delay(10);

    if(i==-1)
    {
      Serial.println("服务器未响应,请检查网络或者报文格式。");
    }else {
      rdata = (char *)malloc(client.available());
      if(rdata == NULL)
        return 1;
      
      client.read((uint8_t*)rdata,client.available());
      
      mqtt_SubscribeUnPackage(rdata,&ack);
      
      if(ack._code!=0x80)
      {
        setSubscribeStatus(topicName,true);
        Serial.println("主题订阅成功。");
      }
      
      free(rdata);
    }
  }else {
    return 2;
  }

  return 0;
}

void node_init(void)
{
  char topicBuf[512];
  uint8_t ret=0;

  node1->init(ssid,password,serverHost,serverPort,user,key,devid,60);
  
  memset(topicBuf,0,sizeof(topicBuf));
  sprintf(topicBuf,"$sys/%s/%s/thing/service/switch_/invoke",user,devid);
  
  if(node1->getStatus()&&node1->subscribeRegister((char *)topicBuf,0x000A))
  {
    Serial.printf("向服务订阅主题:%s。\r\n",topicBuf);
    ret = node1->subscribeToServer((char *)topicBuf,0);
  }
  
  memset(topicBuf,0,sizeof(topicBuf));
  sprintf(topicBuf,"$sys/%s/%s/thing/property/set",user,devid);
  
  if(node1->getStatus()&&node1->subscribeRegister((char *)topicBuf,0x000B))
  {
    Serial.printf("向服务订阅主题:%s。\r\n",topicBuf);
    ret = node1->subscribeToServer((char *)topicBuf,0);
  }
}

void node::PublishReplyToServer(char *part,uint8_t type,uint64_t synID)
{
  StaticJsonDocument<512> propertyBuf;
  char Msg[512]={0};
  char buf[20];
  mqtt_package package={NULL,0,0};
  char *topicName=NULL;
  uint32_t topicNameLen=0;

  if(type){
    topicNameLen += strlen("$sys/");
    topicNameLen += strlen(user);
    topicNameLen += strlen("/");
    topicNameLen += strlen(devid);
    topicNameLen += strlen(part);
  }else {
    topicNameLen += strlen(part);
    topicNameLen += strlen("_reply");
  }

  topicName = (char *)malloc(topicNameLen+1);
  memset(topicName,0,topicNameLen);

  if(type) {
    strncat(topicName,"$sys/",strlen("$sys/"));
    strncat(topicName,user,strlen(user));
    strncat(topicName,"/",strlen("/"));
    strncat(topicName,devid,strlen(devid));
    strncat(topicName,part,strlen(part));
  }else {
    strncat(topicName,part,strlen(part));
    strncat(topicName,"_reply",strlen("_reply"));
  }

  if(type){
    memset(buf,0,sizeof(buf));
    sprintf(buf,"%d",synID);
    propertyBuf["id"]= buf;

    propertyBuf["code"]= 200;

    propertyBuf["msg"]= "success";
    
    serializeJson(propertyBuf, Msg);
    serializeJson(propertyBuf, Serial);
  }else {
    propertyBuf["id"]= synID;

    propertyBuf["code"]= 200;

    propertyBuf["msg"]= "success";

    JsonObject dataObj = propertyBuf.createNestedObject("data");

    dataObj["switch_"] = (bool)config1.heat;

    serializeJson(propertyBuf, Msg);
    serializeJson(propertyBuf, Serial);
    Serial.println(" ");
  }
  
  if(mqtt_PublishPackage(0,0,0,(const char *)topicName,(const char *)Msg,&package)==0)
  {
    uint32_t i;
    client.write(package._data,package._size);

    mqttDeletePackage(&package);
    
    Serial.println("属性响应。");
  }

  free(topicName);
}

uint8_t node::dataProcessing(void){
  if(client.available()){
    StaticJsonDocument<512> propertyBuf;
    mqtt_ConnectACK ack1={0};
    mqtt_Publish ack2={255,255,NULL,0,NULL,0};
    mqtt_SubscribeACK ack3={255,255};
    uint8_t *rdata=NULL;
    uint32_t rdataLen=0;
    uint32_t i;

    rdataLen = client.available();
    
    rdata = (uint8_t *)malloc(rdataLen);

    if(rdata==NULL)
      return 1;

    client.read(rdata,rdataLen);

    switch(mqtt_UnpackageType((const char *)rdata))
    {
      case MQTT_PKT_CONNACK:
        break;
      case MQTT_PKT_PUBLISH:
        Serial.printf("接收到:MQTT_PKT_PUBLISH\r\n");

        if(mqtt_PublishUnPackage((const char *)rdata,&ack2)==0)
        {
          for(i=0;i<ack2.payloadLen;i++)
            Serial.printf("%c",ack2.payload[i]);
          Serial.println(" ");
          DeserializationError error = deserializeJson(propertyBuf,ack2.payload);
          if(error.code()!=DeserializationError::Ok)
            return 11;
  
          if(strstr((const char*)ack2.topicName,"/thing/property/set")!=NULL)
          {
            Serial.printf("topic:/thing/property/set\r\n");
            config1.obj_time = propertyBuf["params"]["obj_time"];
            info1.type_=INFO_HEAT;
            PublishReplyToServer("/thing/property/set_reply",1,atoi(propertyBuf["id"]));
          }else if(strstr((const char*)ack2.topicName,"/thing/service/switch_/invoke")!=NULL){
            Serial.printf("topic:/thing/service/switch_/invoke\r\n");
            if(propertyBuf["params"]["switch_"]==true)
              config1.set_heat = OPEN;
            else if(propertyBuf["params"]["switch_"]==false)
              config1.set_heat = CLOSE;
            PublishReplyToServer((char*)ack2.topicName,0,atoi(propertyBuf["id"]));
          }
          mqttDeletePublishPackage(&ack2);
        }else {
          return 4;
        }
        
        break;
      case MQTT_PKT_PUBACK:
        break;
      case MQTT_PKT_PUBREC:
        break;
      case MQTT_PKT_PUBREL:
        break;
      case MQTT_PKT_PUBCOMP:
        break;
      case MQTT_PKT_SUBACK:
        break;
      case MQTT_PKT_UNSUBACK:
        break;
      case MQTT_PKT_PINGRESP:
        Serial.printf("接收到:MQTT_PKT_PINGRESP\r\n");
        if(ping_PLR)
          ping_PLR--;
        break;
      default:
        return 10;
    }
    free(rdata);
  }else {
    return 2;
  }
  return 0;
}

void nodeLoop(void)
{
  static uint32_t times=0;
  uint8_t ret=0;
  
  delay(100);
  times++;
  
  if(node1->getStatus()&&((times%150)==0)) {
    node1->sendPingToServer();
  }

  if(node1->getStatus()==false) {
    ret = node1->serverConnect(user,key,devid,60);
  }

  if(node1->getStatus()&&(times>=600)) {
    times = 0;
    ret = node1->commitPropertyToServer();
  }

  ret = node1->dataProcessing();
}

#ifndef __Node_H
#define __Node_H

#include "arduino.h"
#include <WiFi.h>
#include "mqtt.h"
#include "ArduinoJson.h"
#include "string.h"
#include "config.h"
#include "info.h"

extern WHCB_config config1;
extern info info1;

class node {

  typedef struct property_{

    uint16_t RT_M;
    uint16_t RT_S;
    bool status;
    uint16_t obj_T;
    uint16_t startup_cnt;
    
  }property;

  typedef struct subscribe_ {
    char *topicName;
    bool status;
    uint16_t packageFlag;
  }subscribe;
  
public:

  node()
  {
    uint8_t i;
    alive = false;
    ping_PLR = 0;
    packageLoss_cnt = 0;
    for(i=0;i<10;i++)
    {
      subs[i].topicName = NULL;
      subs[i].status = false;
      subs[i].packageFlag = 0;
    }
  }
  
  ~node()
  {
    uint8_t i;
    
    free(this->user);
    this->user = NULL;
    free(this->devid);
    this->devid = NULL;
    
    for(i=0;i<10;i++){
      if(subs[i].topicName!=NULL){
        free(subs[i].topicName);
      }
    }
  }

  bool getStatus(void)
  {
    return alive;
  }
  
  void ap_connect(const char *ssid,const char *password);

  bool tcp_connect(const char *host,uint16_t post,uint16_t cnt);

  void tcp_stop(void);

  uint8_t serverConnect(const char *user,const char *key,const char *devid,uint16_t time);
  
  void init(const char *ssid,const char *password,const char *serverHost,uint16_t serverPort,
                        const char *user,const char *key,const char *devid,uint16_t time,uint16_t cnt=3){

    this->user = (char *)malloc(strlen(user)+1);
    memset(this->user,0,strlen(user)+1);
    strncpy(this->user,user,strlen(user)+1);
    this->devid = (char *)malloc(strlen(devid)+1);
    memset(this->devid,0,strlen(devid)+1);
    strncpy(this->devid,devid,strlen(devid)+1);
    
    ap_connect(ssid,password);
    if(tcp_connect(serverHost,serverPort,cnt))
    {
      Serial.printf("%s无法访问\r\n",serverHost);
      Serial.println("TCP:无法与服务器建立连接。");
    }else {
      serverConnect(user,key,devid,60);
    }
    
  };

  uint8_t commitPropertyToServer(void);

  void PublishReplyToServer(char *part,uint8_t type,uint64_t synID);
  
  void sendPingToServer(void);

  uint8_t subscribeToServer(char *topicName,uint8_t Qos);

  uint8_t dataProcessing(void);
  
  bool subscribeUnregister(char *topicName){
    uint8_t i;
    for(i=0;i<10;i++){
      if(subs[i].topicName!=NULL){
        if(strlen(topicName)==strlen(subs[i].topicName)){
          if(strncmp(subs[i].topicName,topicName,strlen(topicName))){
            free(subs[i].topicName);
          }
        }
      }
    }
  }
  
  bool subscribeRegister(char *topicName,uint16_t packageFlag){
    uint8_t i;
    for(i=0;i<10;i++)
    {
      if(subs[i].topicName==NULL){
        subs[i].topicName = (char *)malloc(strlen(topicName)+1);
        memset(subs[i].topicName,0,strlen(topicName)+1);
        strncpy(subs[i].topicName,topicName,strlen(topicName)+1);
        subs[i].packageFlag = packageFlag;
        return true;
      }else {
        if(strlen(subs[i].topicName)==strlen(topicName)){
          if(strncmp(subs[i].topicName,topicName,strlen(topicName))){
            return true;
          }
        }
      }
    }
    return false;
  }

  bool getSubscribeStatus(char *topicName){
    uint8_t i;
    for(i=0;i<10;i++)
    {
      if(subs[i].topicName!=NULL)
      {
        if(strlen(topicName)==strlen(subs[i].topicName))
        {
          if(strncmp(topicName,subs[i].topicName,strlen(topicName))==0)
            return subs[i].status;
        }
      }
    }
    return false;
  }

  int32_t getSubscribePackageFlag(char *topicName){
    uint8_t i;
    for(i=0;i<10;i++)
    {
      if(subs[i].topicName!=NULL)
      {
        if(strlen(topicName)==strlen(subs[i].topicName))
        {
          if(strncmp(topicName,subs[i].topicName,strlen(topicName))==0)
            return subs[i].packageFlag;
        }
      }
    }
    return -1;
  }

private:
  bool setSubscribeStatus(char *topicName,bool status){
    uint8_t i;
    for(i=0;i<10;i++)
    {
      if(subs[i].topicName!=NULL)
      {
        if(strlen(topicName)==strlen(subs[i].topicName))
        {
          if(strncmp(topicName,subs[i].topicName,strlen(topicName))==0)
          {
            subs[i].status = status;
            return true;
          }
        }
      }
    }
    return false;
  }
  
private:
  property property_my;
  WiFiClient client;
  char *user = NULL;
  char *devid = NULL;
  bool alive;
  uint8_t packageLoss_cnt;
  uint8_t ping_PLR; //累计丢包数
  subscribe subs[10];
};

void node_init(void);
void nodeLoop(void);

#endif

#include "WHCB_Node.h"

StaticJsonDocument<512> WHCB_Document;

extern WHCB_config config1;

Scheduler userScheduler; /* 实例一个任务调度器 */
painlessMesh  mesh; /* 实例一个mesh */

//void sendMessage() ; /* 消息发送函数 */

//Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage ); /* 绑定消息回调函数 */
//
//void sendMessage() {
//  String msg = "Hello from node ";
//  msg += mesh.getNodeId();
//  mesh.sendBroadcast( msg );
//  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));
//}

/* 接收回调函数 */
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
  if(msg == "on")
    config1.set_heat = OPEN;
  else
    config1.set_heat = CLOSE;
}

/* 新连接回调函数 */
void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

/* 连接改变回调函数 */
void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

/* 节点时间调整函数 */
void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void WHCB_Node_Init(void)
{
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  mesh.setContainsRoot(true);
  
//  userScheduler.addTask( taskSendMessage );
//  taskSendMessage.enable();
}

void WHCB_Node_update(void)
{
  mesh.update();
}

void WHCB_Document_init(void)
{
  WHCB_Document["node"] = MESH_PREFIX;
  
  WHCB_Document["obj_time_max"] = "obj_time_max";
  WHCB_Document["value"] = "0";

  WHCB_Document["time_remaining_m"] = "time_remaining_m";
  WHCB_Document["value"] = "0";

  WHCB_Document["time_remaining_s"] = "time_remaining_s";
  WHCB_Document["value"] = "0";

  WHCB_Document["obj_time"] = "obj_time";
  WHCB_Document["value"] = "0";

  WHCB_Document["lcd_blk_en"] = "lcd_blk_en";
  WHCB_Document["value"] = "0";

  WHCB_Document["heat"] = "heat";
  WHCB_Document["value"] = "0";
}

void sendMessage_Json(void)
{
  char buffer[512];

  serializeJson(WHCB_Document,buffer);

  Serial.println(buffer);
}

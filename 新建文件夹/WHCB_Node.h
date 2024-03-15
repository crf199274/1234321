#ifndef __WHCB_Node_H
#define __WHCB_Node_H

#include "arduino.h"
#include "painlessMesh.h"
#include "ArduinoJson.h"
#include "config.h"

#define   MESH_PREFIX     "WHCB001_2023"
#define   MESH_PASSWORD   "acb123_2023"
#define   MESH_PORT       5555

void WHCB_Node_Init(void);
void WHCB_Node_update(void);

#endif

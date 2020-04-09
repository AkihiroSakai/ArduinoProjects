typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef signed char int8;
typedef signed short int16;
typedef signed int int32;

#define SSID_MAXLEN 256
#define PASS_MAXLEN 256

#define VERSION_STR "Ver:1.0.0"

#pragma once
typedef struct {
  uint32 dataver;
  uint8 deviceID;
  unsigned char ssid[SSID_MAXLEN];
  char pass[PASS_MAXLEN];
  IPAddress ATEMIP;
  uint8 brightness;
  uint8 cameranum;
  uint32 AutoSwitchTime;
  bool isAutoSwitch;
} ST_PARAM;

#pragma once
typedef struct {
  uint8 wp;
  int tablenum;
  char buff[128];
} ST_KEYBOARD;

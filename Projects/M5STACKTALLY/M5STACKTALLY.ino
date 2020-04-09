/*
 * Copyright 2020- Akihiro Sakai, sakai-filmworks, info@sakai-filmworks.net
 * This code is working on M5STACK with Skaarhoj ATEM library.
 * This source code is distributed in thehope that it will be useful, but WITHOUTANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITYor FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */
 
#include <M5Stack.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Streaming.h>
#include <MemoryFree.h>
#include <SkaarhojPgmspace.h>
#include <ATEMbase.h>
#include <ATEMmax.h>

#include "common.h"
#include "enum.h"
#include "disp.h"
#include "Setup.h"
#include "ATEMSwitcher.h"
#include "ATEMTally.h"


//extern ST_KEYBOARD stPassword;
//extern int setting_cursor;
//extern int ssid_cursor;
//extern bool setting_Edit;
//extern int scannet_num;
//extern bool is34CAMSwitch;

ATEMmax AtemSwitcher;
EN_FUNCMODE funcmode = EN_FUNCMODE_HOME;
ST_PARAM param;

unsigned long lastAutoFocus;
unsigned long lastAutoIris;

void exe_button(void);
void SerialInputCheck(void);


void initParam(void)
{
  if(load_data())
  {
    memset(&param, 0, sizeof(param));
    param.deviceID = 0;
    param.brightness = 50;
    IPAddress ipaddr(192, 168, 1, 100);
    param.ATEMIP = ipaddr;
    param.AutoSwitchTime = 5000;
    param.cameranum = 4;
    save_data();
  }
  else
  {
    if(param.brightness ==0){param.brightness = 50;}  //to avoid backlight turned off.
    M5.Lcd.setBrightness(param.brightness * 2.55);
  }
}


void setup() {
  M5.begin();
  M5.Power.begin();
  Serial.begin(115200);
  M5.Lcd.setTextFont(2);    //16pixel

  exe_display(true);
  initParam();

  randomSeed(analogRead(5));  // For random port selection

  WiFi.mode(WIFI_STA);
  Serial.println((char*)param.ssid);
  Serial.println((char*)param.pass);
  WiFi.begin((char*)param.ssid, (char*) param.pass);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    exe_display(false);
    exe_button();
    SerialInputCheck();

  }

  AtemSwitcher.begin(param.ATEMIP);
  AtemSwitcher.serialOutput(0x80);
  AtemSwitcher.connect();

  lastAutoFocus = millis();
  lastAutoIris = millis() + 2500;
}





#if 1
void SerialInputCheck(void)
{
  int inputchar;
  inputchar = Serial.read();

  if (inputchar != -1 ) {
    // 読み込んだデータが -1 以外の場合　以下の処理を行う

    switch (inputchar) {
      case 'a':
        // 読み込みデータが　o の場合

        Serial.print("*****SWITCH MODE*****\n");
        funcmode = EN_FUNCMODE_SWITCH;

        break;
      case 't':
        // 読み込みデータが　p の場合
        Serial.print("*****TALLY MODE*****\n");
        funcmode = EN_FUNCMODE_TALLY;

        break;
      case 's':
//        funcmode = EN_FUNCMODE_SETUP;
        Setup_Wizard();
      default:
        break;
    }
  } else {
    // 読み込むデータが無い場合は何もしない
  }
}
#endif

void btn_home(void)
{
  int btnA = M5.BtnA.wasPressed();
  int btnB = M5.BtnB.wasPressed();
  int btnC = M5.BtnC.wasPressed();
  
    if (btnA)
    {
      funcmode = EN_FUNCMODE_SWITCH;
      exe_display(true);
    }
    else if (btnB)
    {
      funcmode = EN_FUNCMODE_TALLY;
      exe_display(true);
    }
    else if (btnC)
    {
      funcmode = EN_FUNCMODE_SETUP;
      exe_display(true);
    }
}

void exe_button(void)
{
  switch (funcmode)
  {
    case EN_FUNCMODE_HOME:
      btn_home(); 
      break;
    case EN_FUNCMODE_TALLY:
      btn_tally();
      break;
    case EN_FUNCMODE_SWITCH:
      btn_Switcher();
      break;
    case EN_FUNCMODE_SETUP:
      btn_Setup();
      break;
    default:
      break;
  }
 
  if (M5.BtnA.wasReleased()) 
  {
    drawFooter(true);    
  }

  M5.update();

}
void exe_home(void)
{
  static bool isconnected = false;
  if(isconnected !=  AtemSwitcher.isConnected())
  {
    exe_display(true);  
    isconnected =  AtemSwitcher.isConnected();
  }
  
}
void loop()
{
  exe_display(false);
  //  AtemSwitcher.setCameraControlVideomode(1, 24, 6, 0);
  AtemSwitcher.runLoop();
  switch (funcmode)
  {
    case EN_FUNCMODE_HOME:
      exe_home();
      break;
    case EN_FUNCMODE_TALLY:
      exe_Tally();
      break;
    case EN_FUNCMODE_SWITCH:
      exe_AutoSwitcherMode();
      break;
    case EN_FUNCMODE_SETUP:
      //    Setup_Wizard();
      break;
    default:
      break;
  }
  exe_button();
  SerialInputCheck();
}

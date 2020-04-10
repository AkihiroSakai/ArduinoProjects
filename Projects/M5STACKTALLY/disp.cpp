/*
 * Copyright 2020- Akihiro Sakai, sakai-filmworks, info@sakai-filmworks.net
 * This code is working on M5STACK with Skaarhoj ATEM library.
 * This source code is distributed in thehope that it will be useful, but WITHOUTANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITYor FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "Arduino.h" 
#include <M5Stack.h>

#include <WiFi.h>
//#include <WiFiClient.h>
//#include <SPI.h>
//#include <Ethernet.h>
//#include <Streaming.h>
//#include "EEPROM.h"


#include <MemoryFree.h>
#include <SkaarhojPgmspace.h>
// Include ATEMbase library and make an instance:
// The port number is chosen randomly among high numbers.
#include <ATEMbase.h>
#include <ATEMmax.h>

#include "common.h"
#include "enum.h"
#include  "disp.h"
#include "ATEMSwitcher.h"
#include "ATEMTally.h"
#include "Setup.h"

const char* funcmode_str[] = {
  "M5-TALLY\0",
  "TALLY MODE\0",
  "SWITCHER MODE\0",
  "SETTINGS\0",
};


extern ATEMmax AtemSwitcher;
extern ST_PARAM param;
extern EN_FUNCMODE funcmode;

extern uint8 CAMSTATUS[4];

void SetOutlineRectangle(int x, int y, int width, int height, int borderwidth, int bordercolor, int paintcolor)
{
  M5.Lcd.fillRect(x, y, width, height, paintcolor);
  M5.Lcd.drawRect(x, y, width, borderwidth, bordercolor);
  M5.Lcd.drawRect(x, y + height - borderwidth, width, borderwidth, bordercolor);
  M5.Lcd.drawRect(x, y, borderwidth, height, bordercolor);
  M5.Lcd.drawRect(x + width - borderwidth, y, borderwidth, height, bordercolor);

  //    M5.Lcd.fillRect(x+borderwidth,y+borderwidth,width -borderwidth*2,height-borderwidth*2,paintcolor);

}


void drawHeader(bool init)
{
  uint16 width = M5.Lcd.width();
  uint16 height  = M5.Lcd.height();
  if (init)
  {
    M5.Lcd.fillRect(0, 0, width, 20 , TFT_BLACK);
    M5.Lcd.drawLine(0, 20, width, 20, TFT_WHITE);
    //Title
    M5.Lcd.drawCentreString(funcmode_str[funcmode], width / 2, 2, 2);

  }

  //Wifi Strangth
  if (WiFi.status() == WL_CONNECTED)
  {
    char buff[64];
    int rssi = WiFi.RSSI();
    M5.Lcd.drawRect(0, 0, 20, 19, TFT_BLACK);
    M5.Lcd.drawLine(5, 2, 5, 15, TFT_GREEN);
    M5.Lcd.fillTriangle(2, 2, 8, 2, 5, 5, TFT_GREEN);
    sprintf(buff, "%d\0", rssi);
    if (rssi > -80)
    {
      M5.Lcd.drawLine(8, 12, 8, 15, TFT_WHITE);
    }
    if (rssi > -60)
    {
      M5.Lcd.drawLine(11, 10, 11, 15, TFT_WHITE);
    }
    if (rssi > -40)
    {
      M5.Lcd.drawLine(14, 8, 14, 15, TFT_WHITE);
    }
    if (rssi > -20)
    {
      M5.Lcd.drawLine(17, 6, 17, 15, TFT_WHITE);
    }
    if (rssi > -10)
    {
      M5.Lcd.drawLine(20, 4, 20, 15, TFT_WHITE);
    }
    M5.Lcd.drawString(buff, 22, 2, 2);
  }


  bool isChagerFull = M5.Power.isChargeFull();
  bool isCharging = M5.Power.isCharging();
  int8_t batteryLv = M5.Power.getBatteryLevel();
  int chg_state;
  static int chg_state_bak = -1;
  static int8_t batteryLv_bak = 0;


  if(batteryLv != batteryLv_bak|| (init==true))
  {
    M5.Lcd.fillRect(width - 30, 0, 22, 19, TFT_BLACK);
    M5.Lcd.drawRect(width - 30, 2, 22, 15, TFT_WHITE);
    int btwidth = batteryLv / 5;
    uint16 color = TFT_GREEN;
    if (batteryLv < 30) {
        color = TFT_YELLOW;
    }
    else if (batteryLv < 15) {
      color = TFT_RED;
    }
    M5.Lcd.fillRect(width - 29, 3, btwidth, 13, color);
    batteryLv_bak = batteryLv;
  }

  
  if (isChagerFull)
  {
    chg_state = EN_CHARGED;
  }
  else if (isCharging)
  {
    chg_state = EN_CHARGING;
  }
  else
  {
    chg_state = EN_BATTERY;
  }

  char buff[20];
    switch(chg_state)
    {
      case EN_CHARGED:
          if(chg_state != chg_state_bak|| (init==true))
          {
                M5.Lcd.fillRect(width - 70, 0,  40 , 19, TFT_BLACK);
                 chg_state_bak = chg_state;
                 M5.Lcd.drawRightString("FULL", width - 31, 2, 2);
                 chg_state_bak = chg_state;
          }
      break;
      case   EN_CHARGING:
          if(chg_state != chg_state_bak|| (init==true))
          {
                M5.Lcd.fillRect(width - 70, 0,  40 , 19, TFT_BLACK);
                 chg_state_bak = chg_state;
                  M5.Lcd.drawRightString("CHG", width - 31, 2, 2);
                 chg_state_bak = chg_state;
          }
      break;
      case EN_BATTERY:
        
        if(batteryLv != batteryLv_bak || chg_state_bak !=chg_state|| (init==true))
        {
              M5.Lcd.fillRect(width - 70, 0,  40 , 19, TFT_BLACK);
              sprintf(buff, "%d%%\0", batteryLv);
              M5.Lcd.drawRightString(buff, width - 31, 2, 2);
              batteryLv_bak = batteryLv;
        }
         chg_state_bak = chg_state;
  
      break;
      default:
      break;      
    }
  

  
}


void drawFooter(bool init)
{
  char buff[128];
  if (init)
  {
    uint16 width = M5.Lcd.width();
    uint16 height  = M5.Lcd.height();


    SetOutlineRectangle(0, height - 20, width / 3, 20, 1, TFT_WHITE, TFT_BLACK);
    SetOutlineRectangle(width / 3, height - 20, width / 3 + 1, 20, 1, TFT_WHITE, TFT_BLACK);
    SetOutlineRectangle(width * 2 / 3, height - 20, width / 3 + 1, 20, 1, TFT_WHITE, TFT_BLACK);

    switch (funcmode)
    {
      case EN_FUNCMODE_HOME:
        M5.Lcd.drawCentreString("SWITCH", width / 6, height - 18, 2);
        M5.Lcd.drawCentreString("TALLY", width / 2, height - 18, 2);
        M5.Lcd.drawCentreString("SETTINGS", width * 5 / 6, height - 18, 2);
        break;
      case EN_FUNCMODE_TALLY:
        sprintf(buff,"UNIT:%d",param.deviceID+1);
        M5.Lcd.drawCentreString("EXIT", width / 6, height - 18, 2);
        M5.Lcd.drawCentreString(buff, width / 2, height - 18, 2);
        M5.Lcd.drawCentreString("", width * 5 / 6, height - 18, 2);
        break;
      case EN_FUNCMODE_SWITCH:
        drawFotter_Switch();
        break;
      case EN_FUNCMODE_SETUP:
        drawFotter_Setup();
        break;
    }
  }
}


void disp_popup(char* str)
{
  uint16 width = M5.Lcd.width();
  uint16 height  = M5.Lcd.height();
  SetOutlineRectangle((width - POPUP_WIDTH) / 2, (height - POPUP_HEIGHT) / 2, POPUP_WIDTH, POPUP_HEIGHT, 2, TFT_YELLOW, TFT_BLACK);
  M5.Lcd.drawCentreString(str, width / 2, (height - POPUP_HEIGHT) / 2 + 2, 2);

  M5.Lcd.drawLine((width - POPUP_WIDTH) / 2, (height - POPUP_HEIGHT) / 2 + 20, (width - POPUP_WIDTH) / 2 + POPUP_WIDTH - 1, (height - POPUP_HEIGHT) / 2 + 20, TFT_YELLOW);

}

void disp_home(bool init)
{
  uint16 width = M5.Lcd.width();
  uint16 height  = M5.Lcd.height();
  char buff[128];
  if (init != true) {
    return;
  }
  M5.Lcd.fillRect(0, 21, width, height - 41, TFT_BLACK);

  if(AtemSwitcher.isConnected())
  {
        M5.Lcd.drawString("ATEM is connected.",20,30,2);
        M5.Lcd.drawString(AtemSwitcher.getProductIdName(),20,50,2);
        Serial.println(AtemSwitcher.getProductIdName());
        Serial.println(AtemSwitcher.getVideoModeFormat());
        Serial.println(AtemSwitcher.getWarningText());
        Serial.print(AtemSwitcher.getProtocolVersionMajor());
        Serial.print(".");
        Serial.print(AtemSwitcher.getProtocolVersionMinor());
  }
  else
  {
    
        M5.Lcd.drawString("ATEM is NOT connected.",20,30,2);    
  }

  M5.Lcd.drawRightString(VERSION_STR,width-5,height -40,2);

}

void drawMain(bool init)
{
  switch (funcmode)
  {
    case EN_FUNCMODE_HOME:
      disp_home(init);
      break;
    case EN_FUNCMODE_SETUP:
      disp_setup(init);
      break;
    case EN_FUNCMODE_SWITCH:
      disp_switch(init);
      break;
    case EN_FUNCMODE_TALLY:
    disp_Tally(init);
    default:
      break;
  }

}

void exe_display(bool init)
{
  static unsigned long lastDispTim = 0;
  unsigned long tim = millis();
  if (lastDispTim + 40 > tim && init != true) //25fps
  {
    return;
  }

  lastDispTim = tim;
  drawHeader(init);
  drawFooter(init);

  drawMain(init);

}

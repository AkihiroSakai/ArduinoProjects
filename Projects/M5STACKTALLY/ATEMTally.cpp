/*
 * Copyright 2020- Akihiro Sakai, sakai-filmworks, info@sakai-filmworks.net
 * This code is working on M5STACK with Skaarhoj ATEM library.
 * This source code is distributed in thehope that it will be useful, but WITHOUTANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITYor FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "Arduino.h" 
#include <M5Stack.h>
#include <SkaarhojPgmspace.h>
#include <ATEMbase.h>
#include <ATEMmax.h>
#include "common.h"
#include "enum.h"
#include  "disp.h"


extern ATEMmax AtemSwitcher;
extern ST_PARAM param;
extern EN_FUNCMODE funcmode;

void disp_Tally(bool init)
{
  uint16 width = M5.Lcd.width();
  uint16 height  = M5.Lcd.height();
  uint8 tally = AtemSwitcher.getTallyByIndexTallyFlags(param.deviceID);
  static uint8 tally_bak = 0;
  if (tally_bak != tally || init == true)
  {
    if ( tally & 0x01)
    {
      Serial.print("RED\n");
      M5.Lcd.fillRect(0, 21, width, height - 41, TFT_RED);
    }
    else if (tally & 0x02)
    {
      Serial.print("GREEN\n");
      M5.Lcd.fillRect(0, 21, width, height - 41, TFT_GREEN);
    }
    else
    {
      Serial.print("OFF\n");
      M5.Lcd.fillRect(0, 21, width, height - 41, TFT_BLACK);
    }
    tally_bak = tally;
  }
}

void btn_tally(void)
{
    int btnA = M5.BtnA.wasPressed();
    int btnB = M5.BtnB.wasPressed();
    int btnC = M5.BtnC.wasPressed();
    if (btnA)
    {
    funcmode = EN_FUNCMODE_HOME;
    exe_display(true);
    }
    else if (btnB)
    {
    }
    else if (btnC)
    {
    }
}


void exe_Tally(void)
{
  if(AtemSwitcher.isConnected() != true)
  {
    funcmode = EN_FUNCMODE_HOME;  
    exe_display(true);
  }
  
  
}

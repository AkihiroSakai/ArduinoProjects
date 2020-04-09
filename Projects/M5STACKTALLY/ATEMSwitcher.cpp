/*
 * Copyright 2020- Akihiro Sakai, sakai-filmworks, info@sakai-filmworks.net
 * This code is working on M5STACK with Skaarhoj ATEM library.
 * This source code is distributed in thehope that it will be useful, but WITHOUTANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITYor FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */
 
#include "Arduino.h" 
#include <M5Stack.h>
#include <MemoryFree.h>
#include <SkaarhojPgmspace.h>
// Include ATEMbase library and make an instance:
// The port number is chosen randomly among high numbers.
#include <ATEMbase.h>
#include <ATEMmax.h>

#include "common.h"
#include "enum.h"
#include  "disp.h"
extern ATEMmax AtemSwitcher;
extern ST_PARAM param;
extern EN_FUNCMODE funcmode;

#define AUTOSWITCHTIME_NUM  6
int32 AutoSwitchTimerTable[AUTOSWITCHTIME_NUM] = {3000,5000,10000,15000,30000,60000};

uint8 CAMSTATUS[4]={0,0,0,0};
bool is34CAMSwitch = false;



#define CAMSWITCHPOS_Y  100
#define CAMSWITCH_HEIGHT  50
#define CAMTEXT_HEIGHT (CAMSWITCHPOS_Y+(CAMSWITCH_HEIGHT-16)/2)


void drawFotter_Switch(void)
{
  uint16 width = M5.Lcd.width();
  uint16 height  = M5.Lcd.height();
  char buff[64];
    if(param.isAutoSwitch)
    {
      sprintf(buff,"TIME:%dsec",param.AutoSwitchTime/1000);
      M5.Lcd.drawCentreString("EXIT", width / 6, height - 18, 2);
      M5.Lcd.drawCentreString("MANUAL", width / 2, height - 18, 2);
      M5.Lcd.drawCentreString(buff, width * 5 / 6, height - 18, 2);
    }
    else
    {
      if (M5.BtnA.isPressed())
      {
        M5.Lcd.drawCentreString("EXIT 3sec", width / 6, height - 18, 2);
        M5.Lcd.drawCentreString("AUTO", width / 2, height - 18, 2);
        M5.Lcd.drawCentreString("to CAM3/4", width * 5 / 6, height - 18, 2);            
      }
      else
      {
        M5.Lcd.drawCentreString("SHIFT", width / 6, height - 18, 2);
        if(is34CAMSwitch)
        {
          M5.Lcd.drawCentreString("CAM3", width / 2, height - 18, 2);
          M5.Lcd.drawCentreString("CAM4", width * 5 / 6, height - 18, 2);              
        }
        else
        {
          M5.Lcd.drawCentreString("CAM1", width / 2, height - 18, 2);
          M5.Lcd.drawCentreString("CAM2", width * 5 / 6, height - 18, 2);
        }
      }
    }
}

void disp_switch(bool init)
{
  uint16 width = M5.Lcd.width();
  uint16 height  = M5.Lcd.height();
  char buff[128];
  if (init != true) {
    return;
  }
  M5.Lcd.fillRect(0, 21, width, height - 41, TFT_BLACK);

  uint16 backcolor = TFT_BLACK;
  uint16 textcolor = TFT_WHITE;
  if(CAMSTATUS[0] & 0x01)
  {
    backcolor = TFT_RED;
    textcolor = TFT_WHITE;
  }
  else if(CAMSTATUS[0] & 0x02)
  {
    backcolor = TFT_GREEN;
    textcolor = TFT_WHITE;    
  }
  else
  {
    backcolor = TFT_BLACK;
    textcolor = TFT_WHITE;    
  }
  SetOutlineRectangle(0,CAMSWITCHPOS_Y,width/4,50,2,TFT_WHITE,backcolor);
  M5.Lcd.setTextColor(textcolor, backcolor);
  M5.Lcd.drawCentreString("CAM1",width/8,CAMTEXT_HEIGHT,2);
  
  if(CAMSTATUS[1] & 0x01)
  {
    backcolor = TFT_RED;
    textcolor = TFT_WHITE;
  }
  else if(CAMSTATUS[1] & 0x02)
  {
    backcolor = TFT_GREEN;
    textcolor = TFT_WHITE;    
  }
  else
  {
    backcolor = TFT_BLACK;
    textcolor = TFT_WHITE;    
  }
  SetOutlineRectangle(width/4,CAMSWITCHPOS_Y,width/4,50,2,TFT_WHITE,backcolor);
  M5.Lcd.setTextColor(textcolor, backcolor);
  M5.Lcd.drawCentreString("CAM2",width*3/8,CAMTEXT_HEIGHT,2);

  if(CAMSTATUS[2] & 0x01)
  {
    backcolor = TFT_RED;
    textcolor = TFT_WHITE;
  }
  else if(CAMSTATUS[2] & 0x02)
  {
    backcolor = TFT_GREEN;
    textcolor = TFT_WHITE;    
  }
  else
  {
    backcolor = TFT_BLACK;
    textcolor = TFT_WHITE;    
  }
  SetOutlineRectangle(width/2,CAMSWITCHPOS_Y,width/4,50,2,TFT_WHITE,backcolor);
  M5.Lcd.setTextColor(textcolor, backcolor);
  M5.Lcd.drawCentreString("CAM3",width*5/8,CAMTEXT_HEIGHT,2);

  if(CAMSTATUS[3] & 0x01)
  {
    backcolor = TFT_RED;
    textcolor = TFT_WHITE;
  }
  else if(CAMSTATUS[3] & 0x02)
  {
    backcolor = TFT_GREEN;
    textcolor = TFT_WHITE;    
  }
  else
  {
    backcolor = TFT_BLACK;
    textcolor = TFT_WHITE;    
  }
  SetOutlineRectangle(width*3/4,CAMSWITCHPOS_Y,width/4,50,2,TFT_WHITE,backcolor);
  M5.Lcd.setTextColor(textcolor, backcolor);
  M5.Lcd.drawCentreString("CAM4",width*7/8,CAMTEXT_HEIGHT,2);


  
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);

}


void btn_Switcher(void)
{
    int btnA = M5.BtnA.wasPressed();
    int btnB = M5.BtnB.wasPressed();
    int btnC = M5.BtnC.wasPressed();
      if(M5.BtnA.pressedFor(3000))
      {
            funcmode = EN_FUNCMODE_HOME;
            exe_display(true);        
      }
      if (btnA)
      {
          if(param.isAutoSwitch)
          {
          }
          else
          {
            AtemSwitcher.setProgramInputVideoSource(0, 1);
          }
            exe_display(true);        
      }
      else if (btnB)
      {
        if(param.isAutoSwitch)
        {
          param.isAutoSwitch = false;
          exe_display(true);        
        }
        else
        {    
          if (M5.BtnA.isPressed())   param.isAutoSwitch = true;
          else if(!is34CAMSwitch)          AtemSwitcher.setProgramInputVideoSource(0, 1);
          else                       AtemSwitcher.setProgramInputVideoSource(0, 3);
        }
      }
      else if (btnC)
      {
          if(param.isAutoSwitch)
          {
            for(int i = 0; i< AUTOSWITCHTIME_NUM; i++)
            {
              if(param.AutoSwitchTime == AutoSwitchTimerTable[i])
              {
                if(i+1>=AUTOSWITCHTIME_NUM)    param.AutoSwitchTime = AutoSwitchTimerTable[0]; 
                else                           param.AutoSwitchTime = AutoSwitchTimerTable[i+1];  
                break;
              }
            }  
            
            exe_display(true);
          }
          else
          {   
            if(M5.BtnA.isPressed())    is34CAMSwitch = is34CAMSwitch?false:true; 
            else if(!is34CAMSwitch)          AtemSwitcher.setProgramInputVideoSource(0, 2);
            else                       AtemSwitcher.setProgramInputVideoSource(0, 4);
          }
      }
  
  
}

void exe_AutoSwitcherMode(void)
{
  static int checkDev =0;
  static uint8 CAMSTATUSbak[4] ={0xFF,0xFF,0xFF,0xFF};

  if(AtemSwitcher.isConnected() != false)
  {
    funcmode = EN_FUNCMODE_HOME;  
  }

  CAMSTATUS[checkDev] = AtemSwitcher.getTallyByIndexTallyFlags(checkDev);
  if(memcmp(CAMSTATUSbak, CAMSTATUS,sizeof(CAMSTATUS)))
  {
    exe_display(true);
    memcpy(CAMSTATUSbak, CAMSTATUS,sizeof(CAMSTATUS));
  }
  checkDev++;
  if(checkDev >3)
  {
    checkDev = 0;  
  }
  
  
  if(param.isAutoSwitch == false){return;}
  static unsigned long lastExeTime = 0;
  static uint8 currentCam = EN_CAM1;
  static uint8 ME = 0;
  unsigned long tim = millis();
  if (tim < lastExeTime + param.AutoSwitchTime)
  {
    return;
  }
  lastExeTime = tim;

  Serial.print("Program Input:");
  Serial.println(currentCam);
  AtemSwitcher.setProgramInputVideoSource(ME, currentCam);
  currentCam++;
  if (currentCam > param.cameranum)
  {
    //    ME= ME?0:1;
    currentCam = EN_CAM1;
  }
  //  delay(param.AutoSwitchTime);
}

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
#include <WiFiClient.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Streaming.h>
#include "EEPROM.h"


#include <MemoryFree.h>
#include <SkaarhojPgmspace.h>
// Include ATEMbase library and make an instance:
// The port number is chosen randomly among high numbers.
#include <ATEMbase.h>
#include <ATEMmax.h>

#include "common.h"
#include "enum.h"
#include  "disp.h"

ST_KEYBOARD stPassword;
int setting_cursor = 0;
int ssid_cursor = 0;
bool setting_Edit = false;
int scannet_num = 0;



extern ST_PARAM param;
extern EN_FUNCMODE funcmode;



static   char buff[128];


const char* settingmenu_str[] = {
  "UNIT ID", // EN_SETTING_UNITID,
  "SSID", //EN_SETTING_SSID,
  "PASSWORD", //EN_SETTING_PASS,
  "UNIT IP", //EN_SETTING_IPADDRESS,
  "ATEM IP", //EN_SETTING_ATEM_IP,
  "BRIGHTNESS", //EN_SETTING_BRIGHTNESS,
  "CAMERA NUM"  //EN_SETTING_CAMNUM,
};

void save_data() {
    //Store param to EEPROM
    param.dataver = 0x10000000;
    EEPROM.put<ST_PARAM>(0, param);
    EEPROM.commit();
}

int load_data() {
    EEPROM.begin(1024);

    EEPROM.get<ST_PARAM>(0, param);
    if (param.dataver != 0x10000000){  //Check Parameter data version
        Serial.println("LoadFail");
        return -1;
    }
    else
    {
      Serial.println("LoadOK");  
      return 0;
    }
}


void drawFotter_Setup(void)
{
  uint16 width = M5.Lcd.width();
  uint16 height  = M5.Lcd.height();

  
    if (setting_Edit == true && setting_cursor == EN_SETTING_PASS)
    {
      if (M5.BtnB.isPressed())
      {
        M5.Lcd.drawCentreString("BACKSPACE", width / 6, height - 18, 2);
        M5.Lcd.drawCentreString("SHIFT", width / 2, height - 18, 2);
        M5.Lcd.drawCentreString("ENTER", width * 5 / 6, height - 18, 2);

      }
      else
      {
        M5.Lcd.fillTriangle( width / 6 - 8, height - 15, width / 6, height - 5, width / 6 + 8, height - 15            , TFT_WHITE);
        M5.Lcd.drawCentreString("SHIFT", width / 2, height - 18, 2);
        M5.Lcd.fillTriangle( width * 5 / 6 - 8, height - 5, width * 5 / 6, height - 15, width * 5 / 6 + 8, height - 5            , TFT_WHITE);
      }
    }
    else
    {

      M5.Lcd.drawCentreString("EXIT", width / 6, height - 18, 2);
      //    M5.Lcd.drawCentreString("",width/2,height-18,2);
      M5.Lcd.fillTriangle( width / 2 - 8, height - 15, width / 2, height - 5, width / 2 + 8, height - 15            , TFT_WHITE);
      if (setting_Edit)
      {
        M5.Lcd.fillTriangle( width * 5 / 6 - 8, height - 5, width * 5 / 6, height - 15, width * 5 / 6 + 8, height - 5            , TFT_WHITE);
      }
      else
      {
        M5.Lcd.drawCentreString(">>", width * 5 / 6, height - 18, 2);
      }
    }
}

void disp_setup(bool init)
{
#define TOP_X   10
#define TOP_Y   40
#define LINE_GAP  20
  uint16 width = M5.Lcd.width();
  uint16 height  = M5.Lcd.height();
  char buff[128];
  if (init != true) {
    return;
  }

  if (setting_Edit)
  {
    char buff[128];
    disp_popup((char*)settingmenu_str[setting_cursor]);
    switch (setting_cursor)
    {
      case  EN_SETTING_UNITID:
        sprintf(buff, "%d", param.deviceID + 1);
        break;
      case  EN_SETTING_SSID:
        {
          int shownum = scannet_num > 7 ? 7 : scannet_num;
          int offset = 0;
          if (ssid_cursor > 3)
          {
            offset = ssid_cursor - 3;
            if (ssid_cursor + 4 >= scannet_num)
            {
              offset = scannet_num - 7;
            }
          }

          for (int i = 0; i < shownum; ++i)
          {
            WiFi.SSID(i + offset).getBytes((unsigned char*)buff, sizeof(buff), 0);
            if (ssid_cursor == i + offset)
            {
              M5.Lcd.fillRect((width - POPUP_WIDTH) / 2 + 2,
                              (height - POPUP_HEIGHT) / 2 + 25 + i * 16,
                              POPUP_WIDTH - 4, 16,
                              TFT_BLUE);
              M5.Lcd.setTextColor(TFT_WHITE, TFT_BLUE);
            }
            else
            {
              M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
            }
            M5.Lcd.drawString(buff, (width - POPUP_WIDTH) / 2 + 5, (height - POPUP_HEIGHT) / 2 + 25 + i * 16, 2);


          }
        }
        return;
      case  EN_SETTING_PASS:
        strcpy(buff, stPassword.buff);
        break;
      case  EN_SETTING_IPADDRESS:
        break;
      case  EN_SETTING_ATEM_IP:
        {
          IPAddress ipadr  = param.ATEMIP;
          sprintf(buff, "%d.%d.%d.%d", ipadr[0], ipadr[1], ipadr[2], ipadr[3]);
        }
        break;
      case  EN_SETTING_BRIGHTNESS:
        sprintf(buff, "%d", param.brightness);
        break;
      case  EN_SETTING_CAMNUM:
        break;
    }

    M5.Lcd.drawCentreString(buff, width / 2, height / 2, 2);

    return;
  }




  M5.Lcd.fillRect(0, 21, width, height - 41, TFT_BLACK);

  for (int i = 0; i < EN_SETTING_MAX; i++)
  {
    if (setting_cursor == i)
    {
      M5.Lcd.fillRect(TOP_X, TOP_Y + LINE_GAP * i, width - TOP_X * 2, LINE_GAP - 2, TFT_BLUE);
      M5.Lcd.setTextColor(TFT_WHITE, TFT_BLUE);
    }
    else
    {
      M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    M5.Lcd.drawString(settingmenu_str[i], TOP_X + 2, TOP_Y + 2 + LINE_GAP * i, 2);


    switch (i)
    {
      case  EN_SETTING_UNITID:
        sprintf(buff, "%d", param.deviceID + 1);
        break;
      case  EN_SETTING_SSID:
        //            WiFi.SSID().getBytes((unsigned char*)buff,sizeof(buff),0);
        memset(buff, 0, sizeof(buff));
        strcpy((char*)buff, (char*)param.ssid);
        break;
      case  EN_SETTING_PASS:
        sprintf(buff, "***********");
        break;
      case  EN_SETTING_IPADDRESS:
        {
          IPAddress ipadr  = WiFi.localIP();
          sprintf(buff, "%d.%d.%d.%d", ipadr[0], ipadr[1], ipadr[2], ipadr[3]);
        }
        break;
      case  EN_SETTING_ATEM_IP:
        {
          IPAddress ipadr  = param.ATEMIP;
          sprintf(buff, "%d.%d.%d.%d", ipadr[0], ipadr[1], ipadr[2], ipadr[3]);
        }
        break;
      case  EN_SETTING_BRIGHTNESS:
        sprintf(buff, "%d", param.brightness);
        break;
      case  EN_SETTING_CAMNUM:
        sprintf(buff, "%d", param.cameranum);
        break;
        //            memset(param.ssid,0,sizeof(param.ssid));
        //            WiFi.SSID(num).getBytes(param.ssid,sizeof(param.ssid),0);

    }
    M5.Lcd.drawString(buff, TOP_X + 100, TOP_Y + 2 + LINE_GAP * i, 2);
  }
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
}


char available_char[26+26+26+15] = {
  
  'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
  'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
  '0','1','2','3','4','5','6','7','8','9','@','!','\"',
  '#','$','%','&','\'','(',')','*','+',',','-','.','/',
  ':',';','<','>','=','?','[',']','\\','^','_','{','}','|','~'
};


int exe_keybaord(ST_KEYBOARD* key, int operation)
{
  int ret = 0;
  switch (operation)
  {
    case EN_KEY_UP:
      if (key->buff[key->wp] == 0)
      {
          key->tablenum = 0;        
      }
      else
      {
        key->tablenum++;
        if(key->tablenum >= sizeof(available_char))
        {
          key->tablenum = 0;  
        }  
      }
       key->buff[key->wp] = available_char[key->tablenum];
#if 0
      if (key->buff[key->wp] == 0)
      {
        key->buff[key->wp] = 0x21;
      }
      else if (key->buff[key->wp] == 0x7E)
      {
        key->buff[key->wp] = 0x21;
      }
      else
      {
        key->buff[key->wp]++;
      }
#endif
      break;
    case EN_KEY_DOWN:
#if 0
      if (key->buff[key->wp] == 0)
      {
        key->buff[key->wp] = 0x7E;
      }
      else if (key->buff[key->wp] == 0x21)
      {
        key->buff[key->wp] = 0x7E;
      }
      else
      {
        key->buff[key->wp]--;
      }
#else
      if (key->buff[key->wp] == 0)
      {
          key->tablenum = sizeof(available_char)-1;
      }
      else
      {
        key->tablenum--;
        if(key->tablenum <0)
        {
          key->tablenum = sizeof(available_char)-1;
        }  
      }
       key->buff[key->wp] = available_char[key->tablenum];

#endif
      break;
    case EN_KEY_BACKSPACE:
      key->buff[key->wp] = 0;
      if (key->wp == 0) {
        ret = 1;
      }
      else {
        key->wp --;
      }
      break;
    case EN_KEY_ENTER:
      if (key->buff[key->wp] == 0) {
        ret = 2;
      }
      else {
        key->wp++;
        //        key->buff[key->wp];
      }
      break;
    default:
      break;
  }
  return ret;
}


void exe_button_setting(int operation)
{
  int num;
  switch (setting_cursor)
  {
    case  EN_SETTING_UNITID:
      num = param.deviceID;
      num += operation;
      if (num < 0) {
        num = 0;
      }
      else if (num >= 4) {
        num = 4;
      }
      param.deviceID = num;
      break;
    case  EN_SETTING_SSID:

      ssid_cursor -= operation;
      if (ssid_cursor >= scannet_num)
      {
        ssid_cursor = 0;
      }
      if (ssid_cursor < 0)
      {
        ssid_cursor = scannet_num - 1;
      }
      break;
    case  EN_SETTING_PASS:
      break;
    case  EN_SETTING_IPADDRESS:
      break;
    case  EN_SETTING_ATEM_IP:
      {
        IPAddress local = WiFi.localIP();
        num = param.ATEMIP[3];
        num += operation;
        if (num < 0) {
          num = 0;
        }
        else if (num >= 255) {
          num = 255;
        }
        param.ATEMIP[0] = local[0];
        param.ATEMIP[1] = local[1];
        param.ATEMIP[2] = local[2];
        param.ATEMIP[3] = num;
      }
      break;
    case  EN_SETTING_BRIGHTNESS:
      num = param.brightness;
      num += operation;
      if (num < 1) {
        num = 1;
      }
      else if (num >= 100) {
        num = 100;
      }
      param.brightness = num;
      M5.Lcd.setBrightness(num * 2.55);
      break;
    case  EN_SETTING_CAMNUM:
      num = param.cameranum;
      num += operation;
      if (num < 0) {
        num = 0;
      }
      else if (num >= 16) {
        num = 16;
      }
      param.cameranum = num;

    
      break;
  }
}

void btn_Setup(void)
{
  static bool button_b_held = false;

	int btnA = M5.BtnA.wasPressed();
	int btnB = M5.BtnB.wasPressed();
	int btnC = M5.BtnC.wasPressed();
	if (btnA)
	{
		if (setting_Edit)
		{
			switch(setting_cursor)
			{
			case EN_SETTING_SSID:
				memset(param.ssid, 0, sizeof(param.ssid));
				WiFi.SSID(ssid_cursor).getBytes((unsigned char*)param.ssid, sizeof(param.ssid), 0);
				setting_Edit = false;
				break;
			case EN_SETTING_PASS:
				if (M5.BtnB.isPressed())
				{
					if ( exe_keybaord(&stPassword, EN_KEY_BACKSPACE))
					{
						setting_Edit = false;
					}
				}
				else
				{
					exe_keybaord(&stPassword, EN_KEY_DOWN);
				}
				break;
			default:
				setting_Edit = false;
				break;
			}
		}
		else
		{
			funcmode = EN_FUNCMODE_HOME;
			save_data();
			//Restart????
		}
		exe_display(true);
	}
	
	if (btnB)
	{
	    button_b_held = true;

		if (setting_Edit)
		{
			switch(setting_cursor)
			{
				case EN_SETTING_PASS:
					break;
				default:
					exe_button_setting(-1);
					break;
			}
		}
		else
		{
			setting_cursor++;
			if (setting_cursor >= EN_SETTING_MAX)
			{
				setting_cursor = 0;
			}
		}
		exe_display(true);
	}
	if (btnC)
	{
		if (setting_Edit)
		{
			switch(setting_cursor)
			{
			case EN_SETTING_PASS:
				if (M5.BtnB.isPressed())
				{
					if ( exe_keybaord(&stPassword, EN_KEY_ENTER))
					{
						memset(param.pass,0,sizeof(param.pass));
						strcpy(param.pass,stPassword.buff);
						setting_Edit = false;
					}
				}
				else
				{
					exe_keybaord(&stPassword, EN_KEY_UP);
				}
				break;
			default:
				exe_button_setting(+1);
				break;
			}
		}
		else
		{
			setting_Edit = true;
			switch(setting_cursor)
			{
			case EN_SETTING_SSID:
				{
					scannet_num = WiFi.scanNetworks();
				}
				break;
			case  EN_SETTING_PASS:
				{
					memset(&stPassword, 0, sizeof(stPassword));
				}
				break;			
			default:
				break;			
			}
		}
		exe_display(true);
	}
	if (M5.BtnB.isReleased())
	{
		if (button_b_held)
		{
			if (setting_Edit)
			{
				if (setting_cursor == EN_SETTING_PASS)
				{
				    Serial.println("Released");
				    exe_display(true);

				}
			}
			button_b_held = false;
		}
	}

}



int ReadSerial1Line(char* buff, int len)
{
  int wp = 0;
  while (1)
  {
    if (Serial.available() > 0)
    {
      char inputchar = Serial.read();
      if (inputchar != -1)
      {
        buff[wp] = inputchar;
        Serial.print(inputchar);
        if (wp >= len - 1 || inputchar == '\n' || inputchar == '\r')
        {
          break;
        }
        wp++;
      }
    }
  }
  Serial.print("\n");

  while (Serial.available() > 0)
  {
    Serial.read();
  }
  Serial.print("Read BYTE:");
  Serial.println(wp);
  return wp;

}

uint32 STR2IP(char* buff, int len)
{
  uint8 addr[4] = {0, 0, 0, 0};
  uint32 ret;
  int num = 0;

  for (int n = 0; n < len; n++)
  {
    if (buff[n] >= 0x30 && buff[n] <= 0x39)
    {
      addr[num] = addr[num] * 10 + (buff[n] & 0x0F);
    }
    else if (buff[n] == '.')
    {
      num++;
      if (num >= 4)
      {
        break;
      }
    }
    else
    {
      return 0;
    }
  }
  ret = addr[3] << 24 | addr[2] << 16 | addr[1] << 8 | addr[0];
  Serial.print(addr[0]);
  Serial.print(".");
  Serial.print(addr[1]);
  Serial.print(".");
  Serial.print(addr[2]);
  Serial.print(".");
  Serial.println(addr[3]);
  return ret;
}
int STR2DEC(char* buff, int len)
{
  int ret = 0;
  if (len <= 0) {
    return -1;
  }
  for (int i = 0; i < len; i++)
  {
    if (buff[i] >= 0x30 && buff[i] <= 0x39)
    {
      ret = ret * 10 + (buff[i] & 0x0F);
    }
    else
    {
      return -1;
    }
  }
  return ret;
}


void Setup_Wizard(void)
{
  int inputchar;
  int len;
  Serial.print("*****SETUP WIZARD*****\n");
  Serial.print("1. DEVICE ID. Enter 1-4.\n");
  while (1)
  {
    delay(10);
    len = ReadSerial1Line(buff, sizeof(buff));
    if (len == 1 && buff[0] >= 0x31 && buff[0] <= 0x34)
    {
      param.deviceID = (buff[0] & 0x0F) - 1;

      Serial.print("DEVICE ID:");
      Serial.println(buff[0] & 0x0F);
      break;
    }
  }

  Serial.print("2. WiFi Setting. Select SSID number.\n");
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print("    ");
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.println(WiFi.SSID(i));
      //      Serial.print(" (");
      //      Serial.print(WiFi.RSSI(i));
      //      Serial.print(")");
      //      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
      delay(10);
    }
    while (1)
    {
      delay(10);
      len = ReadSerial1Line(buff, sizeof(buff));
      int num = STR2DEC(buff, len);
      if (num > 0 && num <= n)
      {
        num -= 1;
        memset(param.ssid, 0, sizeof(param.ssid));
        WiFi.SSID(num).getBytes(param.ssid, sizeof(param.ssid), 0);
        break;
      }

    }
    char* out = (char*)param.ssid;
    Serial.print("SSID:");
    Serial.println(out);

    Serial.println("Enter Password");
    while (1)
    {
      delay(10);
      len = ReadSerial1Line(buff, sizeof(buff));
      if (len > 0)
      {
        memset(param.pass, 0, sizeof(param.pass));
        memcpy(param.pass, buff, len);
        break;
      }
    }
    Serial.println("Connection Test");
    while (WiFi.status() == WL_CONNECTED ) {
      WiFi.disconnect(true);
      delay(2000);
    }
    Serial.println("Disconnected");

    WiFi.begin((char*)param.ssid, (char*)param.pass);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println((char*)param.ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

  }

  Serial.print("3. Enter ATEM IP address\n");
  while (1)
  {
    len = ReadSerial1Line(buff, sizeof(buff));
    if (len)
    {
      uint32 ip = STR2IP(buff, len);
      IPAddress addr(ip);
      param.ATEMIP = addr;
      Serial.print("ATEM IP:");
      Serial.println(param.ATEMIP.toString());
      break;
    }
  }

  save_data();
  funcmode = EN_FUNCMODE_HOME;


}

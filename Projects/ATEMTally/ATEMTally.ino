#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Streaming.h>
#include <MemoryFree.h>
#include <SkaarhojPgmspace.h>
// Include ATEMbase library and make an instance:
// The port number is chosen randomly among high numbers.
#include <ATEMbase.h>
#include <ATEMmax.h>

#ifndef STASSID
#define STASSID "abcdefg"
#define STAPSK  "abcedfg"
#endif

#define SSID_MAXLEN 256
#define PASS_MAXLEN 256

typedef enum{
  EN_FUNCMODE_HOME = 0,
  EN_FUNCMODE_TALLY,
  EN_FUNCMODE_AUTOSWITCH,
  EN_FUNCMODE_SETUP,
  EN_FUNCMODE_MAX
}EN_FUNCMODE;
typedef enum{
  EN_BLACK = 0,
  EN_CAM1,
  EN_CAM2,
  EN_CAM3,
  EN_CAM4,
  EN_CAM_MAX
}EN_CAMNUM;

typedef enum{
  EN_SETUP_DEVICEID = 0,
  EN_SETUP_SSID,
  EN_SETUP_PASSWORD,
  EN_SETUP_ATEMIP,
  EN_SETUP_AUTOSWITCHTIME  
}EN_SETUPWIZARD_STEP;

typedef struct{
  
  uint8 deviceID;
  unsigned char ssid[SSID_MAXLEN];
  char pass[PASS_MAXLEN];
  IPAddress ATEMIP;
  uint8 AutoSwitchMode;
  int AutoSwitchTime;
}ST_PARAM;

const char* ssid = STASSID;
const char* password = STAPSK;

IPAddress switcherIp(192, 168, 11, 99);     // <= SETUP!  IP address of the ATEM Switcher

ATEMmax AtemSwitcher;
EN_FUNCMODE funcmode = EN_FUNCMODE_HOME;
ST_PARAM param;

unsigned long lastAutoFocus;
unsigned long lastAutoIris;
void initParam(void)
{
  memset(&param,0,sizeof(param));
  param.deviceID = 0;
  param.AutoSwitchTime = 5000; //5sec
  
}

uint32 gTime = 0;
ETSTimer timer;
int Interval = 1;//タイマーの周期．単位はmsec．
void timer_interrupt(void*)
{
  gTime++;
}

void initTimer()
{
    system_timer_reinit();
    os_timer_setfn(&timer, timer_interrupt, NULL);
    os_timer_arm(&timer,Interval, true);
}

void setup() {
  initParam();
  initTimer();
  randomSeed(analogRead(5));  // For random port selection

  // Start the Ethernet, Serial (debugging) and UDP:
//  Ethernet.begin(mac, clientIp);
  Serial.begin(115200);
  Serial << F("\n- - - - - - - -\nSerial Started\n");


  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());



  // Initialize a connection to the switcher:
  AtemSwitcher.begin(switcherIp);
  AtemSwitcher.serialOutput(0x80);
  AtemSwitcher.connect();


  lastAutoFocus = millis();
  lastAutoIris = millis() + 2500;

  Serial.print("a:AUTO SWITCHER MODE\n");
  Serial.print("t:TALLY MODE\n");
  Serial.print("s:SETUP WIZARD\n");


}


int ReadSerial1Line(char* buff, int len)
{
  int wp = 0;
  while(1)
  {
    if (Serial.available() > 0) 
    {
      char inputchar = Serial.read();
      if(inputchar != -1)
      {
        buff[wp] = inputchar;
        Serial.print(inputchar);
        if(wp >= len-1 || inputchar == '\n' || inputchar == '\r')
        {
          break;
        }
        wp++;
      }
    }
  }
 Serial.print("\n");

 while(Serial.available() > 0) 
 {
   Serial.read();
 }
 Serial.print("Read BYTE:");
 Serial.println(wp);
 return wp;

}

char buff[128];
uint32 STR2IP(char* buff,int len)
{
  uint8 addr[4] = {0,0,0,0};
  uint32 ret;
  int num = 0;

    for(int n = 0;n<len;n++)
    {
      if(buff[n]>=0x30 && buff[n]<=0x39)
      {
        addr[num] = addr[num]*10+(buff[n]&0x0F); 
      }
      else if(buff[n] == '.')
      {
        num++;
        if(num >=4)
        {
          break;
        }
      }
      else
      {
        return 0;
      }
    }
    ret = addr[3]<<24 | addr[2]<<16 | addr[1]<<8 | addr[0];
    Serial.print(addr[0]);
    Serial.print(".");
    Serial.print(addr[1]);
    Serial.print(".");
    Serial.print(addr[2]);
    Serial.print(".");
    Serial.println(addr[3]);
    return ret;
}
int STR2DEC(char* buff,int len)
{
    int ret = 0;
    if(len<=0){return -1;}
    for(int i = 0;i<len;i++)
    {
       if(buff[i] >= 0x30 && buff[i] <= 0x39)
       {
         ret = ret*10 + (buff[i]&0x0F);
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

  while(1)
  {
    delay(10);
    len = ReadSerial1Line(buff,sizeof(buff));
    if(len == 1 && buff[0] >= 0x31 && buff[0] <= 0x34)
    {
        param.deviceID = (buff[0] & 0x0F)-1;
        
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
    while(1)
    {
      delay(10);
      len = ReadSerial1Line(buff,sizeof(buff));
      int num = STR2DEC(buff,len);
      if(num >0 && num <= n)
      {
        num-=1;
        memset(param.ssid,0,sizeof(param.ssid));
        WiFi.SSID(num).getBytes(param.ssid,sizeof(param.ssid),0);
        break;  
      }
      
    }
    char* out = (char*)param.ssid;
    Serial.print("SSID:");
    Serial.println(out);

    Serial.println("Enter Password");
    while(1)
    {
      delay(10);
      len = ReadSerial1Line(buff,sizeof(buff));
      if(len >0)
      {
        memset(param.pass,0,sizeof(param.pass));
        memcpy(param.pass,buff,len);
        break;
      }    
    }
    Serial.println("Connection Test");
    while(WiFi.status() == WL_CONNECTED ){
       WiFi.disconnect(true);
      delay(2000);
    }
    Serial.println("Disconnected");
    
    WiFi.begin((char*)param.ssid,(char*)param.pass);
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
  while(1)
  {
    len = ReadSerial1Line(buff,sizeof(buff));
    if(len)
    {
      uint32 ip = STR2IP(buff,len);
      IPAddress addr(ip);
      param.ATEMIP=addr;
      Serial.print("ATEM IP:");
      Serial.println(param.ATEMIP.toString());
    break;
    }
  }
  funcmode = EN_FUNCMODE_HOME;

  
}



void exe_TallyMode(void)
{
  uint8 tally = AtemSwitcher.getTallyByIndexTallyFlags(param.deviceID);
  static uint8 tally_bak = 0;
  if(tally_bak != tally)
  {
    if ( tally & 0x01)
    {
      Serial.print("RED\n");
    }
    else if (tally & 0x02)
    {
      Serial.print("GREEN\n");
    }
    else
    {
      Serial.print("LED OFF\n");
    }
    tally_bak = tally;
  }
}


void exe_AutoSwitcherMode(void)
{
  static uint32 lastExeTime =0;
  static uint8 currentCam = EN_CAM1;
  static uint8 ME = 0;

  if(gTime < lastExeTime + param.AutoSwitchTime)
  {return;}
  lastExeTime = gTime;

      Serial.print("Program Input:");
  Serial.println(currentCam);
  AtemSwitcher.setProgramInputVideoSource(ME,currentCam);
  currentCam++;
  if(currentCam>=EN_CAM_MAX)
  {
//    ME= ME?0:1;
    currentCam = EN_CAM1;
  }
//  delay(param.AutoSwitchTime);

  
}

void SerialInputCheck(void)
{
  int inputchar;
  inputchar = Serial.read();
 
  if(inputchar != -1 ){
    // 読み込んだデータが -1 以外の場合　以下の処理を行う
 
    switch(inputchar){
      case 'a':
        // 読み込みデータが　o の場合
 
        Serial.print("*****AUTO SWITCH MODE*****\n");
        funcmode = EN_FUNCMODE_AUTOSWITCH;

        break;
      case 't':  
        // 読み込みデータが　p の場合
         Serial.print("*****TALLY MODE*****\n");
        funcmode = EN_FUNCMODE_TALLY;

        break;
      case 's':
        funcmode = EN_FUNCMODE_SETUP;
      default:
      break;
    }
  } else {
    // 読み込むデータが無い場合は何もしない
  }  
}

void loop() 
{
  //  AtemSwitcher.setCameraControlVideomode(1, 24, 6, 0);
  AtemSwitcher.runLoop();
  switch(funcmode)
  {
    case EN_FUNCMODE_HOME:
    break;
    case EN_FUNCMODE_TALLY:
    exe_TallyMode();
    break;
    case EN_FUNCMODE_AUTOSWITCH:
    exe_AutoSwitcherMode();
    break;
    case EN_FUNCMODE_SETUP:
    Setup_Wizard();
    default:
    break;
  }

  SerialInputCheck();
}

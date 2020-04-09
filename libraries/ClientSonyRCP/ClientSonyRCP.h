#ifndef ClientSonyRCP_h
#define ClientSonyRCP_h

#define ClientSonyRCP_Cams 1

#include <Arduino.h>
#include <EthernetUdp.h>

class ClientSonyRCP {
private:
  bool _hasInitialized;
  IPAddress _switcherIP; // IP address of the switcher
  uint8_t _serialOutput;


public:
  ClientSonyRCP();

  void serialOutput(uint8_t level);
  bool hasInitialized();

  void begin(const IPAddress ip);
  void runLoop(uint16_t delayTime = 0);
};

#endif

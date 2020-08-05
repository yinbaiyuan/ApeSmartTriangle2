#ifndef TriangleProtocol_h
#define TriangleProtocol_h

#include <inttypes.h>
#include <Arduino.h>

#define TP_PARSE_CALLBACK void (*callback)(byte, uint8_t*, unsigned int ,bool)
#define TP_TRANSMIT_CALLBACK void (*trans_callback)(uint8_t*, unsigned int)

#define DEFAULT_PROTOCO_TIMEOUT        200

class TriangleProtocol
{
  private:
    TP_PARSE_CALLBACK;
    TP_TRANSMIT_CALLBACK;
    void protocolTimeoutRemove(uint8_t pId);
  public:
    TriangleProtocol();
    ~TriangleProtocol();

    void callbackRegister(TP_PARSE_CALLBACK,TP_TRANSMIT_CALLBACK);

    TriangleProtocol &tpBegin(byte pid);
    TriangleProtocol &tpByte(byte b);
    TriangleProtocol &tpColor(byte r,byte g,byte b);
    TriangleProtocol &tpStr(const String &str,bool pushSize = false);
    void tpTransmit(bool checkTimeout = false);

    TriangleProtocol &tpBeginReceive();
    TriangleProtocol &tpPushData(uint8_t d);
    void tpParse();
    
    void waitProtocolTimeout(uint8_t pId,uint32_t timeout = DEFAULT_PROTOCO_TIMEOUT);
    void protocolLoop();

};

extern TriangleProtocol TPT;

#endif

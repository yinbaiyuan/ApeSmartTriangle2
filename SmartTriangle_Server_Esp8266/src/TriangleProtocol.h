#ifndef TriangleProtocol_h
#define TriangleProtocol_h

#include <inttypes.h>
#include <Arduino.h>

#define TP_PARSE_CALLBACK void (*callback)(byte, uint8_t*, unsigned int ,bool)
#define TP_TRANSMIT_CALLBACK void (*trans_callback)(uint8_t*, unsigned int)

#define GLOBAL_DEFAULT_COLOR   0xFFFFFF      //默认文字颜色
#define PROTOCO_TIMEOUT        200

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
    TriangleProtocol &tpColor(byte r = (GLOBAL_DEFAULT_COLOR >> 16) & 0xFF,
                           byte g = (GLOBAL_DEFAULT_COLOR >> 8) & 0xFF,
                           byte b = (GLOBAL_DEFAULT_COLOR >> 0) & 0xFF);
    TriangleProtocol &tpStr(const String &str,bool pushSize = false);
    void tpTransmit(bool checkTimeout = false);

    TriangleProtocol &tpBeginReceive();
    TriangleProtocol &tpPushData(uint8_t d);
    void tpParse();
    void waitProtocolTimeout(uint8_t pId,uint32_t timeout = PROTOCO_TIMEOUT);
    void protocolLoop();

};

extern TriangleProtocol TPT;

#endif

#ifndef TriangleProtocol_h
#define TriangleProtocol_h

#include <inttypes.h>
#include <Arduino.h>

#define TP_CALLBACK_SIGNATURE void (*callback)(byte, uint8_t*, unsigned int)
#define GLOBAL_DEFAULT_COLOR   0xFFFFFF      //默认文字颜色

class TriangleProtocol
{
  private:
    TP_CALLBACK_SIGNATURE;
  public:
    TriangleProtocol();
    ~TriangleProtocol();
    void callbackRegister(TP_CALLBACK_SIGNATURE);

    TriangleProtocol &tpBegin(byte pid);
    TriangleProtocol &tpByte(byte b);
    TriangleProtocol &tpColor(byte r = (GLOBAL_DEFAULT_COLOR >> 16) & 0xFF,
                           byte g = (GLOBAL_DEFAULT_COLOR >> 8) & 0xFF,
                           byte b = (GLOBAL_DEFAULT_COLOR >> 0) & 0xFF);
    TriangleProtocol &tpStr(const String &str,bool pushSize = false);
    void tpTansmit(uint8_t **ptBuffer,uint8_t *ptLength);

    TriangleProtocol &tpBeginReceive();
    TriangleProtocol &tpPushData(uint8_t d);
    void tpParse();
    

};

extern TriangleProtocol TPT;

#endif

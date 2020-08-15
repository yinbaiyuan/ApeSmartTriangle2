#include "HalfDuplexSerial.h"
#include "TriangleProtocol.h"
#include "ArduiDispatch.h"
#include "ADHueAction.h"
#include "ADBrightnessAction.h"
#include <FastLED.h>
#include "MemoryFree.h"

#define PROTOCOL_VER "0.1.0"

#define PROTOCOL_CALLBACK(x) void pId_callback_##x(byte *payload, unsigned int length, bool isTimeout)

#define PORTOCOL_REGISTER(x)                                      \
  {                                                               \
    if (isTimeout || m_nodeId == payload[0] || payload[0] == 255) \
    {                                                             \
      pId_callback_##x(payload + 1, length - 1, isTimeout);       \
    }                                                             \
  }                                                               \
  break

#define HDSERIAL_PIN 10

#define NUM_LEDS 15

#define LED_PIN 4

enum TriangleStateType
{
  TST_NONE = 0,
  TST_WATING_LOCATE = 1,
  TST_LOCATED = 2,
};

HalfDuplexSerial hdSerial(HDSERIAL_PIN);

CRGB leds[NUM_LEDS];

int selectPin[3] = {6, 7, 8};

uint16_t ledNumOffset = 0;

TriangleStateType m_triangleStateType;

int m_fatherNodePin = -1;

int m_leftLeafNodePin = -1;

int m_rightLeafNodePin = -1;

uint8_t m_nodeId = 254; // 254 代表未分配ID

uint16_t ledNumsConverter(uint16_t n)
{
  return (n + ledNumOffset) % NUM_LEDS;
}

void startSelect(bool isLeft)
{
  if (isLeft)
  {
    pinMode(m_leftLeafNodePin, OUTPUT);
    digitalWrite(m_leftLeafNodePin, HIGH);
  }
  else
  {
    pinMode(m_rightLeafNodePin, OUTPUT);
    digitalWrite(m_rightLeafNodePin, HIGH);
  }
}

void stopSelect(bool isLeft)
{
  if (isLeft)
  {
    pinMode(m_leftLeafNodePin, INPUT);
  }
  else
  {
    pinMode(m_rightLeafNodePin, INPUT);
  }
}

bool seekFatherPin()
{
  if (m_triangleStateType == TST_LOCATED)
  {
    return false;
  }
  m_fatherNodePin = -1;
  for (int i = 0; i < 3; i++)
  {
    if (digitalRead(selectPin[i]) == HIGH)
    {
      m_fatherNodePin = selectPin[i];
      m_leftLeafNodePin = selectPin[(i - 1) < 0 ? 2 : (i - 1)];
      m_rightLeafNodePin = selectPin[(i + 1) > 2 ? 0 : (i + 1)];
      if (m_fatherNodePin == 6)
      {
        ledNumOffset = 5;
      }
      else if (m_fatherNodePin == 8)
      {
        ledNumOffset = 10;
      }
      break;
    }
  }
  if (m_fatherNodePin != -1)
    return true;
  else
    return false;
}

void topologyCheck()
{
}

void flush()
{
  for (int i = 0; i < 3; i++)
  {
    pinMode(selectPin[i], INPUT);
  }
  m_triangleStateType = TST_NONE;
  m_fatherNodePin = -1;
  m_leftLeafNodePin = -1;
  m_rightLeafNodePin = -1;
  m_nodeId = 254;
  Director.flush();
  Director.startAction(millis());
  FastLED.clear();
  FastLED.show();
  Serial.println("TOPOLOGY RESET");
}

void transmitAction()
{
  hdSerial.setMode(SMT_TRANSMIT);
  TPT.tpTransmit();
  hdSerial.setMode(SMT_RECEIVE);
  TPT.tpBeginReceive();
}


/*
  协议类型
  1~50 拓扑仲裁单向消息
  51~100 拓扑仲裁需应答消息
  101~255 效果展示类消息
*/

//重置状态
PROTOCOL_CALLBACK(1)
{
  flush();
  LEDS.setBrightness(payload[0]);
}

//分配ID
PROTOCOL_CALLBACK(2)
{
  if (m_triangleStateType == TST_WATING_LOCATE)
  {
    m_nodeId = payload[0];
    Serial.println("GET NODE ID:" + String(m_nodeId));
    m_triangleStateType = TST_LOCATED;
  }
}

//左仲裁接脚高电平
PROTOCOL_CALLBACK(21)
{
  startSelect(true);
}

//左仲裁接脚低电平
PROTOCOL_CALLBACK(22)
{
  stopSelect(true);
}

//右仲裁接脚高电平
PROTOCOL_CALLBACK(23)
{
  startSelect(false);
}

//右仲裁接脚低电平
PROTOCOL_CALLBACK(24)
{
  stopSelect(false);
}

//查找上级仲裁接脚-根节点查找
PROTOCOL_CALLBACK(51)
{
  if (seekFatherPin())
  {
    //确认上级节点位置
    m_triangleStateType = TST_WATING_LOCATE;
    TPT.tpBegin(51, 0);
    transmitAction();
  }
}

//查找上级仲裁接脚-子节点
PROTOCOL_CALLBACK(52)
{
  if (seekFatherPin())
  {
    //确认上级节点位置
    m_triangleStateType = TST_WATING_LOCATE;
    TPT.tpBegin(52, 0);
    transmitAction();
  }
}

//心跳
PROTOCOL_CALLBACK(100)
{
  TPT.tpBegin(100, 0).tpByte(m_nodeId);
  transmitAction();
}

void effect101Callback(uint8_t b, void *e)
{
  LEDS.setBrightness(b);
  FastLED.show();
}

//在[0][1]毫秒内，所有灯亮度从当前亮度改为[2],如时间为0，则直接修改
PROTOCOL_CALLBACK(101)
{
  uint16_t t = uint16_t(payload[0] << 8) + uint16_t(payload[1]);
  uint8_t b = payload[2];
  if (t > 0)
  {
    ADBrightnessAction *action = ADBrightnessAction::create(effect101Callback, LEDS.getBrightness(), b, t);
    ADActor *actor = ADActor::create(t, action, true);
    Director.addActor(actor);
  } else
  {
    effect101Callback(b, NULL);
  }
}

void effect102Callback(uint8_t h, void *e)
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[ledNumsConverter(i)] = CHSV(h, 255, 255);
  }
  FastLED.show();
}

//在[0][1]毫秒内，所有灯颜色从0号灯当前颜色改为颜色[2],如时间为0，则直接修改
PROTOCOL_CALLBACK(102)
{
  uint16_t t = uint16_t(payload[0] << 8) + uint16_t(payload[1]);
  uint8_t h = payload[2];
  if (t > 0)
  {
    ADHueAction *action = ADHueAction::create(effect102Callback, rgb2hsv_approximate(leds[0]).h, h, t);
    ADActor *actor = ADActor::create(t, action, true);
    Director.addActor(actor);
    //    effect102Callback(h, NULL);
  } else
  {
    effect102Callback(h, NULL);
  }
}

uint8_t m_hueInterval = 10;

void effect103Callback(uint8_t h, void *e)
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[ledNumsConverter(i)] = CHSV((h + i * m_hueInterval) % 256, 255, 255);
  }
  FastLED.show();
}

//在[0][1]毫秒内，将灯颜色从颜色[2]变为颜色[3]，每一个灯珠色相值增加[4],如时间为0，则直接修改
PROTOCOL_CALLBACK(103)
{
  uint16_t t = uint16_t(payload[0] << 8) + uint16_t(payload[1]);
  uint8_t h1 = payload[2];
  uint8_t h2 = payload[3];
  m_hueInterval = payload[4];

  if (t > 0)
  {
    ADHueAction *action = ADHueAction::create(effect103Callback, h1, h2, t);
    ADActor *actor = ADActor::create(t, action, true);
    Director.addActor(actor);
  } else
  {
    effect103Callback(h2, NULL);
  }
}

void effect104Callback(uint8_t h, void *e)
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[ledNumsConverter(i)] = CHSV((h + (i / 5) * m_hueInterval) % 256, 255, 255);
  }
  FastLED.show();
}

//在[0][1]毫秒内，将灯颜色从颜色[2]变为颜色[3]，每五个灯珠为一组色相值增加[4],如时间为0，则直接修改
PROTOCOL_CALLBACK(104)
{
  uint16_t t = uint16_t(payload[0] << 8) + uint16_t(payload[1]);
  uint8_t h1 = payload[2];
  uint8_t h2 = payload[3];
  m_hueInterval = payload[4];
  if (t > 0)
  {
    ADHueAction *action = ADHueAction::create(effect104Callback, h1, h2, t);
    ADActor *actor = ADActor::create(t, action, true);
    Director.addActor(actor);
  } else
  {
    effect104Callback(h2, NULL);
  }
}

void effect200Callback(uint32_t n, void *e)
{
  uint8_t c = (n % 2 == 1) ? 200 : 0;
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB(c, c, c);
  }
  FastLED.show();
}

PROTOCOL_CALLBACK(200)
{
  //  Serial.println("200call");
  ADAction *action = ADAction::create(effect200Callback, 200, 2);
  ADActor *actor = ADActor::create(1000, action, true);
  Director.addActor(actor);

}

void tpCallback(byte pId, byte *payload, unsigned int length, bool isTimeout)
{
  Serial.println("<<==Rec. " + String(pId) + (isTimeout ? " T " : " F ") + String(freeMemory()));
  if (m_triangleStateType == TST_NONE && pId >= 100)
  {
    Serial.println("==>>");
    return;
  }
  switch (pId)
  {
    case 1:
      PORTOCOL_REGISTER(1);
    case 2:
      PORTOCOL_REGISTER(2);
    case 21:
      PORTOCOL_REGISTER(21);
    case 22:
      PORTOCOL_REGISTER(22);
    case 23:
      PORTOCOL_REGISTER(23);
    case 24:
      PORTOCOL_REGISTER(24);
    case 51:
      PORTOCOL_REGISTER(51);
    case 52:
      PORTOCOL_REGISTER(52);
    case 100:
      PORTOCOL_REGISTER(100);
    case 101:
      PORTOCOL_REGISTER(101);
    case 102:
      PORTOCOL_REGISTER(102);
    case 103:
      PORTOCOL_REGISTER(103);
    case 104:
      PORTOCOL_REGISTER(104);
    case 200:
      PORTOCOL_REGISTER(200);
  }
  Serial.println("==>>");
}

void transmitCallback(byte *ptBuffer, unsigned int ptLength)
{
  //  Serial.print("Sent pID=" + String(ptBuffer[3]));
  //  Serial.println(" Length=" + String(ptLength) + " ");
  //  ADLOG_V(freeMemory());
  for (int i = 0; i < ptLength; i++)
  {
    byte c = ptBuffer[i];
    hdSerial.write(c);
  }
}

void setup()
{
  pinMode(A0, INPUT);

  Serial.begin(9600);

  delay(100);

  Serial.println("STARTING.......");

  LEDS.addLeds<WS2813, LED_PIN, RGB>(leds, NUM_LEDS);

  LEDS.setBrightness(200);

  hdSerial.begin(9600);

  hdSerial.setMode(SMT_RECEIVE);

  pinMode(11, INPUT_PULLUP);

  TPT.callbackRegister(tpCallback, transmitCallback);

  TPT.tpBeginReceive();

  flush();

  Director.begin(true);

  Director.startAction(millis());
}

void loop()
{
  TPT.protocolLoop();
  if (hdSerial.serialModeType() == SMT_RECEIVE)
  {
    while (hdSerial.available())
    {
      TPT.tpPushData(hdSerial.read()).tpParse();
    }
  }

  Director.loop(millis());

}

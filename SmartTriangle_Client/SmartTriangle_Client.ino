#include "HalfDuplexSerial.h"
#include "TriangleProtocol.h"
#include "ArduiDispatch.h"
#include "ADHueAction.h"
#include "MemoryFree.h"
#include <FastLED.h>

#define HDSERIAL_PIN  10
#define NUM_LEDS      15
#define LED_PIN       4

enum TriangleStateType
{
  TST_NONE          = 0,
  TST_WATING_LOCATE = 1,
  TST_LOCATED       = 2,
};

HalfDuplexSerial hdSerial(HDSERIAL_PIN);

CRGB leds[NUM_LEDS];

int selectPin[3] = {6, 7, 8};

uint16_t ledNumOffset = 0;

TriangleStateType m_triangleStateType;

int m_fatherNodePin     = -1;

int m_leftLeafNodePin   = -1;

int m_rightLeafNodePin  = -1;

int m_nodeId            = -1;

uint16_t ledNumsConverter(uint16_t n)
{
  return (n + ledNumOffset) % NUM_LEDS;
}

void startSelect(bool isLeft)
{
  Serial.print(isLeft ? "LEFT NODE " + String(m_leftLeafNodePin) : "RIGHT NODE " + String(m_rightLeafNodePin));
  Serial.println(" PIN HIGH");
  if (isLeft)
  {
    pinMode(m_leftLeafNodePin, OUTPUT);
    digitalWrite(m_leftLeafNodePin, HIGH);
  } else
  {
    pinMode(m_rightLeafNodePin, OUTPUT);
    digitalWrite(m_rightLeafNodePin, HIGH);
  }
}

void stopSelect(bool isLeft)
{
  Serial.print(isLeft ? "LEFT NODE " + String(m_leftLeafNodePin) : "RIGHT NODE " + String(m_rightLeafNodePin));
  Serial.println(" PIN INPUT");
  if (isLeft)
  {
    pinMode(m_leftLeafNodePin, INPUT);
  } else
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
      effectInit();
      Serial.println("FOUND FATHER PIN " + String(m_fatherNodePin));
      break;
    }
  }
  if (m_fatherNodePin != -1) return true;
  else return false;
}



void topologyCheck()
{

}

void topologyInit()
{
  for (int i = 0; i < 3; i++)
  {
    pinMode(selectPin[i], INPUT);
  }
  m_triangleStateType = TST_NONE;
  m_fatherNodePin = -1;
  m_leftLeafNodePin = -1;
  m_rightLeafNodePin = -1;
  m_nodeId = -1;
  //  Director.flush();
  Serial.println("TOPOLOGY RESET");
}

void transmitAction()
{
  hdSerial.setMode(SMT_TRANSMIT);
  TPT.tpTransmit();
  hdSerial.setMode(SMT_RECEIVE);
  TPT.tpBeginReceive();
}

void effectInit()
{
  LEDS.setBrightness(200);
}



void effect101Callback(unsigned int n, unsigned int c, void *e)
{
  //  STEffectCallbackDef *effect = (STEffectCallbackDef *)e;
  //  uint8_t h = (long(effect->custom8[1]) - effect->custom8[0]) * (c + 1) / effect->frameCount +  effect->custom8[0];
  //  for (int i = 0; i < NUM_LEDS; i++) {
  //    leds[ledNumsConverter(i)] = CHSV(h, 255, 255);
  //  }
  //  FastLED.show();
}

void effect102Callback(uint8_t h, void *e)
{
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[ledNumsConverter(i)] = CHSV(h, 255, 255);
  }
  FastLED.show();
}

void effect103Callback(uint8_t h, void *e)
{
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[ledNumsConverter(i)] = CHSV((h + i * 10) % 256, 255, 255);
  }
  FastLED.show();
}

void tpCallback(byte pId, byte *payload, unsigned int length , bool isTimeout)
{
  Serial.println("tpCallback pId:" + String(pId) + " Timeout:" + String(isTimeout ? "True" : "False"));
  ADLOG_V(freeMemory());
  switch (pId)
  {
    case 1:
      {
        topologyInit();
      }
      break;
    case 2:
      {
        Serial.println("m_triangleStateType:" + String(m_triangleStateType));
        if (m_triangleStateType == TST_WATING_LOCATE)
        {
          m_nodeId = payload[0];
          Serial.println("NODE ID:" + String(m_nodeId));
          m_triangleStateType = TST_LOCATED;
        }
      }
      break;
    case 21:
      {
        if (m_nodeId == payload[0])
        {
          startSelect(true);
        }
      }
      break;
    case 22:
      {
        if (m_nodeId == payload[0])
        {
          stopSelect(true);
        }
      }
      break;
    case 23:
      {
        if (m_nodeId == payload[0])
        {
          startSelect(false);
        }
      }
      break;
    case 24:
      {
        if (m_nodeId == payload[0])
        {
          stopSelect(false);
        }
      }
      break;
    case 51:
      {
        if (seekFatherPin())
        {
          //确认上级节点位置
          m_triangleStateType = TST_WATING_LOCATE;
          TPT.tpBegin(51);
          transmitAction();
        }
      }
      break;
    case 52:
      {
        if (seekFatherPin())
        {
          //确认上级节点位置
          m_triangleStateType = TST_WATING_LOCATE;
          TPT.tpBegin(52);
          transmitAction();
        }
      }
      break;
    case 101:
      {
        if (m_nodeId == payload[0] || payload[0] == 255)
        {
          uint16_t t = uint16_t(payload[1] << 8) + uint16_t(payload[2]);
          //          ADHueAction *action = ADHueAction::create(effect101Callback, payload[3], payload[4], t);
          //          ADActor *testActor = ADActor::create(t, action);
          //          Director.addActor(testActor);
        }
      }
      break;
    case 102:
      {
        if (m_nodeId == payload[0] || payload[0] == 255)
        {
          uint16_t t = uint16_t(payload[1] << 8) + uint16_t(payload[2]);
          ADHueAction *action = ADHueAction::create(effect102Callback, rgb2hsv_approximate(leds[0]).h, payload[3], t );
          ADActor *actor = ADActor::create(t, action, true);
          Director.addActor(actor);
        }
      }
      break;
    case 103:
      {
        if (m_nodeId == payload[0] || payload[0] == 255)
        {
          uint16_t t = uint16_t(payload[1] << 8) + uint16_t(payload[2]);
          ADHueAction *action = ADHueAction::create(effect103Callback, payload[3], payload[4], t);
          ADActor *testActor = ADActor::create(t, action, true);
          Director.addActor(testActor);
        }
      }
      break;
    case 104:
      {
        if (m_nodeId == payload[0] || payload[0] == 255)
        {
          uint8_t brightness = payload[1];
          LEDS.setBrightness(brightness);
        }
      }
      break;
  }
}

void transmitCallback(byte *ptBuffer, unsigned int ptLength)
{
  for (int i = 0; i < ptLength; i++)
  {
    byte c = ptBuffer[i];
    Serial.write(c);
    hdSerial.write(c);
  }
}

void setup()
{
  pinMode(A0, INPUT);

  Serial.begin(9600);

  Serial.println("STARTING.......");

  LEDS.addLeds<WS2813, LED_PIN, RGB>(leds, NUM_LEDS);

  LEDS.setBrightness(200);

  hdSerial.begin(9600);

  hdSerial.setMode(SMT_RECEIVE);

  pinMode(11, INPUT_PULLUP);

  TPT.callbackRegister(tpCallback, transmitCallback);

  TPT.tpBeginReceive();

  topologyInit();

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

#include "HalfDuplexSerial.h"
#include "TriangleProtocol.h"
#include <FastLED.h>
#include <Vector.h>



#define HDSERIAL_PIN 10
#define NUM_LEDS 15
#define LED_PIN 4
CRGB leds[NUM_LEDS];

enum TriangleStateType
{
  TST_NONE = 0,
  TST_WATING_LOCATE = 1,
  TST_LOCATED = 2,
};

struct STEffectCallbackDef
{
  uint8_t   effectId;
  uint16_t  callbackTimes;
  uint32_t  autoChangeTime;
  uint32_t  currentChangeTime;
  uint16_t  frameCount;
  uint32_t  frameRefreshTime;   //ms
  uint16_t  currentFrameCount;
  uint32_t  currentRefreshTime;
};

STEffectCallbackDef *stEffectCallbackDef_array[32];
Vector<STEffectCallbackDef *> stEffectCallbackVec;

HalfDuplexSerial hdSerial(HDSERIAL_PIN);

int selectPin[3] = {6, 7, 8};
uint16_t ledNumOffset = 0;

TriangleStateType m_triangleStateType;
int m_fatherNodePin = -1;
int m_leftLeafNodePin = -1;
int m_rightLeafNodePin = -1;
int m_nodeId = -1;

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

void tpCallback(byte pId, byte *payload, unsigned int length , bool isTimeout)
{
  Serial.println("tpCallback pId:" + String(pId) + " Timeout:" + String(isTimeout ? "True" : "False"));
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
        if (m_nodeId == payload[0])
        {
          for (int i = 0; i < NUM_LEDS; i++) {
            leds[ledNumsConverter(i)] = CHSV(payload[1], payload[2], payload[3]);
          }
          FastLED.show();
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
    hdSerial.write(c);
  }
}

void setup()
{
  LEDS.addLeds<WS2813, LED_PIN, RGB>(leds, NUM_LEDS);
  stEffectCallbackVec.setStorage(stEffectCallbackDef_array);
  hdSerial.begin(9600);
  hdSerial.setMode(SMT_RECEIVE);
  pinMode(11, INPUT_PULLUP);
  TPT.callbackRegister(tpCallback, transmitCallback);
  TPT.tpBeginReceive();
  topologyInit();

  Serial.begin(9600);
  pinMode(2, INPUT);
  Serial.println("STARTING.......");
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
}

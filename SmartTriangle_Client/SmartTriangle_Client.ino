#include "HalfDuplexSerial.h"
#include "TriangleProtocol.h"
#include <FastLED.h>
#include <Vector.h>

#define HDSERIAL_PIN 10
#define NUM_LEDS 15
#define LED_PIN 4
CRGB leds[NUM_LEDS];
CRGB targetColor;
enum TriangleStateType
{
  TST_NONE = 0,
  TST_WATING_LOCATE = 1,
  TST_LOCATED = 2,
};

typedef void (*EffectCallback)(unsigned int, unsigned int, void *effect);
struct STEffectCallbackDef
{
  uint16_t  callbackTimes;
  uint16_t  frameCount;
  uint32_t  frameRefreshTime;   //ms
  uint16_t  currentFrameCount;
  uint32_t  currentRefreshTime;
  uint8_t   custom8[6];
  uint16_t  custom16[3];
  uint32_t  custom32[1];
  EffectCallback callback;
};
uint32_t preCheckTime = 0;
STEffectCallbackDef *stEffectCallbackDef_array[32];
Vector<STEffectCallbackDef *> stEffectCallbackVec;

STEffectCallbackDef *effectCreate(uint16_t n, uint16_t c, uint32_t t, EffectCallback callback)
{
  STEffectCallbackDef *effect = new STEffectCallbackDef();
  effect->callbackTimes = n;
  effect->frameCount = c;
  effect->currentFrameCount = 0;
  effect->frameRefreshTime = t;
  effect->currentRefreshTime = t;
  effect->callback = callback;
  stEffectCallbackVec.push_back(effect);
  return effect;
}

void effectDelete(STEffectCallbackDef *effect)
{
  for (int i = stEffectCallbackVec.size() - 1; i >= 0; i--)
  {
    STEffectCallbackDef *e = stEffectCallbackVec[i];
    if (effect == e)
    {
      stEffectCallbackVec.remove(i);
      delete effect;
    }
  }
}

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

void effect102Callback(unsigned int n, unsigned int c, void *e)
{
  STEffectCallbackDef *effect = (STEffectCallbackDef *)e;
  uint8_t r = (long(effect->custom8[0]) - effect->custom8[3]) * (c + 1) / effect->frameCount +  effect->custom8[3];
  uint8_t g = (long(effect->custom8[1]) - effect->custom8[4]) * (c + 1) / effect->frameCount +  effect->custom8[4];
  uint8_t b = (long(effect->custom8[2]) - effect->custom8[5]) * (c + 1) / effect->frameCount +  effect->custom8[5];
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[ledNumsConverter(i)] = CRGB(r, g, b);
  }
  FastLED.show();
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
        if (m_nodeId == payload[0] || payload[0] == 255)
        {
          for (int i = 0; i < NUM_LEDS; i++) {
            leds[ledNumsConverter(i)] = CHSV(payload[1], payload[2], payload[3]);
          }
          FastLED.show();
        }
      }
      break;
    case 102:
      {
        if (m_nodeId == payload[0] || payload[0] == 255)
        {
          uint16_t t = uint16_t(payload[1] << 8) + uint16_t(payload[2]);
          STEffectCallbackDef *effect = effectCreate(2, t / 15, 15 , effect102Callback);
          effect->custom8[0] = payload[3];
          effect->custom8[1] = payload[4];
          effect->custom8[2] = payload[5];
          effect->custom8[3] = leds[0].r;
          effect->custom8[4] = leds[1].g;
          effect->custom8[5] = leds[2].b;
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
  Serial.begin(9600);
  Serial.println("STARTING.......");
  
  LEDS.addLeds<WS2813, LED_PIN, RGB>(leds, NUM_LEDS);
  stEffectCallbackVec.setStorage(stEffectCallbackDef_array);
  hdSerial.begin(9600);
  hdSerial.setMode(SMT_RECEIVE);
  pinMode(11, INPUT_PULLUP);
  TPT.callbackRegister(tpCallback, transmitCallback);
  TPT.tpBeginReceive();
  topologyInit();

  preCheckTime = millis();
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
  int effectSize = stEffectCallbackVec.size();
  int diff = millis() - preCheckTime ;

  for (int i = effectSize - 1; i >= 0; i--)//必须逆向遍历
  {
    STEffectCallbackDef *effect = stEffectCallbackVec[i];
    if (diff > 0)
    {
      if (effect->currentRefreshTime > diff)
      {
        effect->currentRefreshTime -= diff;
      } else
      {
        effect->callback(effect->callbackTimes, effect->currentFrameCount, effect);
        effect->currentFrameCount++;
        effect->currentRefreshTime = effect->frameRefreshTime;
        if (effect->currentFrameCount >= effect->frameCount)
        {
          effect->currentFrameCount = 0;
          if (effect->callbackTimes == 1)
          {
            effectDelete(effect);//必须逆向遍历
          } else
          {
            if (effect->callbackTimes > 1)
            {
              effect->callbackTimes--;
            }
          }
        }
      }
    }
  }
  preCheckTime = millis();
}

#include "HalfDuplexSerial.h"
#include "TriangleProtocol.h"
#include "SmartTopology.h"
#include <Vector.h>

enum STLightType
{
  STLT_WAITING_CHECK = 0,
  STLT_CHECKING = 1,
  STLT_SHOW_EFFECT
};

STNodeDef *seekNodeQueue_storage[64];
Vector<STNodeDef *> seekNodeQueue;
STNodeDef *currentNode;
STLightType stLightType;
#define HDSERIAL_PIN D6
#define SLECTED_PIN D5

HalfDuplexSerial hdSerial(HDSERIAL_PIN);

typedef void (*EffectCallback)(unsigned int, unsigned int, void *effect);

struct STEffectCallbackDef
{
  uint32_t autoChangeTime;
  uint32_t currentChangeTime;
  uint16_t currentCallbackTimes;
  uint16_t frameCount;
  uint16_t currentFrameCount;
  uint32_t frameRefreshTime;
  uint32_t currentRefreshTime;
  uint8_t custom8[6];
  uint16_t custom16[3];
  uint32_t custom32[1];
  EffectCallback callback;
};
uint8_t effectPointer;
uint32_t preCheckTime = 0;
STEffectCallbackDef *stEffectCallbackDef_array[32];
Vector<STEffectCallbackDef *> stEffectCallbackVec;

STEffectCallbackDef *effectCreate(uint32_t n, uint16_t c, uint32_t t, EffectCallback callback)
{
  STEffectCallbackDef *effect = new STEffectCallbackDef();
  effect->autoChangeTime = n;
  effect->currentChangeTime = n;
  effect->currentCallbackTimes = 0;
  effect->frameCount = c;
  effect->currentFrameCount = 0;
  effect->frameRefreshTime = t;
  effect->currentRefreshTime = 0;
  effect->callback = callback;
  stEffectCallbackVec.push_back(effect);
  return effect;
}

void effectReset(uint32_t n)
{
  STEffectCallbackDef *effect = stEffectCallbackVec[n];
  if(effect)
  {
    effect->currentChangeTime = effect->autoChangeTime;
    effect->currentCallbackTimes = 0;
    effect->currentFrameCount = 0;
    effect->currentRefreshTime = 0;
  }
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

void waitingReceive()
{
  hdSerial.setMode(SMT_RECEIVE);
  TPT.tpBeginReceive();
}

void waitingTransmit()
{
  hdSerial.setMode(SMT_TRANSMIT);
}

void startSelect()
{
  pinMode(SLECTED_PIN, OUTPUT);
  digitalWrite(SLECTED_PIN, HIGH);
}

void stopSelect()
{
  pinMode(SLECTED_PIN, INPUT);
}

void seekRootNode()
{
  //寻找根节点
  stLightType = STLT_CHECKING;
  startSelect();
  TPT.tpBegin(51).tpTransmit(true);
  waitingReceive();
}


void randomEffect(unsigned int n, unsigned int c, void *effect)
{
  Serial.println("randomEffect n:" + String(n) + " c:" + String(c));
  uint16_t t = 1000;
  TPT.tpBegin(102).tpByte(c).tpByte((t >> 8) & 0xFF).tpByte(t & 0xFF).tpColor(random(50, 150), random(50, 150), random(50, 150)).tpTransmit();
}

void uniformColorEffect(unsigned int n, unsigned int c, void *effect)
{
  Serial.println("uniformColorEffect n:" + String(n) + " c:" + String(c));
  TPT.tpBegin(101).tpByte(255).tpColor(random(50, 150), random(50, 150), random(50, 150)).tpTransmit();
}

void effectSetup()
{
  int count = ST.nodeCount();
  effectCreate(2000*count,count,2000,randomEffect);
  effectCreate(10000,1,10000,uniformColorEffect);
}

void effectClear()
{
  stEffectCallbackVec.clear();
}

void effectLoop()
{
  int effectSize = stEffectCallbackVec.size();
  int diff = millis() - preCheckTime;
  STEffectCallbackDef *effect = stEffectCallbackVec[effectPointer];
  if (diff > 0)
  {
    if (effect->currentRefreshTime > (uint32_t)diff)
    {
      effect->currentRefreshTime -= diff;
    }
    else
    {
      effect->callback(effect->currentCallbackTimes, effect->currentFrameCount, effect);
      effect->currentRefreshTime = effect->frameRefreshTime;
      effect->currentFrameCount++;
      if (effect->currentFrameCount >= effect->frameCount)
      {
        effect->currentFrameCount = 0;
        effect->currentCallbackTimes++;
      }
    }

    if(effect->currentChangeTime > (uint32_t)diff)
    {
      effect->currentChangeTime -= diff;
    }else
    {
      effectPointer++;
      if(effectPointer >= effectSize)
      {
        effectPointer = 0;
      }
      effectReset(effectPointer);
    }
  }
  preCheckTime = millis();
}

void seekNodeByQueue()
{
  if (seekNodeQueue.size() <= 0)
  {
    stLightType = STLT_SHOW_EFFECT;
    effectClear();
    effectSetup();
    preCheckTime = millis();
    return;
  }
  STNodeDef *node = seekNodeQueue[0];
  if (node->leftChildType == STNT_WAITING_CHECK)
  {
    node->leftChildType = STNT_CHECKING;
    TPT.tpBegin(21).tpByte(node->nodeId).tpTransmit();
    delay(100);
    TPT.tpBegin(52).tpTransmit(true);
    waitingReceive();
    currentNode = node;
  }
  else if (node->rightChildType == STNT_WAITING_CHECK)
  {
    node->rightChildType = STNT_CHECKING;
    TPT.tpBegin(23).tpByte(node->nodeId).tpTransmit();
    delay(100);
    TPT.tpBegin(52).tpTransmit(true);
    waitingReceive();
    currentNode = node;
  }
}

void seekLeafNode()
{
  seekNodeQueue.push_back(ST.rootNode());
  seekNodeByQueue();
}




void tpCallback(byte pId, byte *payload, unsigned int length, bool isTimeout)
{
  waitingTransmit();
  Serial.println("tpCallback pId:" + String(pId) + " Timeout:" + String(isTimeout ? "True" : "False"));
  switch (pId)
  {
  case 1:
  {
  }
  break;
  case 2:
  {
  }
  break;
  case 51:
  {
    stopSelect();
    if (isTimeout)
    {
      //无根节点，拓扑结束
    }
    else
    {
      //有节点
      uint8_t nodeId = ST.creatRootNode();
      Serial.println("ROOT NODE ID:" + String(nodeId));
      TPT.tpBegin(2).tpByte(nodeId).tpTransmit();
      seekLeafNode();
    }
  }
  break;
  case 52:
  {
    if (isTimeout)
    {
      //无子节点
      if (currentNode->leftChildType == STNT_CHECKING)
      {
        TPT.tpBegin(22).tpByte(currentNode->nodeId).tpTransmit();
        currentNode->leftChildType = STNT_HAS_NO_CHILD;
      }
      else if (currentNode->rightChildType == STNT_CHECKING)
      {
        TPT.tpBegin(24).tpByte(currentNode->nodeId).tpTransmit();
        currentNode->rightChildType = STNT_HAS_NO_CHILD;
        seekNodeQueue.remove(0);
      }
      seekNodeByQueue();
    }
    else
    {
      //有子节点
      STNodeDef *node = ST.creatNode();
      seekNodeQueue.push_back(node);
      Serial.println("CHILD NODE ID:" + String(node->nodeId));
      if (currentNode->leftChildType == STNT_CHECKING)
      {
        currentNode->leftChildType = STNT_HAS_CHILD;
        currentNode->leftChild = node;
        TPT.tpBegin(22).tpByte(currentNode->nodeId).tpTransmit();
        TPT.tpBegin(2).tpByte(node->nodeId).tpTransmit();
      }
      else if (currentNode->rightChildType == STNT_CHECKING)
      {
        currentNode->rightChildType = STNT_HAS_CHILD;
        currentNode->rightChild = node;
        TPT.tpBegin(24).tpByte(currentNode->nodeId).tpTransmit();
        TPT.tpBegin(2).tpByte(node->nodeId).tpTransmit();
        seekNodeQueue.remove(0);
      }
      else
      {
        seekNodeQueue.pop_back();
        delete node;
      }
      seekNodeByQueue();
    }
  }
  break;
  }
}

void transmitCallback(byte *ptBuffer, unsigned int ptLength)
{
  for (unsigned int i = 0; i < ptLength; i++)
  {
    int c = ptBuffer[i];
    hdSerial.write(c);
  }
}

void setup()
{
  seekNodeQueue.setStorage(seekNodeQueue_storage);
  stLightType = STLT_WAITING_CHECK;
  Serial.begin(115200);
  pinMode(D2, INPUT);

  stEffectCallbackVec.setStorage(stEffectCallbackDef_array);

  hdSerial.begin(9600);
  hdSerial.setMode(SMT_TRANSMIT);
  pinMode(D7, INPUT_PULLUP);
  TPT.callbackRegister(tpCallback, transmitCallback);

  delay(200);

  TPT.tpBegin(1).tpByte(5).tpTransmit(); //所有节点初始化
  seekRootNode();
}

void loop()
{
  TPT.protocolLoop();
  if (hdSerial.serialModeType() == SMT_RECEIVE)
  {
    while (hdSerial.available())
    {
      byte c = hdSerial.read();
      TPT.tpPushData(c).tpParse();
    }
  }
  if (stLightType == STLT_SHOW_EFFECT)
  {
    effectLoop();
  }



  // if(digitalRead(D2)==LOW)
  // {
  //   if(hdSerial.serialModeType() == SMT_RECEIVE)
  //   {
  //     hdSerial.setMode(SMT_TRANSMIT);
  //     Serial.println("TX");
  //   }else
  //   {
  //     hdSerial.setMode(SMT_RECEIVE);
  //     Serial.println("RX");
  //   }
  //   delay(200);
  // }
  // if (hdSerial.serialModeType() == SMT_TRANSMIT)
  // {

  //   while (Serial.available())
  //   {
  //     Serial.println("test");
  //     //TPT.tpPushData(hdSerial.read()).tpParse();
  //     hdSerial.write(Serial.read());
  //   }
  // }
}
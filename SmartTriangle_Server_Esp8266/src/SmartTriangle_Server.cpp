#include "HalfDuplexSerial.h"
#include "TriangleProtocol.h"
#include "SmartTopology.h"
#include <Vector.h>
#include "ArduiDispatch/ArduiDispatch.h"
#include "ArduiDispatch/ADHueAction.h"

#define PROTOCOL_VER  "0.1.0"

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


void uniformColorEffect(unsigned int c, void *effect)
{
  ADLOG_SV("uniformColorEffect",c);
  Serial.printf("%s: S:%d,H:%d,M:%d \n", "uniformColorEffect", ESP.getFreeContStack(), ESP.getFreeHeap(), ESP.getMaxFreeBlockSize());
  TPT.tpBegin(102).tpByte(255).tpUint16(1950).tpByte(random(0, 256)).tpTransmit();
}

void singleRandomEffect(unsigned int n, unsigned int c, void *effect)
{
  TPT.tpBegin(102).tpByte(c).tpUint16(1000).tpByte(random(0, 256)).tpTransmit();
}

void singleLedEffect(unsigned int c, void *effect)
{
  ADLOG_SV("singleLedEffect",c);
  TPT.tpBegin(103).tpByte(255).tpUint16(1950).tpByte(0).tpByte(255).tpTransmit();
}

void randomEffect(unsigned int c, void *action)
{
  // TPT.tpBegin(104).tpByte(255).tpUint16(3000).tpUint16(5000).tpTransmit();
}

void effectSetup()
{
  // uint32_t count = ST.nodeCount();

  ADAction *randomAction = ADAction::create(uniformColorEffect,2000,3,0,false);
  ADActor *randomActor = ADActor::create(6000,randomAction);
  Director.addActor(randomActor);

  ADAction *singleLeAction = ADAction::create(singleLedEffect,2000,3,0,false);
  ADActor *singleLeActor = ADActor::create(6000,singleLeAction);
  Director.addActor(singleLeActor);

  // effectCreate(5000*count,count,5000,randomEffect);
  // effectCreate(10000,2,5000,uniformColorEffect);
  // effectCreate(10000,2,5000,singleLedEffect);
}

void seekNodeByQueue()
{
  if (seekNodeQueue.size() <= 0)
  {
    stLightType = STLT_SHOW_EFFECT;
    effectSetup();
    Director.startAction(millis());
    ADLOG_SV("START",millis());
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
  ADLOG_V(ptLength);
  for (unsigned int i = 0; i < ptLength; i++)
  {
    uint8_t c = ptBuffer[i];
    Serial.println(c,HEX);
    hdSerial.write(c);
  }
}

void setup()
{
  seekNodeQueue.setStorage(seekNodeQueue_storage);
  stLightType = STLT_WAITING_CHECK;
  Serial.begin(115200);
  pinMode(D2, INPUT);

  hdSerial.begin(9600);
  hdSerial.setMode(SMT_TRANSMIT);
  pinMode(D7, INPUT_PULLUP);
  TPT.callbackRegister(tpCallback, transmitCallback);

  Serial.println("STARTING...");
  delay(2000);

  TPT.tpBegin(1).tpByte(5).tpTransmit(); //所有节点初始化
  seekRootNode();

  Director.begin(true);
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
    Director.loop(millis());
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
#include "HalfDuplexSerial.h"
#include "TriangleProtocol.h"
#include "SmartTopology.h"
#include <Vector.h>
#include "ArduiDispatch/ArduiDispatch.h"
#include "ArduiDispatch/ADHueAction.h"

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

#define HDSERIAL_PIN D6

#define SLECTED_PIN D5

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

HalfDuplexSerial hdSerial(HDSERIAL_PIN);

uint8_t m_nodeId = 0;

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
  TPT.tpBegin(51,255).tpTransmit(true);
  waitingReceive();
}

void uniformColorEffect(unsigned int c, void *effect)
{
  TPT.tpBegin(102,255).tpUint16(1900).tpByte(random(0, 256)).tpTransmit();
}

void singleRandomEffect(unsigned int n, unsigned int c, void *effect)
{
  // TPT.tpBegin(102,c).tpUint16(1000).tpByte(random(0, 256)).tpTransmit();
}

void singleLedEffect(unsigned int c, void *effect)
{
  TPT.tpBegin(104,255).tpUint16(2800).tpByte(random(0, 256)).tpByte(random(0, 256)).tpByte(random(0, 256)).tpTransmit();
  //  TPT.tpBegin(100,1).tpTransmit(true);
  //   waitingReceive();
}

void randomEffect(unsigned int c, void *action)
{
  // TPT.tpBegin(104).tpByte(255).tpUint16(3000).tpUint16(5000).tpTransmit();
}

void effectSetup()
{
  // uint32_t count = ST.nodeCount();

  ADAction *randomAction = ADAction::create(uniformColorEffect, 2000, 3, 0, false);
  ADActor *randomActor = ADActor::create(6000, randomAction);
  Director.addActor(randomActor);

  ADAction *singleLeAction = ADAction::create(singleLedEffect, 3000, 2, 0, false);
  ADActor *singleLeActor = ADActor::create(6000, singleLeAction);
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
    ADLOG_SV("Topo SUCCESSED!!!", millis());
    return;
  }
  STNodeDef *node = seekNodeQueue[0];
  if (node->leftChildType == STNT_WAITING_CHECK)
  {
    node->leftChildType = STNT_CHECKING;
    TPT.tpBegin(21,node->nodeId).tpTransmit();
    delay(100);
    TPT.tpBegin(52,255).tpTransmit(true);
    waitingReceive();
    currentNode = node;
  }
  else if (node->rightChildType == STNT_WAITING_CHECK)
  {
    node->rightChildType = STNT_CHECKING;
    TPT.tpBegin(23,node->nodeId).tpTransmit();
    delay(100);
    TPT.tpBegin(52,255).tpTransmit(true);
    waitingReceive();
    currentNode = node;
  }
}

void seekLeafNode()
{
  seekNodeQueue.push_back(ST.rootNode());
  seekNodeByQueue();
}

void transmitCallback(byte *ptBuffer, unsigned int ptLength)
{
  Serial.print("Sent pID=" + String(ptBuffer[3]) +  " Length=" + String(ptLength) + " ");
  ADLOG_V(ESP.getMaxFreeBlockSize());
  for (unsigned int i = 0; i < ptLength; i++)
  {
    uint8_t c = ptBuffer[i];
    Serial.println(c);
    hdSerial.write(c);
  }
}

PROTOCOL_CALLBACK(51)
{
  stopSelect();
  if (isTimeout)
  {
    //无根节点，拓扑结束
    Serial.println(F("Topo Failed!!!"));
  }
  else
  {
    //有节点
    uint8_t nodeId = ST.creatRootNode();
    Serial.println("ROOT NODE ID:" + String(nodeId));
    TPT.tpBegin(2,255).tpByte(nodeId).tpTransmit();
    TPT.tpBegin(200,nodeId).tpTransmit();
    // delay(500);
    seekLeafNode();
  }
}

PROTOCOL_CALLBACK(52)
{
  if (isTimeout)
  {
    //无子节点
    if (currentNode->leftChildType == STNT_CHECKING)
    {
      TPT.tpBegin(22,currentNode->nodeId).tpTransmit();
      currentNode->leftChildType = STNT_HAS_NO_CHILD;
    }
    else if (currentNode->rightChildType == STNT_CHECKING)
    {
      TPT.tpBegin(24,currentNode->nodeId).tpTransmit();
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
      TPT.tpBegin(22,currentNode->nodeId).tpTransmit();
      TPT.tpBegin(2,255).tpByte(node->nodeId).tpTransmit();
      TPT.tpBegin(200,node->nodeId).tpTransmit();
      // delay(500);
    }
    else if (currentNode->rightChildType == STNT_CHECKING)
    {
      currentNode->rightChildType = STNT_HAS_CHILD;
      currentNode->rightChild = node;
      TPT.tpBegin(24,currentNode->nodeId).tpTransmit();
      TPT.tpBegin(2,255).tpByte(node->nodeId).tpTransmit();
      TPT.tpBegin(200,node->nodeId).tpTransmit();
      // delay(500);
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

void tpCallback(byte pId, byte *payload, unsigned int length, bool isTimeout)
{
  waitingTransmit();
  Serial.print("Received pID=" + String(pId) +  " Timeout=" + String(isTimeout ? "True " : "False "));
  ADLOG_V(ESP.getMaxFreeBlockSize());
  switch (pId)
  {
  case 51:
    PORTOCOL_REGISTER(51);
  case 52:
    PORTOCOL_REGISTER(52);
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

  delay(200);
  Serial.println("STARTING...");
  TPT.tpBegin(1,255).tpByte(200).tpTransmit(); //所有节点初始化
  delay(50);
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
}
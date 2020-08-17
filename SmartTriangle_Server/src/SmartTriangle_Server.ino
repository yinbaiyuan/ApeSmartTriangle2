#include "HalfDuplexSerial.h"
#include "TriangleProtocol.h"
#include "SmartTopology.h"
#include <Vector.h>
#include "ArduiDispatch.h"
#include "ADHueAction.h"

#if defined ARDUINO
#include "MemoryFree.h"
#endif

#define PROTOCOL_VER "0.1.0"

#define DEBUG         1

#define PROTOCOL_CALLBACK(x) void pId_callback_##x(byte *payload, unsigned int length, bool isTimeout)

#define PORTOCOL_REGISTER(x)                                      \
  {                                                               \
    if (isTimeout || m_nodeId == payload[0] || payload[0] == 255) \
    {                                                             \
      pId_callback_##x(payload + 1, length - 1, isTimeout);       \
    }                                                             \
  }                                                               \
  break

#if defined ESP8266
#define HDSERIAL_PIN D6
#define HDSERIAL_PIN_BAKE D7
#define SLECTED_PIN D5
#elif defined ARDUINO
#define HDSERIAL_PIN 6
#define HDSERIAL_PIN_BAKE 7
#define SLECTED_PIN 5
#endif

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
  TPT.tpBegin(51, 255).tpTransmit(true);
  waitingReceive();
}

void uniformColorEffect(uint32_t c, void *effect)
{
  TPT.tpBegin(102, 255).tpUint16(1900).tpByte(random(0, 256)).tpTransmit();
}

void singleRandomEffect(uint32_t n, uint32_t c, void *effect)
{
  // TPT.tpBegin(102,c).tpUint16(1000).tpByte(random(0, 256)).tpTransmit();
}

void singleLedEffect(uint32_t c, void *effect)
{
  TPT.tpBegin(102, 255).tpUint16(2800).tpColor(random(0, 256), 255, 255).tpTransmit();
}

void allShowEffect(uint32_t n, void *effect)
{
  TPT.tpBegin(102, 255).tpUint16(3000).tpColor(random(0, 256), 255, 255).tpTransmit();
}

void singleShowEffect(uint32_t n, void *effect)
{
  if(n>25)return;
  uint32_t count = ST.nodeCount();
  if(n%count == 1)
  {
      ST.fullRandomInit(count);
  }
  uint8_t c = ST.fullRandom();
  ADLOG_V(count);
  ADLOG_V(n);
  ADLOG_V(c);
  TPT.tpBegin(102, c).tpUint16(random(1500,2800)).tpColor(random(0, 256), 255, 255).tpTransmit();
}

void flowerShowEffect(unsigned int n, void *effect)
{
  uint32_t count = ST.nodeCount();
  TPT.tpBegin(104, 255).tpUint16(3000).tpColor(random(0, 256), 255, 255).tpColor(random(0, 256), 255, 255).tpByte(50).tpTransmit();
}

void effectSetup()
{
  uint32_t count = ST.nodeCount();

  ADAction *allShowAction = ADAction::create(allShowEffect, 3000, 3, 0, false);
  ADActor *allShowActor = ADActor::create(9000, allShowAction);
  Director.addActor(allShowActor);

  ADAction *singleShowAction = ADAction::create(singleShowEffect, 1000, 30, 0, false);
  ADActor *singleShowActor = ADActor::create(30000, singleShowAction);
  Director.addActor(singleShowActor);

  ADAction *flowerShowAction = ADAction::create(flowerShowEffect, 3000, 3, 0, false);
  ADActor *flowerShowActor = ADActor::create(9000, flowerShowAction);
  Director.addActor(flowerShowActor);

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
    ADLOG_SV("Topo SUCCESSED!!!", millis());
    TPT.tpBegin(200, 255).tpTransmit();
    delay(500);
    Director.startAction(millis());
    return;
  }
  STNodeDef *node = seekNodeQueue[0];
  if (node->leftChildType == STNT_WAITING_CHECK)
  {
    node->leftChildType = STNT_CHECKING;
    TPT.tpBegin(21, node->nodeId).tpTransmit();
    delay(100);
    TPT.tpBegin(52, 255).tpTransmit(true);
    waitingReceive();
    currentNode = node;
  }
  else if (node->rightChildType == STNT_WAITING_CHECK)
  {
    node->rightChildType = STNT_CHECKING;
    TPT.tpBegin(23, node->nodeId).tpTransmit();
    delay(100);
    TPT.tpBegin(52, 255).tpTransmit(true);
    waitingReceive();
    currentNode = node;
  }
}

void seekLeafNode()
{
  seekNodeQueue.push_back(ST.rootNode());
  seekNodeByQueue();
}

PROTOCOL_CALLBACK(51)
{
  stopSelect();
  Serial.println(TPT.parseString(payload));
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
    TPT.tpBegin(2, 255).tpByte(nodeId).tpTransmit();
    TPT.tpBegin(200, nodeId).tpTransmit();
#if DEBUG == 1
    delay(600);
#endif
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
      TPT.tpBegin(22, currentNode->nodeId).tpTransmit();
      currentNode->leftChildType = STNT_HAS_NO_CHILD;
    }
    else if (currentNode->rightChildType == STNT_CHECKING)
    {
      TPT.tpBegin(24, currentNode->nodeId).tpTransmit();
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
      TPT.tpBegin(22, currentNode->nodeId).tpTransmit();
      TPT.tpBegin(2, 255).tpByte(node->nodeId).tpTransmit();
      TPT.tpBegin(200, node->nodeId).tpTransmit();
#if DEBUG == 1
      delay(600);
#endif
    }
    else if (currentNode->rightChildType == STNT_CHECKING)
    {
      currentNode->rightChildType = STNT_HAS_CHILD;
      currentNode->rightChild = node;
      TPT.tpBegin(24, currentNode->nodeId).tpTransmit();
      TPT.tpBegin(2, 255).tpByte(node->nodeId).tpTransmit();
      TPT.tpBegin(200, node->nodeId).tpTransmit();
#if DEBUG == 1
      delay(600);
#endif
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
#if defined ESP8266
  Serial.println("<<==Rec. " + String(pId) + (isTimeout ? " T " : " F ") + String(ESP.getMaxFreeBlockSize()));
#elif defined ARDUINO
  Serial.println("<<==Rec. " + String(pId) + (isTimeout ? " T " : " F ") + String(freeMemory()));
#endif
  switch (pId)
  {
    case 51:
      PORTOCOL_REGISTER(51);
    case 52:
      PORTOCOL_REGISTER(52);
  }
  Serial.println("==>>");
}


void transmitCallback(byte *ptBuffer, unsigned int ptLength)
{

#if defined ESP8266
  Serial.println("<<==Sent " + String(ptBuffer[3]) + " Length=" + String(ptLength) + " " +  String(ESP.getMaxFreeBlockSize()));
#elif defined ARDUINO
  Serial.println("<<==Sent " + String(ptBuffer[3]) + " Length=" + String(ptLength) + " " +  String(freeMemory()));
#endif
  for (unsigned int i = 0; i < ptLength; i++)
  {
    uint8_t c = ptBuffer[i];
    hdSerial.write(c);
  }
  Serial.println("==>>");
}

void setup()
{

  seekNodeQueue.setStorage(seekNodeQueue_storage);
  stLightType = STLT_WAITING_CHECK;
  Serial.begin(115200);

  hdSerial.begin(57600);
  hdSerial.setMode(SMT_TRANSMIT);
  pinMode(HDSERIAL_PIN_BAKE, INPUT_PULLUP);
  TPT.callbackRegister(tpCallback, transmitCallback);

  pinMode(A0, INPUT);
  randomSeed(analogRead(A0));
  delay(2000);//等待2秒，让所有客户端启动
  Serial.println("STARTING...");
  /*
     0b00000001 配置信息
     右1位 客户端调试模式

  */
  TPT.tpBegin(1, 255).tpByte(200).tpByte(0b00000001).tpTransmit(); //所有节点初始化
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

#include "HalfDuplexSerial.h"
#include "TriangleProtocol.h"
#include "SmartTopology.h"
#include <Vector.h>

STNodeDef *seekNodeQueue_storage[64];
Vector<STNodeDef *> seekNodeQueue;
STNodeDef *currentNode;

#define HDSERIAL_PIN D6
#define SLECTED_PIN D5

HalfDuplexSerial hdSerial(HDSERIAL_PIN);

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
  startSelect();
  TPT.tpBegin(51).tpTransmit(true);
  waitingReceive();
}

void seekAction()
{
  if (seekNodeQueue.size() <= 0)
    return;
  STNodeDef *node = seekNodeQueue[0];
  // currentNode = node;

}

void seekLeafNode()
{
  seekNodeQueue.push_back(ST.rootNode());
  seekAction();

  // TPT.tpBegin(21).tpByte(node->nodeId).tpTransmit();
  // // TPT.tpBegin(51).tpTransmit(true);
  // waitingReceive();
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

  Serial.begin(115200);
  pinMode(D2, INPUT);

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
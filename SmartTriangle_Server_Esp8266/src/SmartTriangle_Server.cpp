#include "HalfDuplexSerial.h"
#include "TriangleProtocol.h"
#include "SmartTopology.h"

HalfDuplexSerial hdSerial(D6);

void receiveAction()
{
  TPT.tpTransmit(true);
  hdSerial.setMode(SMT_RECEIVE);
  TPT.tpBeginReceive();
}

void startSelect()
{
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);
}

void stopSelect()
{
  pinMode(5, INPUT);
}

void seekRootNode()
{
  //寻找根节点
  startSelect();
  TPT.tpBegin(51);
  receiveAction();
}

void seekLeafNode(STNodeDef *node)
{
  TPT.tpBegin(21).tpByte(node->nodeId).tpTransmit();
  TPT.tpBegin(51);
  receiveAction();
}

void tpCallback(byte pId, byte *payload, unsigned int length , bool isTimeout)
{
  hdSerial.setMode(SMT_TRANSMIT);
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
        } else
        {
          //有节点
          uint8_t nodeId = ST.creatRootNode();
          Serial.println("nodeId:"+String(nodeId));
          TPT.tpBegin(2).tpByte(nodeId).tpTransmit();
          seekLeafNode(ST.currentNode());
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
  Serial.begin(115200);
  pinMode(2, INPUT);
  
  hdSerial.begin(9600);
  hdSerial.setMode(SMT_TRANSMIT);
  pinMode(D7, INPUT_PULLUP);
  TPT.callbackRegister(tpCallback, transmitCallback);
  
  delay(200);
  TPT.tpBegin(1).tpTransmit();

  seekRootNode();
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

#include "HalfDuplexSerial.h"
#include "TriangleProtocol.h"

HalfDuplexSerial hdSerial(10);

int selectPin[3] = {5, 6, 7};
int ledPin[3] = {2, 3, 4};

enum TriangleStateType
{
  TST_NONE = 0,
  TST_WATING_LOCATE = 1,
  TST_LOCATED,
};

TriangleStateType m_triangleStateType;
int m_fatherNodePin = -1;
int m_leftLeafNodePin = -1;
int m_rightLeafNodePin = -1;
int m_ledPin = -1;
int m_nodeId = -1;


void startSelect(bool isLeft)
{
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
  m_fatherNodePin = -1;
  for (int i = 0; i < 3; i++)
  {
    if (digitalRead(selectPin[i]) == HIGH)
    {
      m_fatherNodePin = selectPin[i];
//      m_leftLeafNodePin = selectPin((i - 1) < 0 ? 2 : (i - 1));
//      m_rightLeafNodePin = selectPin((i + 1) > 2 ? 0 : (i + 1));
      break;
    }
  }
  if(m_fatherNodePin != -1) return true;
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
}

void transmitAction()
{
  hdSerial.setMode(SMT_TRANSMIT);
  TPT.tpTransmit();
  hdSerial.setMode(SMT_RECEIVE);
  TPT.tpBeginReceive();
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
    case 21:
      {
        if (m_nodeId == payload[0])
        {
          startSelect(true);
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
    case 2:
      {
        if (m_triangleStateType == TST_WATING_LOCATE)
        {
          m_nodeId = payload[0];
          Serial.println("nodeId:" + String(m_nodeId));
          m_triangleStateType = TST_LOCATED;
        }
      }
      break;
  }
}

void transmitCallback(byte *ptBuffer, unsigned int ptLength)
{
  for (int i = 0; i < ptLength; i++)
  {
    int c = ptBuffer[i];
    hdSerial.write(c);
  }
}

void setup()
{
  hdSerial.begin(9600);
  hdSerial.setMode(SMT_RECEIVE);
  pinMode(11, INPUT_PULLUP);
  TPT.callbackRegister(tpCallback, transmitCallback);
  TPT.tpBeginReceive();
  topologyInit();


  Serial.begin(9600);
  pinMode(2, INPUT);
}

void loop()
{
  TPT.protocolLoop();
  if (hdSerial.serialModeType() == SMT_RECEIVE)
  {
    while (hdSerial.available())
    {
//      Serial.write(hdSerial.read());
      TPT.tpPushData(hdSerial.read()).tpParse();
    }
  }
}

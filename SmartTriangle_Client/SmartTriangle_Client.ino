#include "HalfDuplexSerial.h"
#include "TriangleProtocol.h"

HalfDuplexSerial hdSerial(10);

int selectPin[3] = {5,6,7};
int ledPin[3] = {2,3,4};

enum TriangleStateType
{
    TST_NONE = 0,
    TST_POSITIONED = 1,
    SMT_PARENT,
};

TriangleStateType m_triangleStateType;
int m_fatherNodePin;
int m_leftLeafNodePin;
int m_rightLeafNodePin;
int m_ledPin;

void topologyCheck()
{
  for(int i = 0;i<3;i++)
  {
    pinMode(selectPin[i],INPUT);
  }
}

void tpCallback(byte pId, byte *payload, unsigned int length)
{
  Serial.println("callback" + String(pId));
  switch(pId)
  {
    case 1:
    {
      Serial.println("r:"+String(payload[0])+" g:"+String(payload[1])+" b:"+String(payload[2]));
      String myText = "";
      for (int i = 3; i < length; i++)
      {
        char c = payload[i];
        myText += c;
      }
      Serial.println(myText);
    }
  }
}

void setup()
{
  hdSerial.begin(9600);
  hdSerial.setMode(SMT_RECEIVE);
  pinMode(11,INPUT_PULLUP);
  TPT.callbackRegister(tpCallback);
  TPT.tpBeginReceive();
  for(int i = 0;i<3;i++)
  {
    pinMode(selectPin[i],INPUT);
  }
  m_triangleStateType = TST_NONE;
  
  Serial.begin(9600);
  pinMode(2, INPUT);
}

void loop()
{
  topologyCheck();
//  if (digitalRead(2) == LOW)
//  {
//    if (hdSerial.serialModeType() == SMT_TRANSMIT)
//    {
//      Serial.println("RX");
//      hdSerial.setMode(SMT_RECEIVE);
//    } else if (hdSerial.serialModeType() == SMT_RECEIVE)
//    {
//      Serial.println("TX");
//      hdSerial.setMode(SMT_TRANSMIT);
//    }
//    delay(500);
//  }
  if (hdSerial.serialModeType() == SMT_TRANSMIT)
  {
    while (Serial.available())
    {
      int c = Serial.read();
      hdSerial.write(c);
    }
  } else if (hdSerial.serialModeType() == SMT_RECEIVE)
  {
//    Serial.println("rec");
    while (hdSerial.available())
    {
      int c = hdSerial.read();
      TPT.tpPushData(c).tpParse();
//      Serial.println(c);
//      Serial.write(c);
    }
  }
}

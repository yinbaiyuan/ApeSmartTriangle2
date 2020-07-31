#include "TriangleProtocol.h"
#include <Arduino.h>

static uint8_t m_ptBuffer[256];
    static uint8_t m_ptLength;

TriangleProtocol::TriangleProtocol()
{
  this->callback = NULL;
}

TriangleProtocol::~TriangleProtocol()
{
  this->callback = NULL;
}

void TriangleProtocol::callbackRegister(TP_CALLBACK_SIGNATURE)
{
  this->callback = callback;
}

TriangleProtocol &TriangleProtocol::tpBegin(byte pid)
{
  m_ptBuffer[0] = 0;
  m_ptBuffer[1] = 0;
  m_ptLength = 2;
  m_ptBuffer[m_ptLength++] = pid;
  return TPT;
}

TriangleProtocol &TriangleProtocol::tpByte(byte b)
{
  m_ptBuffer[m_ptLength++] = b;
  return TPT;
}

TriangleProtocol &TriangleProtocol::tpColor(byte r,byte g,byte b)
{
  m_ptBuffer[m_ptLength++] = r;
  m_ptBuffer[m_ptLength++] = g;
  m_ptBuffer[m_ptLength++] = b;
  return TPT;
}

TriangleProtocol &TriangleProtocol::tpStr(const String &str,bool pushSize)
{
  int length = str.length();
  if(pushSize)
  {
    m_ptBuffer[m_ptLength++] = length;
  }
  for (int i = 0; i < length; i++)
  {
    m_ptBuffer[m_ptLength++] = str[i];
  }
  return TPT;
}

void TriangleProtocol::tpTansmit(uint8_t **ptBuffer, uint8_t *ptLength)
{
  m_ptBuffer[1] = m_ptLength;
  *ptBuffer = m_ptBuffer;
  *ptLength = m_ptLength;
}

TriangleProtocol &TriangleProtocol::tpBeginReceive()
{
  m_ptLength = 0;
  m_ptBuffer[0] = 0;
  m_ptBuffer[1] = 0;
}

TriangleProtocol &TriangleProtocol::tpPushData(uint8_t d)
{
//  Serial.println("d:"+String(d)+" m_ptLength:"+String(m_ptLength));
  if(m_ptLength == 0 && d != 0)
  {
    return;
  }
  m_ptBuffer[m_ptLength++]=d;
  return TPT;
}

void TriangleProtocol::tpParse()
{
  uint8_t pLength = m_ptBuffer[1];
//  Serial.println("pLength:"+String(pLength));
//  Serial.println("m_ptLength:"+String(m_ptLength));
  
  if(pLength<=2)return;
  if(pLength>m_ptLength)return;
  Serial.println("pLength:"+String(pLength));
  Serial.println("m_ptLength:"+String(m_ptLength));
  if(pLength==m_ptLength)
  {
    //符合解析条件
    uint8_t pId = m_ptBuffer[2];
    Serial.println("pId:"+String(pId));
    this->callback(pId,m_ptBuffer+3,pLength-3);
    //解析完成
    TPT.tpBeginReceive();
  }
}

TriangleProtocol TPT;

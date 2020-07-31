#include "TriangleProtocol.h"
#include <Arduino.h>
#include <Vector.h>

struct PIdTimeoutDef
{
  uint8_t pId;
  uint32_t recordTime;
  uint32_t timeout;
};

PIdTimeoutDef* storage_array[10];
Vector<PIdTimeoutDef *> pIdTimeoutVec;

static uint8_t m_ptBuffer[256];
static uint8_t m_ptLength;

TriangleProtocol::TriangleProtocol()
{
  this->callback = NULL;
  pIdTimeoutVec.setStorage(storage_array);
}

TriangleProtocol::~TriangleProtocol()
{
  this->callback = NULL;
}

void TriangleProtocol::callbackRegister(TP_PARSE_CALLBACK, TP_TRANSMIT_CALLBACK)
{
  this->callback = callback;
  this->trans_callback = trans_callback;
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

TriangleProtocol &TriangleProtocol::tpColor(byte r, byte g, byte b)
{
  m_ptBuffer[m_ptLength++] = r;
  m_ptBuffer[m_ptLength++] = g;
  m_ptBuffer[m_ptLength++] = b;
  return TPT;
}

TriangleProtocol &TriangleProtocol::tpStr(const String &str, bool pushSize)
{
  int length = str.length();
  if (pushSize)
  {
    m_ptBuffer[m_ptLength++] = length;
  }
  for (int i = 0; i < length; i++)
  {
    m_ptBuffer[m_ptLength++] = str[i];
  }
  return TPT;
}

void TriangleProtocol::tpTransmit(bool checkTimeout)
{
  m_ptBuffer[1] = m_ptLength;
  this->trans_callback(m_ptBuffer, m_ptLength);
  if(checkTimeout)
  {
    this->waitProtocolTimeout(m_ptBuffer[2]);
  }
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
  if (m_ptLength == 0 && d != 0)
  {
    return;
  }
  m_ptBuffer[m_ptLength++] = d;
  return TPT;
}

void TriangleProtocol::tpParse()
{
  uint8_t pLength = m_ptBuffer[1];
  if (pLength <= 2)return;
  if (pLength > m_ptLength)return;
  if (pLength == m_ptLength)
  {
    //符合解析条件
    uint8_t pId = m_ptBuffer[2];
    this->protocolTimeoutRemove(pId);
    this->callback(pId, m_ptBuffer + 3, pLength - 3, false);
    //解析完成
    TPT.tpBeginReceive();

  }
}

void TriangleProtocol::waitProtocolTimeout(uint8_t pId,uint32_t timeout)
{
  PIdTimeoutDef *pIdTimeoutDef = new PIdTimeoutDef();
  pIdTimeoutDef->pId = pId;
  pIdTimeoutDef->recordTime = millis();
  pIdTimeoutDef->timeout = timeout;
  pIdTimeoutVec.push_back(pIdTimeoutDef);
}

void TriangleProtocol::protocolTimeoutRemove(uint8_t pId)
{
  for (int i = pIdTimeoutVec.size() - 1; i >= 0; i--)
  {
    PIdTimeoutDef* pIdTimeoutDef = pIdTimeoutVec[i];
    if (pIdTimeoutDef->pId = pId)
    {
      pIdTimeoutVec.remove(i);
      delete pIdTimeoutDef;
      pIdTimeoutDef = NULL;
    }
  }
}
void TriangleProtocol::protocolLoop()
{

  for (int i = pIdTimeoutVec.size() - 1; i >= 0; i--)
  {
    PIdTimeoutDef* pIdTimeoutDef = pIdTimeoutVec[i];
    if (millis() - pIdTimeoutDef->recordTime > pIdTimeoutDef->timeout)
    {
      this->callback(pIdTimeoutDef->pId, NULL, 0, true);
      pIdTimeoutVec.remove(i);
      delete pIdTimeoutDef;
      pIdTimeoutDef = NULL;
    }
  }
}

TriangleProtocol TPT;

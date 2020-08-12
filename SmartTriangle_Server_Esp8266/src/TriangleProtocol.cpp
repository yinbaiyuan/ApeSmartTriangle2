#include "TriangleProtocol.h"
#include <Arduino.h>
#include "Vector.h"

struct PIdTimeoutDef
{
  uint8_t pId;
  uint32_t recordTime;
  uint32_t timeout;
};

PIdTimeoutDef *pIdTimeoutVec_array[10];
Vector<PIdTimeoutDef *> pIdTimeoutVec;

#define MAX_PROTOCOL_BUFFER 512

static uint8_t m_ptBuffer[MAX_PROTOCOL_BUFFER];
static uint8_t m_ptLength;

TriangleProtocol::TriangleProtocol()
{
  this->callback = NULL;
  this->trans_callback = NULL;
  pIdTimeoutVec.setStorage(pIdTimeoutVec_array);
}

TriangleProtocol::~TriangleProtocol()
{
  this->callback = NULL;
  this->trans_callback = NULL;
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

TriangleProtocol &TriangleProtocol::tpUint16(uint16_t i)
{
  m_ptBuffer[m_ptLength++] = (i >> 8) & 0xFF;
  m_ptBuffer[m_ptLength++] = (i >> 0) & 0xFF;
  return TPT;
}

TriangleProtocol &TriangleProtocol::tpUint32(uint32_t i)
{
  m_ptBuffer[m_ptLength++] = (i >> 24) & 0xFF;
  m_ptBuffer[m_ptLength++] = (i >> 16) & 0xFF;
  m_ptBuffer[m_ptLength++] = (i >> 8) & 0xFF;
  m_ptBuffer[m_ptLength++] = (i >> 0) & 0xFF;
  return TPT;
}

TriangleProtocol &TriangleProtocol::tpColor(byte r, byte g, byte b)
{
  m_ptBuffer[m_ptLength++] = r;
  m_ptBuffer[m_ptLength++] = g;
  m_ptBuffer[m_ptLength++] = b;
  return TPT;
}

TriangleProtocol &TriangleProtocol::tpStr(const String &str)
{
  int length = str.length();
  m_ptBuffer[m_ptLength++] = length;
  for (int i = 0; i < length; i++)
  {
    m_ptBuffer[m_ptLength++] = str[i];
  }
  return TPT;
}

void TriangleProtocol::tpTransmit(bool checkTimeout)
{
  m_ptBuffer[m_ptLength] = m_ptLength + 1;
  m_ptBuffer[1] = m_ptLength + 1;

  this->trans_callback(m_ptBuffer, m_ptLength + 1);
  if (checkTimeout)
  {
    this->waitProtocolTimeout(m_ptBuffer[2]);
  }
}

TriangleProtocol &TriangleProtocol::tpBeginReceive()
{
  m_ptLength = 0;
  memset(m_ptBuffer, 0, sizeof(uint8_t) * MAX_PROTOCOL_BUFFER);
  return TPT;
}

TriangleProtocol &TriangleProtocol::tpPushData(uint8_t d)
{
  //  Serial.println("d:"+String(d)+" m_ptLength:"+String(m_ptLength));
  if (m_ptLength == 0 && d != 0)
  {
    return TPT;
  }
  m_ptBuffer[m_ptLength++] = d;
  return TPT;
}

void TriangleProtocol::tpParse()
{
  uint8_t pLength = m_ptBuffer[1];
  if (pLength <= 2)
    return;
  if (pLength > m_ptLength)
    return;
  if (pLength <= m_ptLength)
  {
    if (pLength != m_ptBuffer[pLength - 1])
    {
      //数据校验失败
    }
    else
    {
      //符合解析条件
      uint8_t pId = m_ptBuffer[2];
      this->protocolTimeoutRemove(pId);
      this->callback(pId, m_ptBuffer + 3, pLength - 3, false);
    }
  }
  TPT.tpBeginReceive();
}

void TriangleProtocol::waitProtocolTimeout(uint8_t pId, uint32_t timeout)
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
    PIdTimeoutDef *pIdTimeoutDef = pIdTimeoutVec[i];
    if (pIdTimeoutDef->pId == pId)
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
    PIdTimeoutDef *pIdTimeoutDef = pIdTimeoutVec[i];
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

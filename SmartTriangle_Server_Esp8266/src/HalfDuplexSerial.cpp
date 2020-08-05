#include "HalfDuplexSerial.h"
#include <Arduino.h>

HalfDuplexSerial::HalfDuplexSerial(int8_t pin)
{
  m_transmitSeirial = new SoftwareSerial(-1, pin);
  m_receiveSerial = new SoftwareSerial(pin, -1);
  m_pin = pin;
  m_serialModeType = SMT_NONE;
}

HalfDuplexSerial::~HalfDuplexSerial()
{
  delete m_transmitSeirial;
  delete m_receiveSerial;
}

void HalfDuplexSerial::begin(long speed)
{
  m_transmitSeirial->begin(speed);
  m_receiveSerial->begin(speed);
}

void HalfDuplexSerial::end()
{
  m_receiveSerial->end();
  m_transmitSeirial->end();
  m_serialModeType = SMT_NONE;
}

size_t HalfDuplexSerial::write(uint8_t byte)
{
  if (m_serialModeType == SMT_TRANSMIT)
  {
    return m_transmitSeirial->write(byte);
  } else
  {
    return -1;
  }
}

int HalfDuplexSerial::read()
{
  if (m_serialModeType == SMT_RECEIVE)
  {
    return m_receiveSerial->read();
  } else
  {
    return -1;
  }
}

int HalfDuplexSerial::available()
{
  if (m_serialModeType == SMT_RECEIVE)
  {
    return m_receiveSerial->available();
  } else
  {
    return 0;
  }
}

void HalfDuplexSerial::setMode(SerialModeType smt)
{
  switch (smt)
  {
    case SMT_TRANSMIT:
      {
        pinMode(m_pin, OUTPUT);
        m_transmitSeirial->listen();
        m_serialModeType = SMT_TRANSMIT;
      }
      break;
    case SMT_RECEIVE:
      {
        pinMode(m_pin, INPUT_PULLUP);
        m_receiveSerial->listen();
        m_serialModeType = SMT_RECEIVE;
      }
      break;
    default:
      break;
  }
}

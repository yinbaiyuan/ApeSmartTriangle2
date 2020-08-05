#ifndef HalfDuplexSerial_h
#define HalfDuplexSerial_h

#include <inttypes.h>
#include <SoftwareSerial.h>

enum SerialModeType
{
    SMT_NONE = 0,
    SMT_TRANSMIT = 1,
    SMT_RECEIVE,
};

class HalfDuplexSerial
{
private:
    SoftwareSerial *m_transmitSeirial;
    SoftwareSerial *m_receiveSerial;
    SerialModeType m_serialModeType;
    int m_pin;
public:

    HalfDuplexSerial(int8_t pin);
    ~HalfDuplexSerial();

    void begin(long speed);
    void end();
    size_t write(uint8_t byte);
    int read();
    int available();

    void setMode(SerialModeType smt);
    SerialModeType serialModeType(){return m_serialModeType;}
    
};

#endif

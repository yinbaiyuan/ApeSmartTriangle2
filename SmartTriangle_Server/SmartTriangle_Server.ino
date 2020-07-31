#include "HalfDuplexSerial.h"
#include "TriangleProtocol.h"

HalfDuplexSerial hdSerial(10);

void tpCallback(byte pId, byte *payload, unsigned int length)
{
  Serial.println("callback" + String(pId));
}

void setup()
{
  hdSerial.begin(9600);
  hdSerial.setMode(SMT_TRANSMIT);
  pinMode(11, INPUT_PULLUP);
  TPT.callbackRegister(tpCallback);
  TPT.tpBeginReceive();
  Serial.begin(9600);
  pinMode(2, INPUT);
}



void loop()
{
  if (digitalRead(2) == LOW)
  {
    if (hdSerial.serialModeType() == SMT_TRANSMIT)
    {
      Serial.println("RX");
      hdSerial.setMode(SMT_RECEIVE);
    } else if (hdSerial.serialModeType() == SMT_RECEIVE)
    {
      Serial.println("TX");
      hdSerial.setMode(SMT_TRANSMIT);
    }

    delay(500);
  }

  delay(1000);

  uint8_t *ptBuffer = NULL;
  uint8_t ptLength = 0;
  TPT.tpBegin(1).tpColor(15, 15, 15).tpStr("TEST").tpTansmit(&ptBuffer, &ptLength);
  for (int i = 0; i < ptLength; i++)
  {
    int c = ptBuffer[i];
    Serial.println(c);
    hdSerial.write(c);
  }


  //  if (hdSerial.serialModeType() == SMT_TRANSMIT)
  //  {
  //    while (Serial.available())
  //    {
  //      int c = Serial.read();
  //      hdSerial.write(c);
  //    }
  //  } else if (hdSerial.serialModeType() == SMT_RECEIVE)
  //  {
  //    while (hdSerial.available())
  //    {
  //      int c = hdSerial.read();
  ////      Serial.println(c);
  //      Serial.write(c);
  //    }
  //  }
}

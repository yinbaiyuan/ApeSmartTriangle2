#ifndef __ADCONFIG_H__
#define __ADCONFIG_H__

#define DEBUG   1

#define ADLOG_V(n)      Serial.println(#n + String(":") + String(n))
#define ADLOG_SV(s,n)   Serial.println(s + String(" ") + #n + String(":") + String(n))
#define ADLOG_S(s)      Serial.println(s)

#include <stdint.h>
#include <Arduino.h>


#endif // __ADCONFIG_H__
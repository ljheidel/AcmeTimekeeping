#ifndef AcmeTimekeeping_h
#define AcmeTimekeeping_h

#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#error Platform not supported
#endif

#define NTP_RETRIES 30
#define TIME_ZONE 0
#define DST 0
#define USE_NTP true

#include <Wire.h>
#include <WiFiUdp.h>
#include <time.h>
#include <RtcDS3231.h>
#include <AcmeSyslog.h>

class AcmeTimekeeping {
  private:
    void setNTPValid(bool n);
    void setRTCValid(bool r);
    void setRTCPresent(bool r);
  public:
    AcmeTimekeeping();
    void init();
    bool getRTCValid();
    bool getNTPValid();
    bool getRTCPresent();
    void setRTCFromSystemTime();
    void setSystemTimeFromRTC();
    uint32_t getSystemTime();
    uint32_t getRTCTime();
};

#endif

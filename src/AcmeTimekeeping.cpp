#include "AcmeTimekeeping.h"

unsigned long lastMillis = 0;
time_t now;
time_t nowish = 1510592825;
bool ntpValid = false;
bool rtcValid = false;
bool rtcPresent = false;
bool useLocalTime = true;

RtcDS3231<TwoWire> Rtc(Wire);
AcmeSyslog acme_syslog(USE_SERIAL);

AcmeTimekeeping::AcmeTimekeeping(){
  
}

void AcmeTimekeeping::init() {

  acme_syslog.init();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  Rtc.Begin();
  
  if (!Rtc.IsDateTimeValid()) {
    if(Rtc.LastError() != 0) {
      rtcPresent = false;
      rtcValid = false;
      acme_syslog.logf(LOG_CRIT, "RTC communications error: %i", Rtc.LastError());
    } else {
      rtcPresent = true;
      rtcValid = false;
      acme_syslog.logf(LOG_ERR, "RTC lost confidence in DateTime. RTC needs set.");
    }
  } else {
    rtcPresent = true;
    rtcValid = true;
    acme_syslog.logf(LOG_INFO, "RTC present and time is valid.");
  }

  if (rtcPresent) {
    if (!Rtc.GetIsRunning()) {
      acme_syslog.logf(LOG_ERR, "RTC not running.  Starting now");
      Rtc.SetIsRunning(true);
    }
  
    RtcDateTime nowRTC = Rtc.GetDateTime();
    acme_syslog.logf(LOG_INFO, "Current RTC Time is %02u/%02u/%04u %02u:%02u:%02u", nowRTC.Month(), nowRTC.Day(), nowRTC.Year(), nowRTC.Hour(), nowRTC.Minute(), nowRTC.Second());
    
    if (nowRTC < compiled) {
      acme_syslog.logf(LOG_ERR,"RTC is older than compile time!");
      rtcValid = false;
    } else if (nowRTC > compiled) {
      acme_syslog.logf(LOG_INFO,"RTC is newer than compile time. (this is expected)");
    } else if (nowRTC == compiled) {
      acme_syslog.logf(LOG_NOTICE,"RTC is the same as compile time! (not expected but all is fine)");
    }
    
    Rtc.Enable32kHzPin(false);
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
  }

  if (WiFi.status() == WL_CONNECTED  && USE_NTP) {
    int i = 0;
    acme_syslog.logf(LOG_INFO, "Trying to set system time using SNTP");
    configTime(TIME_ZONE * 3600, DST * 3600, "pool.ntp.org", "time.nist.gov");
    now = time(nullptr);
    while (i++ < NTP_RETRIES) {
      acme_syslog.logf(LOG_DEBUG, "Attempting to sync time %i...", i);
      now = time(nullptr);
      delay(500);
      if (now > nowish) {
        ntpValid = true;
        acme_syslog.logf(LOG_INFO, "SNTP sync achieved.  System time set to NTP.");
        break;
      }
    }
    if (!ntpValid) acme_syslog.logf(LOG_ERR, "Could not sync to NTP.");
  }
  
  if (ntpValid) {
    setRTCFromSystemTime();
  } else if (rtcValid) {
    setSystemTimeFromRTC();      
  } else {
    acme_syslog.logf(LOG_CRIT, "No valid time source.");
    struct timeval tv;
    tv.tv_sec = 0;
    settimeofday(&tv, NULL);
  }
 
  acme_syslog.logf(LOG_INFO, "done!");
  struct tm timeinfo;
  now = time(nullptr);
  gmtime_r(&now, &timeinfo);
  acme_syslog.logf(LOG_INFO, "Current system time: %s", asctime(&timeinfo));
  
}

uint32_t getRTCTime(){
  RtcDateTime nowRTC = Rtc.GetDateTime();
  return nowRTC.Epoch32Time();
}

uint32_t getSystemTime(){
  return (uint32_t)time(nullptr);
}

void AcmeTimekeeping::setRTCFromSystemTime() {
  acme_syslog.logf(LOG_INFO, "Setting RTC to system time.");
  RtcDateTime timeToSet;
  timeToSet.InitWithEpoch32Time(time(nullptr));
  Rtc.SetDateTime(timeToSet); 
}

void AcmeTimekeeping::setSystemTimeFromRTC() {
  acme_syslog.logf(LOG_INFO, "Setting system time to RTC time."); 
  RtcDateTime nowRTC = Rtc.GetDateTime();
  struct timeval tv;
  tv.tv_sec = nowRTC.Epoch32Time();
  settimeofday(&tv, NULL);
}

void AcmeTimekeeping::setRTCValid(bool r){
  rtcValid = r;
  return;
}

bool AcmeTimekeeping::getRTCValid(){
  return rtcValid;
}

void AcmeTimekeeping::setRTCPresent(bool r){
  rtcPresent = r;
  return;
}

bool AcmeTimekeeping::getRTCPresent(){
  return rtcPresent;
}

void AcmeTimekeeping::setNTPValid(bool n) {
  ntpValid = n;
  return;
}

bool AcmeTimekeeping::getNTPValid(){
  return ntpValid;
}


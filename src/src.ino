#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#error Platform not supported
#endif

#include "AcmeSyslog.h"
#include "AcmeTimekeeping.h"

AcmeSyslog acme_syslog2(USE_SERIAL);

#ifndef INTERNET_SSID
#define INTERNET_SSID "idIoTnet"
#define INTERNET_PSK  "BookCoverGirlBoy430"
#endif
#define WIFI_RETRIES 15

const char* internet_ssid     = INTERNET_SSID;
const char* internet_psk      = INTERNET_PSK;

AcmeTimekeeping acme_timekeeping;

void setup(){
  acme_syslog2.init();
  connectToStaticInternetAP();
  connectToWiFi();
  acme_timekeeping.init();
}

void loop(){
//  time_t now = time(nullptr);
//  struct tm timeinfo;
//  gmtime_r(&now, &timeinfo);
//  acme_syslog2.logf(LOG_INFO, "Current system time: %s", asctime(&timeinfo));
//  delay(1000);
}

bool connectToStaticInternetAP(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(internet_ssid, internet_psk);

  return connectToWiFi();
}

bool connectToWiFi(){
  int i = 1;
  long d;
  
  while (WiFi.status() != WL_CONNECTED && (i++ < WIFI_RETRIES)) {
    d = 1000;
    acme_syslog2.logf(LOG_DEBUG, "waiting %i millis...", d);
    delay(d);
    acme_syslog2.logf(LOG_INFO, "Connecting to ...%s", internet_ssid);
  }

  if (WiFi.status() != WL_CONNECTED) {
    return false;
  } else {
    return true;
  }
}

#ifndef _MAIN_H
#define _MAIN_H
#include <Arduino.h>
#define FILESYSTEM LittleFS
#define FLashFS LitteFS
#include <time.h>                       // time() ctime()
#ifdef ESP8266
#include <sys/time.h>                   // struct timeval
#endif
 #include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
  //needed for library
  #include <ESPAsyncDNSServer.h>
#include <ESPAsync_WiFiManager.h>
#include "TickTwo.h"
#include <FS.h>
#include <LittleFS.h>
#include "wifi.h"
#include "tools.h"
#include "web.h"

//#include <DoubleResetDetect.h>
#include <ESP_DoubleResetDetector.h>
#include "ntp.h"
#include "ispindel.h"
#include "cronpush.h"

#endif // _MAIN_H

# Simple Arduino MKR1000 webserver & MQTT Client

Created With PlatformIO

After Arduino has connected to WiFi, it creates simple webserver that you can use to change Led ON/OFF and send a MQTT message to "arduino/test". 

MQTT message interval is 10 seconds and custom message from the website has 5s delay.

## wifiCreds.h needed for your WiFi and MQTT Server configuration

MQTT Server port is 1883.

```cpp
#ifndef WIFICREDS_H
#define WIFICREDS_H

#define MY_SSID "yourSSID"
#define MY_PASS "yourPASS"
#define MY_MQTT_SERVER "yourMqttServer"

#endif
```

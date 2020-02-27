#include <Arduino.h>
#include "wifiCreds.h"

#include <WiFi101.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiSSLClient.h>
#include <WiFiUdp.h>

#include <SPI.h>

#include "main.h"

#include <MQTT.h>

// Used for MQTT
WiFiClient wnet;
MQTTClient mqtt;
uint32_t lastMillis = 0;

char mqttServer[] = MY_MQTT_SERVER;
const int mqttPort = 1883;

int pingResult;
bool tryPing = false;

char ssid[] = MY_SSID;
char pass[] = MY_PASS;

uint8_t ledpin = 6;
bool val = true;

int status = WL_IDLE_STATUS;
WiFiServer server(8099);

//MQTT Connect
void connect() {
  Serial.print("MQTT Checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("..MQTT no Wifi..");
    delay(1000);
  }
  Serial.print("\nWiFi is CONNECTED\n");
  Serial.print("\nMQTT connection loop...\n");

  uint32_t connCount = 0;
  while(!mqtt.connect("arduino", "try", "try")) {
    ++connCount;

    // PING the mqttServer
    if(tryPing) {
      Serial.print("Pinging ");
      Serial.print(mqttServer);
      Serial.print(": ");

      pingResult = WiFi.ping(mqttServer);

      if(pingResult >= 0) {
        Serial.print("Ping SUCCESS RTT = ");
        Serial.print(pingResult);
        Serial.println(" ms");
      } else {
        Serial.print("Ping FAILED Error code: ");
        Serial.println(pingResult);
      }
    }

    Serial.print("MQTT Trying to connect ");
    Serial.print(connCount);
    Serial.println();
    delay(1000);
  }

  Serial.println("\nMQTT Connection established!");

  mqtt.subscribe("arduino/in");
}

//MQTT Message
void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

  if(payload == "Hello") {
    Serial.println("Hello received from arduino/in, responding back...");
    mqtt.publish("arduino/test", "Hello back from Arduino!");
  }

}
//Custom MQTT Message that is parsed from arduinos "websites" input
void customPublish(char *SendMsg) {
  mqtt.publish("arduino/test", SendMsg);
  Serial.println("Custom message PUBLISHED");
}


void setup() {
  delay(10000);
  Serial.begin(9600);
  // Serial.begin(115200);
  Serial.print("Start Serial ");
  pinMode(ledpin, OUTPUT);

  Serial.print("WiFi101 shield: ");
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("NOT PRESENT");
    return;
  }
  Serial.println("DETECTED");

  while ( status != WL_CONNECTED) {
    digitalWrite(ledpin, LOW);
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);
    digitalWrite(ledpin, HIGH);

    status = WiFi.begin(ssid, pass);

    delay(10000);
  }

  server.begin();
  printWiFiStatus();
  mqtt.begin(mqttServer, mqttPort, wnet);
  mqtt.onMessage(messageReceived);

  connect();
  digitalWrite(ledpin, HIGH);
}

void loop() {

  // HTTP Server
  WiFiClient client = server.available();

  if (client) {
    Serial.println("new client");
    String currentLine = "";

    while ( client.connected() ) {
      if ( client.available() ) {
        char c = client.read();
        // Serial.write(c);

        if(currentLine.length() < 100) {
          currentLine += c;
        }

        if(c == '\n') {

          Serial.print("HTTPServer captured: ");
          Serial.println(currentLine);
          
          // if(currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html;charset=UTF-8");
            client.println();

            client.println(F("<meta charset=\"UTF-8\"></meta>"));
            client.print("<a href=\"/?led=on&\">Turn LED ON</a><br/>");
            client.print("<a href=\"/?led=off&\">Turn LED OFF</a><br/>");

            client.println("<FORM ACTION='/' method=get accept-charset=\"UTF-8\">");

            client.println("MQTT Message: <INPUT TYPE=TEXT NAME='MQTT' VALUE='' SIZE='25' MAXLENGTH='50'><br/>");

            client.println("<INPUT TYPE=SUBMIT NAME='submit' VALUE='Send Message'>");

            client.println("</FORM>");
            client.println();

            // break;
            delay(1);
            client.stop();

            
            // Parse GET parameters
            char *getData = &currentLine[0];
            char *token = strtok(getData, "?");
            if(token) {
              char *getName = strtok(NULL, "=");
              if( strcmp(getName, "MQTT") == 0 ) {

                char *MQTTMsg = strtok(NULL, "&");
                // Replace the '+'-signs with spaces
                for (int i = 0; MQTTMsg[i]; i++) {
                  if(MQTTMsg[i] == '+') {
                    MQTTMsg[i] = ' ';
                  }
                }

                Serial.println("Sending message in 5 seconds:");
                delay(5000);
                customPublish(MQTTMsg);

              }

              if( strcmp(getName, "led") == 0) {
                char *ledState = strtok(NULL, "&");

                if( strcmp(ledState, "on") == 0) {
                  digitalWrite(ledpin, HIGH);
                }
                if( strcmp(ledState, "off") == 0) {
                  digitalWrite(ledpin, LOW);
                }

              }

            }

            currentLine = "";
          // } else currentLine = "";

        }
      } 
    }

    client.stop();
    Serial.println("client disconnected");
  }

  // MQTT Loop
  mqtt.loop();

  if(!mqtt.connected()) 
    connect();

  if (millis() - lastMillis > 10000) {
    lastMillis = millis();
    mqtt.publish("arduino/test", "Arduino here..");
  }

}

void printWiFiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP: ");
  Serial.println(ip);

  long rssi = WiFi.RSSI();
  Serial.print("Signal Strength: ");
  Serial.print(rssi);
  Serial.println(" dBm");


}
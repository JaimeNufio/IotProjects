#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

ESP8266WiFiMulti WiFiMulti;

void connectToWifi(){
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("\n[CONNECTING] WAIT %d...", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("NufioWifi", "FlapjackAndWaffles");
}

void setup() {

  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  connectToWifi();
}

void loop() {
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;

    HTTPClient http;
    String serverName = "http://192.168.1.158:3000"; 
    String routeName = "/interact";
    String deviceName= "?name=WATER";

    String path = serverName+routeName+deviceName;
    Serial.printf("\n[HTTP]");
    Serial.printf(path.c_str());

    Serial.print("\n[HTTP] begin...");
    if (http.begin(client,path.c_str())) {  // HTTP


      Serial.print("\n[HTTP] GET...");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("\n[HTTP] GET... code: %d", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          // Serial.printf("\n[PAYLOAD] - %s",payload);
          Serial.printf("\n[PAYLOAD] %s",payload.c_str());
        }
      } else {
        Serial.printf("\n[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.print("\n[HTTP] Unable to connect");
    }
  }

  Serial.print("\n[STANDBY] Waiting to re-ping");
  for (uint8_t t = 5; t > 0; t--) {
    Serial.printf(".");
    delay(1000);
  }
  Serial.println();
  Serial.flush();
}

//References:
// https://randomnerdtutorials.com/esp8266-nodemcu-http-get-post-arduino/#http-get-1

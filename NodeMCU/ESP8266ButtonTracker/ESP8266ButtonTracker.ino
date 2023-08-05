#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "ssd1306.h"
#include "nano_gfx.h"
// #include "sova.h"

ESP8266WiFiMulti WiFiMulti;

void setupLCD(){
    ssd1306_setFixedFont(ssd1306xled_font6x8);
    ssd1306_128x64_i2c_init();
    ssd1306_clearScreen();
}

void displayText(const char *str){

}

void checkConnectionAndSend(){
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    sendRequest();
  } else {
    Serial.print("\n[HTTP] Unable to connect");
  }
}

void connectToWifi(){
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("\n[CONNECTING] WAIT %d...", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("NufioWifi", "FlapjackAndWaffles");
}

void sendRequest(){  
  WiFiClient client;

  HTTPClient http;
  String serverName = "http://192.168.1.158:3000"; 
  String routeName = "/interact";
  String deviceName= "?name=WATER";

  String path = serverName+routeName+deviceName;
  Serial.printf("\n[HTTP] GET ");
  Serial.printf(path.c_str());

  Serial.print("\n[HTTP] begin...");
  if (http.begin(client,path.c_str())) {  // HTTP


    Serial.print("\n[HTTP] GET...");
    int httpCode = http.GET();

    if (httpCode > 0) {
      Serial.printf("\n[HTTP] GET... code: %d", httpCode);

      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = http.getString();
        Serial.printf("\n[PAYLOAD] %s",payload.c_str());
      }
    } else {
      Serial.printf("\n[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
}

bool isDown = false;
unsigned int lastPressed = 0;
unsigned int checked = 0;

void ButtonDown(){

  int sensorVal = digitalRead(2);
  checked++;

  if (isDown == false && sensorVal == 0){

    if (millis() - lastPressed < 5000){
     if (checked%2000==0){
      Serial.println("Too fast.");
    }
      return;
    }

    checked = 0;
    Serial.println("Button Down");
    isDown = true;
    lastPressed = millis();

  }else if (sensorVal == 1 && isDown == true){
    Serial.println("\nButton Released");

    isDown = false;
  }else if (sensorVal == 0 && isDown == true){
    if (checked%200==0){
      Serial.print(".");
    }if (checked%1000==0){
      Serial.println("");
    }
  }else{
    // Serial.println("Button idle");
  }

  
}

void setup() {
  pinMode(D4, INPUT_PULLUP); //D4
  Serial.begin(115200);
  connectToWifi();
  lastPressed = millis();
}

void loop() {

  // delay(200);
  ButtonDown();

 
}

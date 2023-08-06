#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "ssd1306.h"
#include "nano_gfx.h"
// #include "sova.h"

ESP8266WiFiMulti WiFiMulti;


bool isDown = false;
unsigned int lastPressed = 0;
unsigned int checked = 0;
unsigned int lastInteractionTimestamp = 0;

const char* ntpServer = "pool.ntp.org";
const int timeZone = -5;  // -5 for Eastern Standard Time (EST)
WiFiUDP udp;

NTPClient timeClient(udp, ntpServer, -5, 0);

WiFiClient client;

HTTPClient http;
String serverName = "http://192.168.1.158:3000"; 


unsigned long getNtpTime(){
  timeClient.update();
  unsigned long unixTimestamp = timeClient.getEpochTime();
  return unixTimestamp;
}

void setupLCD(){
    ssd1306_setFixedFont(ssd1306xled_font6x8);
    ssd1306_128x64_i2c_init();
    ssd1306_clearScreen();
}

void checkConnectionAndSendInteraction(){
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    sendInteractionRequest();
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

  delay(1000);
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    sendRecallRequest();
  } else {
    Serial.print("\n[HTTP] Unable to connect");
  }
}


void sendRecallRequest(){  
  String routeName = "/recall";
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
        Serial.printf("\n[PAYLOAD - RECALL] %d\n\n",payload.c_str());
        lastInteractionTimestamp = atoi(payload.c_str());
      }
    } else {
      Serial.printf("\n[HTTP - RECALL] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
}

void sendInteractionRequest(){  
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
        Serial.printf("\n[PAYLOAD - INTERACT] %d\n\n",atoi(payload.c_str()));
        lastInteractionTimestamp = atoi(payload.c_str());
      }
    } else {
      Serial.printf("\n[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
}

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
    checkConnectionAndSendInteraction();
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

unsigned long counter = 0;
void updateTime(){
  counter++;

  // Serial.println(counter);
  if (counter < 50) {return;} else {counter = 0;}
  if (WiFiMulti.run() == WL_CONNECTED) {
    unsigned long ntpTime = getNtpTime();
    if (ntpTime != 0) {
      Serial.printf("Latest Timestamp:%d\n",ntpTime);
      Serial.printf("Difference:%d\n",(ntpTime-lastInteractionTimestamp));
    } else {
      Serial.println("Failed to get timestamp!");
      // delay(500);
      // updateTime();
    }
  }
}

void setup() {
  pinMode(D4, INPUT_PULLUP); //D4
  Serial.begin(115200);
  connectToWifi();
  lastPressed = millis();
}

void loop() {

  delay(100);
  ButtonDown();
  updateTime();
 
}

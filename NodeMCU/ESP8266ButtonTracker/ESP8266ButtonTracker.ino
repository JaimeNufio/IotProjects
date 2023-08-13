#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "ssd1306.h"
#include "nano_gfx.h"

bool isDown = false;
unsigned int lastPressed = 0;
unsigned int checked = 0;
unsigned int currentTimestamp = 0;
unsigned int lastInteractionTimestamp = 0;

// wifi connections
WiFiClient client;
HTTPClient http;
ESP8266WiFiMulti WiFiMulti;
String serverName = "http://192.168.1.158:3000"; // the node server. 

// wifi, but for getting time from NTP server
const char* ntpServer = "pool.ntp.org";
const int timeZone = 0;  // -5 for Eastern Standard Time (EST)
WiFiUDP udp;
NTPClient timeClient(udp, ntpServer, timeZone, 0);

// interval information for getting the time again.
const unsigned long interval = 15000;  // 3 seconds in milliseconds
unsigned long previousMillis = 0;

int displayNum = 0;

uint8_t buffer[128*64];
NanoCanvas canvas(128,64, buffer);

unsigned long getNtpTime(){
  timeClient.update();
  unsigned long unixTimestamp = timeClient.getEpochTime();
  return unixTimestamp;
}

void setupLCD(){
  ssd1306_128x64_i2c_init();
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  canvas.fillRect(0, 5, 128, 5, 0xFF);
  canvas.fillRect(0, 8, 128, 8, 0xFF);
  canvas.fillRect(0, 15, 128, 15, 0xFF);
  canvas.blt(0,0);
  canvas.printFixed(20, 3, " WATER TRACKER ", STYLE_BOLD );
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  canvas.printFixed(0, 20, "Last Pressed:", STYLE_NORMAL );
  canvas.blt(0,0);
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
        Serial.printf("\n[PAYLOAD - RECALL] %s\n\n",(payload.c_str()));
        Serial.println(payload);
        lastInteractionTimestamp = atoi(payload.c_str());
        printf("RECALLED last timestamp known: %d\n",lastInteractionTimestamp);
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

void updateTimeTick(){

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) { previousMillis = currentMillis;} 
  else {return;}

  updateTime();
}

void updateTime(){
  if (WiFiMulti.run() == WL_CONNECTED) {
    unsigned long ntpTime = getNtpTime();
    if (ntpTime != 0) {
      Serial.printf("Fetched Timestamp: %d\n",ntpTime);
      Serial.printf("Difference: %d\n",(ntpTime-lastInteractionTimestamp));
      currentTimestamp = ntpTime;
    } else {
      Serial.println("Failed to get timestamp!");
      updateTime();
    }
  }else{
    Serial.println("Failed to get timestamp, not connected to internet!");
    delay(1000);
    updateTime();
  }
}

void updateTimeDisplay(int num){  

  char text[] = "";
  int effectiveNum = num;

  if (num < 3){
    num = 0;
    strcpy(text," Moments Ago");
  }else if (num < 60){ // less than a minute
    strcpy(text," Seconds Ago");
    effectiveNum = num;
  }else if (num < 60*60){ //less than an hour
    strcpy(text," Minutes Ago");
    effectiveNum = trunc(num/60);
  }else if (num < 60*60*24){ // less than a day
    strcpy(text," Hours Ago");
    effectiveNum = trunc(num/60/60);
  }else{  // days.
    strcpy(text," Days Ago");
    effectiveNum = trunc(num/60/60/24);
  }

  if (effectiveNum == displayNum) { return; }
  displayNum = effectiveNum;

    char numStr[64];
    itoa(effectiveNum,numStr,10);

  if (num > 0){
    strcat(numStr,text);
  }else{
    strcpy(numStr,text);
  }

  canvas.fillRect(0,36, 128, 64, 0x0);
  ssd1306_setFixedFont(ssd1306xled_font8x16);
  canvas.printFixed(10, 36, numStr, STYLE_BOLD );
  canvas.blt(0,0);
}

void setup() {
  pinMode(D4, INPUT_PULLUP); //D4
  setupLCD();
  Serial.begin(115200);
  
  canvas.fillRect(0,36, 128, 64, 0x0);
  ssd1306_setFixedFont(ssd1306xled_font8x16);
  canvas.printFixed(10, 36, "Connecting...", STYLE_BOLD );
  canvas.blt(0,0);

  connectToWifi();
  updateTime();

  canvas.fillRect(0,36, 128, 64, 0x0);
  ssd1306_setFixedFont(ssd1306xled_font8x16);
  canvas.blt(0,0);
}

void loop() {

  updateTimeTick();

  int sinceLastInteraction = currentTimestamp - lastInteractionTimestamp;
  updateTimeDisplay(sinceLastInteraction); 
  // printf("sinceLastInteraction: %d\n",sinceLastInteraction);
  // printf("currentTimestamp: %d\n",currentTimestamp);
  // printf("lastInteractionTimestamp: %d\n",lastInteractionTimestamp);
  ButtonDown();
  delay(100);

}

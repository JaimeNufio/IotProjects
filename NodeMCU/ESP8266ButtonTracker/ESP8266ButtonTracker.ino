#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

ESP8266WiFiMulti WiFiMulti;

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
bool millisLast = 0;

void ButtonDown(){

  int sensorVal = digitalRead(2);

  if (isDown == false && sensorVal == 0){
    Serial.println("Button Down");
    isDown = true;
    if (millis - millisLast > 5000){
      Serial.println("")
      millisLast = 0;
    }
  }else if (sensorVal == 1 && isDown == true){
    Serial.println("Button Released");
    isDown = false;
  }else if (sensorVal == 0 && isDown == true){
    Serial.println("Button Remains Down");
  }else{
    Serial.println("Button idle");
  }

  
}

void setup() {
  pinMode(2, INPUT_PULLUP);
  Serial.begin(115200);
  connectToWifi();
  millisLast = millis()
}

void loop() {

  delay(200);
  ButtonDown();

 
  // checkConnectionAndSend();


  // Serial.print("\n[STANDBY] Waiting to re-ping");
  // for (uint8_t t = 5; t > 0; t--) {
  //   Serial.printf(".");
  //   delay(1000);
  // }
  // Serial.println();
  // Serial.flush();
}

//References:
// https://randomnerdtutorials.com/esp8266-nodemcu-http-get-post-arduino/#http-get-1

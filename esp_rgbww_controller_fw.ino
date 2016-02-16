#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ESP8266HTTPUpdateServer.h>
#include <RGBWWLed.h>

//Prefix for mdns hostname
#define HOSTNAMEPREFIX "rgbww-"

//RGBWW Pins
#define BLUEPIN 14
#define GREENPIN 12
#define REDPIN 13
#define WWPIN 5
#define CWPIN 4

std::unique_ptr<ESP8266WebServer> server;
ESP8266HTTPUpdateServer httpUpdater;
WiFiManager wifiManager;
RGBWWLed rgbled;

//global vars
int inputvoltage;
unsigned long previousMillis = 0;       
const long interval = 5000;  


void handleRGBWW(){
  int rgbw[5];
  Serial.println(F("RGB request"));
  if (server->hasArg("r")){
    rgbw[0] = server->arg("r").toInt();
  }
  if (server->hasArg("g")){
    rgbw[1] = server->arg("g").toInt();
  }
  if (server->hasArg("b")){
    rgbw[2] = server->arg("b").toInt();
  }
  if (server->hasArg("ww")){
    rgbw[3] = server->arg("ww").toInt();
  }
  if (server->hasArg("cw")){
    rgbw[4] = server->arg("cw").toInt();
  }
  rgbled.setOutputRaw( rgbw[0], rgbw[1], rgbw[2], rgbw[3], rgbw[4]);
  server->send(200, "text/plain", "ok");
}

void handleHSV() {
  float hue, sat , val;
  Serial.println(F("HSV request"));
  if (server->hasArg("h") && server->hasArg("s") && server->hasArg("v")){
    hue = server->arg("h").toFloat();
    sat = server->arg("s").toFloat();
    val = server->arg("v").toFloat();
    rgbled.setOutput(HSV(hue, sat, val));
    server->send(200, "text/plain", "ok");
    
  } else {
  
    server->send(200, "text/plain", "missing params");
  }
}

void handleHSVtransition() {
  float hue, sat , val;
  int tm;
  bool shortway = true;
  Serial.println(F("HSV transition request"));
  if (server->hasArg("h") && server->hasArg("s") && server->hasArg("v")){
    hue = server->arg("h").toFloat();
    sat = server->arg("s").toFloat();
    val = server->arg("v").toFloat();
    HSV color(hue, sat, val);
    if (server->hasArg("l")) {
      shortway = false;
    }
    if (server->hasArg("tm")) {
      DEBUG("with time");
      tm = server->arg("tm").toInt();
      server->send(200, "text/plain", "ok");
      
      rgbled.setHSV(color, tm, shortway);
    } else {
      server->send(200, "text/plain", "ok");
      rgbled.setHSV(color);
    } 
  } else {
  
    server->send(200, "text/plain", "missing param");
  }
  
}

void resetWifiManager() {
  
  server->send(200, "text/plain", "ok");
  wifiManager.resetSettings();
  delay(3000);
  ESP.reset();
  delay(2000);
  
}

void handleswitchMode() {
  Serial.println(F("switchmode request"));
  if (server->hasArg("rgb")){
    rgbled.setMode(MODE_RGB);    
  } else {
    rgbled.setMode(MODE_RGBWW);
  }
  server->send(200, "text/plain", "ok");
}


void checkVoltage() {
unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis >= interval) {
    // save the last time you read the sensor 
    previousMillis = currentMillis;   
 
    inputvoltage = analogRead(A0);
    //Serial.println(inputvoltage);
  }
  
}

void checkRAM() {
unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis >= interval) {
    // save the last time you read the sensor 
    previousMillis = currentMillis;   
    Serial.print(F("Free Ram:   "));
    Serial.println(ESP.getFreeHeap());
  }

}


void setup() {
    //Serial
    Serial.begin(115200);
    Serial.println(F("Booting"));
   
    //Hostname that will be used for AP and mdns
	  String hostname(HOSTNAMEPREFIX);
    hostname += String(ESP.getChipId());

    //WiFiManager
    //reset settings - for testing
    //wifiManager.resetSettings();
    
    //wifiManager.setSaveConfigCallback(saveConfigCallback);
    wifiManager.autoConnect(hostname.c_str());

    //mdns
    if (!MDNS.begin(hostname.c_str())) {
      Serial.println(F("Error setting up MDNS responder!"));
      while(1) { 
        delay(1000);
        }
    }

    //Init Webserver
    //reset std unique ptr
    server.reset(new ESP8266WebServer(WiFi.localIP(), 80));
    server->on("/rgbww", handleRGBWW);
    server->on("/hsv", handleHSV);
    server->on("/hsvt", handleHSVtransition);
    server->on("/mode", handleswitchMode);
    server->on("/rst", resetWifiManager); 
    
    server->begin();
    
    MDNS.addService("http", "tcp", 80);
    
     //OTA
    httpUpdater.setup(server.get()); 
    Serial.print(F("Update via http://"));
    Serial.print(hostname);
    Serial.println(F(".local/update"));

    
    //Init RGBLED
    rgbled.init(REDPIN, GREENPIN, BLUEPIN, WWPIN, CWPIN);
    Serial.println(F("RGBWW Controller ready"));

}


void loop() {

    server->handleClient();
    rgbled.show();
    checkRAM();
    //checkVoltage();
    //short break for esp to catch up
    delay(1);
}

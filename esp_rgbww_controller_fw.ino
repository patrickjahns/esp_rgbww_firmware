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

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
WiFiManager wifiManager;
RGBWWLed rgbled;

int rgb[3];
int rgbw[5];
float hue;
float sat;
float val;

void updatePWM() {
    analogWrite(REDPIN, rgbw[0]);
    analogWrite(GREENPIN, rgbw[1]);
    analogWrite(BLUEPIN, rgbw[2]);
    analogWrite(WWPIN, rgbw[3]);
    analogWrite(CWPIN, rgbw[4]); 
}

void handleRGBWW(){
  Serial.println("RGB request");
  if (httpServer.hasArg("r")){
    rgbw[0] = httpServer.arg("r").toInt();
  }
  if (httpServer.hasArg("g")){
    rgbw[1] = httpServer.arg("g").toInt();
  }
  if (httpServer.hasArg("b")){
    rgbw[2] = httpServer.arg("b").toInt();
  }
  if (httpServer.hasArg("ww")){
    rgbw[3] = httpServer.arg("ww").toInt();
  }
  if (httpServer.hasArg("cw")){
    rgbw[4] = httpServer.arg("cw").toInt();
  }
  updatePWM();
  httpServer.send(200, "text/plain", "ok");
}

void handleHSV() {
  Serial.println("HSV request");
  if (httpServer.hasArg("h")){
    hue = httpServer.arg("h").toFloat();
  }
  if (httpServer.hasArg("s")){
    sat = httpServer.arg("s").toFloat();
  }
  if (httpServer.hasArg("v")){
    val = httpServer.arg("v").toFloat();
  }
  rgbled.HSVtoRGB(hue,sat,val, rgb);
  rgbw[0]=rgb[0];
  rgbw[1]=rgb[1];
  rgbw[2]=rgb[2];
  updatePWM();
  httpServer.send(200, "text/plain", "ok");
}



void setup() {
    //Output config
    pinMode(REDPIN, OUTPUT);
    pinMode(GREENPIN, OUTPUT);
    pinMode(BLUEPIN, OUTPUT);
    pinMode(WWPIN, OUTPUT);
    pinMode(CWPIN, OUTPUT); 
	
	  //Hostname that will be used for AP and mdns
	  String hostname(HOSTNAMEPREFIX);
    hostname += String(ESP.getChipId());

    //Serial
    Serial.begin(115200);
    Serial.println(F("Booting"));
    
    //WiFiManager
    wifiManager.autoConnect(hostname.c_str());

    //mdns
    if (!MDNS.begin(hostname.c_str())) {
      Serial.println(F("Error setting up MDNS responder!"));
      while(1) { 
        delay(1000);
        }
    }
    
    //OTA
    httpUpdater.setup(&httpServer);

    //Init Webserver
    httpServer.on("/rgbww", handleRGBWW);
    httpServer.on("/hsv", handleHSV);
    httpServer.begin();

    MDNS.addService("http", "tcp", 80);
    Serial.println(F("RGBWW Controller ready"));
    Serial.print(F("Update via http://"));
    Serial.print(hostname);
    Serial.println(F(".local/update"));

}


void loop() {
    httpServer.handleClient();
    delay(1);  
    
}

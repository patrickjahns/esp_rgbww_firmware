# Firmware for the RGBWW Fhem controller

This repository provides the firmware for the esp_rgbww_controller board

- [Documentation](#documentation)
  - [Installation](#installation)
  - [Current API](#current-api)
- [Work in Progress](#work-in-progress)
  - [ToDos](#todos)
  - [Ideas](#idea-space)
  - [API ideas](#api-ideas)
- [Contributing](#contributing)
- [Useful Links and Sources](#links)



## Important Information
The Firmware is a work in progress and the current arduino sketch is just to test/demo the basic functions while providing a common starting point for joined development. Most of the ideas have been summarized from a discussion at the [FHEM Forum (German)](http://forum.fhem.de/index.php/topic,34464.0.html)

The current functionality is:
- Create Wifi Accesspoint (SSID: rgbww-chipd) if no configuration for Wifi or the ESP cant connect to the last know network
- OTA via webinterface (access via http://rgbww-chipid.local/update) - rgbww-chipd can be obtained by writing down the SSID of the accesspoint
- Set the LED channels via RGBW or HSV values

<br><br>
## Documentation

### Installation
#### Flashing
Download latest binary from github and flash to the controller
<br>
#### OTA Update
Download latest binary from github, access http://rgbww-chipid.local/update

__WARNING__ If anything goes wrong during the OTA update, you will need to reflash via a serial programmer

<br>
#### Compiling yourself
Compiling the current version of the project requires:
* at least [ESP8266 Arduino Board definitions 2.1-rc2](https://github.com/esp8266/Arduino/#available-versions)
* [WifiManager library](https://github.com/tzapu/WiFiManager)
* Latest [RGBWWLed Library](https://github.com/patrickjahns/RGBWWLed)


<br><br>
## Current API
Quick overview of available commands

####RGBWW
Color output using RGB, WarmWhite (WW), ColdWhite(CW)

`http://rgbww-chipid.local/rgbww?r=RED&g=GREEN&b=BLUE&ww=WARMWHITE&cw=COLDWHITE`


All variables (RED,GREEN,BLUE,WARMWHITE,COLDWHITE) have a value between 0-1023 (10Bit ESP8266 PWM range)

####HSV
Color output by sending HSV values

`http://rgbww-chipid.local/hsv?h=HUE&s=SAT&v=VAL`

Valid values

Var | Range
--- | ---
HUE | 0.0 - 360.0
SAT | 0 - 100.0
VAL | 0 - 100.0
<br><br><br>
## Work in Progress
### Changelog
* 02.02.2016
Initial commit and README

### ToDos
* ~~Wifi connection portal~~ => [WifiManager](https://github.com/tzapu/WiFiManager)
* ~~OTA Update~~ => [Arduino OTA ESP Library](https://github.com/esp8266/Arduino/blob/master/doc/ota_updates/ota_updates.md#web-browser)
* API => see [API ideas](api-ideas)
* configuration portal
* RGBWW LED functions => see separate project [RGBWWLed Library](https://github.com/patrickjahns/RGBWWLed)

## Idea Space
Some thoughts and ideas for future versions

* __configuration portal__

  After the successful module setup and connection to an Wifi Accesspoint, it provides a configuration portal that allows to change settings for the module or reset it.
  Ideas for Settings
  - MQTT (ServerIP, PORT etc.)
  - Color correction
  - Mode of controller (RGB, RGBWW, RGBCW, RGBWW+CW, WW+CW)
  - reset controller
  - include link to OTA update page
<br><br>
* __communication__

  use mqtt for communication between module and controlling software
<br><br>
* __LED controlling__

  LED functions will be separated so library can be reused
<br><br>
* __H801 Wifi Module__

  The firmware might also be used for the [H801 Wifi Module](http://www.aliexpress.com/item/rgb-strip-WiFi-controller-1-port-control-15-rgb-lights-communicate-with-Android-phone-to-dim/32301423622.html) - currently untested. See [chaozlabs blog](http://chaozlabs.blogspot.de/2015/08/esp8266-in-wild-wifi-led-controller-hack.html) for more information on hacking the H801 Module. (OTA might not work with H801 - the flash size is unknown to me)

<br><br>
### API ideas
The following presents a list with ideas for future API calls

```
HSVColor(H,S,V,K,T,L)
```
Provide a color via using the HSV model. Additionally add K(Kelvin) as parameter to define the temperature of white
T is the duration for transition between the colors (T=0 change instantly).
Usually the change between two colors is done via the shortest way, by setting L=1 the model will use the longer way
The starting color is the active color

__Valid Values__

Var | Range
--- | ---
H   | 0.0 - 360.0
S   | 0.0 - 100.0
V   | 0.0 - 100.0
K   | 1000 - 10000
T   | 0 - 3600
L   | 0/1
<br><br>
```
HSVColor(H,S,V,K,H',S',V',K`,T,L)
```
Same principle as above, but this time fade from H,S,V,K to H',S',V',K'
<br><br>
```
RGBColor(R,G,B,W,K,T,L)
```
Set the current color via the RGB colorspace. Additionally add white(W) and provide the color temperature of white (K)
T is the duration for transition between the colors (T=0 change instantly).
Usually the change between two colors is done via the shortest way, by setting L=1 the model will use the longer way

__Valid Values__

Var | Range
--- | ---
R   | 0 - 255
G   | 0 - 255
B   | 0 - 255
W   | 0 - 255
K   | 1000 - 10000
T   | 0 - 3600
L   | 0/1
<br><br>
```
RGBColor(R,G,B,W,R',G',B',W',T,L)
```
Same as above - this time transition from R,G,B,W,K to R',G',B',W',K'
<br><br>
```
effect(effectname)
```
start a pre-programmed effect (i.e. rainbow, disco ...)
<br><br>
```
cancleTransition
```
cancel a current transition, stops at the current color
<br><br>
```
setSettings
```
set settings like colorcorrection etc.
<br><br>
```
status
```
returns active color values
<br><br>
```
info
```
provide information about controller (firmware, settings, etc...)
<br><br>
## Contributing

I encourage you to contribute to this project. All ideas, thoughts, issues and of course code is welcomed.
Just fork it and go ahead. Also contributions to the RGBWWLed library are highly welcome

Please be sure to develop in a separate branch (not master)
<br><br>
## Links

- [FHEM Forum](http://forum.fhem.de/)
- [ESP8266 Arduino Repository](https://github.com/esp8266/Arduino)
- [ESP8266 WifiManager lib](https://github.com/tzapu/WiFiManager)
- [ESP8266 MQTT client](https://github.com/knolleary/pubsubclient)
- [RGBWWLed Library](https://github.com/patrickjahns/RGBWWLed)

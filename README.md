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


## Important Information
The Firmware is a work in progress and the current arduino sketch ist just to test/demo the basic functions while providing a common starting point for joined development. Most of the ideas have been summarized from a discussion at the [FHEM Forum (German)](http://forum.fhem.de/index.php/topic,34464.0.html)

The current functionality is:
- Create Wifi Accesspoint (SSID: rgbww-chipd) if no configuration for Wifi or the ESP cant connect to the last know network
- OTA via webinterface (access via http://rgbww-chipid.local/update) - rgbww-chipd can be obtained by writing down the SSID of the accesspoint
- Set the LED channels via RGBW or HSV values

The firmware might also be used for the [H801 Wifi Module](http://www.aliexpress.com/item/rgb-strip-WiFi-controller-1-port-control-15-rgb-lights-communicate-with-Android-phone-to-dim/32301423622.html). See [chaozlabs blog](http://chaozlabs.blogspot.de/2015/08/esp8266-in-wild-wifi-led-controller-hack.html) for more information on hacking the H801 Module

## Documentation 

### Installation
#### Flashing 
Download latest binary from github and flash to the controller

#### OTA Update
Download latest binary from github, access http://rgbww-chipid.local/update
__WARNING__ Although unlikely - if anything goes wrong you need a Serial (FTDI) programmer to recover

#### Compiling yourself
Use arduino IDE (>2.0) and also use the latest [WifiManager library](https://github.com/tzapu/WiFiManager)
Also include include the latest RGBWWLed Library

### Current API
Color output using RGB, WarmWhite (WW), ColdWhite(WW) 

####RGBWW
```
http://rgbww-chipid.local/rgbww?r=RED&g=GREEN&b=BLUE&ww=WARMWHITE&cw=COLDWHITE
```
Valid values for RED,GREEN,BLUE,WARMWHITE,COLDWHITE is 0-1023 (10bit PWM steps of the ESP)


####HSV
Color output by sending HSV values
```
http://rgbww-chipid.local/hsv?h=HUE&s=SAT&v=VAL
```
Valid values for
HUE 0.0 - 360.0
SAT,VAL 0 - 100.0


## Work in Progress
### Changelog
* 02.02.2016
Initial commit and README

### ToDos
* ~~Wifi connection portal~~ => [WifiManager](https://github.com/tzapu/WiFiManager)
* ~~OTA Update~~ => [Arduino OTA ESP Library](https://github.com/esp8266/Arduino/blob/master/doc/ota_updates/ota_updates.md#web-browser)
* API => for more see [API Ideas]
* configuration portal 
* implement LED functions => [Seperate Library Project]

## Idea Space 
Some thoughts and ideas for future versions

__configuration portal__

After the module is successfullt setup and connects to an Wifi Accesspoints, provide a configuration portal that allows to change settings for the module or reset it.
Settings for change
- MQTT (ServerIP, PORT etc.)
- ColorCorrection 
- Mode of Controller (RGB, RGBWW, RGBCW, RGBWW+CD, WW+CW)
- reset controller
- include link to OTA update page

__communication__

use mqtt for communication between module and controlling software

__LED controlling__

LED functions will be seperated so library can be reused


#### API ideas
Thoughts on functionality to be provided by api

```
HSVColor(H,S,V,K,T,L)
```
Provide a color via using the HSV model. Additionally add K(Kelvin) as parameter to define the temperature of white
T is the duration for transition between the colors (T=0 change instantly). 
Usually the change between two colors is done via the shortest way, by setting L=1 the model will use the longer way
The starting color is the active color

Valid values:
H 0.0 - 360.0
S 0.0 - 100.0
V 0.0 - 100.0
K 1000 - 10000
T 0 - 3600
L 0/1

```
HSVColor(H,S,V,K,H',S',V',K`,T,L)
``` 
Same principle as above, but this time fade from H,S,V,K to H',S',V',K'

```
RGBColor(R,G,B,W,K,T,L)
```
Set the current color via the RGB colorspace. Additionally add white(W) and provide the color temperature of white (K)
T is the duration for transition between the colors (T=0 change instantly).
Usually the change between two colors is done via the shortest way, by setting L=1 the model will use the longer way
Valid Values
R 0 - 255
G 0 - 255
B 0 - 255
W 0 - 255
K 1000 - 10000
T 0 - 3600
L 0/1

```
RGBColor(R,G,B,W,R',G',B',W',T,L)
```
Same as above - this time transition from R,G,B,W,K to R',G',B',W',K' 



```
effect(effectname)
```
start a pre-programmed effect (i.e. rainbow, disco ...)


```
cancleTransition
```
cancel a current transition, stops at the current color

```
setSettings
```
set settings like colorcorrection etc

```
status
```
returns active color values

```
info
```
provide information about controller (firmware, settings, etc...)

## Contributing

I highly encourage you to contribute to this project. All ideas, thougts, issues and of course code is welcomed.
Just fork it and go ahead. Also contributions to the RGBWWLed library are highly welcome

Please be sure to develop in a seperate branch (not master)

# RGBWWFirmware
### Firmware for the RGBWW Fhem controller

This repository provides the firmware for the esp_rgbww_controller board

- [Documentation](#documentation)
  - [Installation](#installation)
  - [Current API](#current-api)
- [Work in Progress](#work-in-progress)
  - [ToDos](#todos)
  - [Ideas](#idea-space)
- [Contributing](#contributing)
- [Useful Links and Sources](#links)

<br><br>
## Documentation

The firmware is based on [Sming framework] (https://github.com/SmingHub/Sming) nonOS branch. 

### Installation
There are different ways of installing the firmware. 
Either you can flash a precompiled binary and file system or you can compile it yourself

### Flash precompiled binary
Binaries can be found at [http://patrickjahns.github.io/esp_rgbww_firmware/](http://patrickjahns.github.io/esp_rgbww_firmware/)

#### Compile and Flash
Compiling the current version of the project requires:
* Modified [Sming Framework](https://github.com/patrickjahns/Sming/tree/rgbwwdev) (patrickjahns/Sming branch rgbwwdev)
* [RGBWWLed](https://github.com/patrickjahns/RGBWWLed/)

The [Sming Framework Wiki](https://github.com/SmingHub/Sming/wiki) lists different guides for installing the framework and eclipse in order to compile it yourself.


<br><br>
## Current API
The controller provides a simple JSON API for communication

###API Endpoints
Brief documentation of API endpoints

On success - endpoints return either data or `{ 'success' : true }`.  
If a request fails, endpoints return an error. Example: `{ 'error' : 'missing param' }`

```
/config
```

`GET` return current configuration values

`POST` change the setting values
<br><br>
```
/info
```
`GET` return information about the controller
<br><br>
```
/color
```

`GET` return current color 

`POST` change color
<br><br>
```
/networks
```

`GET` list available networks / scanning status

`POST {'cmd':'scan'}` start a network scan 
<br><br>
```
/connect
```

`POST` connect to specified network (ssid/password)

`GET` receive status of connection attempt
<br><br>
```
/system
```

`POST` issue commands to execute [reset/restart/forget_wifi]
<br><br>
```
/update
```

`POST` start OTA  

`GET` return updates status
<br><br>
```
/ping
```

`GET` check connection
<br><br><br>
## Work in Progress
### Changelog
* 20.03.2016  
  added OTA functionality  
  updated API endpoints  

* 10.03.2016  
  Update repository with latest development version utilizing SMING framework  
  
* 08.03.2016  
  Notice that development will switch to SMING framework  
  
* 02.02.2016  
  Initial commit and README

### ToDos


## Idea Space
Some thoughts and ideas for implemenation

* __configuration portal__

  After the successful module setup and connection to an Wifi Accesspoint, provide a more enhanced configuration portal. 
  Features for configuration might be:
  - MQTT (ServerIP, PORT etc.)
  - TCP/UDP Server eanble/disable
  - Color correction
  - Mode of controller (RGB, RGBWW, RGBCW, RGBWW+CW)
  - reset controller
  - OTA update
<br><br>
* __communication__
  - provide different ways of communication (HTTP json API/ mqtt client/ UDP? / TCP?) to flexibel use this controller with different home automation software/projects

<br><br>
* __H801 Wifi Module__

  The firmware might also be used for the [H801 Wifi Module](http://www.aliexpress.com/item/rgb-strip-WiFi-controller-1-port-control-15-rgb-lights-communicate-with-Android-phone-to-dim/32301423622.html) - currently untested. See [chaozlabs blog](http://chaozlabs.blogspot.de/2015/08/esp8266-in-wild-wifi-led-controller-hack.html) for more information on hacking the H801 Module. (OTA might not work with H801 - the flash size is unknown to me)


## Contributing

I encourage you to contribute to this project. All ideas, thoughts, issues and of course code is welcomed.

For more information on how to contribute, please check [contribution guidelines](CONTRIBUTING.md)
<br><br>
## Links

- [FHEM Forum](http://forum.fhem.de/)
- [Sming Framework](https://github.com/SmingHub/Sming)
- [RGBWWLed Library](https://github.com/patrickjahns/RGBWWLed)

# Firmware for the RGBWW Fhem controller

# Important Information
__Development has switched from Arduino ESP framework to SMING.__

During development I experienced the PWM implementation of the Arduino ESP framework as not reliable (see [#836] (https://github.com/esp8266/Arduino/issues/836)). It was not possible to use the underlying "HardwarePWM" provided be the espressif SDK (See [#1654](https://github.com/esp8266/Arduino/issues/1654)).

The development of a firmware for the RGBWW Controller board will be continued using the [SMING](https://github.com/SmingHub/Sming) ) framework. 
[For current version please check the development branch](https://github.com/patrickjahns/esp_rgbww_firmware/tree/develop)

For the previous arduino sketch and documentation see [arduino-framework-deprecated branch](https://github.com/patrickjahns/esp_rgbww_firmware/tree/arduino-framework-deprecated)



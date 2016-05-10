## Local build configuration
## Parameters configured here will override default and ENV values.
## Uncomment and change examples:

## Add your source directories here separated by space
MODULES = app
# EXTRA_INCDIR = include

## ESP_HOME sets the path where ESP tools and SDK are located.
## Windows:
# ESP_HOME = c:\Espressif

## MacOS / Linux:
# ESP_HOME = /opt/esp-open-sdk

## SMING_HOME sets the path where Sming framework is located.
## Windows:
# SMING_HOME = c:\tools\sming\Sming 

## MacOS / Linux
# SMING_HOME = /opt/sming/Sming

## COM port parameter is reqruied to flash firmware correctly.
## Windows: 
# COM_PORT = COM3

## MacOS / Linux:
# COM_PORT = /dev/tty.usbserial

## Com port speed
COM_SPEED	= 115200

## Configure flash parameters (for ESP12-E and other new boards):
# SPI_MODE = dio

# SPI EEPROM SIZE
SPI_SIZE = 4M

#### SPIFFS options ####
# folder with files to include
SPIFF_FILES = webapp

# size of filesystem
#SPIFF_SIZE ?= 786432 #~768KB spiffs size
SPIFF_SIZE      = 284288 #~512KB spiffs size


#### rBoot options ####
# use rboot build mode
RBOOT_ENABLED = 1

#  enable tmp rom switching
RBOOT_RTC_ENABLED = 1

# enable big flash support (for multiple roms, each in separate 1mb block of flash)
RBOOT_BIG_FLASH = 1

# two rom mode (where two roms sit in the same 1mb block of flash)
RBOOT_TWO_ROMS  = 0

# where does the filesystem reside
RBOOT_SPIFFS_0  = 0x100000
RBOOT_SPIFFS_1  = 0x300000 

## output file for first rom (.bin will be appended)
#RBOOT_ROM_0     ?= rom0
## input linker file for first rom
#RBOOT_LD_0      ?= rom0.ld
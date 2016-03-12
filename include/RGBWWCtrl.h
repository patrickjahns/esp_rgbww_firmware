/**
 * @file
 * @author  Patrick Jahns http://github.com/patrickjahns
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * https://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 *
 */

#ifndef RGBWWCTRL_H_
#define RGBWWCTRL_H_

//default defines

#define DEFAULT_AP_SECURED false
#define DEFAULT_AP_PASSWORD "rgbwwctrl"
#define DEFAULT_SETTINGS_SECURED false
#define DEFAULT_SETTINGS_PASSWORD "rgbwwctrl"


#define DEFAULT_TCP_PORT 12000
#define DEFAULT_UDP_PORT 13000

#define DEFAULT_COLORTEMP_WW 2700
#define DEFAULT_COLORTEMP_CW 6000

#define BLUEPIN 14
#define GREENPIN 12
#define REDPIN 13
#define WWPIN 5
#define CWPIN 4

#define RGBWW_USE_ESP_HWPWM
#define DEBUG_RGBWW TRUE

//includes
#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/RGBWWLed/RGBWWLed.h>
#include <config.h>
#include <application.h>
#include <webserver.h>


#endif /* RGBWWCTRL_H_ */

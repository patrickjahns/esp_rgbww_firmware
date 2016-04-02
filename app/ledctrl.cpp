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
#include <RGBWWCtrl.h>

void APPLedCtrl::init() {
	RGBWWLed::init(REDPIN, GREENPIN, BLUEPIN, WWPIN, CWPIN, PWM_FREQUENCY);
	setAnimationCallback(led_callback);
	setup();
	color.load();
    HSVK c = HSVK(color.h, color.s, color.v, color.k);
    setOutput(c);
}

void APPLedCtrl::setup() {
    colorutils.setBrightnessCorrection(app.cfg.color.brightness.red,
    		app.cfg.color.brightness.green,
			app.cfg.color.brightness.blue,
			app.cfg.color.brightness.ww,
			app.cfg.color.brightness.cw);
    colorutils.setHSVcorrection(app.cfg.color.hsv.red,
    		app.cfg.color.hsv.yellow,
			app.cfg.color.hsv.green,
			app.cfg.color.hsv.cyan,
			app.cfg.color.hsv.blue,
			app.cfg.color.hsv.magenta);
    colorutils.setColorMode((RGBWW_COLORMODE)app.cfg.color.outputmode);
    colorutils.setHSVmodel((RGBWW_HSVMODEL)app.cfg.color.hsv.model);

}

void APPLedCtrl::show_led() {
	show();
}

void APPLedCtrl::start() {
	ledTimer.initializeMs(20, TimerDelegate(&APPLedCtrl::show_led, this)).start();
}

void APPLedCtrl::stop() {
	ledTimer.stop();
}

void APPLedCtrl::save_color() {
	debugapp("APPLedCtrl::save_color");
	HSVK c = getCurrentColor();
	color.h = c.h;
	color.s = c.s;
	color.v = c.v;
	color.k = c.k;
	color.save();
}

void APPLedCtrl::led_callback(RGBWWLed* rgbwwctrl) {
	debugapp("APPLedCtrl::led_callback");
	app.rgbwwctrl.save_color();
}

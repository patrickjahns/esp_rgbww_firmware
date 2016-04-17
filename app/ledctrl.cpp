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
	debugapp("APPLedCtrl::init");
	RGBWWLed::init(REDPIN, GREENPIN, BLUEPIN, WWPIN, CWPIN, PWM_FREQUENCY);
	setAnimationCallback(led_callback);
	setup();
	color.load();
	debugapp("H: %i | s: %i | v: %i | ct: %i", color.h, color.s, color.v, color.ct);
    HSVCT s = HSVCT(color.h, color.s, 0, color.ct);
	HSVCT c = HSVCT(color.h, color.s, color.v, color.ct);
    fadeHSV(s, c, 700); //fade to color in 700ms
}

void APPLedCtrl::setup() {
	debugapp("APPLedCtrl::setup");

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
	//int curmicros = millis();
	if(!show()){
		//debugapp("start %i end: %i, diff %i", curmicros, millis(), millis() - curmicros);
	}

}

void APPLedCtrl::start() {
	debugapp("APPLedCtrl::start");
	ledTimer.initializeMs(RGBWW_MINTIMEDIFF, TimerDelegate(&APPLedCtrl::show_led, this)).start();
}

void APPLedCtrl::stop() {
	debugapp("APPLedCtrl::stop");
	ledTimer.stop();
}

void APPLedCtrl::color_save() {
	debugapp("APPLedCtrl::save_color");
	HSVCT c = getCurrentColor();
	color.h = c.h;
	color.s = c.s;
	color.v = c.v;
	color.ct = c.ct;
	color.save();
}

void  APPLedCtrl::color_reset() {
	debugapp("APPLedCtrl::reset_color");
	color.h = 0;
	color.s = 0;
	color.v = 0;
	color.ct = 0;
	color.save();
}

void APPLedCtrl::test_channels() {
	ChannelOutput red = ChannelOutput(1023, 0, 0, 0, 0);
	ChannelOutput green = ChannelOutput(0, 1023, 0, 0, 0);
	ChannelOutput blue = ChannelOutput(0, 0, 1023, 0, 0);
	ChannelOutput ww = ChannelOutput(0, 0, 0, 1023, 0);
	ChannelOutput cw = ChannelOutput(0, 0, 0, 0, 1023);
	ChannelOutput black = ChannelOutput(0, 0, 0, 0, 0);

	fadeRAW(black, red, 2000, true);
	fadeRAW(black, 2000, true);
	fadeRAW(green, 2000, true);
	fadeRAW(black, 2000, true);
	fadeRAW(blue, 2000, true);
	fadeRAW(black, 2000, true);
	fadeRAW(ww, 2000, true);
	fadeRAW(black, 2000, true);
	fadeRAW(cw, 2000, true);
	fadeRAW(black, 2000, true);
}

void APPLedCtrl::led_callback(RGBWWLed* rgbwwctrl) {
	debugapp("APPLedCtrl::led_callback");
	app.rgbwwctrl.color_save();
}

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
#ifndef APP_LEDCTRL_H_
#define APP_LEDCTRL_H_

#define APP_COLOR_FILE ".color"

struct ColorStorage {
	int h = 0;
	int s = 0;
	int v = 0;
	int ct = 0;

	void load(bool print = false) {
		StaticJsonBuffer < 72 > jsonBuffer;
		if (exist()) {
			int size = fileGetSize(APP_COLOR_FILE);
			char* jsonString = new char[size + 1];
			fileGetContent(APP_COLOR_FILE, jsonString, size + 1);
			JsonObject& root = jsonBuffer.parseObject(jsonString);
			h = root["h"];
			s = root["s"];
			v = root["v"];
			ct = root["ct"];
			if (print) {
				root.prettyPrintTo(Serial);
			}
			delete[] jsonString;
		}
	}

	void save(bool print = false) {
		DynamicJsonBuffer jsonBuffer;
		JsonObject& root = jsonBuffer.createObject();
		root["h"] = h;
		root["s"] = s;
		root["v"] = v;
		root["ct"] = ct;
		String rootString;
		if (print) {
			root.prettyPrintTo(Serial);
		}
		root.printTo(rootString);
		fileSetContent(APP_COLOR_FILE, rootString);
	}
	bool exist() {
		return fileExist(APP_COLOR_FILE);
	}
};

typedef Delegate<bool(void)> ledctrlDelegate;

class APPLedCtrl: public RGBWWLed {

public:
	void init();
	void setup();

	void start();
	void stop();
	void color_save();
	void color_reset();
	void test_channels();

	void show_led();
	static void led_callback(RGBWWLed* rgbwwctrl);

private:
	ColorStorage color;
	Timer ledTimer;

};

#endif

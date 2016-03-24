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

Application app;

// Sming Framework INIT method - called during boot
void init() {
	Serial.printf("RGBWW Controller v %s\r\n", fw_version);
	app.init();
}

// seperated application init
void Application::init() {


	// Mount file system, in order to work with files
	spiffs_mount_manual(RBOOT_SPIFFS_0 + 0x40200000, SPIFF_SIZE);

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(false); // don`t show system debug messages

	//load settings
	//TODO check for reset here and delete file or reload file depending on



	if (cfg.exist()) {
		cfg.load(true);
	} else {
		debugapp("Application::init - it is first run");
		_first_run = true;
		cfg.save();
	}

	// initialize led ctrl
	rgbwwctrl.init();

	// initialize networking
	network.init();

	// Run Services on system ready
	System.onReady(SystemReadyDelegate(&Application::startServices, this));


}

bool Application::isFirstRun() {
	return _first_run;
}

// Will be called when system initialization was completed
void Application::startServices()
{
	rgbwwctrl.start();
	webserver.start();

	//start TCP
	//start UDP
	//start mqtt client
}


void Application::restart() {
	Serial.println("Restarting");
	System.restart();
}


void Application::reset() {
	debugapp("Application::reset");
	cfg.reset(); // reset configuration
	network.forget_wifi(); // reset wifi
	delay(1000);
	restart();
}

bool Application::delayedCMD(String cmd, int delay) {
	debugapp("Application::delayedCMD cmd: %s - delay: %i", cmd.c_str(), delay);
	if(cmd.equals("reset")) {
		_systimer.initializeMs(delay, TimerDelegate(&Application::reset, this)).startOnce();
	} else if(cmd.equals("restart")) {
		_systimer.initializeMs(delay, TimerDelegate(&Application::restart, this)).startOnce();
	} else if(cmd.equals("stopap")) {
		network.stopAp(2000);
	} else if(cmd.equals("stopapandrestart")) {
		network.stopAp(delay);
		_systimer.initializeMs(delay+4000, TimerDelegate(&Application::restart, this)).startOnce();
	} else if (cmd.equals("forget_wifi")) {
		network.startAp();
		_systimer.initializeMs(delay, TimerDelegate(&AppWIFI::forget_wifi, &network)).startOnce();
	} else {
		return false;
	}
	return true;
}

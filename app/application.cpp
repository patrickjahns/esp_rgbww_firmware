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

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // don`t show system debug messages
	//System.setCpuFrequencye(CF_160MHz);

	// seperated application init
	app.init();

	// Run Services on system ready
	System.onReady(SystemReadyDelegate(&Application::startServices, &app));
}




void Application::init() {

	Serial.printf("RGBWW Controller v %s\r\n", fw_version);
	//load settings
	Serial.println();

	// load boot information
	uint8 bootmode, bootslot;
	if(rboot_get_last_boot_mode(&bootmode)) {
		if(bootmode == MODE_TEMP_ROM) {
			debugapp("Application::init - booting after OTA");
		}
		else
		{
			debugapp("Application::init - normal boot");
		}
		_bootmode = bootmode;
	}

	if(rboot_get_last_boot_rom(&bootslot)) {
		_romslot = bootslot;
	}

	// mount filesystem
	mountfs(getRomSlot());

	// check ota
	ota.checkAtBoot();

	// load config
	if (cfg.exist()) {
		cfg.load();
	} else {
		debugapp("Application::init - first run");
		_first_run = true;
		cfg.save();
	}

	// initialize led ctrl
	rgbwwctrl.init();

	// initialize networking
	network.init();

	// initialize webserver
	app.webserver.init();

}

// Will be called when system initialization was completed
void Application::startServices()
{
	debugapp("Application::startServices");
	rgbwwctrl.start();
	webserver.start();

}


void Application::restart() {
	Serial.println("Restarting");
	System.restart();
}


void Application::reset() {
	debugapp("Application::reset");
	Serial.println("resetting controller");
	cfg.reset();
	rgbwwctrl.color_reset();
	network.forget_wifi();
	delay(100);
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
	} else if(cmd.equals("stopap_restart")) {
		network.stopAp(delay);
		_systimer.initializeMs(delay+4000, TimerDelegate(&Application::restart, this)).startOnce();
	} else if (cmd.equals("forget_wifi")) {
		network.startAp();
		network.scan();
		_systimer.initializeMs(delay, TimerDelegate(&AppWIFI::forget_wifi, &network)).startOnce();
	} else if(cmd.equals("test_channels")){
		rgbwwctrl.test_channels();
    }else if(cmd.equals("switch_rom")){
		switchRom();
		_systimer.initializeMs(delay, TimerDelegate(&Application::restart, this)).startOnce();
    } else {
		return false;
	}
	return true;
}

void Application::mountfs(int slot) {
	debugapp("Application::mountfs rom slot: %i", slot);
	if (slot == 0) {
		debugapp("Application::mountfs trying to mount spiffs at %x, length %d", RBOOT_SPIFFS_0 + 0x40200000, SPIFF_SIZE);
		spiffs_mount_manual(RBOOT_SPIFFS_0 + 0x40200000, SPIFF_SIZE);
	} else {
		debugapp("Application::mountfs trying to mount spiffs at %x, length %d", RBOOT_SPIFFS_1 + 0x40200000, SPIFF_SIZE);
		spiffs_mount_manual(RBOOT_SPIFFS_1 + 0x40200000, SPIFF_SIZE);
	}
	_fs_mounted = true;
}

void Application::umountfs() {
	debugapp("Application::umountfs");
	spiffs_unmount();
	_fs_mounted = false;
}

void Application::switchRom() {
	debugapp("Application::switchRom");
	int slot = getRomSlot();
	if (slot == 0) slot = 1; else slot = 0;
	rboot_set_current_rom(slot);
}

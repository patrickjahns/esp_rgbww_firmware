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

void ApplicationOTA::start(String romurl, String spiffsurl) {
	debugapp("ApplicationOTA::start");
	Serial.println("Starting OTA ...");
	reset();
	uint8 slot;
	rboot_config bootconf;
	fwstatus = OTASTATUS::OTA_PROCESSING;
	if(otaUpdater) delete otaUpdater;
	otaUpdater = new rBootHttpUpdate();

	bootconf = rboot_get_config();
	slot = bootconf.current_rom;

	if (slot == 0) slot = 1; else slot = 0;
	rom_slot = slot;
	otaUpdater->addItem(bootconf.roms[slot], romurl);

	//TODO proper RBOOT values
	if (slot == 0) {
		otaUpdater->addItem(RBOOT_SPIFFS_0, spiffsurl);
	} else {
		otaUpdater->addItem(RBOOT_SPIFFS_1, spiffsurl);
	}
	//ADD SPIFFS
	otaUpdater->setCallback(otaUpdateDelegate(&ApplicationOTA::rBootCallback, this));

	otaUpdater->start();
}

void ApplicationOTA::reset() {
	debugapp("ApplicationOTA::reset");
	fwstatus = OTASTATUS::OTA_NOT_UPDATING;
	if(otaUpdater) delete otaUpdater;
}

OTASTATUS ApplicationOTA::getStatus() {
	return fwstatus;
}

void ApplicationOTA::beforeOTA() {
	debugapp("ApplicationOTA::beforeOTA");
	//
}

void ApplicationOTA::afterOTA() {
	debugapp("ApplicationOTA::afterOTA");
	if(fwstatus == OTASTATUS::OTA_SUCCESS) {
		//
		app.umountfs(); //unmount old fs
		app.mountfs(rom_slot); // mount new fs
		app.cfg.save(); // save settings
		app.rgbwwctrl.color.save(); // save current color
		app.umountfs(); // umount new fs
		app.mountfs(); // mount old fs
	}
}

bool ApplicationOTA::isProccessing() {
	return fwstatus == OTASTATUS::OTA_PROCESSING;
}

void ApplicationOTA::rBootCallback(bool result) {
	debugapp("ApplicationOTA::rBootCallback");
	if(result == true) {

		fwstatus = OTASTATUS::OTA_SUCCESS;
		rboot_set_current_rom(rom_slot);

		Serial.println("OTA successful");
		// restart after 10s - gives clients enough time
		// to fetch status and init restart themselves
		// don`t automatically restart
		// app.delayedCMD("restart", 10000);
	} else {
		fwstatus = OTASTATUS::OTA_FAILED;
		Serial.println("OTA failed");
	}

}


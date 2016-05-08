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
	status = OTASTATUS::OTA_PROCESSING;
	if(otaUpdater) delete otaUpdater;
	otaUpdater = new rBootHttpUpdate();

	rboot_config bootconf = rboot_get_config();
	rom_slot = app.getRomSlot();

	if (rom_slot == 0) rom_slot = 1; else rom_slot = 0;

	otaUpdater->addItem(bootconf.roms[rom_slot], romurl);

	if (rom_slot == 0) {
		otaUpdater->addItem(RBOOT_SPIFFS_0, spiffsurl);
	} else {
		otaUpdater->addItem(RBOOT_SPIFFS_1, spiffsurl);
	}
	otaUpdater->setCallback(otaUpdateDelegate(&ApplicationOTA::rBootCallback, this));
	beforeOTA();
	otaUpdater->start();
}

void ApplicationOTA::reset() {
	debugapp("ApplicationOTA::reset");
	status = OTASTATUS::OTA_NOT_UPDATING;
	if(otaUpdater) delete otaUpdater;
}

void ApplicationOTA::beforeOTA() {
	debugapp("ApplicationOTA::beforeOTA");

	// save failed to old rom
	saveStatus(OTASTATUS::OTA_FAILED);
}

void ApplicationOTA::afterOTA() {
	debugapp("ApplicationOTA::afterOTA");
	if(status == OTASTATUS::OTA_SUCCESS_REBOOT) {

		// unmount old Filesystem - mount new filesystem
		app.umountfs();
		app.mountfs(rom_slot);

		// save settings / color into new rom space
		app.cfg.save();
		app.rgbwwctrl.color_save();

		// save success to new rom
		saveStatus(OTASTATUS::OTA_SUCCESS);

		// remount old filesystem
		app.umountfs();
		app.mountfs(app.getRomSlot());

	}
}

void ApplicationOTA::rBootCallback(bool result) {
	debugapp("ApplicationOTA::rBootCallback");
	if(result == true) {

		// set new temporary boot rom
		debugapp("ApplicationOTA::rBootCallback temp boot %i", rom_slot);
		if(rboot_set_temp_rom(rom_slot)) {
			status = OTASTATUS::OTA_SUCCESS_REBOOT;
			Serial.println("OTA successful");
		} else {
			status = OTASTATUS::OTA_FAILED;
			Serial.println("OTA failed - could not change the rom");
		}
		// restart after 10s - gives clients enough time
		// to fetch status and init restart themselves
		// don`t automatically restart
		// app.delayedCMD("restart", 10000);
	} else {
		status = OTASTATUS::OTA_FAILED;
		Serial.println("OTA failed");
	}
	afterOTA();

}

void ApplicationOTA::checkAtBoot() {
	debugapp("ApplicationOTA::checkAtBoot");
	status = loadStatus();
	if(app.isTempBoot()) {
		debugapp("ApplicationOTA::checkAtBoot permanently enabling rom %i", app.getRomSlot());
		rboot_set_current_rom(app.getRomSlot());
		saveStatus(OTASTATUS::OTA_NOT_UPDATING);
	}
}

void ApplicationOTA::saveStatus(OTASTATUS status) {
	debugapp("ApplicationOTA::saveStatus");
	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();
	root["status"] = int(status);
	String rootString;
	root.printTo(rootString);
	fileSetContent(OTA_STATUS_FILE, rootString);
}

OTASTATUS ApplicationOTA::loadStatus() {
	debugapp("ApplicationOTA::loadStatus");
	if(fileExist(OTA_STATUS_FILE)) {
		DynamicJsonBuffer jsonBuffer;
		int size = fileGetSize(OTA_STATUS_FILE);
		char* jsonString = new char[size + 1];
		fileGetContent(OTA_STATUS_FILE, jsonString, size + 1);
		JsonObject& root = jsonBuffer.parseObject(jsonString);
		OTASTATUS status = (OTASTATUS)root["status"].as<int>();
		delete[] jsonString;
		return status;
	}
	else
	{
		return OTASTATUS::OTA_NOT_UPDATING;
	}
}


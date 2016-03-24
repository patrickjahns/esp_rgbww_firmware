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


/***************************************************************
 * 			WebappOTA
 ***************************************************************/


WebappOTA::WebappOTA() {
	curitem = 0;
	delegate = NULL;
}


void WebappOTA::setCallback(webappUpdateDelegate callback) {
	this->delegate = callback;
}


void WebappOTA::addItem(String filename, String url) {
	webappUpdateItem it;
	it.filename = filename;
	it.url = url;
	items.add(it);
}


void WebappOTA::start() {
	//backup files
	for (int i = 0; i < items.count(); i++) {
		if(fileExist(items[i].filename)) {
			String backup = items[i].filename + ".bk";
			if(fileExist(backup)) {
				fileDelete(backup);
			}
			fileRename(items[i].filename, backup);
			debugapp("WebappOTA::start renaming %s %s", items[i].filename.c_str(), backup.c_str());
		}
	}

	//initialize updatetimer
	timer.initializeMs(500, TimerDelegate(&WebappOTA::onTimer, this)).start();
}


void WebappOTA::finished(bool result) {
	timer.stop();
	if(delegate)
	{
		delegate(result);
	} else {
		if(result) {
			success();
		} else {
			failure();
		}
	}
}


void WebappOTA::success() {
	//delete backup files
	for (int i = 0; i < items.count(); i++) {
		String backup = items[i].filename + ".bk";
		debugapp("WebappOTA::success removing %s", backup.c_str());
		fileDelete(backup);
	}
	debugapp("WebappOTA::success WEBAPP Update successful");
	items.clear();
}


void WebappOTA::failure() {
	for (int i = 0; i < items.count(); i++) {
		if(fileExist(items[i].filename)) {
			fileDelete(items[i].filename);
		}

		String backup = items[i].filename + ".bk";
		if(fileExist(backup)) {
			fileRename(backup, items[i].filename);
			debugapp("WebappOTA::failure restoring backup %s", backup.c_str());
		} else {
			debugapp("WebappOTA::failure ERROR - no backup %s", backup.c_str());
		}
	}
	debugapp("WebappOTA::failure WEBAPP Update failed");
	items.clear();
}


void WebappOTA::onTimer() {
	if (TcpClient::isProcessing()) return;

	if (TcpClient::getConnectionState() == eTCS_Successful) {

		if (!isSuccessful()) {
			//TODO more verbose error information
			finished(false);
			return;
		}

		curitem++;
		if (curitem >= items.count()) {
			debugapp("WebappOTA::onTimer Webapp downloads finished");
			finished(true);
			return;
		}

	} else if (TcpClient::getConnectionState() == eTCS_Failed) {
		finished(false);
		return;
	}
	webappUpdateItem  &it = items[curitem];
	debugapp("WebappOTA::onTimer Downloading %s (%s)\r\n", it.url.c_str(), it.filename.c_str());
	downloadFile(it.url, it.filename);
}

/***************************************************************
 * 			ApplicationOTA
 ***************************************************************/

void ApplicationOTA::cleanupOTAafterReset() {
	//TODO: rewrite so we scan for *.bk files and restore them
	bool cleanup = false;
    if (fileExist("init.html.bk")) {
    	fileDelete("init.html.gz");
    	fileRename("init.html.bk", "init.html.gz");
    	cleanup = true;
    }
    if (fileExist("index.html.bk")) {
    	fileDelete("index.html.gz");
    	fileRename("index.html.bk", "index.html.gz");
    	cleanup = true;
    }
    if (fileExist("app.min.css.bk")) {
    	fileDelete("app.min.css.gz");
    	fileRename("app.min.css.bk", "app.min.css.gz");
    	cleanup = true;
    }
    if (fileExist("app.min.js.bk")) {
    	fileDelete("app.min.js.gz");
    	fileRename("app.min.js.bk", "app.min.js.gz");
    	cleanup = true;
    }
    if(cleanup) {
    	Serial.println("restored webapplication files after failed OTA");
    }
}


void ApplicationOTA::rBootCallback(bool result) {
	if(result == true) {
		fwstatus = OTASTATUS::OTA_SUCCESS;
		Serial.println("Firmware update successfull");
	} else {
		fwstatus = OTASTATUS::OTA_FAILED;
		Serial.println("Firmware update failed");
	}
	finished();
}


void ApplicationOTA::webappCallback(bool result) {
	if(result == true) {
		webappstatus = OTASTATUS::OTA_SUCCESS;
		Serial.println("Webapp update successfull");
	} else {
		webappstatus = OTASTATUS::OTA_FAILED;
		Serial.println("Webapp update failed");
	}
	finished();
}


void ApplicationOTA::finished() {

	if(fwstatus == OTASTATUS::OTA_PROCESSING || webappstatus == OTASTATUS::OTA_PROCESSING) return;
	if((fwstatus != OTASTATUS::OTA_NOT_UPDATING && fwstatus == OTASTATUS::OTA_FAILED) ||
			(webappstatus != OTASTATUS::OTA_NOT_UPDATING && webappstatus == OTASTATUS::OTA_FAILED)) {

		webappUpdater->failure();
		Serial.println("OTA failed");

	} else {
		webappUpdater->success();
		rboot_set_current_rom(rom_slot);
		Serial.println("OTA successful");
		// restart after 10s - gives clients enough time
		// to fetch status and init restart themselves
		app.delayedCMD("restart", 10000);
	}

}

void ApplicationOTA::start() {
	if(fwstatus == OTASTATUS::OTA_PROCESSING) {
		startFirmwareOTA();
	}
	if(webappstatus == OTASTATUS::OTA_PROCESSING) {
		startWebappOTA();
	}

}


void ApplicationOTA::reset() {
	fwstatus = OTASTATUS::OTA_NOT_UPDATING;
	webappstatus = OTASTATUS::OTA_NOT_UPDATING;
	if(otaUpdater) delete otaUpdater;
	if(webappUpdater) delete webappUpdater;
}


OTASTATUS ApplicationOTA::getFirmwareStatus(){
	return fwstatus;
}


OTASTATUS ApplicationOTA::getWebappStatus(){
	return webappstatus;
}


OTASTATUS ApplicationOTA::getStatus() {
	if (getFirmwareStatus() == OTASTATUS::OTA_NOT_UPDATING ||
			getWebappStatus() == OTASTATUS::OTA_NOT_UPDATING)
	{
		return OTASTATUS::OTA_NOT_UPDATING;
	}
	if (getFirmwareStatus() == OTASTATUS::OTA_PROCESSING ||
			getWebappStatus() == OTASTATUS::OTA_PROCESSING)
	{
		return OTASTATUS::OTA_PROCESSING;
	}
	if((fwstatus != OTASTATUS::OTA_NOT_UPDATING && fwstatus == OTASTATUS::OTA_FAILED) ||
			(webappstatus != OTASTATUS::OTA_NOT_UPDATING && webappstatus == OTASTATUS::OTA_FAILED)) {
		return OTASTATUS::OTA_FAILED;
	} else {
		return OTASTATUS::OTA_SUCCESS;
	}
}


void ApplicationOTA::startFirmwareOTA() {
	Serial.println("Starting Firmware update...");
	// start update
	otaUpdater->start();
}


void ApplicationOTA::initFirmwareUpdate(String url) {
	uint8 slot;
	rboot_config bootconf;
	fwstatus = OTASTATUS::OTA_PROCESSING;
	if(otaUpdater) delete otaUpdater;
	otaUpdater = new rBootHttpUpdate();

	bootconf = rboot_get_config();
	slot = bootconf.current_rom;
	if (slot == 0) slot = 1; else slot = 0;
	rom_slot = slot;
	otaUpdater->addItem(bootconf.roms[slot], url);
	otaUpdater->setCallback(otaUpdateDelegate(&ApplicationOTA::rBootCallback, this));
}


void ApplicationOTA::startWebappOTA() {
	Serial.println("Staring webapp update.... ");
	// start webappupdate
	webappUpdater->start();
}


void ApplicationOTA::initWebappUpdate(String urls[], int count) {
	webappstatus = OTASTATUS::OTA_PROCESSING;
	if(webappUpdater) delete webappUpdater;
	webappUpdater = new WebappOTA();
	for (int i = 0; i < count; i++) {
		String fname = urls[i].substring(urls[i].lastIndexOf("/")+1, urls[i].length());
		webappUpdater->addItem(fname, urls[i]);
	}
	webappUpdater->setCallback(webappUpdateDelegate(&ApplicationOTA::webappCallback, this));
}

#include <RGBWWCtrl.h>



void cleanupOTAafterReset() {
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
		fwstatus = OTASTATUS::SUCCESS;
		Serial.println("Firmware update successfull");
	} else {
		fwstatus = OTASTATUS::FAILED;
		Serial.println("Firmware update failed");
	}
	finished();
}


void ApplicationOTA::webappCallback(bool result) {
	if(result == true) {
		webappstatus = OTASTATUS::SUCCESS;
		Serial.println("Webapp update successfull");
	} else {
		webappstatus = OTASTATUS::FAILED;
		Serial.println("Webapp update failed");
	}
	finished();
}


void ApplicationOTA::finished() {

	if(fwstatus == OTASTATUS::PROCESSING || webappstatus == OTASTATUS::PROCESSING) return;
	if((fwstatus != OTASTATUS::NOT_UPDATING && fwstatus == OTASTATUS::FAILED) ||
			(webappstatus != OTASTATUS::NOT_UPDATING && webappstatus == OTASTATUS::FAILED)) {

		webappUpdater->failure();
		Serial.println("OTA failed");

	}
	else
	{
		webappUpdater->success();
		rboot_set_current_rom(rom_slot);
		Serial.println("OTA successful");
	}

}

void ApplicationOTA::start() {
	if(fwstatus == OTASTATUS::PROCESSING) {
		startFirmwareOTA();
	}
	if(webappstatus == OTASTATUS::PROCESSING) {
		startWebappOTA();
	}

}


void ApplicationOTA::reset() {
	fwstatus = OTASTATUS::NOT_UPDATING;
	webappstatus = OTASTATUS::NOT_UPDATING;
	if(otaUpdater) delete otaUpdater;
	if(webappUpdater) delete webappUpdater;

}


OTASTATUS ApplicationOTA::getFirmwareStatus(){
	return fwstatus;
}


OTASTATUS ApplicationOTA::getWebappStatus(){
	return webappstatus;
}



void ApplicationOTA::initFirmwareUpdate(String url) {

	uint8 slot;
	rboot_config bootconf;
	fwstatus = OTASTATUS::PROCESSING;

	// need a clean object, otherwise if run before and failed will not run again
	if(otaUpdater) delete otaUpdater;
	otaUpdater = new rBootHttpUpdate();

	// select rom slot to flash
	bootconf = rboot_get_config();
	slot = bootconf.current_rom;
	if (slot == 0) slot = 1; else slot = 0;
	rom_slot = slot;
	otaUpdater->addItem(bootconf.roms[slot], url);
	// callback
	otaUpdater->setCallback(otaUpdateDelegate(&ApplicationOTA::rBootCallback, this));
}


void ApplicationOTA::initWebappUpdate(String urls[], int count) {
	if(webappUpdater) delete webappUpdater;
	webappUpdater = new WebappOTA();
	//TODO: change to parsing urls
	for (int i = 0; i < count; i++) {
		String fname = urls[i].substring(urls[i].lastIndexOf("/")+1, urls[i].length());
		webappUpdater->addItem(fname, urls[i]);
	}
	webappUpdater->setCallback(webappUpdateDelegate(&ApplicationOTA::webappCallback, this));
	webappstatus = OTASTATUS::PROCESSING;
}


void ApplicationOTA::startFirmwareOTA() {

	Serial.println("Starting Firmware update...");
	// start update
	otaUpdater->start();
}


void ApplicationOTA::startWebappOTA() {
	Serial.println("Staring webapp update.... ");
	// start webappupdate
	webappUpdater->start();
}


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
		//debugf(" - item: %d, addr: %X, len: %d bytes", i, items[i].targetOffset, items[i].size);
		if(fileExist(items[i].filename)) {
			String backup = items[i].filename + ".bk";
			if(fileExist(backup)) {
				fileDelete(backup);
			}
			fileRename(items[i].filename, backup);
			debugf("renaming %s %s", items[i].filename.c_str(), backup.c_str());
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
	}
	else
	{
		if(result) {
			success();
		}
		else
		{
			failure();
		}
	}
}


void WebappOTA::success() {
	//delete backup files
	for (int i = 0; i < items.count(); i++) {
		//debugf(" - item: %d, addr: %X, len: %d bytes", i, items[i].targetOffset, items[i].size);

		String backup = items[i].filename + ".bk";
		debugf("removing %s", backup.c_str());
		fileDelete(backup);
	}
	debugf("WEBAPP Update successful");
	items.clear();
}


void WebappOTA::failure() {
	for (int i = 0; i < items.count(); i++) {
		//debugf(" - item: %d, addr: %X, len: %d bytes", i, items[i].targetOffset, items[i].size);

		if(fileExist(items[i].filename)) {
			fileDelete(items[i].filename);
			String backup = items[i].filename + ".bk";
			fileRename(backup, items[i].filename);
			debugf("backup %s", backup.c_str());
		}
	}
	debugf("WEBAPP Update failed");
	items.clear();
}


void WebappOTA::onTimer() {
	if (TcpClient::isProcessing()) return;

	if (TcpClient::getConnectionState() == eTCS_Successful) {

		if (!isSuccessful()) {
			finished(false);
			return;
		}

		curitem++;
		if (curitem >= items.count()) {
			debugf("Webapp downloads finished");
			finished(true);
			return;
		}

	} else if (TcpClient::getConnectionState() == eTCS_Failed) {
		finished(false);
		return;
	}
	webappUpdateItem  &it = items[curitem];
	debugf("Downloading %s (%s)\r\n", it.url.c_str(), it.filename.c_str());
	downloadFile(it.url, it.filename);
}

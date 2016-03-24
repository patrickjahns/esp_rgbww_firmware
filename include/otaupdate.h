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
#ifndef OTAUPDATE_H_
#define OTAUPDATE_H_

enum OTASTATUS {
	OTA_NOT_UPDATING = 0,
	OTA_PROCESSING = 1,
	OTA_SUCCESS = 2,
	OTA_FAILED = 3
};


typedef Delegate<void(bool result)> webappUpdateDelegate;

struct webappUpdateItem {
	String url;
	String filename;
};

class WebappOTA: private HttpClient
{
public:
	WebappOTA();
	virtual ~WebappOTA(){};

	void start();
	void setCallback(webappUpdateDelegate callback);
	void addItem(String filename, String url);
	void success();
	void failure();

protected:
	Vector<webappUpdateItem> items;
	Timer timer;
	int curitem;
	webappUpdateDelegate delegate;

protected:
	void onTimer();
	void finished(bool result);

};


class ApplicationOTA
{
public:

	void start();
	void reset();
	void initFirmwareUpdate(String url);
	void initWebappUpdate(String urls[], int count);
	OTASTATUS getStatus();
	OTASTATUS getFirmwareStatus();
	OTASTATUS getWebappStatus();

	static void cleanupOTAafterReset();

protected:
	rBootHttpUpdate* otaUpdater;
	WebappOTA* webappUpdater;
	uint8 rom_slot;
	OTASTATUS fwstatus = OTASTATUS::OTA_NOT_UPDATING;
	OTASTATUS webappstatus = OTASTATUS::OTA_NOT_UPDATING;

protected:
	void startFirmwareOTA();
	void startWebappOTA();
	void finished();

	void rBootCallback(bool result);
	void webappCallback(bool result);
};





#endif // OTAUPDATE_H_

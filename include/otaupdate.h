#ifndef OTAUPDATE_H_
#define OTAUPDATE_H_

enum OTASTATUS {
	NOT_UPDATING = 0,
	PROCESSING = 1,
	SUCCESS = 2,
	FAILED = 3
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
	OTASTATUS getFirmwareStatus();
	OTASTATUS getWebappStatus();

	static void cleanupOTAafterReset();

protected:
	rBootHttpUpdate* otaUpdater;
	WebappOTA* webappUpdater;
	uint8 rom_slot;
	OTASTATUS fwstatus = OTASTATUS::NOT_UPDATING;
	OTASTATUS webappstatus = OTASTATUS::NOT_UPDATING;

	void startFirmwareOTA();
	void startWebappOTA();
	void finished();

	void rBootCallback(bool result);
	void webappCallback(bool result);
};





#endif // OTAUPDATE_H_

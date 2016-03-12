#ifndef APPLICATION_H_
#define APPLICATION_H_

static const char * fw_version = FWVERSION;
static const char * fw_git_version = GITVERSION;
static const char * fw_git_date = GITDATE;

// connection status
enum CONNECTION_STATUS {
	IDLE = 0,
	CONNECTING = 1,
	CONNECTED = 2,
	ERR = 3
};

// main forward declarations
void scanNetworks();
void setupRGBWW();
void startAp();
void stopAp();
void restart();
void connectOk();
void connectFail();

// forward declaration for global vars
static Timer systemTimer;
static BssList networks;
static bool scanning = false;
static RGBWWLed rgbwwctrl;
static ApplicationSettingsStorage cfg;
static ActiveColorStorage stored_color;

#endif // APPLICATION_H_

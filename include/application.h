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
void stopAPandReset();

//TODO: move application into its own class and call class from init
//TODO: move webserver into its own class

// forward declaration for global vars
extern Timer systemTimer;
extern BssList networks;
extern bool scanning;
extern RGBWWLed rgbwwctrl;
extern ApplicationOTA ota;
extern ApplicationSettings cfg;
extern ColorStorage stored_color;

#endif // APPLICATION_H_

#include <RGBWWCtrl.h>

const IPAddress ApIP = IPAddress("192.168.4.1");
DNSServer* dnsServer = NULL;


Timer ledTimer;


void restart() {
	System.restart();
}

// Will be called when system initialization was completed
void startServices()
{
	startWebServer();
	//start TCP
	//start UDP
	//start mqtt client
}


void stopAp() {
	debugf("Stopping accesspoint and DNS server");
	if (WifiAccessPoint.isEnabled()) {
		WifiAccessPoint.enable(false);
	}
	if (dnsServer!= NULL) {
		dnsServer->stop();
		delete dnsServer;
	}

}


void startAp() {
	byte DNS_PORT = 53;
	debugf("Starting accesspoint");
	WifiAccessPoint.setIP(ApIP);
	WifiAccessPoint.enable(true);

	if (cfg.network.ap.secured) {
		WifiAccessPoint.config(cfg.network.ap.ssid, cfg.network.ap.password, AUTH_WPA2_PSK);
	} else {
		WifiAccessPoint.config(cfg.network.ap.ssid, "", AUTH_OPEN);
	}

	debugf("Starting dns service");
	if (dnsServer == NULL) {
		dnsServer = new DNSServer;
		dnsServer->start(DNS_PORT, "*", ApIP);
	}
}


void networkScanCompleted(bool succeeded, BssList list)
{
	if (succeeded)
	{
		networks.clear();
		for (int i = 0; i < list.count(); i++)
			if (!list[i].hidden && list[i].ssid.length() > 0)
				networks.add(list[i]);
	}
	networks.sort([](const BssInfo& a, const BssInfo& b){ return b.rssi - a.rssi; } );
	scanning = false;
}


void scanNetworks() {
	scanning = true;
	WifiStation.startScan(networkScanCompleted);
}


void connectStartOk() {
	if(WifiAccessPoint.isEnabled()) {
		systemTimer.initializeMs(1000, stopAp).startOnce();
	}
};


void connectStartFail() {
	//couldn`t connect during initial startup
	startAp();
};


void connectOk(){
	//successfully connected
	cfg.network.connection.ssid = WifiStation.getSSID();
	cfg.network.connection.password = WifiStation.getPassword();
	cfg.save();
}

void connectFail() {
	//fail when trying to connect to new AP
}

void showLed() {
	//Main Loop for LEDs
	rgbwwctrl.show();
}

void saveRGBWW(RGBWWLed* rgbwwctrl) {
	debugf("callback from RGBWW lib");
	HSVK c = rgbwwctrl->getCurrentColor();
	stored_color.color.h = c.h;
	stored_color.color.s = c.s;
	stored_color.color.v = c.v;
	stored_color.color.k = c.k;
	stored_color.save();
}

void setupRGBWW() {
	rgbwwctrl.init(REDPIN, GREENPIN, BLUEPIN, WWPIN, CWPIN);
	rgbwwctrl.setAnimationCallback(saveRGBWW);
    rgbwwctrl.colorutils.setBrightnessCorrection(cfg.color.brightness.red,
    		cfg.color.brightness.green,
			cfg.color.brightness.blue,
			cfg.color.brightness.ww,
			cfg.color.brightness.cw);
    rgbwwctrl.colorutils.setHSVcorrection(cfg.color.hsv.red,
    		cfg.color.hsv.yellow,
			cfg.color.hsv.green,
			cfg.color.hsv.cyan,
			cfg.color.hsv.blue,
			cfg.color.hsv.magenta);
    rgbwwctrl.colorutils.setColorMode((RGBWW_COLORMODE)cfg.color.outputmode);
    rgbwwctrl.colorutils.setHSVmodel((RGBWW_HSVMODEL)cfg.color.hsv.model);

}





void init()
{

	//Serial.println("RGBWW Controller %s", GITVERSION);
	// Mount file system, in order to work with files
	spiffs_mount_manual(RBOOT_SPIFFS_0 + 0x40200000, SPIFF_SIZE);

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true);
	//Serial.commandProcessing(true);

	//disable wifi sleep
	wifi_set_sleep_type(NONE_SLEEP_T);

	//load settings
	cfg.load(true);

	//setup everything led related
    setupRGBWW();

    //load last color
    stored_color.load();
    HSVK c = HSVK(stored_color.color.h, stored_color.color.s, stored_color.color.v, stored_color.color.k);
    rgbwwctrl.setOutput(c);

    //start led loop
    ledTimer.initializeMs(20, showLed).start();

    //setup networking related
    //don`t enable/disable again to save eeprom cycles
    if(!WifiStation.isEnabled()) {
    	WifiStation.enable(true);
    }
    if(WifiAccessPoint.isEnabled()) {
    	WifiAccessPoint.enable(false);
    }
	if (cfg.exist())
	{
		debugf("Wifistation SSID %s -  PASS %s", WifiStation.getSSID().c_str(), WifiStation.getPassword().c_str());
		debugf("Saved SSID %s -  PASS %s", cfg.network.connection.ssid.c_str(), cfg.network.connection.password.c_str());
		if (cfg.network.connection.ssid.length() > 0 || WifiStation.getSSID().length() > 0) {
			if (WifiStation.getPassword() != cfg.network.connection.password || WifiStation.getSSID() != cfg.network.connection.ssid) {
				WifiStation.config(cfg.network.connection.ssid, cfg.network.connection.password);
			}
			if (!cfg.network.connection.dhcp && !cfg.network.connection.ip.isNull())
			{

				//don`t disable if already enabled - save eeprom cycles
				debugf("setting static ip");
				if(WifiStation.isEnabledDHCP()) {
					WifiStation.enableDHCP(false);
				}
				//only set if changed - save eeprom cycles
				if ( !(WifiStation.getIP() == cfg.network.connection.ip) ||
						!(WifiStation.getNetworkGateway() == cfg.network.connection.gateway) ||
						!(WifiStation.getNetworkMask() == cfg.network.connection.netmask)) {
					debugf("updating ip configuration");
					WifiStation.setIP(cfg.network.connection.ip, cfg.network.connection.netmask, cfg.network.connection.gateway);
				}
			} else {
				debugf("enabling dhcp");
				//don`t enable again if already enabled - save eeprom cycles
				if(!WifiStation.isEnabledDHCP()) {
					WifiStation.enableDHCP(true);
				}
			}
			WifiStation.waitConnection(connectStartOk, 15, connectStartFail);
		} else {
			startAp();
			scanNetworks();
		}
	} else {

		debugf("initial run - setting up accesspoint details");
		cfg.network.connection.mdnshostname = "rgbww-" + String(system_get_chip_id());
		cfg.network.ap.ssid = "RGBWW-" + String(system_get_chip_id());

		debugf("setting version details");
		cfg.save();
		startAp();
		scanNetworks();
	}



	// Run Services on system ready
	System.onReady(startServices);

}

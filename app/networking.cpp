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

AppWIFI::AppWIFI() {
	_ApIP = IPAddress(String(DEFAULT_AP_IP));
	_client_err_msg = "";
	_con_ctr = 0;
	_scanning = false;
	_dns_active = false;
	_new_connection = false;
	_client_status = CONNECTION_STATUS::IDLE;
}


BssList AppWIFI::getAvailableNetworks() {
	return _networks;
}


void AppWIFI::scan() {
	_scanning = true;
	WifiStation.startScan(ScanCompletedDelegate(&AppWIFI::scanCompleted, this));
}


bool AppWIFI::isScanning() {
	return _scanning;
}


CONNECTION_STATUS AppWIFI::get_con_status(){
	return _client_status;
};


String AppWIFI::get_con_err_msg() {
	return _client_err_msg;

};


void AppWIFI::scanCompleted(bool succeeded, BssList list) {
	debugapp("AppWIFI::scanCompleted");
	if (succeeded)
	{
		_networks.clear();
		for (int i = 0; i < list.count(); i++)
			if (!list[i].hidden && list[i].ssid.length() > 0)
				_networks.add(list[i]);
	}
	_networks.sort([](const BssInfo& a, const BssInfo& b){ return b.rssi - a.rssi; } );
	_scanning = false;
}


void AppWIFI::forget_wifi() {
	debugapp("AppWIFI::forget_wifi");
	WifiStation.config("", "");
	WifiStation.disconnect();
	_client_status = CONNECTION_STATUS::IDLE;
}


void AppWIFI::init() {
	wifi_set_sleep_type(NONE_SLEEP_T);

    //don`t enable/disable again to save eeprom cycles
    if(!WifiStation.isEnabled()) {
		debugapp("AppWIFI::init enable WifiStation");
    	WifiStation.enable(true, true);
    }
    if(WifiAccessPoint.isEnabled()) {
		debugapp("AppWIFI::init WifiAccessPoint disablw");
    	WifiAccessPoint.enable(false, true);
    }
    _con_ctr = 0;
    if(app.isFirstRun()) {
		debugapp("AppWIFI::init initial run - setting up AP");
		app.cfg.network.connection.mdnshostname = String(DEFAULT_AP_SSIDPREFIX) + String(system_get_chip_id());
		app.cfg.network.ap.ssid = String(DEFAULT_AP_SSIDPREFIX) + String(system_get_chip_id());
		app.cfg.save();
		WifiAccessPoint.setIP(_ApIP);
    }

	WifiEvents.onStationDisconnect(onStationDisconnectDelegate(&AppWIFI::_STADisconnect, this));
	WifiEvents.onStationConnect(onStationConnectDelegate(&AppWIFI::_STAConnected, this));
	WifiEvents.onStationGotIP(onStationGotIPDelegate(&AppWIFI::_STAGotIP, this));
	if(WifiStation.getSSID() == "") {
		debugapp("AppWIFI::init no AP to connect to - start own AP");
		// No wifi to connect to - initialize AP
		startAp();
		// already scan for avaialble networks to speedup things later
		scan();
	} else {
		//configure WifiClient
		if (!app.cfg.network.connection.dhcp && !app.cfg.network.connection.ip.isNull())
		{
			debugapp("AppWIFI::init setting static ip");
			if(WifiStation.isEnabledDHCP()) {
				debugapp("AppWIFI::init disabled dhcp");
				WifiStation.enableDHCP(false);
			}
			if ( !(WifiStation.getIP() == app.cfg.network.connection.ip) ||
					!(WifiStation.getNetworkGateway() == app.cfg.network.connection.gateway) ||
					!(WifiStation.getNetworkMask() == app.cfg.network.connection.netmask)) {
				debugapp("AppWIFI::init updating ip configuration");
				WifiStation.setIP(app.cfg.network.connection.ip, app.cfg.network.connection.netmask, app.cfg.network.connection.gateway);
			}
		} else {
			debugapp("AppWIFI::init dhcp");
			if(!WifiStation.isEnabledDHCP()) {
				debugapp("AppWIFI::init enabling dhcp");
				WifiStation.enableDHCP(true);
			}
		}
	}

}


void AppWIFI::connect(String ssid, bool new_con /* = false */) {
	connect(ssid, "", new_con);
}


void AppWIFI::connect(String ssid, String pass, bool new_con /* = false */){
	debugapp("AppWIFI::connect ssid %s newcon %d", ssid.c_str(), new_con);
	_con_ctr = 0;
	_new_connection = true;
	_client_status = CONNECTION_STATUS::CONNECTING;

	WifiStation.config(ssid, pass);
	WifiStation.connect();
}



void AppWIFI::_STADisconnect(String ssid, uint8_t ssid_len, uint8_t bssid[6], uint8_t reason) {
	debugapp("AppWIFI::_STADisconnect reason - %i - counter %i", reason, _con_ctr);
	if(_con_ctr >= DEFAULT_CONNECTION_RETRIES || WifiStation.getConnectionStatus() == eSCS_WrongPassword) {
		_client_status = CONNECTION_STATUS::ERR;
		_client_err_msg = WifiStation.getConnectionStatusName();
		debugapp("AppWIFI::_STADisconnect err %s", _client_err_msg.c_str());
		WifiStation.disconnect();
		if(_new_connection) {
			WifiStation.config("", "");
		} else {
			scan();
			startAp();
		}
		_con_ctr = 0;
		return;
	}
	_con_ctr++;
}


void AppWIFI::_STAConnected(String ssid, uint8_t ssid_len, uint8_t bssid[6], uint8_t reason) {
	debugapp("AppWIFI::_STAConnected reason - %i", reason);

}


void AppWIFI::_STAGotIP(IPAddress ip, IPAddress mask, IPAddress gateway) {
	debugapp("AppWIFI::_STAGotIP");
	_con_ctr = 0;
	_client_status = CONNECTION_STATUS::CONNECTED;
	stopAp(90000); //disabling ap after 90seconds
}


void AppWIFI::stopAp(int delay) {
	debugapp("AppWIFI::stopAp delay %i", delay);
	if(WifiAccessPoint.isEnabled()) {
		_timer.initializeMs(delay, TimerDelegate(&AppWIFI::stopAp, this)).startOnce();
	}
}


void AppWIFI::stopAp(){

	debugapp("AppWIFI::stopAp");
	Serial.println("Disabling AP and DNS server");
	_timer.stop();
	if (WifiAccessPoint.isEnabled()) {
		debugapp("AppWIFI::stopAp WifiAP disable");
		WifiAccessPoint.enable(false, false);
	}
	if(_dns_active) {
		debugapp("AppWIFI::stopAp DNS disable");
		_dns.close();
	}
}


void AppWIFI::startAp(){
	byte DNS_PORT = 53;
	debugapp("AppWIFI::startAp");
	Serial.println("Enabling AP and DNS server");
	if (!WifiAccessPoint.isEnabled()) {
		debugapp("AppWIFI:: WifiAP enable");
		WifiAccessPoint.enable(true, false);
		if (app.cfg.network.ap.secured) {
			WifiAccessPoint.config(app.cfg.network.ap.ssid, app.cfg.network.ap.password, AUTH_WPA2_PSK);
		} else {
			WifiAccessPoint.config(app.cfg.network.ap.ssid, "", AUTH_OPEN);
		}
	}
	if(!_dns_active) {
		debugapp("AppWIFI:: DNS enable");
		_dns_active = true;
		_dns.setErrorReplyCode(DNSReplyCode::NoError);
		_dns.start(DNS_PORT, "*", _ApIP);
	}
}

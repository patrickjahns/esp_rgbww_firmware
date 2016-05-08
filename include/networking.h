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
#ifndef APP_NETWORKING_H_
#define APP_NETWORKING_H_



enum CONNECTION_STATUS {
	IDLE = 0,
	CONNECTING = 1,
	CONNECTED = 2,
	ERR = 3
};

class AppWIFI {

public:
	AppWIFI();
	virtual ~AppWIFI(){};

	void init();

	void connect(String ssid, String pass, bool new_con = false);
	void connect(String ssid, bool new_con = false);
	inline CONNECTION_STATUS get_con_status() { return _client_status; };
	inline String get_con_err_msg() { return _client_err_msg; };

	void startAp();
	void stopAp();
	void stopAp(int delay);

	void scan();
	inline bool isScanning() { return _scanning; };
	BssList getAvailableNetworks();

	void forget_wifi();

private:
	int _con_ctr;
	bool _scanning;
	bool _dns_active;
	bool _new_connection;
	String	_client_err_msg;
	String _tmp_ssid;
	String _tmp_password;
	Timer _timer;
	BssList _networks;
	DNSServer _dns;
	IPAddress _ApIP;

	CONNECTION_STATUS _client_status;

private:
	void _STADisconnect(String ssid, uint8_t ssid_len, uint8_t bssid[6], uint8_t reason);
	void _STAConnected(String ssid, uint8_t ssid_len, uint8_t bssid[6], uint8_t reason);
	void _STAGotIP(IPAddress ip, IPAddress mask, IPAddress gateway);
	void scanCompleted(bool succeeded, BssList list);
};

#endif //APP_NETWORKING_H_

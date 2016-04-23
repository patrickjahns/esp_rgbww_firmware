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
#include <Services/WebHelpers/base64.h>



ApplicationWebserver::ApplicationWebserver() {
	_running = false;
}

void ApplicationWebserver::init() {
	setDefaultHandler(HttpPathDelegate(&ApplicationWebserver::onFile, this));
	enableHeaderProcessing("Authorization");
	addPath("/", HttpPathDelegate(&ApplicationWebserver::onIndex, this));
	addPath("/webapp", HttpPathDelegate(&ApplicationWebserver::onWebapp, this));
	addPath("/config", HttpPathDelegate(&ApplicationWebserver::onConfig, this));
	addPath("/info", HttpPathDelegate(&ApplicationWebserver::onInfo, this));
	addPath("/color", HttpPathDelegate(&ApplicationWebserver::onColor, this));
	addPath("/animation", HttpPathDelegate(&ApplicationWebserver::onAnimation, this));
	addPath("/networks", HttpPathDelegate(&ApplicationWebserver::onNetworks, this));
	addPath("/scan_networks", HttpPathDelegate(&ApplicationWebserver::onScanNetworks, this));
	addPath("/system", HttpPathDelegate(&ApplicationWebserver::onSystemReq, this));
	addPath("/update", HttpPathDelegate(&ApplicationWebserver::onUpdate, this));
	addPath("/connect", HttpPathDelegate(&ApplicationWebserver::onConnect, this));
	addPath("/generate_204", HttpPathDelegate(&ApplicationWebserver::generate204, this));
	addPath("/ping", HttpPathDelegate(&ApplicationWebserver::onPing, this));
	_init = true;
}

void ApplicationWebserver::start()
{
	if (_init == false) {
		init();
	}
	listen(80);
	_running = true;
}

void ApplicationWebserver::stop() {
	close();
	_running = false;
}

bool ApplicationWebserver::isRunning() {
	return _running;
}


bool ICACHE_FLASH_ATTR ApplicationWebserver::authenticated(HttpRequest &request, HttpResponse &response){
	if (!app.cfg.general.api_secured) return true;
    String userPass=request.getHeader("Authorization");
    int headerLength=userPass.length()-6; // header in form of: "Basic MTIzNDU2OmFiY2RlZmc="so the 6 is to get to beginning of 64 encoded string
    if(headerLength>50){
        return false;
    }

    unsigned char decbuf[headerLength]; // buffer for the decoded string
    int outlen = base64_decode(headerLength,userPass.c_str()+6, headerLength, decbuf);
    decbuf[outlen] = 0;
    userPass = String((char*)decbuf);
    if(userPass.endsWith(app.cfg.general.api_password)){
        return true;
    }

    response.authorizationRequired() ;
    response.setHeader("WWW-Authenticate","Basic realm=\"RGBWW Server\"");
    response.setHeader("401 wrong credentials", "wrong credentials");
    response.setHeader("Connection","close");
    return false;

}

String ApplicationWebserver::getApiCodeMsg(API_CODES code) {
	switch(code) {
		case API_CODES::API_MISSING_PARAM :
			return String("missing param");
		case API_CODES::API_UNAUTHORIZED:
			return String("authorization required");
		default:
			return String("bad request");
	}
}

void ApplicationWebserver::sendApiResponse(HttpResponse &response, JsonObjectStream* stream, int code /* = 200 */) {
	response.setAllowCrossDomainOrigin("*");
	if(code != 200) {
		response.badRequest();
	}
	response.sendJsonObject(stream);
}

void ApplicationWebserver::sendApiCode(HttpResponse &response, API_CODES code, String msg /* = "" */) {
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();
	if (msg == "") {
		msg = getApiCodeMsg(code);
	}
	if (code == API_CODES::API_SUCCESS) {
		json["success"] = true;
		sendApiResponse(response, stream, 200);
	} else {
		json["error"] = msg;
		sendApiResponse(response, stream, 400);
	}

}

void ApplicationWebserver::onFile(HttpRequest &request, HttpResponse &response)
{
	String file = request.getPath();
	if (file[0] == '/')	file = file.substring(1);
	if (file[0] == '.') {
		response.forbidden();
		return;
	}

	if (!fileExist(file) && !fileExist(file+".gz") && WifiAccessPoint.isEnabled()) {
		//if accesspoint is active and we couldn`t find the file - redirect to index
		debugapp("ApplicationWebserver::onFile redirecting");
		response.redirect("http://"+WifiAccessPoint.getIP().toString()+"/webapp");
	} else {
		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile(file);
	}

}

void ApplicationWebserver::onIndex(HttpRequest &request, HttpResponse &response)
{
	if(WifiAccessPoint.isEnabled()) {
		response.redirect("http://"+WifiAccessPoint.getIP().toString()+"/webapp");
	} else {
		response.redirect("http://"+WifiStation.getIP().toString()+"/webapp");
	}

}

void ApplicationWebserver::onWebapp(HttpRequest &request, HttpResponse &response)
{
	if(!authenticated(request, response)) return;
	//don`t allow other requests than get
	if(request.getRequestMethod() != RequestMethod::GET) {
		response.badRequest();
		return;
	}
	if (!WifiStation.isConnected() ) {
		// not yet connected - serve initial settings page
		response.sendFile("init.html");
	} else {
		// we are connected to ap - serve normal settings page
		response.sendFile("index.html");
	}
}


void ApplicationWebserver::onConfig(HttpRequest &request, HttpResponse &response)
{

	if(!authenticated(request, response)) return;

	if(request.getRequestMethod() != RequestMethod::POST && request.getRequestMethod() != RequestMethod::GET) {
		sendApiCode(response, API_CODES::API_BAD_REQUEST);
		return;
	}

	if (request.getRequestMethod() == RequestMethod::POST) {
		if (request.getBody() == NULL) {

			sendApiCode(response, API_CODES::API_BAD_REQUEST);
			return;

		}

		bool error = false;
		String error_msg = getApiCodeMsg(API_CODES::API_BAD_REQUEST);
		DynamicJsonBuffer jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject(request.getBody());

		// remove comment for debugging
		//root.prettyPrintTo(Serial);


		bool ip_updated = false;
		bool color_updated = false;
		bool ap_updated = false;
		if (!root.success()) {
			sendApiCode(response, API_CODES::API_BAD_REQUEST);
			return;
		}
		if (root["network"].success()) {

			if(root["network"]["connection"].success()) {

				if(root["network"]["connection"]["dhcp"].success()) {

					if(root["network"]["connection"]["dhcp"] != app.cfg.network.connection.dhcp) {
						app.cfg.network.connection.dhcp = root["network"]["connection"]["dhcp"];
						ip_updated = true;
					}
				}
				if (!app.cfg.network.connection.dhcp) {
					//only change if dhcp is off - otherwise ignore
					IPAddress ip, netmask, gateway;
					if(root["network"]["connection"]["ip"].success()) {
						ip = root["network"]["connection"]["ip"].asString();
						if(!(ip == app.cfg.network.connection.ip)) {
							app.cfg.network.connection.ip = ip;
							ip_updated = true;
						}
					} else {
						error = true;
						error_msg = "missing ip";
					}
					if(root["network"]["connection"]["netmask"].success()) {
						netmask = root["network"]["connection"]["netmask"].asString();
						if(!(netmask == app.cfg.network.connection.netmask)) {
							app.cfg.network.connection.netmask = netmask;
							ip_updated = true;
						}
					} else {
						error = true;
						error_msg = "missing netmask";
					}
					if(root["network"]["connection"]["gateway"].success()) {
						gateway = root["network"]["connection"]["gateway"].asString();
						if(!(gateway == app.cfg.network.connection.gateway)) {
							app.cfg.network.connection.gateway = gateway;
							ip_updated = true;
						}
					} else {
						error = true;
						error_msg = "missing gateway";
					}

				}

			}
			if(root["network"]["ap"].success()) {


				if(root["network"]["ap"]["ssid"].success()) {
					if (root["network"]["ap"]["ssid"] != app.cfg.network.ap.ssid) {
						app.cfg.network.ap.ssid = root["network"]["ap"]["ssid"].asString();
						ap_updated = true;
					}
				}
				if(root["network"]["ap"]["secured"].success()) {
						if (root["network"]["ap"]["secured"].as<bool>()){
							if(root["network"]["ap"]["password"].success()) {
								if (root["network"]["ap"]["password"] != app.cfg.network.ap.password) {
									app.cfg.network.ap.secured = root["network"]["ap"]["secured"];
									app.cfg.network.ap.password = root["network"]["ap"]["password"].asString();
									ap_updated = true;
								}
							} else {
								error = true;
								error_msg = "missing password for securing ap";
							}
						} else if (root["network"]["ap"]["secured"] != app.cfg.network.ap.secured)
						{
							root["network"]["ap"]["secured"] == app.cfg.network.ap.secured;
							ap_updated = true;
						}
				}

			}
			if(root["network"]["mqtt"].success()) {
				//TODO: what to do if changed?
				if(root["network"]["mqtt"]["enabled"].success()) {
					if (root["network"]["mqtt"]["enabled"] != app.cfg.network.mqtt.enabled) {
						app.cfg.network.mqtt.enabled = root["network"]["mqtt"]["enabled"];
					}
				}
				if(root["network"]["mqtt"]["server"].success()) {
					if (root["network"]["mqtt"]["server"] != app.cfg.network.mqtt.server) {
						app.cfg.network.mqtt.server = root["network"]["mqtt"]["server"].asString();
					}
				}
				if(root["network"]["mqtt"]["port"].success()) {
					if (root["network"]["mqtt"]["port"] != app.cfg.network.mqtt.port) {
						app.cfg.network.mqtt.port = root["network"]["mqtt"]["port"];
					}
				}
				if(root["network"]["mqtt"]["username"].success()) {
					if (root["network"]["mqtt"]["username"] != app.cfg.network.mqtt.username) {
						app.cfg.network.mqtt.username = root["network"]["mqtt"]["username"].asString();
					}
				}
				if(root["network"]["mqtt"]["password"].success()) {
					if (root["network"]["mqtt"]["password"] != app.cfg.network.mqtt.password) {
						app.cfg.network.mqtt.password = root["network"]["mqtt"]["password"].asString();
					}
				}
			}
			//disabled until functionality is implemented
			/*
			if(root["network"]["udpserver"].success()) {
				//TODO: what to do if changed?
				if(root["network"]["udpserver"]["enabled"].success()) {
					if (root["network"]["udpserver"]["enabled"] != app.cfg.network.udpserver.enabled) {
						app.cfg.network.udpserver.enabled = root["network"]["udpserver"]["enabled"];
					}
				}
				if (root["network"]["udpserver"]["port"].success()) {
					if (root["network"]["udpserver"]["port"] != app.cfg.network.udpserver.port) {
						app.cfg.network.udpserver.port = root["network"]["udpserver"]["port"];
					}
				}
			}
			if(root["network"]["tcpserver"].success()) {
				//TODO: what to do if changed?
				if(root["network"]["tcpserver"]["enabled"].success()) {
					if (root["network"]["tcpserver"]["enabled"] != app.cfg.network.tcpserver.enabled) {
						app.cfg.network.tcpserver.enabled = root["network"]["tcpserver"]["enabled"];
					}
				}
				if (root["network"]["tcpserver"]["port"].success()) {
					if (root["network"]["tcpserver"]["port"] != app.cfg.network.tcpserver.port) {
						app.cfg.network.tcpserver.port = root["network"]["tcpserver"]["port"];
					}
				}
			}
			*/
		}

		if (root["color"].success())
		{

			if (root["color"]["hsv"].success()) {
				if (root["color"]["hsv"]["model"].success()){
					if (root["color"]["hsv"]["model"] != app.cfg.color.hsv.model) {
						app.cfg.color.hsv.model = root["color"]["hsv"]["model"].as<int>();
						color_updated = true;
					}
				}
				if (root["color"]["hsv"]["red"].success())
				{
					if (root["color"]["hsv"]["red"].as<float>() != app.cfg.color.hsv.red)
					{
						app.cfg.color.hsv.red = root["color"]["hsv"]["red"].as<float>();
						color_updated = true;
					}
				}
				if (root["color"]["hsv"]["yellow"].success())
				{
					if (root["color"]["hsv"]["yellow"].as<float>() != app.cfg.color.hsv.yellow)
					{
						app.cfg.color.hsv.yellow = root["color"]["hsv"]["yellow"].as<float>();
						color_updated = true;
					}
				}
				if (root["color"]["hsv"]["green"].success())
				{
					if (root["color"]["hsv"]["green"].as<float>() != app.cfg.color.hsv.green)
					{
						app.cfg.color.hsv.green = root["color"]["hsv"]["green"].as<float>();
						color_updated = true;
					}
				}
				if (root["color"]["hsv"]["cyan"].success())
				{
					if (root["color"]["hsv"]["cyan"].as<float>() != app.cfg.color.hsv.cyan)
					{
						app.cfg.color.hsv.cyan = root["color"]["hsv"]["cyan"].as<float>();
						color_updated = true;
					}
				}
				if (root["color"]["hsv"]["blue"].success())
				{
					if (root["color"]["hsv"]["blue"].as<float>() != app.cfg.color.hsv.blue)
					{
						app.cfg.color.hsv.blue = root["color"]["hsv"]["blue"].as<float>();
						color_updated = true;
					}
				}
				if (root["color"]["hsv"]["magenta"].success())
				{
					if (root["color"]["hsv"]["magenta"].as<float>() != app.cfg.color.hsv.magenta)
					{
						app.cfg.color.hsv.magenta = root["color"]["hsv"]["magenta"].as<float>();
						color_updated = true;
					}
				}
			}
			if (root["color"]["outputmode"].success()) {
				if(root["color"]["outputmode"] != app.cfg.color.outputmode) {
					app.cfg.color.outputmode = root["color"]["outputmode"].as<int>();
					color_updated = true;
				}
			}
			if (root["color"]["brightness"].success()) {

				if (root["color"]["brightness"]["red"].success()) {
					if (root["color"]["brightness"]["red"].as<int>() != app.cfg.color.brightness.red) {
						app.cfg.color.brightness.red = root["color"]["brightness"]["red"].as<int>();
						color_updated = true;
					}
				}
				if (root["color"]["brightness"]["green"].success()) {
					if (root["color"]["brightness"]["green"].as<int>() != app.cfg.color.brightness.green) {
						app.cfg.color.brightness.green = root["color"]["brightness"]["green"].as<int>();
						color_updated = true;
					}
				}
				if (root["color"]["brightness"]["blue"].success()) {
					if (root["color"]["brightness"]["blue"].as<int>() != app.cfg.color.brightness.blue) {
						app.cfg.color.brightness.blue = root["color"]["brightness"]["blue"].as<int>();
						color_updated = true;
					}
				}
				if (root["color"]["brightness"]["ww"].success()) {
					if (root["color"]["brightness"]["ww"].as<int>() != app.cfg.color.brightness.ww) {
						app.cfg.color.brightness.ww = root["color"]["brightness"]["ww"].as<int>();
						color_updated = true;
					}
				}
				if (root["color"]["brightness"]["cw"].success()) {
					if (root["color"]["brightness"]["cw"].as<int>() != app.cfg.color.brightness.cw) {
						app.cfg.color.brightness.cw = root["color"]["brightness"]["cw"].as<int>();
						color_updated = true;
					}
				}
			}
			if (root["color"]["colortemp"].success()) {

				if (root["color"]["colortemp"]["ww"].success()) {
					if (root["color"]["colortemp"]["cw"].as<int>() != app.cfg.color.colortemp.ww) {
						app.cfg.color.colortemp.ww = root["color"]["colortemp"]["ww"].as<int>();
						color_updated = true;
					}
				}
				if (root["color"]["colortemp"]["cw"].success()) {
					if (root["color"]["colortemp"]["cw"].as<int>() != app.cfg.color.colortemp.cw) {
						app.cfg.color.colortemp.cw = root["color"]["colortemp"]["cw"].as<int>();
						color_updated = true;
					}
				}
			}
		}

		if(root["security"].success())
		{
			if(root["security"]["api_secured"].success()){
				if (root["security"]["api_secured"].as<bool>()) {
					if(root["security"]["api_password"].success()){
						if(root["security"]["api_password"] != app.cfg.general.api_password) {
							app.cfg.general.api_secured = root["security"]["api_secured"];
							app.cfg.general.api_password = root["security"]["api_password"].asString();
						}

					} else {
						error = true;
						error_msg = "missing password to secure settings";
					}
				}
			}
		}


		// update and save settings if we haven`t received any error until now
		if (!error) {
			if (ip_updated) {
				if (root["restart"].success()) {
					if (root["restart"] == true) {
						debugapp("ApplicationWebserver::onConfig ip settings changed - rebooting");
						app.delayedCMD("restart", 3000); // wait 3s to first send response
						//json["data"] = "restart";

					}
				}
			};
			if(ap_updated) {
				if (root["restart"].success()) {
					if (root["restart"] == true && WifiAccessPoint.isEnabled()) {
						debugapp("ApplicationWebserver::onConfig wifiap settings changed - rebooting");
						app.delayedCMD("restart", 3000); // wait 3s to first send response
						//json["data"] = "restart";

					}
				}
			}
			if (color_updated) {
				debugapp("ApplicationWebserver::onConfig color settings changed - refreshing");

				//refresh settings
				app.rgbwwctrl.setup();

				//refresh current output
				app.rgbwwctrl.refresh();

			}
			app.cfg.save();
			sendApiCode(response, API_CODES::API_SUCCESS);
		} else {
			sendApiCode(response, API_CODES::API_MISSING_PARAM, error_msg);
		}

	} else {
		JsonObjectStream* stream = new JsonObjectStream();
		JsonObject& json = stream->getRoot();
		// returning settings
		JsonObject& net = json.createNestedObject("network");
		JsonObject& con = net.createNestedObject("connection");
		con["dhcp"] = WifiStation.isEnabledDHCP();

		//con["ip"] = WifiStation.getIP().toString();
		//con["netmask"] = WifiStation.getNetworkMask().toString();
		//con["gateway"] = WifiStation.getNetworkGateway().toString();

		con["ip"] = app.cfg.network.connection.ip.toString();
		con["netmask"] = app.cfg.network.connection.netmask.toString();
		con["gateway"] = app.cfg.network.connection.gateway.toString();

		JsonObject& ap = net.createNestedObject("ap");
		ap["secured"] = app.cfg.network.ap.secured;
		ap["ssid"] = app.cfg.network.ap.ssid.c_str();

		JsonObject& mqtt = net.createNestedObject("mqtt");
		mqtt["enabled"] = app.cfg.network.mqtt.enabled;
		mqtt["server"] = app.cfg.network.mqtt.server.c_str();
		mqtt["port"] = app.cfg.network.mqtt.port;
		mqtt["username"] = app.cfg.network.mqtt.username.c_str();

		//mqtt["password"] = app.cfg.network.mqtt.password.c_str();

		// disabled until functionality is implemented
		//JsonObject& udp = net.createNestedObject("udpserver");
		//udp["enabled"] = app.cfg.network.udpserver.enabled;
		//udp["port"] = app.cfg.network.udpserver.port;

		//JsonObject& tcp = net.createNestedObject("tcpserver");
		//tcp["enabled"] = app.cfg.network.tcpserver.enabled;
		//tcp["port"] = app.cfg.network.tcpserver.port;

		JsonObject& color = json.createNestedObject("color");
		color["outputmode"] = app.cfg.color.outputmode;

		JsonObject& hsv = color.createNestedObject("hsv");
		hsv["model"] = app.cfg.color.hsv.model;

		hsv["red"] = app.cfg.color.hsv.red;
		hsv["yellow"] = app.cfg.color.hsv.yellow;
		hsv["green"] = app.cfg.color.hsv.green;
		hsv["cyan"] = app.cfg.color.hsv.cyan;
		hsv["blue"] = app.cfg.color.hsv.blue;
		hsv["magenta"] = app.cfg.color.hsv.magenta;

		JsonObject& brighntess = color.createNestedObject("brightness");
		brighntess["red"] = app.cfg.color.brightness.red;
		brighntess["green"] = app.cfg.color.brightness.green;
		brighntess["blue"] = app.cfg.color.brightness.blue;
		brighntess["ww"] = app.cfg.color.brightness.ww;
		brighntess["cw"] = app.cfg.color.brightness.cw;

		JsonObject& ctmp = color.createNestedObject("colortemp");
		ctmp["ww"] = app.cfg.color.colortemp.ww;
		ctmp["cw"] = app.cfg.color.colortemp.cw;

		JsonObject& s = json.createNestedObject("security");
		s["api_secured"] = app.cfg.general.api_secured;
		sendApiResponse(response, stream);
	}
}

void ApplicationWebserver::onInfo(HttpRequest &request, HttpResponse &response)
{
	if(!authenticated(request, response)) return;
	if(request.getRequestMethod() != RequestMethod::GET) {
		sendApiCode(response, API_CODES::API_BAD_REQUEST);
		return;
	}
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& data = stream->getRoot();
	data["deviceid"] = String(system_get_chip_id());
	data["firmware"] = fw_version;
	data["git_version"] = fw_git_version;
	data["git_date"] = fw_git_date;
	data["config_version"] = app.cfg.configversion;
	data["sming"] = SMING_VERSION;
	JsonObject& rgbww = data.createNestedObject("rgbww");
	rgbww["version"] = RGBWW_VERSION;
	rgbww["queuesize"] = RGBWW_ANIMATIONQSIZE;
	JsonObject& con = data.createNestedObject("connection");
	con["connected"] = WifiStation.isConnected();
	con["ssid"] = WifiStation.getSSID ();
	con["dhcp"] = WifiStation.isEnabledDHCP();
	con["ip"] = WifiStation.getIP().toString();
	con["netmask"] = WifiStation.getNetworkMask().toString();
	con["gateway"] = WifiStation.getNetworkGateway().toString();
	con["mac"] = WifiStation.getMAC();
	//con["mdnshostname"] = app.cfg.network.connection.mdnshostname.c_str();
	sendApiResponse(response, stream);
}

void ApplicationWebserver::onColor(HttpRequest &request, HttpResponse &response)
{
	if(!authenticated(request, response)) return;
	if(request.getRequestMethod() != RequestMethod::POST && request.getRequestMethod() != RequestMethod::GET) {
		sendApiCode(response, API_CODES::API_BAD_REQUEST);
		return;
	}


	bool error = false;
	if (request.getRequestMethod() == RequestMethod::POST)
	{
		String body = request.getBody();
		if ( body == NULL || body.length() > 128)
		{
			sendApiCode(response, API_CODES::API_BAD_REQUEST);
			return;

		} else {

			DynamicJsonBuffer jsonBuffer;
			JsonObject& root = jsonBuffer.parseObject(body);
			//root.prettyPrintTo(Serial);

			if(root["kelvin"].success()) {
				int t, k = 0;
				bool q = false;


				k = root["kelvin"].as<int>();
				if(root["t"].success()) {
					t = root["t"].as<int>();
				}
				if(root["q"].success()) {
					q = root["q"];
				}
				//TODO: hand to rgbctrl
			} else  if (root["hsv"].success()) {
				if(root["hsv"]["h"].success() && root["hsv"]["s"].success() && root["hsv"]["v"].success()) {
					float h, s, v;
					int t, ct = 0;
					int d = 1;
					bool q = false;
					String cmd = "solid";
					HSVCT c;
					h = constrain(root["hsv"]["h"].as<float>(), 0.0, 360.0);
					s = constrain(root["hsv"]["s"].as<float>(), 0.0, 100.0);
					v = constrain(root["hsv"]["v"].as<float>(), 0.0, 100.0);

					if(root["hsv"]["ct"].success()) {
						ct = root["hsv"]["ct"].as<int>();
						if(ct != 0 && (ct < 100 || ct > 10000 || (ct > 500 && ct < 2000))) {
							sendApiCode(response, API_CODES::API_BAD_REQUEST, "bad param for ct");
							return;
						}
					}
					if (root["cmd"].success()) {
						cmd = root["cmd"].asString();
					}

					if(root["t"].success()) {
						t = root["t"].as<int>();
					}
					if(root["q"].success()) {
						q = root["q"];
						if (q) {
							if(app.rgbwwctrl.isAnimationQFull()) {
								sendApiCode(response, API_CODES::API_BAD_REQUEST, "queue is full");
								return;
							}
						}
					}
					if(root["d"].success()) {
						d = root["d"].as<int>();
					}

					c = HSVCT(h, s, v, ct);
					debugapp("ApplicationWebserver::onColor HSV  H %f S %f V %f CT %i", h, s, v, ct);
					if(cmd.equals("fade")) {
						app.rgbwwctrl.fadeHSV(c, t, d, q);
					} else {
						app.rgbwwctrl.setHSV(c, t, q);
					}

				} else {
					sendApiCode(response, API_CODES::API_MISSING_PARAM);
					return;
				}
			} else if(root["raw"].success()) {
				if(root["raw"]["r"].success() && root["raw"]["g"].success() && root["raw"]["b"].success() && \
					root["raw"]["ww"].success() && root["raw"]["cw"].success()) {
						int t, r, g, b, ww, cw = 0;
						String cmd = "solid";
						bool q = false;

						//r = (int(constrain(root["raw"]["r"].as<float>(), 0.0, 255.0)*100) * RGBWW_PWMMAXVAL) / 100;
						//g = (int(constrain(root["raw"]["g"].as<float>(), 0.0, 255.0)*100) * RGBWW_PWMMAXVAL)/ 100;
						//b = (int(constrain(root["raw"]["b"].as<float>(), 0.0, 255.0)*100) * RGBWW_PWMMAXVAL)/ 100;
						//cw = (int(constrain(root["raw"]["cw"].as<float>(), 0.0, 255.0)*100) * RGBWW_PWMMAXVAL)/ 100;
						//ww = (int(constrain(root["raw"]["ww"].as<float>(), 0.0, 255.0)*100) * RGBWW_PWMMAXVAL)/ 100;
						r = constrain(root["raw"]["r"].as<int>(), 0, 1023);
						g = constrain(root["raw"]["g"].as<int>(), 0, 1023);
						b = constrain(root["raw"]["b"].as<int>(), 0, 1023);
						ww = constrain(root["raw"]["ww"].as<int>(), 0, 1023);
						cw = constrain(root["raw"]["cw"].as<int>(), 0, 1023);
						if (root["cmd"].success()) {
							cmd = root["cmd"].asString();
						}
						if(root["t"].success()) {
							t = root["t"].as<int>();
						}
						if(root["q"].success()) {
							q = root["q"];
							if (q) {
								if(app.rgbwwctrl.isAnimationQFull()) {
									sendApiCode(response, API_CODES::API_BAD_REQUEST, "queue is full");
									return;
								}
							}
						}

						ChannelOutput output = ChannelOutput(r, g, b, ww, cw);
						debugapp("ApplicationWebserver::onColor RAW  r:%i g:%i b:%i ww:%i cw:%i", r, g, b, ww, cw);
						if(cmd.equals("fade")) {
							app.rgbwwctrl.fadeRAW(output, t, q);
						} else {
							app.rgbwwctrl.setRAW(output, t, q);
						}
				} else {
					sendApiCode(response, API_CODES::API_MISSING_PARAM);
					return;
				}
			} else {
				sendApiCode(response, API_CODES::API_MISSING_PARAM);
				return;
			}
			sendApiCode(response, API_CODES::API_SUCCESS);

		}
	} else {
		JsonObjectStream* stream = new JsonObjectStream();
		JsonObject& json = stream->getRoot();
		String mode = request.getQueryParameter("mode", "hsv");
		if(mode.equals("raw")) {
			JsonObject& raw = json.createNestedObject("raw");
			ChannelOutput output = app.rgbwwctrl.getCurrentOutput();
			raw["r"] = output.r;
			raw["g"] = output.g;
			raw["b"] = output.b;
			raw["ww"] = output.ww;
			raw["cw"] = output.cw;

		} else if (mode.equals("temp")) {
			json["kelvin"] = 0;
			//TODO get kelvin from controller
		} else {
			JsonObject& hsv = json.createNestedObject("hsv");

			float h, s, v;
			int ct;
			HSVCT c = app.rgbwwctrl.getCurrentColor();
			c.asRadian(h, s, v, ct);
			hsv["h"] = h;
			hsv["s"] = s;
			hsv["v"] = v;
			hsv["ct"] = ct;
		}
		sendApiResponse(response, stream);
	}


}

void ApplicationWebserver::onAnimation(HttpRequest &request, HttpResponse &response)
{
	if(!authenticated(request, response)) return;
	if(request.getRequestMethod() != RequestMethod::POST && request.getRequestMethod() != RequestMethod::GET) {
		sendApiCode(response, API_CODES::API_BAD_REQUEST);
		return;
	}


	bool error = false;
	if (request.getRequestMethod() == RequestMethod::POST)
	{
		String body = request.getBody();
		if ( body == NULL || body.length() > 128)
		{
			sendApiCode(response, API_CODES::API_BAD_REQUEST);
			return;

		} else {

			DynamicJsonBuffer jsonBuffer;
			JsonObject& root = jsonBuffer.parseObject(body);
			//root.prettyPrintTo(Serial);
		}
		sendApiCode(response, API_CODES::API_SUCCESS);
	} else {
		JsonObjectStream* stream = new JsonObjectStream();
		JsonObject& json = stream->getRoot();

		sendApiResponse(response, stream);
	}


}


void ApplicationWebserver::onNetworks(HttpRequest &request, HttpResponse &response)
{

	if(!authenticated(request, response)) return;
	if(request.getRequestMethod() != RequestMethod::GET) {
		sendApiCode(response, API_CODES::API_BAD_REQUEST);
		return;
	}
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();

	bool error = false;

	if(app.network.isScanning()) {
		json["scanning"] = true;
	} else {
		json["scanning"] = false;
		JsonArray& netlist = json.createNestedArray("available");
		BssList networks = app.network.getAvailableNetworks();
		for (int i = 0; i < networks.count(); i++)
		{
			if (networks[i].hidden) continue;
			JsonObject &item = netlist.createNestedObject();
			item["id"] = (int)networks[i].getHashId();
			item["ssid"] = networks[i].ssid;
			item["signal"] = networks[i].rssi;
			item["encryption"] = networks[i].getAuthorizationMethodName();
			//limit to max 25 networks
			if (i >= 25) break;
		}
	}
	sendApiResponse(response, stream);
}


void ApplicationWebserver::onScanNetworks(HttpRequest &request, HttpResponse &response) {
	if(request.getRequestMethod() != RequestMethod::POST) {
		sendApiCode(response, API_CODES::API_BAD_REQUEST);
		return;
	}
	if(!app.network.isScanning()) {
		app.network.scan();
	}

	sendApiCode(response, API_CODES::API_SUCCESS);
}


void ApplicationWebserver::onConnect(HttpRequest &request, HttpResponse &response)
{
	if(!authenticated(request, response)) return;
	if(request.getRequestMethod() != RequestMethod::POST && request.getRequestMethod() != RequestMethod::GET) {
		sendApiCode(response, API_CODES::API_BAD_REQUEST);
		return;
	}

	if (request.getRequestMethod() == RequestMethod::POST)
	{

		String body = request.getBody();
		if ( body == NULL ) {

			sendApiCode(response, API_CODES::API_BAD_REQUEST);
			return;

		}
		DynamicJsonBuffer jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject(body);
		String ssid = "";
		String password = "";
		if (root["ssid"].success()) {
			ssid = root["ssid"].asString();
			if(root["password"].success()) {
				password = root["password"].asString();
			}
			debugapp("ssid %s - pass %s", ssid.c_str(), password.c_str());
			app.network.connect(ssid, password, true);
			sendApiCode(response, API_CODES::API_SUCCESS);
			return;

		} else {
			sendApiCode(response, API_CODES::API_MISSING_PARAM);
			return;
		}
	} else {
		JsonObjectStream* stream = new JsonObjectStream();
		JsonObject& json = stream->getRoot();

		CONNECTION_STATUS status = app.network.get_con_status();
		json["status"] = int(status);
		if ( status == CONNECTION_STATUS::ERR ) {
			json["error"] = app.network.get_con_err_msg();
		} else if (status ==  CONNECTION_STATUS::CONNECTED ) {
			// return connected
			if(app.cfg.network.connection.dhcp) {
				json["ip"] = WifiStation.getIP().toString();
			} else {
				json["ip"] = app.cfg.network.connection.ip.toString();
			}
			json["dhcp"] = app.cfg.network.connection.dhcp;
			json["ssid"] = WifiStation.getSSID();

		}
		sendApiResponse(response, stream);
	}
}

void ApplicationWebserver::onSystemReq(HttpRequest &request, HttpResponse &response) {
	if(!authenticated(request, response)) return;
	if(request.getRequestMethod() != RequestMethod::POST) {
			sendApiCode(response, API_CODES::API_BAD_REQUEST);
			return;
	}

	bool error = false;
	String body = request.getBody();
	if ( body == NULL )
	{
		sendApiCode(response, API_CODES::API_BAD_REQUEST);
		return;
	} else {
		StaticJsonBuffer<64> jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject(body);

		if(root["cmd"].success()) {

			String cmd = root["cmd"].asString();

			if (cmd.equals("debug")) {
				if(root["enable"].success()) {
					if(root["enable"]) {
						Serial.systemDebugOutput(true);
					} else {
						Serial.systemDebugOutput(false);
					}
				} else {
					error = true;
				}
			} else if(!app.delayedCMD(cmd, 3000)) {
				error = true;
			}

		} else  {
			error = true;
		}

	}
	if (!error) {
		sendApiCode(response, API_CODES::API_SUCCESS);
	} else {
		sendApiCode(response, API_CODES::API_MISSING_PARAM);
	}

}

void ApplicationWebserver::onUpdate(HttpRequest &request, HttpResponse &response) {
	if(!authenticated(request, response)) return;


	bool error = false;
	if (request.getRequestMethod() == RequestMethod::POST)
	{
		if (request.getBody() == NULL)
		{
			sendApiCode(response, API_CODES::API_BAD_REQUEST);
			return;
		} else {
			DynamicJsonBuffer jsonBuffer;
			JsonObject& root = jsonBuffer.parseObject(request.getBody());
			if(root["rom"].success() || root["webapp"].success()) {
				app.ota.reset();
				if(root["rom"].success()) {
					if(root["rom"]["url"].success()) {
						app.ota.initFirmwareUpdate(root["rom"]["url"].asString());
					} else {
						error = true;
					}
				}

				if (root["webapp"].success()){
					if(root["webapp"]["url"].success() && root["webapp"]["url"].is<JsonArray&>()) {

						JsonArray& jsonurls = root["webapp"]["url"].asArray();
						int i = 0;
						int items = jsonurls.size();
						String urls[items];
						for(JsonArray::iterator it=jsonurls.begin(); it!=jsonurls.end(); ++it)
						{
							urls[i] = it->asString();
							i += 1;

						}
						app.ota.initWebappUpdate(urls, items);
					} else {
						error = true;
					}
				}
			} else {
				error = true;
			}

		}
		if (!error) {
			app.ota.start();
			sendApiCode(response, API_CODES::API_SUCCESS);
			return;
		} else {
			sendApiCode(response, API_CODES::API_MISSING_PARAM);
		}
	} else {
		JsonObjectStream* stream = new JsonObjectStream();
		JsonObject& json = stream->getRoot();
		json["ota_status"] = int(app.ota.getStatus());
		json["rom_status"] = int(app.ota.getFirmwareStatus());
		json["webapp_status"] = int(app.ota.getWebappStatus());
		sendApiResponse(response, stream);
	}


}

//simple call-response to check if we can reach server
void ApplicationWebserver::onPing(HttpRequest &request, HttpResponse &response) {
	if(request.getRequestMethod() != RequestMethod::POST) {
		sendApiCode(response, API_CODES::API_BAD_REQUEST);
		return;
	}
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();
	json["ping"] = "pong";
	sendApiResponse(response, stream);
}

void ApplicationWebserver::generate204(HttpRequest &request, HttpResponse &response) {
	response.setHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	response.setHeader("Pragma", "no-cache");
	response.setHeader("Expires", "-1");
	response.setHeader("Content-Lenght", "0");
	response.setContentType("text/plain");
	response.setStatusCode(204, "NO CONTENT");

}



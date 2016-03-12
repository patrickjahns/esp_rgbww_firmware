#include <RGBWWCtrl.h>
#include <Services/WebHelpers/base64.h>


HttpServer server;

bool ICACHE_FLASH_ATTR authenticated(HttpRequest &request, HttpResponse &response){
	if (!cfg.general.settings_secured) return true;
    String userPass=request.getHeader("Authorization");
    int headerLength=userPass.length()-6; // header in form of: "Basic MTIzNDU2OmFiY2RlZmc="so the 6 is to get to beginning of 64 encoded string
    if(headerLength>50){
        return false;
    }

    unsigned char decbuf[headerLength]; // buffer for the decoded string
    int outlen = base64_decode(headerLength,userPass.c_str()+6, headerLength, decbuf);
    decbuf[outlen] = 0;
    userPass = String((char*)decbuf);
    if(userPass.endsWith(cfg.general.settings_password)){
        return true;
    }

    response.authorizationRequired() ; //authenHeader
    response.setHeader("WWW-Authenticate","Basic realm=\"RGBWW Server\"");
    response.setHeader("401 wrong credentials", "wrong credentials");
    response.setHeader("Connection","close");
    return false;

}

String apiErrMsg(API_ERR_CODES code) {
	switch(code) {
		case API_ERR_CODES::MISSING_PARAM :
			return String("missing param");
		default:
			return String("bad request");
	}
}

void onFile(HttpRequest &request, HttpResponse &response)
{
	String file = request.getPath();
	if (file[0] == '/')
		file = file.substring(1);

	if (file[0] == '.')
		response.forbidden();
	else
	{
		if (!fileExist(file) && !fileExist(file+".gz") && WifiAccessPoint.isEnabled()) {
			//if accesspoint is active and we couldn`t find the file - redirect to index
			response.redirect("http://"+WifiAccessPoint.getIP().toString());
		} else {
			response.setCache(86400, true); // It's important to use cache for better performance.
			response.sendFile(file);
		}
	}
}

void onIndex(HttpRequest &request, HttpResponse &response)
{

	if(!authenticated(request, response)) return;
	if (!WifiStation.isConnected() ) {
		// not yet connected - serve initial settings page
		response.sendFile("init.html");
	} else {
		// we are connected to ap - serve normal settings page
		response.sendFile("index.html");
	}

}


void onConfig(HttpRequest &request, HttpResponse &response)
{
	//TODO: more verbose error handling
	if(!authenticated(request, response)) return;
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();

	if (request.getRequestMethod() == RequestMethod::POST)
	{
		if (request.getBody() == NULL)
		{
			json["error"] = apiErrMsg(API_ERR_CODES::BAD_REQUEST);
			return;
		}
		else
		{
			bool error = false;
			String error_msg = apiErrMsg(API_ERR_CODES::BAD_REQUEST);
			DynamicJsonBuffer jsonBuffer;
			JsonObject& root = jsonBuffer.parseObject(request.getBody());
			//root.prettyPrintTo(Serial);
			bool ip_updated = false;
			bool color_updated = false;
			bool ap_updated = false;
			if (!root.success()) {
				error = true;
			} else {
				if (root["network"].success()) {

					if(root["network"]["connection"].success()) {

						if(root["network"]["connection"]["dhcp"].success()) {

							if(root["network"]["connection"]["dhcp"] != cfg.network.connection.dhcp) {
								cfg.network.connection.dhcp = root["network"]["connection"]["dhcp"];
								ip_updated = true;
							}
						}
						if (!cfg.network.connection.dhcp) {
							//only change if dhcp is off - otherwise ignore
							IPAddress ip, netmask, gateway;
							if(root["network"]["connection"]["ip"].success()) {
								ip = root["network"]["connection"]["ip"].asString();
								if(!(ip == cfg.network.connection.ip)) {
									cfg.network.connection.ip = ip;
									ip_updated = true;
								}
							} else {
								error = true;
								error_msg = "missing ip";
							}
							if(root["network"]["connection"]["netmask"].success()) {
								netmask = root["network"]["connection"]["netmask"].asString();
								if(!(netmask == cfg.network.connection.netmask)) {
									cfg.network.connection.netmask = netmask;
									ip_updated = true;
								}
							} else {
								error = true;
								error_msg = "missing netmask";
							}
							if(root["network"]["connection"]["gateway"].success()) {
								gateway = root["network"]["connection"]["gateway"].asString();
								if(!(gateway == cfg.network.connection.gateway)) {
									cfg.network.connection.gateway = gateway;
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
							if (root["network"]["ap"]["ssid"] != cfg.network.ap.ssid) {
								cfg.network.ap.ssid = root["network"]["ap"]["ssid"].asString();
								ap_updated = true;
							}
						}
						if(root["network"]["ap"]["secured"].success()) {
								if (root["network"]["ap"]["secured"]){
									if(root["network"]["ap"]["password"].success()) {
										if (root["network"]["ap"]["password"] != cfg.network.ap.password) {
											cfg.network.ap.secured = root["network"]["ap"]["secured"];
											cfg.network.ap.password = root["network"]["ap"]["password"].asString();
											ap_updated = true;
										}
									} else {
										error = true;
										error_msg = "missing password for securing ap";
									}
								} else if (root["network"]["ap"]["secured"] != cfg.network.ap.secured)
								{
									root["network"]["ap"]["secured"] == cfg.network.ap.secured;
									ap_updated = true;
								}
						}

					}
					if(root["network"]["mqtt"].success()) {
						//TODO: what to do if changed?

						if(root["network"]["mqtt"]["enabled"].success()) {
							if (root["network"]["mqtt"]["enabled"] != cfg.network.mqtt.enabled) {
								cfg.network.mqtt.enabled = root["network"]["mqtt"]["enabled"];
							}
						}
						if(root["network"]["mqtt"]["server"].success()) {
							if (root["network"]["mqtt"]["server"] != cfg.network.mqtt.server) {
								cfg.network.mqtt.server = root["network"]["mqtt"]["server"].asString();
							}
						}
						if(root["network"]["mqtt"]["port"].success()) {
							if (root["network"]["mqtt"]["port"] != cfg.network.mqtt.port) {
								cfg.network.mqtt.port = root["network"]["mqtt"]["port"];
							}
						}
						if(root["network"]["mqtt"]["username"].success()) {
							if (root["network"]["mqtt"]["username"] != cfg.network.mqtt.username) {
								cfg.network.mqtt.username = root["network"]["mqtt"]["username"].asString();
							}
						}
						if(root["network"]["mqtt"]["password"].success()) {
							if (root["network"]["mqtt"]["password"] != cfg.network.mqtt.password) {
								cfg.network.mqtt.password = root["network"]["mqtt"]["password"].asString();
							}
						}
					}
					if(root["network"]["udpserver"].success()) {
						//TODO: what to do if changed?
						if(root["network"]["udpserver"]["enabled"].success()) {
							if (root["network"]["udpserver"]["enabled"] != cfg.network.udpserver.enabled) {
								cfg.network.udpserver.enabled = root["network"]["udpserver"]["enabled"];
							}
						}
						if (root["network"]["udpserver"]["port"].success()) {
							if (root["network"]["udpserver"]["port"] != cfg.network.udpserver.port) {
								cfg.network.udpserver.port = root["network"]["udpserver"]["port"];
							}
						}
					}
					if(root["network"]["tcpserver"].success()) {
						//TODO: what to do if changed?
						if(root["network"]["tcpserver"]["enabled"].success()) {
							if (root["network"]["tcpserver"]["enabled"] != cfg.network.tcpserver.enabled) {
								cfg.network.tcpserver.enabled = root["network"]["tcpserver"]["enabled"];
							}
						}
						if (root["network"]["tcpserver"]["port"].success()) {
							if (root["network"]["tcpserver"]["port"] != cfg.network.tcpserver.port) {
								cfg.network.tcpserver.port = root["network"]["tcpserver"]["port"];
							}
						}
					}
				}

				if (root["color"].success())
				{
					//TODO DRY
					if (root["color"]["hsv"].success()) {
						if (root["color"]["hsv"]["model"].success()){
							if (root["color"]["hsv"]["model"] != cfg.color.hsv.model) {
								cfg.color.hsv.model = root["color"]["hsv"]["model"].as<int>();
								color_updated = true;
							}
						}
						if (root["color"]["hsv"]["red"].success())
						{
							if (root["color"]["hsv"]["red"].as<float>() != cfg.color.hsv.red)
							{
								cfg.color.hsv.red = root["color"]["hsv"]["red"].as<float>();
								color_updated = true;
							}
						}
						if (root["color"]["hsv"]["yellow"].success())
						{
							if (root["color"]["hsv"]["yellow"].as<float>() != cfg.color.hsv.yellow)
							{
								cfg.color.hsv.yellow = root["color"]["hsv"]["yellow"].as<float>();
								color_updated = true;
							}
						}
						if (root["color"]["hsv"]["green"].success())
						{
							if (root["color"]["hsv"]["green"].as<float>() != cfg.color.hsv.green)
							{
								cfg.color.hsv.green = root["color"]["hsv"]["green"].as<float>();
								color_updated = true;
							}
						}
						if (root["color"]["hsv"]["cyan"].success())
						{
							if (root["color"]["hsv"]["cyan"].as<float>() != cfg.color.hsv.cyan)
							{
								cfg.color.hsv.cyan = root["color"]["hsv"]["cyan"].as<float>();
								color_updated = true;
							}
						}
						if (root["color"]["hsv"]["blue"].success())
						{
							if (root["color"]["hsv"]["blue"].as<float>() != cfg.color.hsv.blue)
							{
								cfg.color.hsv.blue = root["color"]["hsv"]["blue"].as<float>();
								color_updated = true;
							}
						}
						if (root["color"]["hsv"]["magenta"].success())
						{
							if (root["color"]["hsv"]["magenta"].as<float>() != cfg.color.hsv.magenta)
							{
								cfg.color.hsv.magenta = root["color"]["hsv"]["magenta"].as<float>();
								color_updated = true;
							}
						}
					}
					if (root["color"]["outputmode"].success()) {
						if(root["color"]["outputmode"] != cfg.color.outputmode) {
							cfg.color.outputmode = root["color"]["outputmode"].as<int>();
							color_updated = true;
						}
					}
					if (root["color"]["brightness"].success()) {
						//TODO DRY
						if (root["color"]["brightness"]["red"].success()) {
							if (root["color"]["brightness"]["red"].as<int>() != cfg.color.brightness.red) {
								cfg.color.brightness.red = root["color"]["brightness"]["red"].as<int>();
								color_updated = true;
							}
						}
						if (root["color"]["brightness"]["green"].success()) {
							if (root["color"]["brightness"]["green"].as<int>() != cfg.color.brightness.green) {
								cfg.color.brightness.green = root["color"]["brightness"]["green"].as<int>();
								color_updated = true;
							}
						}
						if (root["color"]["brightness"]["blue"].success()) {
							if (root["color"]["brightness"]["blue"].as<int>() != cfg.color.brightness.blue) {
								cfg.color.brightness.blue = root["color"]["brightness"]["blue"].as<int>();
								color_updated = true;
							}
						}
						if (root["color"]["brightness"]["ww"].success()) {
							if (root["color"]["brightness"]["ww"].as<int>() != cfg.color.brightness.ww) {
								cfg.color.brightness.ww = root["color"]["brightness"]["ww"].as<int>();
								color_updated = true;
							}
						}
						if (root["color"]["brightness"]["cw"].success()) {
							if (root["color"]["brightness"]["cw"].as<int>() != cfg.color.brightness.cw) {
								cfg.color.brightness.cw = root["color"]["brightness"]["cw"].as<int>();
								color_updated = true;
							}
						}
					}
					if (root["color"]["colortemp"].success()) {
						//TODO: DRY
						if (root["color"]["colortemp"]["ww"].success()) {
							if (root["color"]["colortemp"]["cw"].as<int>() != cfg.color.colortemp.ww) {
								cfg.color.colortemp.ww = root["color"]["colortemp"]["ww"].as<int>();
								color_updated = true;
							}
						}
						if (root["color"]["colortemp"]["cw"].success()) {
							if (root["color"]["colortemp"]["cw"].as<int>() != cfg.color.colortemp.cw) {
								cfg.color.colortemp.cw = root["color"]["colortemp"]["cw"].as<int>();
								color_updated = true;
							}
						}
					}
				}

				if(root["security"].success())
				{
					if(root["security"]["settings_secured"].success()){
						cfg.general.settings_secured = root["security"]["settings_secured"];
						if (root["security"]["settings_secured"]) {
							if(root["security"]["settings_password"].success()){
								if(root["security"]["settings_password"] != cfg.general.settings_password) {

									cfg.general.settings_password = root["security"]["settings_password"].asString();
								}
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
							systemTimer.initializeMs(3000, restart).startOnce();
							//TODO: change to be more precise
							json["data"] = "restart";
							debugf("ip settings changed - rebooting");
						}
					}
				};
				if(ap_updated) {
					if (root["restart"].success()) {
						if (root["restart"] == true && WifiAccessPoint.isEnabled()) {
							systemTimer.initializeMs(3000, restart).startOnce();
							json["data"] = "restart";
							debugf("wifiap settings changed - rebooting");
						}
					}
				}
				if (color_updated) {
					//refresh settings
					setupRGBWW();
					//refresh current output
					rgbwwctrl.refresh();
					debugf("color settings changed - refreshing");
				}
				cfg.save();
				json["success"] = (bool)true;
			} else {
				json["error"] = error_msg;
			}


		}


	} else {

		// returning settings
		JsonObject& net = json.createNestedObject("network");
		JsonObject& con = net.createNestedObject("connection");
		con["dhcp"] = WifiStation.isEnabledDHCP();
		con["ip"] = WifiStation.getIP().toString();
		con["netmask"] = WifiStation.getNetworkMask().toString();
		con["gateway"] = WifiStation.getNetworkGateway().toString();
		//con["mdnshostname"] = cfg.network.connection.mdnshostname.c_str();

		JsonObject& ap = net.createNestedObject("ap");
		ap["secured"] = cfg.network.ap.secured;
		ap["ssid"] = cfg.network.ap.ssid.c_str();

		JsonObject& mqtt = net.createNestedObject("mqtt");
		mqtt["enabled"] = cfg.network.mqtt.enabled;
		mqtt["server"] = cfg.network.mqtt.server.c_str();
		mqtt["port"] = cfg.network.mqtt.port;
		mqtt["username"] = cfg.network.mqtt.username.c_str();
		mqtt["password"] = cfg.network.mqtt.password.c_str();

		JsonObject& udp = net.createNestedObject("udpserver");
		udp["enabled"] = cfg.network.udpserver.enabled;
		udp["port"] = cfg.network.udpserver.port;

		JsonObject& tcp = net.createNestedObject("tcpserver");
		tcp["enabled"] = cfg.network.tcpserver.enabled;
		tcp["port"] = cfg.network.tcpserver.port;

		JsonObject& color = json.createNestedObject("color");
		color["outputmode"] = cfg.color.outputmode;

		JsonObject& hsv = color.createNestedObject("hsv");
		hsv["model"] = cfg.color.hsv.model;

		hsv["red"] = cfg.color.hsv.red;
		hsv["yellow"] = cfg.color.hsv.yellow;
		hsv["green"] = cfg.color.hsv.green;
		hsv["cyan"] = cfg.color.hsv.cyan;
		hsv["blue"] = cfg.color.hsv.blue;
		hsv["magenta"] = cfg.color.hsv.magenta;

		JsonObject& brighntess = color.createNestedObject("brightness");
		brighntess["red"] = cfg.color.brightness.red;
		brighntess["green"] = cfg.color.brightness.green;
		brighntess["blue"] = cfg.color.brightness.blue;
		brighntess["ww"] = cfg.color.brightness.ww;
		brighntess["cw"] = cfg.color.brightness.cw;

		JsonObject& ctmp = color.createNestedObject("colortemp");
		ctmp["ww"] = cfg.color.colortemp.ww;
		ctmp["cw"] = cfg.color.colortemp.cw;

		JsonObject& s = json.createNestedObject("security");
		s["settings_secured"] = cfg.general.settings_secured;

	}
	response.setAllowCrossDomainOrigin("*");
	response.sendJsonObject(stream);
}

void onInfo(HttpRequest &request, HttpResponse &response)
{
	if(!authenticated(request, response)) return;
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();
	JsonObject& data = json.createNestedObject("data");
	data["deviceid"] = String(system_get_chip_id());
	data["firmware"] = fw_version;
	data["config_version"] = cfg.configversion;
	data["sming"] = SMING_VERSION;
	data["rgbwwled"] = RGBWW_VERSION;
	JsonObject& con = data.createNestedObject("connection");
	con["connected"] = WifiStation.isConnected();
	con["ssid"] = cfg.network.connection.ssid.c_str();
	con["dhcp"] = WifiStation.isEnabledDHCP();
	con["ip"] = WifiStation.getIP().toString();
	con["netmask"] = WifiStation.getNetworkMask().toString();
	con["gateway"] = WifiStation.getNetworkGateway().toString();
	con["mac"] = WifiStation.getMAC();
	response.setAllowCrossDomainOrigin("*");
	response.sendJsonObject(stream);
}

void onColor(HttpRequest &request, HttpResponse &response)
{
	if(!authenticated(request, response)) return;
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();
	bool error = false;
	if (request.getRequestMethod() == RequestMethod::POST)
	{
		String body = request.getBody();
		if ( body == NULL || body.length() > 128)
		{
			json["error"] = apiErrMsg(API_ERR_CODES::BAD_REQUEST);
			return;
		}
		else
		{

			StaticJsonBuffer<128> jsonBuffer;
			JsonObject& root = jsonBuffer.parseObject(body);
			//root.prettyPrintTo(Serial);

			if (root["color"].success()) {
				if(root["color"]["h"].success() && root["color"]["s"].success() && root["color"]["v"].success()) {
					float h, s, v;
					int t, k = 0;
					bool q = false;
					bool d = false;
					HSVK c;
					h = root["color"]["h"].as<float>();
					s = root["color"]["s"].as<float>();
					v = root["color"]["v"].as<float>();

					if(root["color"]["k"].success()) {
						k = root["color"]["k"].as<int>();
					}
					if (root["cmd"].success()) {
						debugf("cmd");
						if(root["cmd"]["t"].success()) {
							t = root["cmd"]["t"].as<int>();
						}
						if(root["cmd"]["q"].success()) {
							q = root["cmd"]["q"];
						}
						if(root["cmd"]["d"].success()) {
							d = root["cmd"]["d"];
						}
					}
					c = HSVK(h, s, v, k);
					debugf("H %i S %i V %i K %i", c.h, c.s, c.v, c.k);
					rgbwwctrl.setHSV(c, t, q);

				} else {

					error = true;
				}
			} else {
				error = true;
			}

			if (error) {
				json["error"] = apiErrMsg(API_ERR_CODES::MISSING_PARAM);
			} else {
				json["success"] = (bool)true;
			}
		}
	}
	else {
		JsonObject& data = json.createNestedObject("data");
		JsonObject& color = data.createNestedObject("color");
		float h, s, v;
		int k;
		HSVK c = rgbwwctrl.getCurrentColor();
		c.asRadian(h, s, v, k);
		color["h"] = h;
		color["s"] = s;
		color["v"] = v;
		color["k"] = k;

	}
	response.setAllowCrossDomainOrigin("*");
	response.sendJsonObject(stream);
}




void onNetworks(HttpRequest &request, HttpResponse &response)
{

	if(!authenticated(request, response)) return;
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();

	bool error = false;

	if (request.getRequestMethod() == RequestMethod::POST)
	{
		String body = request.getBody();
		if ( body == NULL || body.length() > 64)
		{
			json["error"] = apiErrMsg(API_ERR_CODES::BAD_REQUEST);
			return;
		}
		else
		{
			StaticJsonBuffer<64> jsonBuffer;
			JsonObject& root = jsonBuffer.parseObject(body);
			if (root["cmd"].success()) {

				if(!scanning) {
					scanNetworks();
				}
				json["success"] = (bool)true;
			} else {
				error = true;
				json["error"] = apiErrMsg(API_ERR_CODES::BAD_REQUEST);
			}

		}
	} else {
		if(scanning) {
			json["scanning"] = true;
		} else {
			json["scanning"] = false;
			JsonArray& netlist = json.createNestedArray("available");
			for (int i = 0; i < networks.count(); i++)
			{
				if (networks[i].hidden) continue;
				JsonObject &item = netlist.createNestedObject();
				item["id"] = (int)networks[i].getHashId();
				item["title"] = networks[i].ssid;
				item["signal"] = networks[i].rssi;
				item["encryption"] = networks[i].getAuthorizationMethodName();
				//limit to max 25 networks
				if (i >= 25) break;
			}
		}
	}
	response.setAllowCrossDomainOrigin("*");
	response.sendJsonObject(stream);
}




void onConnect(HttpRequest &request, HttpResponse &response)
{
	if(!authenticated(request, response)) return;
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();
	//JsonObject& json = json.createNestedObject("response");
	//bool error = false;
	if (request.getRequestMethod() == RequestMethod::POST)
	{
		String body = request.getBody();
		if ( body == NULL || body.length() > 64)
		{
			response.badRequest();
			return;
		}
		else
		{
			StaticJsonBuffer<96> jsonBuffer;
			JsonObject& root = jsonBuffer.parseObject(body);
			if (root["ssid"].success() && root["password"].success()) {
				String ssid = root["ssid"].asString();
				String password = root["password"].asString();
				if (password !=  WifiStation.getPassword() || ssid != WifiStation.getSSID()) {
					WifiStation.config(ssid, password);
					WifiStation.waitConnection(connectOk, 5, connectFail);
				}
				else
				{
					/* We already know that password and SSID so don`t try to connect again
					 * Don`t send information about this back, since it would allow for
					 * bruteforce checking for the password by asking controller again and again
					 */

				}
				json["success"] = (bool)true;

			}
			else
			{
				json["error"] = apiErrMsg(API_ERR_CODES::BAD_REQUEST);
			}
		}

	} else {

		EStationConnectionStatus status = WifiStation.getConnectionStatus();
		if ( status == eSCS_Idle ) {
			//waiting to connect
			json["status"] = int(CONNECTION_STATUS::IDLE);
		}
		else if (status ==  eSCS_GotIP )
		{
			// return connected
			json["status"] = int(CONNECTION_STATUS::CONNECTED);
			if(cfg.network.connection.dhcp) {
				json["ip"] = WifiStation.getIP().toString();
			} else {
				json["ip"] = cfg.network.connection.ip.toString();
			}
			json["restart"] = true;
			//TODO: stop AP first and then initialize a restart afterward
			//systemTimer.initializeMs(3000, stopAp).startOnce();
			systemTimer.initializeMs(3000, restart).startOnce();
			json["ssid"] = WifiStation.getSSID();
		}
		else if (status == eSCS_Connecting) {
			//return connecting status
			json["status"] = int(CONNECTION_STATUS::CONNECTING);
		}
		else
		{
			//FAILED
			json["status"] = int(CONNECTION_STATUS::ERR);
			json["error"] = WifiStation.getConnectionStatusName();
			WifiStation.config("", "");
			WifiStation.disconnect();
		}
	}

	response.setAllowCrossDomainOrigin("*");
	response.sendJsonObject(stream);
}

void onSystemReq(HttpRequest &request, HttpResponse &response) {
	if(!authenticated(request, response)) return;
	if (request.getRequestMethod() != RequestMethod::POST) {
		response.badRequest();
		return;
	}
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();
	bool error = false;

	// only allow post commands - otherwise http error


	String body = request.getBody();
	if ( body == NULL || body.length() > 64)
	{
		response.badRequest();
		return;
	}
	else
	{
		StaticJsonBuffer<64> jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject(body);

		if(root["cmd"].success()) {
			String cmd = root["cmd"].asString();
			if(cmd.indexOf("reset") != -1) {
				cfg.reset();
				WifiStation.config("", "");
				systemTimer.initializeMs(3000, restart).startOnce();
			}
			else if(cmd.indexOf("restart") != -1) {
				systemTimer.initializeMs(3000, restart).startOnce();
			}
			else if (cmd.indexOf("forget_wifi") != -1){
				WifiStation.config("", "");
				cfg.network.connection.password = "";
				cfg.network.connection.ssid = "";
				cfg.save();
				systemTimer.initializeMs(3000, restart).startOnce();
			}
			else
			{
				error = true;
			}
		}
		else
		{
			error = true;
		}
		if (error) {
			json["error"] = apiErrMsg(API_ERR_CODES::MISSING_PARAM);
		}
	}
	if (!error) {
		json["success"] = (bool)true;
	}
	response.setAllowCrossDomainOrigin("*");
	response.sendJsonObject(stream);

}

void onUpdate(HttpRequest &request, HttpResponse &response) {
	if(!authenticated(request, response)) return;
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();

	bool error = false;
	if (request.getRequestMethod() == RequestMethod::POST)
	{
		if (request.getBody() == NULL)
		{
			response.badRequest();
			return;
		}
		else
		{
			StaticJsonBuffer<1024> jsonBuffer;
			JsonObject& root = jsonBuffer.parseObject(request.getBody());
			bool force = false;
			bool updating = false;
			if (root["force"].success()) {
				force = root["force"];
			}
			if(root["rom"].success()) {

			}
			if (root["webapp"].success()){

			}
			if (updating) {

				json["updating"] = true;
				//start updatetimer
			}
			else
			{
				json["updating"] = false;
			}
		}
		if (!error) {
			json["success"] = (bool)true;
		}
	} else {
		//return update status
	}

	response.setAllowCrossDomainOrigin("*");
	response.sendJsonObject(stream);
}

//simple call-response to check if we can reach server
void onPing(HttpRequest &request, HttpResponse &response) {
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();

	json["ping"] = "pong";
	response.setAllowCrossDomainOrigin("*");
	response.sendJsonObject(stream);
}



void startWebServer()
{
	server.listen(80);
	server.setDefaultHandler(onFile);
	server.enableHeaderProcessing("Authorization");
	server.addPath("/", onIndex);
	server.addPath("/config", onConfig);
	server.addPath("/info", onInfo);
	server.addPath("/color", onColor);
	server.addPath("/networks", onNetworks);
	server.addPath("/system", onSystemReq);
	server.addPath("/update", onUpdate);
	server.addPath("/connect", onConnect);
	server.addPath("/ping", onPing);
}

void stopWebServer() {

}

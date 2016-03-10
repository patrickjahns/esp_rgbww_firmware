#include <RGBWWCtrl.h>
#include <Services/WebHelpers/base64.h>

//Timer updateTimer;
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
    response.setHeader("401 Wrong credentials", "wrong credentials");
    response.setHeader("Connection","close");
    return false;

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
	if(!authenticated(request, response)) return;
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& jdata = stream->getRoot();
	jdata["success"] = (bool)false;

	if (request.getRequestMethod() == RequestMethod::POST)
	{
		if (request.getBody() == NULL)
		{

			JsonObject& data = jdata.createNestedObject("data");
			data["error"] = "not a valid request";
			debugf("NULL bodyBuf");

		}
		else
		{
			DynamicJsonBuffer jsonBuffer;
			JsonObject& root = jsonBuffer.parseObject(request.getBody());
			//root.prettyPrintTo(Serial);
			jdata["data"] = "saved";
			if (root["network"].success()) {

				if(root["network"]["connection"].success()) {
					bool updated = false;
					if(root["network"]["connection"]["dhcp"].success()) {

						if(root["network"]["connection"]["dhcp"] != cfg.network.connection.dhcp) {
							cfg.network.connection.dhcp = root["network"]["connection"]["dhcp"];
							updated = true;
						}
					}
					if (!cfg.network.connection.dhcp) {
						//only change if dhcp is off - otherwise ignore
						IPAddress ip, netmask, gateway;
						if(root["network"]["connection"]["ip"].success()) {

							ip = root["network"]["connection"]["ip"].asString();
							if(!(ip == cfg.network.connection.ip)) {
								cfg.network.connection.ip = ip;
								updated = true;
							}
						}
						if(root["network"]["connection"]["netmask"].success()) {
							netmask = root["network"]["connection"]["netmask"].asString();
							if(!(netmask == cfg.network.connection.netmask)) {
								cfg.network.connection.netmask = netmask;
								updated = true;
							}
						}
						if(root["network"]["connection"]["gateway"].success()) {
							gateway = root["network"]["connection"]["gateway"].asString();
							if(!(gateway == cfg.network.connection.gateway)) {
								cfg.network.connection.gateway = gateway;
								updated = true;
							}
						}
						if (updated) {
							if (root["restart"].success()) {
								if (root["restart"] == true) {
									systemTimer.initializeMs(3000, restart).startOnce();
									//TODO: change to be more precise
									jdata["data"] = "restart";
									debugf("ip settings changed - rebooting");
								}
							}
						};
					}

				}
				if(root["network"]["ap"].success()) {
					bool updated = false;

					if(root["network"]["ap"]["ssid"].success()) {
						if (root["network"]["ap"]["ssid"] != cfg.network.ap.ssid) {
							cfg.network.ap.ssid = root["network"]["ap"]["ssid"].asString();
							updated = true;
						}
					}
					if(root["network"]["ap"]["secured"].success()) {
							if (root["network"]["ap"]["secured"]){
								if(root["network"]["ap"]["password"].success()) {
									if (root["network"]["ap"]["password"] != cfg.network.ap.password) {
										cfg.network.ap.secured = root["network"]["ap"]["secured"];
										cfg.network.ap.password = root["network"]["ap"]["password"].asString();
										updated = true;
									}
								}
							} else if (root["network"]["ap"]["secured"] != cfg.network.ap.secured)
							{
								root["network"]["ap"]["secured"] == cfg.network.ap.secured;
								updated = true;
							}
					}
					if(updated) {
						if (root["restart"].success()) {
							if (root["restart"] == true && WifiAccessPoint.isEnabled()) {
								systemTimer.initializeMs(3000, restart).startOnce();
								jdata["data"] = "restart";
								debugf("wifiap settings changed - rebooting");
							}
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
				bool updated = false;
				if (root["color"]["hsv"].success()) {
					if (root["color"]["hsv"]["model"].success()){
						if (root["color"]["hsv"]["model"] != cfg.color.hsv.model) {
							cfg.color.hsv.model = root["color"]["hsv"]["model"].as<int>();
							updated = true;
						}
					}
					if (root["color"]["hsv"]["red"].success())
					{
						if (root["color"]["hsv"]["red"].as<float>() != cfg.color.hsv.red)
						{
							cfg.color.hsv.red = root["color"]["hsv"]["red"].as<float>();
							updated = true;
						}
					}
					if (root["color"]["hsv"]["yellow"].success())
					{
						if (root["color"]["hsv"]["yellow"].as<float>() != cfg.color.hsv.yellow)
						{
							cfg.color.hsv.yellow = root["color"]["hsv"]["yellow"].as<float>();
							updated = true;
						}
					}
					if (root["color"]["hsv"]["green"].success())
					{
						if (root["color"]["hsv"]["green"].as<float>() != cfg.color.hsv.green)
						{
							cfg.color.hsv.green = root["color"]["hsv"]["green"].as<float>();
							updated = true;
						}
					}
					if (root["color"]["hsv"]["cyan"].success())
					{
						if (root["color"]["hsv"]["cyan"].as<float>() != cfg.color.hsv.cyan)
						{
							cfg.color.hsv.cyan = root["color"]["hsv"]["cyan"].as<float>();
							updated = true;
						}
					}
					if (root["color"]["hsv"]["blue"].success())
					{
						if (root["color"]["hsv"]["blue"].as<float>() != cfg.color.hsv.blue)
						{
							cfg.color.hsv.blue = root["color"]["hsv"]["blue"].as<float>();
							updated = true;
						}
					}
					if (root["color"]["hsv"]["magenta"].success())
					{
						if (root["color"]["hsv"]["magenta"].as<float>() != cfg.color.hsv.magenta)
						{
							cfg.color.hsv.magenta = root["color"]["hsv"]["magenta"].as<float>();
							updated = true;
						}
					}
				}
				if (root["color"]["outputmode"].success()) {
					if(root["color"]["outputmode"] != cfg.color.outputmode) {
						cfg.color.outputmode = root["color"]["outputmode"].as<int>();
						updated = true;
					}
				}
				if (root["color"]["brightness"].success()) {
					if (root["color"]["brightness"]["red"].success()) {
						if (root["color"]["brightness"]["red"].as<int>() != cfg.color.brightness.red) {
							cfg.color.brightness.red = root["color"]["brightness"]["red"].as<int>();
							updated = true;
						}
					}
					if (root["color"]["brightness"]["green"].success()) {
						if (root["color"]["brightness"]["green"].as<int>() != cfg.color.brightness.green) {
							cfg.color.brightness.green = root["color"]["brightness"]["green"].as<int>();
							updated = true;
						}
					}
					if (root["color"]["brightness"]["blue"].success()) {
						if (root["color"]["brightness"]["blue"].as<int>() != cfg.color.brightness.blue) {
							cfg.color.brightness.blue = root["color"]["brightness"]["blue"].as<int>();
							updated = true;
						}
					}
					if (root["color"]["brightness"]["ww"].success()) {
						if (root["color"]["brightness"]["ww"].as<int>() != cfg.color.brightness.ww) {
							cfg.color.brightness.ww = root["color"]["brightness"]["ww"].as<int>();
							updated = true;
						}
					}
					if (root["color"]["brightness"]["cw"].success()) {
						if (root["color"]["brightness"]["cw"].as<int>() != cfg.color.brightness.cw) {
							cfg.color.brightness.cw = root["color"]["brightness"]["cw"].as<int>();
							updated = true;
						}
					}
				}
				if (root["color"]["colortemp"].success()) {
					if (root["color"]["colortemp"]["ww"].success()) {
						if (root["color"]["colortemp"]["cw"].as<int>() != cfg.color.colortemp.ww) {
							cfg.color.colortemp.ww = root["color"]["colortemp"]["ww"].as<int>();
							updated = true;
						}
					}
					if (root["color"]["colortemp"]["cw"].success()) {
						if (root["color"]["colortemp"]["cw"].as<int>() != cfg.color.colortemp.cw) {
							cfg.color.colortemp.cw = root["color"]["colortemp"]["cw"].as<int>();
							updated = true;
						}
					}
				}
				if (updated) {
					//refresh settings
					setupRGBWW();
					//refresh current output
					rgbwwctrl.refresh();
				}


			}
			if(root["security"].success())
			{
				if(root["security"]["settings_secured"].success()){
					cfg.general.settings_secured = root["security"]["settings_secured"];
				}
				if(root["security"]["settings_password"].success()){
					if(root["security"]["settings_password"] != cfg.general.settings_password) {
						cfg.general.settings_password = root["security"]["settings_password"].asString();
					}
				}
			}
			cfg.save();

			jdata["success"] = (bool)true;

		}


	} else {

		// returning settings
		JsonObject& data = jdata.createNestedObject("data");
		JsonObject& net = data.createNestedObject("network");
		JsonObject& con = net.createNestedObject("connection");
		con["connected"] = WifiStation.isConnected();
		con["ssid"] = cfg.network.connection.ssid.c_str();
		con["dhcp"] = WifiStation.isEnabledDHCP();
		con["ip"] = WifiStation.getIP().toString();
		con["netmask"] = WifiStation.getNetworkMask().toString();
		con["gateway"] = WifiStation.getNetworkGateway().toString();
		con["mac"] = WifiStation.getMAC();
		con["mdnshostname"] = cfg.network.connection.mdnshostname.c_str();

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

		JsonObject& c = data.createNestedObject("color");
		c["outputmode"] = cfg.color.outputmode;

		JsonObject& h = c.createNestedObject("hsv");
		h["model"] = cfg.color.hsv.model;

		h["red"] = cfg.color.hsv.red;
		h["yellow"] = cfg.color.hsv.yellow;
		h["green"] = cfg.color.hsv.green;
		h["cyan"] = cfg.color.hsv.cyan;
		h["blue"] = cfg.color.hsv.blue;
		h["magenta"] = cfg.color.hsv.magenta;

		JsonObject& b = c.createNestedObject("brightness");
		b["red"] = cfg.color.brightness.red;
		b["green"] = cfg.color.brightness.green;
		b["blue"] = cfg.color.brightness.blue;
		b["ww"] = cfg.color.brightness.ww;
		b["cw"] = cfg.color.brightness.cw;

		JsonObject& ctmp = c.createNestedObject("colortemp");
		ctmp["ww"] = cfg.color.colortemp.ww;
		ctmp["cw"] = cfg.color.colortemp.cw;

		JsonObject& s = data.createNestedObject("security");
		s["settings_secured"] = cfg.general.settings_secured;

		JsonObject& i = data.createNestedObject("info");
		//i["version"] = cfg.appversion;
		i["config_version"] = cfg.configversion;
		i["sming"] = SMING_VERSION;
		i["rgbww"] = RGBWW_VERSION;

		jdata["success"] = (bool)true;
	}
	response.setAllowCrossDomainOrigin("*");
	response.sendJsonObject(stream);
}


void onColor(HttpRequest &request, HttpResponse &response)
{
	if(!authenticated(request, response)) return;
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& jdata = stream->getRoot();
	jdata["success"] = (bool)false;
	bool error = false;
	if (request.getRequestMethod() == RequestMethod::POST)
	{
		if (request.getBody() == NULL)
		{
			error = true;
			debugf("NULL bodyBuf");
		}
		else
		{

			DynamicJsonBuffer jsonBuffer;
			JsonObject& root = jsonBuffer.parseObject(request.getBody());
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
					jdata["success"] = true;
				} else {
					error = true;
				}
			} else {
				error = true;
			}

		}
		if (error) {
			JsonObject& data = jdata.createNestedObject("data");
			data["error"] = "invalid data";
		}

	}
	else {
		JsonObject& data = jdata.createNestedObject("data");
		JsonObject& color = data.createNestedObject("color");
		float h, s, v;
		int k;
		HSVK c = rgbwwctrl.getCurrentColor();
		c.asRadian(h, s, v, k);
		color["h"] = h;
		color["s"] = s;
		color["v"] = v;
		color["k"] = k;
		jdata["success"] = true;
	}
	response.setAllowCrossDomainOrigin("*");
	response.sendJsonObject(stream);
}




void onNetworkList(HttpRequest &request, HttpResponse &response)
{
	//TODO: rewrite so we start a scan if there are no networks in the list
	if(!authenticated(request, response)) return;
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();
	JsonObject& data = json.createNestedObject("data");

	json["success"] = (bool)true;

	bool connected = WifiStation.isConnected();
	data["connected"] = connected;
	if (connected)
	{
		data["network"]= WifiStation.getSSID();
	}

	if(scanning) {
		data["scan"] = true;
	} else {
		data["scan"] = false;
		JsonArray& netlist = data.createNestedArray("available");
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

	response.setAllowCrossDomainOrigin("*");
	response.sendJsonObject(stream);
}

void onRefreshNetworkList(HttpRequest &request, HttpResponse &response) {
	if(!authenticated(request, response)) return;
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();
	json["success"] = (bool)false;
	if(!scanning) {
		json["success"] = (bool)true;
		scanNetworks();
	}
	response.setAllowCrossDomainOrigin("*");
	response.sendJsonObject(stream);
}



void onConnect(HttpRequest &request, HttpResponse &response)
{
	if(!authenticated(request, response)) return;
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();
	JsonObject& data = json.createNestedObject("data");

	json["success"] = (bool)false;
	if (request.getRequestMethod() == RequestMethod::POST)
	{
		if (request.getBody() == NULL)
		{
			debugf("NULL bodyBuf");
		}
		else
		{

			DynamicJsonBuffer jsonBuffer;
			JsonObject& root = jsonBuffer.parseObject(request.getBody());
			if (root["ssid"].success() && root["password"].success()) {
				String ssid = root["ssid"].asString();
				String password = root["password"].asString();
				if (password !=  WifiStation.getPassword() || ssid != WifiStation.getSSID()) {
					WifiStation.config(ssid, password);
					WifiStation.waitConnection(connectOk, 5, connectFail);
					// return connecting code
					json["success"] = (bool)true;
					data["connecting"] = (bool)true;
					data["connected"] = (bool)false;
				}
				else
				{
					json["success"] = (bool)true;
					data["connecting"] = (bool)false;
					data["connected"] = (bool)true;
					//already connected to that network
				}

			}
			else
			{	data["error"] = "missing parameter";
				//return error code
			}
		}

	} else {

		EStationConnectionStatus status = WifiStation.getConnectionStatus();
		if ( status == eSCS_Idle ) {
			//waiting to connect
			data["connecting"] = (bool)false;
			data["connected"] = (bool)false;
			json["status"] = (bool)true;
		}
		else if (status ==  eSCS_GotIP )
		{
			// return connected
			data["connecting"] = (bool)false;
			data["connected"] = (bool)true;
			if(cfg.network.connection.dhcp) {
				data["ip"] = WifiStation.getIP().toString();
			} else {
				data["ip"] = cfg.network.connection.ip.toString();
			}
			data["restart"] = true;
			//TODO: stop AP first and then initialize a restart afterward
			//systemTimer.initializeMs(3000, stopAp).startOnce();
			systemTimer.initializeMs(3000, restart).startOnce();
			data["ssid"] = WifiStation.getSSID();
			json["success"] = (bool)true;
		}
		else if (status == eSCS_Connecting) {
			//return connecting status

			data["connecting"] = (bool)true;
			data["connected"] = (bool)false;
			json["success"] = (bool)true;
		}
		else
		{
			//FAILED
			data["connecting"] = (bool)false;
			data["connected"] = (bool)false;
			data["error"] = WifiStation.getConnectionStatusName();
			json["success"] = (bool)true;
			WifiStation.config("", "");
			WifiStation.disconnect();
		}
	}
	response.setAllowCrossDomainOrigin("*");
	response.sendJsonObject(stream);
}

void onSystemReq(HttpRequest &request, HttpResponse &response) {
	if(!authenticated(request, response)) return;
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();
	json["success"] = (bool)false;
	if (request.getRequestMethod() == RequestMethod::POST)
	{
		if (request.getBody() == NULL)
		{
			debugf("NULL bodyBuf");
		}
		else
		{
			DynamicJsonBuffer jsonBuffer;
			JsonObject& root = jsonBuffer.parseObject(request.getBody());
			if(root["cmd"].success()) {

				String cmd = root["cmd"].asString();
				if(cmd.indexOf("reset") != -1) {
					cfg.reset();
					WifiStation.config("", "");
					systemTimer.initializeMs(3000, restart).startOnce();
					json["success"] = true;
				}
				else if(cmd.indexOf("restart") != -1) {
					systemTimer.initializeMs(3000, restart).startOnce();
					json["success"] = true;
				}
				else if (cmd.indexOf("forget_wifi") != -1){
					WifiStation.config("", "");
					cfg.network.connection.password = "";
					cfg.network.connection.ssid = "";
					cfg.save();
					systemTimer.initializeMs(3000, restart).startOnce();
					json["success"] = true;
				}
			}
		}
	}
	if (json["success"]) {
		json["error"] = "wrong command";
	}
	response.setAllowCrossDomainOrigin("*");
	response.sendJsonObject(stream);
}

void onUpdate(HttpRequest &request, HttpResponse &response) {
	if(!authenticated(request, response)) return;
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();
	JsonObject& data = json.createNestedObject("data");
	json["success"] = (bool)false;
	if (request.getRequestMethod() == RequestMethod::POST)
	{
		if (request.getBody() == NULL)
		{
			debugf("NULL bodyBuf");
		}
		else
		{
			DynamicJsonBuffer jsonBuffer;
			JsonObject& root = jsonBuffer.parseObject(request.getBody());
			bool force = false;
			bool updating = false;
			if (root["force"].success()) {
				force = root["force"];
			}
			if(root["rom"].success()) {
				if(force) {
					//TODO: add to download item list
				}
			}
			if (root["resources"].success()){


			}
			if (updating) {
				data["updating"] = true;
				//start updatetimer
			}
			else
			{
				data["updating"] = false;
			}
			json["success"] = true;
		}
	} else {
		json["success"] = true;
		//return update status
	}

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
	server.addPath("/color", onColor);
	server.addPath("/get-networks", onNetworkList);
	server.addPath("/refreshnetworks", onRefreshNetworkList);
	server.addPath("/system", onSystemReq);
	server.addPath("/update", onUpdate);
	server.addPath("/connect", onConnect);
}


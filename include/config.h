/*
 *
 * AppSettings.h
 *
 */
#include <SmingCore/SmingCore.h>

#ifndef CONFIG_H_
#define CONFIG_H_

#define APP_COLOR_FILE ".color"
#define APP_SETTINGS_FILE ".cfg"

#define CFG_VERSION "1"

struct ActiveColorStorage
{
		struct hsvkcolor {
			int h = 0;
			int s = 0;
			int v = 0;
			int k = 0;
		};

		hsvkcolor color;

		void load(bool print = false)
		{
			StaticJsonBuffer<72> jsonBuffer;
			if (exist())
			{
				int size = fileGetSize(APP_COLOR_FILE);
				char* jsonString = new char[size + 1];
				fileGetContent(APP_COLOR_FILE, jsonString, size + 1);
				JsonObject& root = jsonBuffer.parseObject(jsonString);
				color.h = root["h"];
				color.s = root["s"];
				color.v = root["v"];
				color.k = root["k"];
				if (print) {
					root.prettyPrintTo(Serial);
				}
				delete[] jsonString;
			}
		}

		void save(bool print = false)
		{
			DynamicJsonBuffer jsonBuffer;
			JsonObject& root = jsonBuffer.createObject();
			root["h"] = color.h;
			root["s"] = color.s;
			root["v"] = color.v;
			root["k"] = color.k;
			String rootString;
			if (print) {
				root.prettyPrintTo(Serial);
			}
			root.printTo(rootString);
			fileSetContent(APP_COLOR_FILE, rootString);
		}

		bool exist() { return fileExist(APP_COLOR_FILE); }
};


struct ApplicationSettingsStorage
{
	struct network {

		struct connection {
			String ssid;
			String password;
			String mdnshostname;
			bool dhcp = true;
			IPAddress ip;
			IPAddress netmask;
			IPAddress gateway;
		};

		struct udpserver {
			bool 	enabled = false;
			int	 	port = DEFAULT_UDP_PORT;
		};

		struct tcpserver {
			bool 	enabled = false;
			int		port = DEFAULT_TCP_PORT;
		};

		struct mqtt {
			bool	enabled;
			String 	server;
			int		port;
			String	username;
			String 	password;
		};

		struct ap {
			bool secured = DEFAULT_AP_SECURED;
			String ssid;
			String password = DEFAULT_AP_PASSWORD;
		};

		connection connection;

		udpserver udpserver;
		tcpserver tcpserver;
		mqtt mqtt;
		ap ap;

	};



	struct color {
		struct hsv {
			int model = 0;
			float red = 0;
			float yellow = 0;
			float green = 0;
			float cyan = 0;
			float blue = 0;
			float magenta = 0;
		};

		struct brightness {
			int red = 100;
			int green = 100;
			int blue = 100;
			int ww = 100;
			int cw = 100;
		};

		struct colortemp {
			int ww = DEFAULT_COLORTEMP_WW;
			int cw = DEFAULT_COLORTEMP_CW;
		};

		hsv hsv;
		brightness brightness;
		colortemp colortemp;
		int outputmode = 0;
	};

	struct general {
		bool	settings_secured = DEFAULT_SETTINGS_SECURED;
		String	settings_password = DEFAULT_SETTINGS_PASSWORD;
	};

	general general;
	network network;
	color color;
	String configversion = CFG_VERSION;


	void load(bool print = false)
	{
		DynamicJsonBuffer jsonBuffer;
		if (exist())
		{
			int size = fileGetSize(APP_SETTINGS_FILE);
			char* jsonString = new char[size + 1];
			fileGetContent(APP_SETTINGS_FILE, jsonString, size + 1);
			JsonObject& root = jsonBuffer.parseObject(jsonString);

			//connection
			network.connection.ssid = root["network"]["connection"]["ssid"].asString();
			network.connection.password = root["network"]["connection"]["password"].asString();
			network.connection.mdnshostname = root["network"]["connection"]["hostname"].asString();
			network.connection.dhcp = root["network"]["connection"]["dhcp"];
			network.connection.ip = root["network"]["connection"]["ip"].asString();
			network.connection.netmask = root["network"]["connection"]["netmask"].asString();
			network.connection.gateway = root["network"]["connection"]["gateway"].asString();

			//accesspoint
			network.ap.secured = root["network"]["ap"]["secured"];
			network.ap.ssid = root["network"]["ap"]["ssid"].asString();
			network.ap.password  = root["network"]["ap"]["password"].asString();

			//tcp
			network.tcpserver.enabled  = root["network"]["tcpserver"]["enabled"];
			network.tcpserver.port = root["network"]["tcpserver"]["port"];
			//udp
			network.udpserver.enabled  = root["network"]["udpserver"]["enabled"];
			network.udpserver.port = root["network"]["udpserver"]["port"];

			//mqtt
			network.mqtt.enabled = root["network"]["mqtt"]["enabled"];
			network.mqtt.server = root["network"]["mqtt"]["server"].asString();
			network.mqtt.port = root["network"]["mqtt"]["port"];
			network.mqtt.username  = root["network"]["mqtt"]["username"].asString();
			network.mqtt.password  = root["network"]["mqtt"]["password"].asString();

			//color
			color.outputmode = root["color"]["outputmode"];

			//hsv
			color.hsv.model = root["color"]["hsv"]["model"];
			color.hsv.red = root["color"]["hsv"]["red"];
			color.hsv.yellow = root["color"]["hsv"]["yellow"];
			color.hsv.green = root["color"]["hsv"]["green"];
			color.hsv.cyan = root["color"]["hsv"]["cyan"];
			color.hsv.blue = root["color"]["hsv"]["blue"];
			color.hsv.magenta = root["color"]["hsv"]["magenta"];

			//brightness
			color.brightness.red = root["color"]["brightness"]["red"];
			color.brightness.green = root["color"]["brightness"]["green"];
			color.brightness.blue = root["color"]["brightness"]["blue"];
			color.brightness.ww = root["color"]["brightness"]["ww"];
			color.brightness.cw = root["color"]["brightness"]["cw"];

			//general
			general.settings_password = root["general"]["settings_password"].asString();
			general.settings_secured = root["general"]["settings_secured"];

			//TODO check if we can actually load the config
			configversion = root["general"]["config_version"].asString();
			if (print) {
				root.prettyPrintTo(Serial);
			}

			delete[] jsonString;
		}

	}

	void save(bool print = false)
	{
		DynamicJsonBuffer jsonBuffer;
		JsonObject& root = jsonBuffer.createObject();

		JsonObject& net = root.createNestedObject("network");
		JsonObject& con = net.createNestedObject("connection");
		con["ssid"] = network.connection.ssid.c_str();
		con["password"] = network.connection.password.c_str();
		con["dhcp"] = network.connection.dhcp;
		con["ip"] = network.connection.ip.toString();
		con["netmask"] = network.connection.netmask.toString();
		con["gateway"] = network.connection.gateway.toString();
		con["mdnhostname"] = network.connection.mdnshostname.c_str();

		JsonObject& jap = net.createNestedObject("ap");
		jap["secured"] = network.ap.secured;
		jap["ssid"] = network.ap.ssid.c_str();
		jap["password"] = network.ap.password.c_str();

		JsonObject& jmqtt = net.createNestedObject("mqtt");
		jmqtt["enabled"] = network.mqtt.enabled;
		jmqtt["server"] = network.mqtt.server.c_str();
		jmqtt["port"] = network.mqtt.port;
		jmqtt["username"] = network.mqtt.username.c_str();
		jmqtt["password"] = network.mqtt.password.c_str();

		JsonObject& judp = net.createNestedObject("udpserver");
		judp["enabled"] = network.udpserver.enabled;
		judp["port"] = network.udpserver.port;

		JsonObject& jtcp = net.createNestedObject("tcpserver");
		jtcp["enabled"] = network.tcpserver.enabled;
		jtcp["port"] = network.tcpserver.port;

		JsonObject& c = root.createNestedObject("color");
		c["outputmode"] = color.outputmode;

		JsonObject& h = c.createNestedObject("hsv");
		h["model"] = color.hsv.model;
		h["red"] = color.hsv.red;
		h["yellow"] = color.hsv.yellow;
		h["green"] = color.hsv.green;
		h["cyan"] = color.hsv.cyan;
		h["blue"] = color.hsv.blue;
		h["magenta"] = color.hsv.magenta;

		JsonObject& b = c.createNestedObject("brightness");
		b["red"] = color.brightness.red;
		b["green"] = color.brightness.green;
		b["blue"] = color.brightness.blue;
		b["ww"] = color.brightness.ww;
		b["cw"] = color.brightness.cw;

		JsonObject& t = c.createNestedObject("colortemp");
		t["ww"] = color.colortemp.ww;
		t["cw"] = color.colortemp.cw;

		JsonObject& g = jsonBuffer.createObject();
		root["general"] = g;
		g["settings_secured"] = general.settings_secured;
		g["settings_password"] = general.settings_password;
		g["config_version"] = CFG_VERSION;

		String rootString;
		if (print) {
			root.prettyPrintTo(Serial);
		}
		root.printTo(rootString);
		fileSetContent(APP_SETTINGS_FILE, rootString);

	}

	bool exist() { return fileExist(APP_SETTINGS_FILE); }

	void reset() {
		if (exist()) {
			fileDelete(APP_SETTINGS_FILE);
		}
	}
};



#endif /* INCLUDE_APPSETTINGS_H_ */

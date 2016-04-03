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


ApplicationMQTTClient::ApplicationMQTTClient() {
	mqtt = NULL;
}


ApplicationMQTTClient::~ApplicationMQTTClient() {
	// cleanup before destroying object
	if(mqtt != NULL) {
		delete mqtt;
	}
}


void ApplicationMQTTClient::start(){
	Serial.println("Start MQTT");
	if (mqtt != NULL) {
		 delete mqtt;
	}
	//TODO: add settings from config
	mqtt = new MqttClient("192.168.1.1", MqttStringSubscriptionCallback(&ApplicationMQTTClient::onMessageReceived, this));
}

void ApplicationMQTTClient::stop() {
	 delete mqtt;
	 mqtt = NULL;
}

bool ApplicationMQTTClient::isRunning() {
	return (mqtt != NULL);
}

void ApplicationMQTTClient::onMessageReceived(String topic, String message) {
	Serial.print(topic);
	Serial.print(":\r\n\t"); // Prettify alignment for printing
	Serial.println(message);
}

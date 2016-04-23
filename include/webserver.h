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
#ifndef APP_WEBSERVER_H_
#define APP_WEBSERVER_H_


enum API_CODES {
	API_SUCCESS = 0,
	API_BAD_REQUEST = 1,
	API_MISSING_PARAM = 2,
	API_UNAUTHORIZED = 3,
};


class ApplicationWebserver: private HttpServer
{
public:
	ApplicationWebserver();
	virtual ~ApplicationWebserver() {};

	void start();
	void stop();
	void init();
	bool isRunning();

	String getApiCodeMsg(API_CODES code);

private:
	bool _init = false;
	bool _running = false;

private:
	bool authenticated(HttpRequest &request, HttpResponse &response);
	void onFile(HttpRequest &request, HttpResponse &response);
	void onIndex(HttpRequest &request, HttpResponse &response);
	void onWebapp(HttpRequest &request, HttpResponse &response);
	void onConfig(HttpRequest &request, HttpResponse &response);
	void onInfo(HttpRequest &request, HttpResponse &response);
	void onColor(HttpRequest &request, HttpResponse &response);
	void onAnimation(HttpRequest &request, HttpResponse &response);
	void onNetworks(HttpRequest &request, HttpResponse &response);
	void onScanNetworks(HttpRequest &request, HttpResponse &response);
	void onSystemReq(HttpRequest &request, HttpResponse &response);
	void onUpdate(HttpRequest &request, HttpResponse &response);
	void onConnect(HttpRequest &request, HttpResponse &response);
	void generate204(HttpRequest &request, HttpResponse &response);
	void onPing(HttpRequest &request, HttpResponse &response);
	void sendApiResponse(HttpResponse &response, JsonObjectStream* stream, int code = 200);
	void sendApiCode(HttpResponse &response, API_CODES code, String msg = "");
};

#endif // APP_WEBSERVER_H_

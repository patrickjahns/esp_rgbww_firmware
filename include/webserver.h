#ifndef WEBSERVER_H_
#define WEBSERVER_H_


enum API_ERR_CODES {
	BAD_REQUEST = 0,
	MISSING_PARAM = 1,
	NUM = 2
};

String apiErrMsg(API_ERR_CODES code);
void stopWebServer();
void startWebServer();


#endif // WEBSERVER_H_

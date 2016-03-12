#ifndef VERSION_H_
#define VERSION_H_
#define	APP_VERSION "0.5"

extern const char * fw_git_sha;
extern const char * fw_git_date;
extern const char * fw_version;

// connection status
enum CONNECTION_STATUS {
	IDLE = 0,
	CONNECTING = 1,
	CONNECTED = 2,
	ERR = 3
};


// webservice
enum API_ERR_CODES {
	BAD_REQUEST = 0,
	MISSING_PARAM = 1,
	NUM = 2
};


#endif /* VERSION_H_ */

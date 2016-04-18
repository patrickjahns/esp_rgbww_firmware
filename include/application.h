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
#ifndef APPLICATION_H_
#define APPLICATION_H_


static const char* fw_version = FWVERSION;
static const char* fw_git_version = GITVERSION;
static const char* fw_git_date = GITDATE;

// main forward declarations
class Application {

public:
	void init();
	void loop();

	void startServices();
	void stopServices();
	void reset();
	void restart();

	void clearButton();

	bool delayedCMD(String cmd, int delay);
	bool isFirstRun();


public:
	AppWIFI network;
	ApplicationWebserver webserver;
	APPLedCtrl rgbwwctrl;
	ApplicationOTA ota;
	ApplicationSettings cfg;


private:
	Timer _systimer;
	bool _first_run = false;
};
// forward declaration for global vars
extern Application app;


#endif // APPLICATION_H_

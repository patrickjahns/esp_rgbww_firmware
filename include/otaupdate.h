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
#ifndef OTAUPDATE_H_
#define OTAUPDATE_H_

enum OTASTATUS {
	OTA_NOT_UPDATING = 0,
	OTA_PROCESSING = 1,
	OTA_SUCCESS = 2,
	OTA_FAILED = 3
};


class ApplicationOTA
{
public:

	void start(String romurl, String spiffsurl);
	OTASTATUS getStatus();

	bool isProccessing();

protected:
	rBootHttpUpdate* otaUpdater;
	uint8 rom_slot;
	OTASTATUS fwstatus = OTASTATUS::OTA_NOT_UPDATING;

protected:
	void rBootCallback(bool result);
	void reset();
	void beforeOTA();
	void afterOTA();
};





#endif // OTAUPDATE_H_

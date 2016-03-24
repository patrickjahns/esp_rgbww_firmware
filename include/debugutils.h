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
#ifndef APP_DEBUGUTILS_H
#define APP_DEBUGUTILS_H

#ifdef DEBUG_APP
	#if DEBUG_APP == 1
		#define debugapp(fmt, ...) Serial.printf(fmt"\r\n", ##__VA_ARGS__)
	#else
		#define debugapp(...)
	#endif // DEBUG_APP
#else
	#define debugapp(...)
#endif // DEBUG_APP

#endif //APP_DEBUGUTILS_H

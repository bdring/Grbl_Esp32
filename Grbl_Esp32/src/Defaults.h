#pragma once

/*
  Defaults.h - defaults settings configuration file
  Part of Grbl

  Copyright (c) 2012-2016 Sungeun K. Jeon for Gnea Research LLC

	2018 -	Bart Dring This file was modifed for use on the ESP32
					CPU. Do not use this with Grbl for atMega328P

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
    All of these settings check to see if they have been defined already
    before defining them. This allows to to easily set them eslewhere.
    You only need to set ones that are important or unique to your
    machine. The rest will be pulled from here.
    */

// Grbl generic default settings. Should work across different machines.
#ifndef DEFAULT_USER_MACRO0
#    define DEFAULT_USER_MACRO0 ""
#endif

#ifndef DEFAULT_USER_MACRO1
#    define DEFAULT_USER_MACRO1 ""
#endif

#ifndef DEFAULT_USER_MACRO2
#    define DEFAULT_USER_MACRO2 ""
#endif

#ifndef DEFAULT_USER_MACRO3
#    define DEFAULT_USER_MACRO3 ""
#endif

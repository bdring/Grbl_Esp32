/*
  commands.h - ESP3D configuration class

  Copyright (c) 2014 Luc Lebosse. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef COMMANDS_h
#define COMMANDS_h
#include "config.h"

//Authentication level
typedef enum {
    LEVEL_GUEST = 0,
    LEVEL_USER = 1,
    LEVEL_ADMIN = 2
} level_authenticate_type;

// Define line flags. Includes comment type tracking and line overflow detection.
#define LINE_FLAG_OVERFLOW bit(0)
#define LINE_FLAG_COMMENT_PARENTHESES bit(1)
#define LINE_FLAG_COMMENT_SEMICOLON bit(2)

class ESPResponseStream;


class COMMANDS
{
public:
    static bool check_command (const char *, int * cmd, String & cmd_params);
    static String get_param (String & cmd_params, const char * id, bool withspace);
    static bool execute_internal_command  (int cmd, String cmd_params, level_authenticate_type auth_level = LEVEL_GUEST ,  ESPResponseStream  *espresponse= NULL);
    static void wait(uint32_t milliseconds);
    static void handle();
    static void restart_ESP();
#ifdef ENABLE_AUTHENTICATION
    static bool isadmin (String & cmd_params);
    static bool isuser (String & cmd_params);
    static bool isLocalPasswordValid (const char * password);
#endif
    private :
    static bool restart_ESP_module;
};

#endif

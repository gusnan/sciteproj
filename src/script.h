/**
 * script.h - Script code for sciteproj
 *
 *  Copyright 2012-2020 Andreas Rönnquist
 *
 * This file is part of SciteProj.
 *
 * SciteProj is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SciteProj is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SciteProj.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef __HEADER_SCRIPT_
#define __HEADER_SCRIPT_

/**
 *
 */
lua_State *init_script ();
void register_cfunctions (lua_State *lua);
int load_script (lua_State *lua, char *filename);
void run_script (lua_State *lua);
void done_script (lua_State *lua);

int load_script_buffer (lua_State *lua, const char *buffer);

int lua_get_boolean (lua_State *lua, char *variable_name);
double lua_get_number (lua_State *lua, char *variable_name);

gboolean lua_global_exists (lua_State *lua, char *variable_name);

GSList *load_filter_from_lua ();

#endif /*__HEADER_SCRIPT_*/

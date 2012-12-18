/**
 * script.c - Script code for sciteproj
 *
 *  Copyright 2012 Andreas Rönnquist
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
 
#include <gtk/gtk.h>

#include <glib.h>
#include <glib/gi18n.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <locale.h>

#include "script.h"

#include "tree_manipulation.h"

/**
 *
 */
lua_State *
init_script()
{
	lua_State *lua=luaL_newstate();
	luaL_openlibs(lua);

	register_cfunctions(lua);

	return lua;
}


/**
 *
 */
void
register_cfunctions(lua_State *lua)
{

}


/**
 *
 */
int
load_script(lua_State *lua,char *filename)
{
	if (lua) {
		int result=luaL_loadfile(lua,filename);

		if (!result) {
			//script_loaded=TRUE;
		} else {

			// We got an error, print it
			printf("%s\n",lua_tostring(lua,-1));

			lua_pop(lua,1);

			return -1;
		}
	} else {
		return -1;
	}

	return 0;
}


/**
 *
 */
void
run_script(lua_State *lua)
{
	int s = lua_pcall( lua, 0, LUA_MULTRET, 0 );

	if (s) {

		char *error_msg;

		error_msg=(char*)lua_tostring( lua, -1 );

		//std::string luaErrorString=getLuaErrorString(s);

		/*
		mssOut.str( "" );
		mssOut << "Script::runScript : Error caught running script "
			<< sScriptName << "\n"
			<< "    Error code is " << luaErrorString << ".\n"
			<< "    Error msg is: " << errorMsg;
		throw( ScriptException( mssOut.str().c_str(), errorMsg ) );
		*/

		printf(_("Error: %s"),error_msg);
		printf("\n");
	}
}


/**
 *
 */
void
done_script(lua_State *lua)
{
	if (lua)
		lua_close(lua);
}


/**
 *
 */
GSList *load_filter_from_lua()
{
	gchar *script_filename=g_build_filename(get_project_directory(),"sciteprojrc.lua",NULL);
	lua_State *lua=NULL;
	GSList *list=NULL;

	if (g_file_test(script_filename,G_FILE_TEST_EXISTS)) {
		
		lua=init_script();
		
		if (load_script(lua,script_filename)!=0) {
			goto EXITPOINT;
		}
		
		run_script(lua);
		
		lua_getglobal(lua, "hide_filter");
		
		if (!lua_istable(lua, -1)) {
			printf("Isn't a table!\n");
			goto EXITPOINT;
		}
		
		lua_pushnil(lua);
		
		while(lua_next(lua,-2)) {
			if (lua_isstring(lua, -1)) {
				char *temp=(char *)lua_tostring(lua, -1);

				list=g_slist_append(list,g_strdup(temp));
			}
			lua_pop(lua,1);
		}
		lua_pop(lua,1);
	}
	
EXITPOINT:
	g_free(script_filename);

	if (lua)
		done_script(lua);

	return list;		
}

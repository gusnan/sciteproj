/**
 * sort.c - Helpers for sorting
 *
 *  Copyright 2012-2016 Andreas Rönnquist
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
#include <glib.h>
#include <gtk/gtk.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "file_utils.h"
#include "sort.h"
#include "clicked_node.h"
#include "gui.h"
#include "tree_manipulation.h"

#include "script.h"

/**
 *
 */
gint compare_strings_bigger(gconstpointer a, gconstpointer b)
{
	const gchar *test1 = get_filename_from_full_path((gchar*)a);
	const gchar *test2 = get_filename_from_full_path((gchar*)b);

	return g_ascii_strcasecmp(test1, test2);
}


/**
 *
 */
gint compare_strings_smaller(gconstpointer a, gconstpointer b)
{
	const gchar *test1 = get_filename_from_full_path((gchar*)a);
	const gchar *test2 = get_filename_from_full_path((gchar*)b);

	return g_ascii_strcasecmp(test2, test1);
}



/**
 *
 */
gint file_sort_by_extension_bigger_func(gconstpointer a, gconstpointer b)
{
	gint result=0;

	gchar *filename1 = (gchar*)a;
	gchar *filename2 = (gchar*)b;

	gchar *ext1 = get_file_extension(filename1);
	gchar *ext2 = get_file_extension(filename2);

	result = g_ascii_strcasecmp(ext1, ext2);
	if (result == 0) {
		result = g_ascii_strcasecmp(filename1, filename2);
	}

	return result;
}


/**
 *
 */
gint file_sort_by_extension_smaller_func(gconstpointer a, gconstpointer b)
{
	gint result=0;

	gchar *filename1 = (gchar*)a;
	gchar *filename2 = (gchar*)b;

	gchar *ext1 = get_file_extension(filename1);
	gchar *ext2 = get_file_extension(filename2);

	result = g_ascii_strcasecmp(ext2, ext1);
	if (result == 0) {
		result = g_ascii_strcasecmp(filename2, filename1);
	}

	return result;
}


/**
 *
 */
void sort_ascending_cb()
{
	GError *err = NULL;

	if (clicked_node.valid && clicked_node.type == ITEMTYPE_FILE) {
		goto EXITPOINT;
	}

	sort_children(&(clicked_node.iter), &err, compare_strings_smaller);

EXITPOINT:
	//
	if (err) g_error_free(err);
}


/**
 *
 */
void sort_descending_cb()
{
	GError *err = NULL;

	if (clicked_node.valid && clicked_node.type == ITEMTYPE_FILE) {
		goto EXITPOINT;
	}

	sort_children(&clicked_node.iter, &err, compare_strings_bigger);


EXITPOINT:
	//
	if (err) g_error_free(err);
}


/**
 *
 */
void sort_ascending_by_extension_cb()
{
	GError *err = NULL;

	if (clicked_node.valid && clicked_node.type == ITEMTYPE_FILE) {
		goto EXITPOINT;
	}

	sort_children(&clicked_node.iter, &err, file_sort_by_extension_smaller_func);

EXITPOINT:
	if (err) g_error_free(err);
}


/**
 *
 */
void sort_descending_by_extension_cb()
{
	GError *err = NULL;

	if (clicked_node.valid && clicked_node.type == ITEMTYPE_FILE) {
		goto EXITPOINT;
	}

	sort_children(&clicked_node.iter, &err, file_sort_by_extension_bigger_func);

EXITPOINT:
	if (err) g_error_free(err);

}


/**
 *
 */
GCompareFunc get_sort_order_of_folder(gchar *folder_name)
{
	// We default to compare_strings_smaller
	GCompareFunc result = compare_strings_smaller;

	gchar *script_filename = g_build_filename(get_project_directory(),
	                         "sciteprojrc.lua", NULL);

	lua_State *lua = NULL;

	if (g_file_test(script_filename, G_FILE_TEST_EXISTS)) {
		int num = -1;
		lua = init_script();

		if (load_script(lua, script_filename) != 0) {
			printf("error loading script: %s\n", script_filename);
			goto EXITPOINT;
		}

		run_script(lua);

		lua_getglobal(lua, "sort_order");

		// Make sure that we have a value at all
		if (lua_isnil(lua, -1)) {
			goto EXITPOINT;
		}

		// and make sure it really is a table
		if (!lua_istable(lua, -1)) {
			printf("sort_order is expected to be a table!\n");
			goto EXITPOINT;
		}

		lua_pushnil(lua);

		while(lua_next(lua, -2)) {

			gchar *key = NULL;

			if (lua_type(lua, -2) == LUA_TSTRING) { // key type is string

				key = g_strdup(lua_tostring(lua, -2));

				gchar *temp = clean_folder(key);

				g_free(key);

				key = temp;

				//printf(" key: %s\n", key);
			}

			if (lua_type(lua, -1) == LUA_TNUMBER) { // value is number

				num = lua_tonumber(lua, -1);

				//printf(" value: %d\n", num);
			}

			/* gboolean abs_path_to_relative_path(const gchar *absPath,
																gchar **relativePath,
																const gchar *basePath,
																GError **err);
			// convert the absolute path to a relative path
			*/

			gchar *relative_path = NULL;

			if (g_strcmp0(folder_name, get_project_directory()) == 0) {
				relative_path = g_strdup("."); //get_project_directory();
			}

			if (!relative_path) {
				if (abs_path_to_relative_path(folder_name,
				                              &relative_path,
				                              get_project_directory(),
				                              NULL)) {

					//printf("relative_path: %s\n", relative_path);
				}
			}

			//printf("Relative path: %s\n", relative_path);

			gchar *folder_cleaned = clean_folder(relative_path);

			//printf("folder_cleaned: %s\n", folder_cleaned);

			if (g_strcmp0(key, folder_cleaned) == 0) {

				switch (num)
				{
				case 0:
					result = compare_strings_smaller;
					break;
				case 1:
					result = compare_strings_bigger;
					break;
				case 2:
					result = file_sort_by_extension_smaller_func;
					break;
				case 3:
					result = file_sort_by_extension_bigger_func;
					break;
				};
			}

			if (key) g_free(key);

			if (relative_path) g_free(relative_path);
			if (folder_cleaned) g_free(folder_cleaned);

			lua_pop(lua, 1);
		}
		lua_pop(lua,1);
	}

EXITPOINT:

	g_free(script_filename);

	if (lua)
		done_script(lua);

	return result;
}
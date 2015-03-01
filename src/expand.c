/**
 * expand.c - expand a folder in the treeview
 *
 *	 Copyright 2012 Andreas Rönnquist
 *
 * This file is part of SciteProj.
 *
 *	devilspie2 is free software: you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as published
 *	by the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	devilspie2 is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with devilspie2.
 *	If not, see <http://www.gnu.org/licenses/>.
 */
#include <glib.h>
#include <gtk/gtk.h>

#include <string.h>
#include <sys/stat.h>
#include <glib/gi18n.h>

#include <locale.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "clicked_node.h"
#include "gui.h"
#include "tree_manipulation.h"

#include "file_utils.h"

#include "script.h"

#include "expand.h"


/**
 *
 */
void expand_tree(GtkTreeModel *tree_model, GtkTreeIter *start_iter)
{
    GtkTreeIter child_iter;

    gint type;
    gchar *filepath;

    if (start_iter) {
        if (gtk_tree_model_iter_children(tree_model,&child_iter,start_iter)) {

            do {

                gtk_tree_model_get(tree_model, &child_iter,
                                   COLUMN_ITEMTYPE, &type,
                                   COLUMN_FILEPATH, &filepath, -1);

                if (type==ITEMTYPE_GROUP) {

                    // Check if we should expand this row
                    //printf("filepath :%s\n", filepath);

                    GtkTreePath *path=gtk_tree_model_get_path(tree_model, &child_iter);

                    if (get_expand_folder(filepath)) {
                        expand_tree_row(path, FALSE);
                        expand_tree(tree_model, &child_iter);
                    }
                }
            } while (gtk_tree_model_iter_next(tree_model, &child_iter));
        }
    }
}


/**
 *
 */
void start_expand_tree(GtkTreeModel *tree_model, GtkTreeIter *iter)
{
    if (iter)
        expand_tree(tree_model, iter);
}


/**
 *
 */
gboolean get_expand_folder(gchar *folder_name)
{
    gboolean result=FALSE;

    gchar *script_filename = g_build_filename(get_project_directory(), "sciteprojrc.lua", NULL);

    lua_State *lua=NULL;


    if (g_file_test(script_filename, G_FILE_TEST_EXISTS)) {

        int num=-1;

        lua=init_script();

        if (load_script(lua, script_filename)!=0) {
            printf("error loading script: %s\n", script_filename);
            goto EXITPOINT;
        }

        run_script(lua);

        lua_getglobal(lua, "open_folders");

        // Make sure we have a value at all
        if (lua_isnil(lua, -1)) {
            goto EXITPOINT;
        }

        // And make sure that it is a table
        if (!lua_istable(lua,-1)) {
            printf("open_folders is supposed to be a table!\n");
            goto EXITPOINT;
        }

        lua_pushnil(lua);

        while(lua_next(lua, -2)) {

            gchar *key=NULL;

            if (lua_type(lua, -2)==LUA_TSTRING) { // key type is string

                key=g_strdup(lua_tostring(lua, -2));
                gchar *temp=clean_folder(key);
                g_free(key);
                key=temp;
            }

            if (lua_type(lua, -1)==LUA_TBOOLEAN) { // value is boolean

                num = lua_toboolean(lua, -1);
            }

            // gboolean abs_path_to_relative_path(const gchar *absPath, gchar **relativePath, const gchar *basePath, GError **err);
            // convert the absolute path to a relative path

            gchar *relative_path=NULL;

            if (g_strcmp0(folder_name, get_project_directory())==0) {
                relative_path=g_strdup("."); //get_project_directory();
            }

            if (!relative_path) {
                if (abs_path_to_relative_path(folder_name, &relative_path, get_project_directory(), NULL)) {

                }
            }

            gchar *folder_cleaned=clean_folder(relative_path);

            if (g_strcmp0(key, folder_cleaned)==0) {

                result=FALSE;
                if (num) result=TRUE;
            }

            if (key) g_free(key);

            if (relative_path) g_free(relative_path);
            if (folder_cleaned) g_free(folder_cleaned);
            lua_pop(lua, 1);
        }
        lua_pop(lua, 1);

    }


EXITPOINT:

    g_free(script_filename);

    if (lua)
        done_script(lua);


    return result;
}

/**
 * sort.c - Helpers for sorting
 *
 *  Copyright 2012-2013 Andreas Rönnquist
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

#include "file_utils.h"
#include "sort.h"
#include "clicked_node.h"
#include "gui.h"
#include "tree_manipulation.h"

/**
 *
 */
gint compare_strings_bigger(gconstpointer a,gconstpointer b)
{
	const gchar *test1=get_filename_from_full_path((gchar*)a);
	const gchar *test2=get_filename_from_full_path((gchar*)b);

	return g_ascii_strcasecmp(test1,test2);
}


/**
 *
 */
gint compare_strings_smaller(gconstpointer a,gconstpointer b)
{
	const gchar *test1=get_filename_from_full_path((gchar*)a);
	const gchar *test2=get_filename_from_full_path((gchar*)b);

	return g_ascii_strcasecmp(test2,test1);
}


/**
 *
 */
void sort_ascending_cb()
{
	GError *err = NULL;

	if (clicked_node.valid && clicked_node.type==ITEMTYPE_FILE) {
		goto EXITPOINT;
	}

	sort_children(&(clicked_node.iter),&err,compare_strings_smaller);

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

	if (clicked_node.valid && clicked_node.type==ITEMTYPE_FILE) {
		goto EXITPOINT;
	}

	sort_children(&clicked_node.iter,&err,compare_strings_bigger);


EXITPOINT:
	//
	if (err) g_error_free(err);
}


/**
 *
 */
void sort_ascending_by_extension_cb()
{
	GError *err=NULL;
	
	if (clicked_node.valid && clicked_node.type==ITEMTYPE_FILE) {
		goto EXITPOINT;
	}
	
	sort_children(&clicked_node.iter,&err,compare_strings_smaller);
	
EXITPOINT:
	if (err) g_error_free(err);
}


/**
 *
 */
void sort_descending_by_extension_cb()
{
	GError *err=NULL;
	
	if (clicked_node.valid && clicked_node.type==ITEMTYPE_FILE) {
		goto EXITPOINT;
	}
	
	sort_children(&clicked_node.iter,&err,compare_strings_bigger);
	
EXITPOINT:
	if (err) g_error_free(err);

}

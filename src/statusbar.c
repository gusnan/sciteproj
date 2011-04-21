/**
 * statusbar.c - statusbar for SciteProj
 *
 *  Copyright 2009-2011 Andreas Rönnquist
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

#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "statusbar.h"

#define APP_SCITEPROJ_ERROR g_quark_from_static_string("APP_GUI_ERROR")

GtkWidget *statusbar=NULL;

guint context_id;


/**
 *		init_statusbar
 */
gboolean init_statusbar(GtkWidget *widget,GError **err)
{
	statusbar=gtk_statusbar_new();
	
	if (!statusbar) {
		g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Could not init statusbar", __func__);
		return FALSE;
	}
	
	gtk_statusbar_set_has_resize_grip(GTK_STATUSBAR(statusbar),TRUE);
	
	gtk_widget_set_size_request(statusbar, 1, -1);

	context_id=gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar),"Info");
	
	set_statusbar_text("Welcome to SciteProj\n");
	
	gtk_widget_set_size_request(statusbar, -1, 20);
	
	gtk_box_pack_end (GTK_BOX (widget), statusbar,FALSE, FALSE, 0);
	gtk_widget_show (statusbar);

	
	return TRUE;
}


/**
 *		set_statusbar_text
 */
void set_statusbar_text(const gchar *text)
{
	int co=0;
	// new string - fill it with characters from text indata, but skip
	// non-showable characters.
	
	gchar *newstring=(gchar*)(g_malloc((int)(strlen(text)+1)));
	
	int newco=0;
	for (co=0;co<(int)strlen(text);co++) {
		if (text[co]!='\n') {
			newstring[newco]=text[co];
			newco++;
		}
	}
	
	newstring[newco]='\0';
	
	// Pop what message that was previously on the statusbar stack
	gtk_statusbar_pop(GTK_STATUSBAR(statusbar),context_id);
	
	// Push the new message (the statusbar will show the message that
	// is on top of the statusbar stack, the one pushed will be shown)
	// We popped the last one, because we don't take advantage of the 
	// context_id system of the statusbar.
	gtk_statusbar_push(GTK_STATUSBAR(statusbar),context_id,newstring);
	
	g_free(newstring);
}


/**
 *		done_statusbar
 */
void done_statusbar()
{
	if (statusbar!=NULL) gtk_widget_destroy(statusbar);
}

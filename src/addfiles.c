/**
 * addfiles.c - Interface for adding files to the project
 *
 *  Copyright 2011-2012 Andreas Rönnquist
 *
 * This file is part of SciteProj
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
#include <glib-object.h>
#include <glib/gi18n.h>

#include <locale.h>

#include "clicked_node.h"

#include "gui.h"
#include "tree_manipulation.h"

#include "addfiles.h"




/**
 * Callback for menu manager to populate GUI widgets
 *
 * @param ui is the GtkUIManager
 * @param widget is the GtkWidget to add to the UI
 * @param container is the container to add widget to
 */
void menu_add_widget_cb(GtkUIManager *ui, GtkWidget *widget, GtkContainer *container)
{
    // use Grid instead of box packing on GTK3
#if GTK_MAJOR_VERSION>=3
    gtk_grid_attach(GTK_GRID(container),widget,0,0,1,1);
#else
    gtk_box_pack_start(GTK_BOX(container), widget, FALSE, FALSE, 0);
#endif
    gtk_widget_show(widget);
}


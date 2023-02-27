/**
 * clicked_node.h - clicked node struct for SciteProj
 *
 *  Copyright 2006 Roy Wood, 2009-2023 Andreas Rönnquist
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
#include <glib/gi18n.h>

#include "clicked_node.h"

ClickedNode *create_clicked_node(gboolean valid, GtkTreeIter iter, gchar *name, gint type)
{
   ClickedNode *new_node = g_malloc(sizeof(ClickedNode));

   new_node->valid = valid;
   new_node->iter = iter;
   new_node->name = name;
   new_node->type = type;

   return new_node;
}

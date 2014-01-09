/**
 * addfiles.h - Interface for adding files to the project
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
#ifndef __HEADER_ADDFILES_
#define __HEADER_ADDFILES_

/**
 *
 */
void ask_name_add_group(GtkTreeIter *nodeIter);

void menu_add_widget_cb(GtkUIManager *ui, GtkWidget *widget, GtkContainer *container);


#endif /*__HEADER_ADDFILES_*/

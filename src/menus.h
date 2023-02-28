/**
 * menus.h - Menus for SciteProj
 *
 *  Copyright 2009-2023 Andreas Rönnquist
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
#ifndef __HEADER_MENUS_
#define __HEADER_MENUS_

/*
	Menu definitions
*/
extern GtkAccelGroup *accelerator_group;

extern GtkWidget *menuBar;

extern GtkWidget *groupPopupMenu;

extern GtkWidget *fileMenuEntry;
extern GtkWidget *editMenuEntry;
extern GtkWidget *viewMenuEntry;
extern GtkWidget *helpMenuEntry;

extern GtkWidget *filePopupMenu;
extern GtkWidget *fileRightClickPopupMenu;
extern GtkWidget *groupRightClickPopupMenu;
extern GtkWidget *sortPopupMenu;

extern GtkWidget *recentPopupMenu;

extern GtkWidget *quitMenuItem;

int init_menus(GtkWidget* widget);


#endif /*__HEADER_MENUS_*/

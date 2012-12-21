/**
 * graphics.h - graphics code for SciteProj
 *
 *  Copyright 2009-2012 Andreas Rönnquist
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
#ifndef __HEADER_GRAPHICS_
#define __HEADER_GRAPHICS_

/**
 *
 */
extern GdkPixbuf *directory_closed_pixbuf;
extern GdkPixbuf *directory_open_pixbuf;

extern GdkPixbuf *program_icon_pixbuf;

gboolean load_graphics(GtkWidget *widget, GError **err);
void unload_graphics();

extern GdkCursor *standard_cursor;
extern GdkCursor *busy_cursor;


#endif /*__HEADER_GRAPHICS_*/

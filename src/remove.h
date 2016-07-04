/**
 * remove.h - code for removing nodes
 *
 *  Copyright 2011-2016 Andreas Rönnquist
 *
 * This file is part of SciteProj
 *
 * SciteProj is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SciteProj is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SciteProj.
 * If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __HEADER_REMOVE_
#define __HEADER_REMOVE_

void do_remove_node(gboolean ignore_clicked_node);
void popup_remove_node_cb();

void removeitem_menu_cb();

#endif /*__HEADER_REMOVE_*/

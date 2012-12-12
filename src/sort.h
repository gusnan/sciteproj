/**
 * sort.h - Helpers for sorting
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
#ifndef __HEADER_SORT_
#define __HEADER_SORT_

/**
 *
 */
gint compare_strings_bigger(gconstpointer a,gconstpointer b);
gint compare_strings_smaller(gconstpointer a,gconstpointer b);

gint file_sort_by_extension_func(gconstpointer a, gconstpointer b);

void sort_ascending_cb();
void sort_descending_cb();

#endif /*__HEADER_SORT_*/

/**
 * dialogs.h - the programs dialogs in one place
 *
 *  Copyright 2023 Andreas Rönnquist
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

#ifndef __HEADER_DIALOGS_
#define __HEADER_DIALOGS_


int do_dialog_with_file_list (gchar *title, GList *file_list);
int warning_dialog(const char *title, const char *fmt, ...);

int get_requested_file_name (gchar *window_title, gchar *label_text, gchar **string_result);

#endif /*__HEADER_DIALOGS_*/

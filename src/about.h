/**
 * about.h - about dialog for SciteProj
 *
 *	 Copyright 2008-2018 Andreas Rönnquist
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
#ifndef __HEADER_ABOUT_
#define __HEADER_ABOUT_

/* extern gchar *sVersion;*/

void show_about_dialog();
void show_usage_dialog();

void show_version();

void init_version_string();
void done_version_string();

extern gchar *version_string;


#endif /*__HEADER_ABOUT_*/

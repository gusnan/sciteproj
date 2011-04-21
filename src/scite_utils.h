/**
 * scite_utils_linux.h - Code for working with Scite (Linux version)
 *
 *  Copyright 2006 Roy Wood, 2009-2011 Andreas Rönnquist
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
#ifndef __HEADER_SCITE_UTILS_
#define __HEADER_SCITE_UTILS_

//your declarations

extern gboolean scite_exists;


// Fork a child process and launch Scite from it
//gboolean launch_scite(gchar *sciteExecutableName, GError **err);
gboolean launch_scite(gchar *instring,GError **err);


// Send a command to Scite, launching Scite if necessary
gboolean send_scite_command(gchar *command, GError **err);


// Determine whether Scite is currently launched and the communication pipes are open
gboolean scite_ready();


// Activate the SciTE window (i.e. bring it the front)
gboolean activate_scite(GError **err);


gboolean check_if_scite_exists();

void init_scite_connection();

gboolean open_filename(gchar *filename,gchar *project_directory,GError **err);


#endif /*__HEADER_SCITE_UTILS_*/


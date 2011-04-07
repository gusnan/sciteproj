#
#   CMakeLists.txt
#  
#    Copyright 2010-2011 Andreas Ronnquist
#  
#   This file is part of SciteProj.
#   
#   SciteProj is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#  
#   SciteProj is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#  
#   You should have received a copy of the GNU General Public License
#   along with SciteProj.  If not, see <http://www.gnu.org/licenses/>.
#  
# 
Project (SciteProj C)

# Stuff for cmake (To make it work with both cmake 2.4 and 2.6...)
cmake_minimum_required(VERSION 2.4)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
	cmake_policy(SET CMP0005 NEW)
endif(COMMAND cmake_policy)


set(EXECUTABLE_NAME sciteproj)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/../bin/)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/../lib)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

message(STATUS "cmake module path:" ${CMAKE_SOURCE_DIR}/cmake)

option (DEBUG "Build Debug executable" off)

if (DEBUG)
	message(STATUS "Building Debug executable")
	set(CMAKE_BUILD_TYPE "Debug")
	add_definitions(-D_DEBUG)

	add_definitions(-Wno-write-strings)
	# add_definitions(-Wno-missing-field-initializers)
	add_definitions(-Wall)
	
	add_definitions(-Wc++-compat)

else (DEBUG)
	message(STATUS "Building Release executable")
	add_definitions(-fomit-frame-pointer)
	
	add_definitions(-Wall)
	
endif (DEBUG)

add_definitions(-DGTK_DISABLE_DEPRECATED)
add_definitions(-DGDK_DISABLE_DEPRECATED)
add_definitions(-DGSEAL_ENABLE)

Find_Package ( GTK2 REQUIRED )

if (NOT GTK2_FOUND)
	message(FATAL_ERROR "GTK not found!")
endif (NOT GTK2_FOUND)

include_directories(include ${GTK2_INCLUDE_DIRS} ${GTK2_GDK_INCLUDE_DIR} "/usr/include/gdk-pixbuf-2.0")

message(STATUS "System name:${CMAKE_SYSTEM_NAME}")

set(SOURCES
	src/about.c
	src/addfiles.c
	src/clipboard.c
	src/drag_drop.c
	src/file_utils.c
	src/filelist.c
	src/folder_to_xml.c
	src/graphics.c
	src/gui.c
	src/main.c
	src/prefs.c
	src/properties_dialog.c
	src/recent_files.c
	src/remove.c
	src/rename.c
	src/scite_utils.c
	src/search.c
	src/statusbar.c
	src/string_utils.c
	src/tree_manipulation.c
	src/xml_processing.c
)

link_libraries (
   gdk_pixbuf-2.0 ${GTK2_GDK_LIBRARY} ${GTK2_GTK_LIBRARY} gthread-2.0 gobject-2.0 X11 glib-2.0
)

add_executable (
   ${EXECUTABLE_NAME}
   ${SOURCES}
)

# -------------------------------------------------------------------------
# installation stuff:
# -------------------------------------------------------------------------

set(PIX_FOLDER ${CMAKE_INSTALL_PREFIX}/share/pixmaps)

install(FILES graphics/sciteproj.xpm DESTINATION ${PIX_FOLDER})

install(FILES sciteproj.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
install(FILES sciteproj.1 DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man1)

install_targets(/bin ${EXECUTABLE_NAME})

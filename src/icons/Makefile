#
#   Copyright (C) 2009-2023 Andreas Rönnquist
#   This file is distributed under the same license
#   as the sciteproj package, see COPYING file.
#

ifndef PREFIX
	ifdef INSTALL_PREFIX
		PREFIX=$(INSTALL_PREFIX)
	else
		PREFIX=/usr/local
	endif
endif

NAME = sciteproj
VERSION = $(shell cat ../VERSION)
DATADIR = ${DESTDIR}${PREFIX}/share
LOCALEDIR = ${DATADIR}/locale

#SOURCES = about.c clipboard.c clicked_node.c file_utils.c expand.c\
#folder_config.c graphics.c gui.c gui_callbacks.c load_folder.c\
#main.c menus.c prefs.c properties_dialog.c recent_files.c remove.c\
#scite_utils.c script.c sort.c statusbar.c string_utils.c tree_manipulation.c\
#menus.h

XG_ARGS = --keyword=_ --keyword=N_ --keyword=NC_:1c,2 -w 80 --package-name=${NAME} --package-version=${VERSION} --msgid-bugs-address=andreas@ronnquist.net
LANGUAGES = sv

# 
#LANGUAGES_MO = $(foreach currlang,$(LANGUAGES),$(currlang).mo)
#LANGUAGES_PO = $(foreach currlang,$(LANGUAGES),$(currlang).po)

all: build icon_resources_c icon_resources_h

build: icon_resources_c icon_resources_h

icon_resources_c: icons.gresource.xml
	glib-compile-resources --generate-source --target=icons_resources.c ./icons.gresource.xml

icon_resources_h: icons.gresource.xml
	glib-compile-resources --generate-header --target=icons_resources.h ./icons.gresource.xml

#install: all install-dirs
#	for po in ${LANGUAGES}; \
#	do install -m 0644 $${po}.mo ${LOCALEDIR}/$${po}/LC_MESSAGES/${NAME}.mo; \
#	done

#install-dirs:
#	for po in ${LANGUAGES}; \
#	do install -d ${LOCALEDIR}/$${po}/LC_MESSAGES; \
#	done

#uninstall:
#	for po in ${LANGUAGES}; \s
#	do rm -f  ${LOCALEDIR}/$${po}/LC_MESSAGES/${NAME}.mo; \
#	done

clean:
	rm -f icons_resources.c icons_resources.h

.PHONY: all build install install-dirs uninstall clean clean-pox clean-build update-pot update-po icon_resources_c icon_resources_h


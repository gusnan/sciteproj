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

XG_ARGS = --keyword=_ --keyword=N_ --keyword=NC_:1c,2 -w 80 --package-name=${NAME} --package-version=${VERSION} --msgid-bugs-address=andreas@ronnquist.net
LANGUAGES = sv

all: build icon_resources_c icon_resources_h

build: icon_resources_c icon_resources_h

icon_resources_c: icons.gresource.xml
	glib-compile-resources --generate-source --target=icons_resources.c ./icons.gresource.xml

icon_resources_h: icons.gresource.xml
	glib-compile-resources --generate-header --target=icons_resources.h ./icons.gresource.xml

clean:
	rm -f icons_resources.c icons_resources.h

.PHONY: all build install install-dirs uninstall clean clean-pox clean-build update-pot update-po icon_resources_c icon_resources_h


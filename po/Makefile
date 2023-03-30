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

SOURCES = about.c clipboard.c clicked_node.c create_folder.c delete.c \
dialogs.c file_utils.c expand.c graphics.c gui.c gui_callbacks.c load_folder.c \
main.c menus.c prefs.c properties_dialog.c recent_files.c remove.c \
scite_utils.c script.c selection.c sort.c statusbar.c string_utils.c \
tree_manipulation.c menus.h

XG_ARGS = --keyword=_ --keyword=N_ --keyword=NC_:1c,2 -w 80 --package-name=${NAME} --package-version=${VERSION} --msgid-bugs-address=andreas@ronnquist.net
LANGUAGES = sv

# 
LANGUAGES_MO = $(foreach currlang,$(LANGUAGES),$(currlang).mo)
LANGUAGES_PO = $(foreach currlang,$(LANGUAGES),$(currlang).po)

all: build

${NAME}.pot:
	xgettext ${XG_ARGS} --directory=../src/ ${SOURCES} -d ${NAME} -o ./${NAME}.pot

update-pot: ${NAME}.pot

%.pox: %.po
	msgmerge -o $@ --previous $< ${NAME}.pot

build: $(LANGUAGES_MO)

$(LANGUAGES_MO): $(LANGUAGES_PO)
	for po in ${LANGUAGES}; \
	do msgfmt -v --statistics -c -o t-$${po} $${po}.po && mv -f t-$${po} $${po}.mo; \
	done

update-po:
	for po in ${LANGUAGES}; \
	do msgmerge -o $${po}.po $${po}.po ${NAME}.pot; \
	done

install: all install-dirs
	for po in ${LANGUAGES}; \
	do install -m 0644 $${po}.mo ${LOCALEDIR}/$${po}/LC_MESSAGES/${NAME}.mo; \
	done

install-dirs:
	for po in ${LANGUAGES}; \
	do install -d ${LOCALEDIR}/$${po}/LC_MESSAGES; \
	done

uninstall:
	for po in ${LANGUAGES}; \
	do rm -f  ${LOCALEDIR}/$${po}/LC_MESSAGES/${NAME}.mo; \
	done

clean-build:
	rm -f *.mo

clean-pox:
	rm -f *.pox

clean: clean-build
	rm -f *~

.PHONY: all build install install-dirs uninstall clean clean-pox clean-build update-pot update-po


#
# Sciteproj po makefile
# $Id$
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

SOURCES = about.c addfiles.c clipboard.c drag_drop.c error.c file_utils.c \
filelist.c folder_to_xml.c graphics.c gui.c main.c prefs.c properties_dialog.c \
recent_files.c remove.c rename.c scite_utils.c search.c statusbar.c \
string_utils.c tree_manipulation.c xml_processing.c

XG_ARGS = --keyword=_ --keyword=N_ -w 80 --package-name=${NAME} --package-version=${VERSION} --msgid-bugs-address=gusnan@gusnan.se
LANGUAGES = sv_SE

all: build

${NAME}.pot:
	xgettext ${XG_ARGS} --directory=../src/ ${SOURCES} -d ${NAME} -o ./${NAME}.pot

update-pot: ${NAME}.pot

%.pox: %.po
	msgmerge -o $@ --previous $< ${NAME}.pot 

build:
	for po in ${LANGUAGES}; \
	do msgfmt -v --statistics -c -o t-$${po} $${po}.po && mv -f t-$${po} $${po}.mo; \
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

.PHONY: all build install install-dirs uninstall clean clean-pox clean-build update-pot


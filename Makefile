#
#   Copyright (C) 2009-2018 Andreas Rönnquist
#   This file is distributed under the same license
#   as the sciteproj package, see COPYING file.
#

ifndef CC
	CC=gcc
endif
ifndef PKG_CONFIG
	PKG_CONFIG=pkg-config
endif
SRC=src
BIN=bin
GRPH=graphics
OBJ=obj

ifdef DEBUG
	STD_CFLAGS=-Wall -g3 -ggdb -D_DEBUG
else
	STD_CFLAGS=-Wall -Wformat -Wno-format-extra-args -Wformat-security -Wformat-nonliteral -Wformat=2 -Wdeprecated-declarations
endif

OBJECTS=$(OBJ)/about.o $(OBJ)/clipboard.o $(OBJ)/expand.o\
$(OBJ)/file_utils.o $(OBJ)/folder_config.o\
$(OBJ)/graphics.o $(OBJ)/gui.o $(OBJ)/gui_callbacks.o\
$(OBJ)/icon.o $(OBJ)/load_folder.o $(OBJ)/menus.o $(OBJ)/main.o $(OBJ)/prefs.o\
$(OBJ)/properties_dialog.o $(OBJ)/recent_files.o $(OBJ)/remove.o\
$(OBJ)/scite_utils.o $(OBJ)/script.o $(OBJ)/sort.o $(OBJ)/statusbar.o\
$(OBJ)/string_utils.o $(OBJ)/tree_manipulation.o

GRAPHICS_INCLUDES=$(GRPH)/dir-close.xpm \
$(GRPH)/dir-open.xpm \
$(GRPH)/sciteproj.xpm


ifndef PREFIX
	ifdef INSTALL_PREFIX
		PREFIX=$(INSTALL_PREFIX)
	else
		PREFIX=/usr/local
	endif
endif

NAME=sciteproj
PROG=${BIN}/${NAME}
DEPEND=Makefile.dep
DATADIR= ${DESTDIR}${PREFIX}/share
LOCALEDIR = ${DATADIR}/locale
VERSION=$(shell cat ./VERSION)

PKG_GTK=gtk+-3.0
CHECK_GTK3=1

LIB_CFLAGS=$(shell $(PKG_CONFIG) --cflags --silence-errors $(PKG_GTK) $(PKG_WNCK) lua5.3 || $(PKG_CONFIG) --cflags $(PKG_GTK) $(PKG_WNCK) lua)
STD_LDFLAGS=
LIBS=-lX11 $(shell $(PKG_CONFIG) --libs --silence-errors $(PKG_GTK) $(PKG_WNCK) lua5.3 || $(PKG_CONFIG) --libs $(PKG_GTK) $(PKG_WNCK) lua)

LOCAL_CFLAGS=$(STD_CFLAGS) $(DEPRECATED) $(CFLAGS) $(LIB_CFLAGS)
LOCAL_LDFLAGS=$(STD_CFLAGS) $(LDFLAGS) $(STD_LDFLAGS)

LOCAL_CPPFLAGS=$(CPPFLAGS)

LOCAL_CFLAGS+=-DGTK_DISABLE_SINGLE_INCLUDES
LOCAL_CFLAGS+=-DGSEAL_ENABLE

LOCAL_CFLAGS+=-DGDK_PIXBUF_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED

LOCAL_CFLAGS+=-DLOCALEDIR=\"$(LOCALEDIR)\" -DPACKAGE=\"$(NAME)\" -DSCITEPROJ_VERSION=\"$(VERSION)\"

all: $(BIN)/$(NAME)
	${MAKE} -C po -j1 all

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(LOCAL_CFLAGS) $(LOCAL_CPPFLAGS) -c $< -o $@

$(BIN)/$(NAME): $(OBJECTS)
	$(CC) $(LOCAL_CFLAGS) $(LOCAL_LDFLAGS) $(OBJECTS) -o $(PROG) $(LIBS)

clean:
	rm -rf $(OBJECTS) $(PROG) $(DEPEND)
	${MAKE} -C po clean

install:
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 $(PROG) $(DESTDIR)$(PREFIX)/bin
	install -d $(DESTDIR)$(PREFIX)/share/pixmaps
	install -m 644 graphics/sciteproj.xpm $(DESTDIR)$(PREFIX)/share/pixmaps
	${MAKE} -C po install

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/$(PROG)
	rm -f $(DESTDIR)$(PREFIX)/share/pixmaps/sciteproj.xpm
	${MAKE} -C po uninstall

$(DEPEND):
	$(CC) $(LOCAL_CFLAGS) -MM $(SRC)/*.c | sed -e "s/\([A-Za-z0-9+-0._&+-]*:\)/\$(OBJ)\/\1/g" -e "s/obj\/C\:/\/C/g" > $(DEPEND)

-include $(DEPEND)

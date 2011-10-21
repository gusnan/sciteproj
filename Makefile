CC=gcc
SRC=src
BIN=bin
GRPH=graphics

ifdef DEBUG
	STD_CFLAGS=-c -Wall -g3 -ggdb -D_DEBUG
else
	STD_CFLAGS=-c -Wall
endif

PROG=bin/sciteproj

OBJECTS=about.o addfiles.o clipboard.o drag_drop.o file_utils.o filelist.o \
folder_to_xml.o graphics.o gui.o main.o prefs.o properties_dialog.o \
recent_files.o remove.o rename.o scite_utils.o search.o statusbar.o  \
string_utils.o tree_manipulation.o xml_processing.o

GRAPHICS_INCLUDES=$(GRPH)/dir-close.xpm \
$(GRPH)/dir-open.xpm \
$(GRPH)/text-x-cpp.xpm \
$(GRPH)/text-x-h.xpm \
$(GRPH)/text-x-txt.xpm \
$(GRPH)/text-x-java.xpm \
$(GRPH)/text-x-lua.xpm \
$(GRPH)/sciteproj.xpm

ifndef DESTDIR
DESTDIR=/usr/local
endif

ifdef GTK2
	PKG_GTK=gtk+-2.0
	CHECK_GTK3=1
else
	PKG_GTK=gtk+-3.0
endif

LIB_CFLAGS=`pkg-config --cflags $(PKG_GTK)`
STD_LDFLAGS= `pkg-config --libs $(PKG_GTK)`

LOCAL_CFLAGS=$(STD_CFLAGS) $(DEPRECATED) $(CFLAGS) $(LIB_CFLAGS) 
LOCAL_LDFLAGS=$(LDFLAGS) $(STD_LDFLAGS) 

ifdef CHECK_GTK3
	LOCAL_CFLAGS+=-DGTK_DISABLE_SINGLE_INCLUDES
	LOCAL_CFLAGS+=-DGSEAL_ENABLE

	CHECK_DEPRECATED=1
endif

ifdef CHECK_DEPRECATED
	LOCAL_CFLAGS+=-DGDK_PIXBUF_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED
endif 

all: $(BIN)/sciteproj

%.o: $(SRC)/%.c
	$(CC) $(LOCAL_CFLAGS) -c $< -o $@

$(BIN)/sciteproj: $(OBJECTS)
	$(CC) $(LOCAL_LDFLAGS) $(OBJECTS) -o $(PROG)

clean:
	rm -rf $(OBJECTS) $(PROG) Makefile.dep

install:
	install -d $(DESTDIR)/bin
	install -m 755 $(PROG) $(DESTDIR)/bin
	install -d $(DESTDIR)/share/pixmaps
	install -m 644 graphics/sciteproj.xpm $(DESTDIR)/share/pixmaps

uninstall:
	rm -f $(DESTDIR)/$(PROG)
	rm -f $(DESTDIR)/share/pixmaps/sciteproj.xpm

Makefile.dep:
	$(CC) -MM $(SRC)/*.c > Makefile.dep

-include Makefile.dep

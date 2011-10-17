CC=gcc
SRC=src
OBJ=obj
BIN=bin
GRPH=graphics

ifdef DEBUG
	STD_CFLAGS=-c -Wall -g3 -ggdb
else
	STD_CFLAGS=-c -Wall
endif

PROG=bin/sciteproj

OBJECTS=$(OBJ)/about.o \
$(OBJ)/addfiles.o \
$(OBJ)/clipboard.o \
$(OBJ)/drag_drop.o \
$(OBJ)/file_utils.o \
$(OBJ)/filelist.o \
$(OBJ)/folder_to_xml.o \
$(OBJ)/graphics.o \
$(OBJ)/gui.o \
$(OBJ)/main.o \
$(OBJ)/prefs.o \
$(OBJ)/properties_dialog.o \
$(OBJ)/recent_files.o \
$(OBJ)/remove.o \
$(OBJ)/rename.o \
$(OBJ)/scite_utils.o \
$(OBJ)/search.o \
$(OBJ)/statusbar.o \
$(OBJ)/string_utils.o \
$(OBJ)/tree_manipulation.o \
$(OBJ)/xml_processing.o \

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

LOCAL_CFLAGS=$(STD_CFLAGS) $(LIB_CFLAGS) $(DEPRECATED)
LOCAL_LDFLAGS=$(STD_LDFLAGS)

ifdef CHECK_GTK3
	LOCAL_CFLAGS+=-DGTK_DISABLE_SINGLE_INCLUDES
	LOCAL_CFLAGS+=-DGSEAL_ENABLE
	
	CHECK_DEPRECATED=1
endif

ifdef CHECK_DEPRECATED
	LOCAL_CFLAGS+=-DGDK_PIXBUF_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED
endif 

all: bin/sciteproj

$(OBJ)/about.o: $(SRC)/about.h $(SRC)/graphics.h $(SRC)/about.c
	$(CC) $(LOCAL_CFLAGS) $(SRC)/about.c -o $(OBJ)/about.o

$(OBJ)/addfiles.o: $(SRC)/gui.h $(SRC)/clicked_node.h $(SRC)/tree_manipulation.h $(SRC)/addfiles.h $(SRC)/addfiles.c
	$(CC) $(LOCAL_CFLAGS) $(SRC)/addfiles.c -o $(OBJ)/addfiles.o

$(OBJ)/clipboard.o: $(SRC)/clicked_node.h $(SRC)/gui.h $(SRC)/tree_manipulation.h $(SRC)/file_utils.h $(SRC)/string_utils.h $(SRC)/clipboard.h $(SRC)/clipboard.c
	$(CC) $(LOCAL_CFLAGS) $(SRC)/clipboard.c -o $(OBJ)/clipboard.o

$(OBJ)/drag_drop.o: $(SRC)/drag_drop.h $(SRC)/string_utils.h $(SRC)/tree_manipulation.h $(SRC)/drag_drop.c
	$(CC) $(LOCAL_CFLAGS) $(SRC)/drag_drop.c -o $(OBJ)/drag_drop.o

$(OBJ)/file_utils.o: $(SRC)/file_utils.h $(SRC)/file_utils.c
	$(CC) $(LOCAL_CFLAGS) $(SRC)/file_utils.c -o $(OBJ)/file_utils.o

$(OBJ)/filelist.o: $(SRC)/filelist.h $(SRC)/filelist.c
	$(CC) $(LOCAL_CFLAGS) $(SRC)/filelist.c -o $(OBJ)/filelist.o

$(OBJ)/folder_to_xml.o: $(SRC)/folder_to_xml.h $(SRC)/folder_to_xml.c
	$(CC) $(LOCAL_CFLAGS) $(SRC)/folder_to_xml.c -o $(OBJ)/folder_to_xml.o

$(OBJ)/graphics.o: $(SRC)/string_utils.h $(SRC)/graphics.h $(SRC)/about.h $(SRC)/graphics.c $(GRAPHICS_INCLUDES)
	$(CC) $(LOCAL_CFLAGS) $(SRC)/graphics.c -o $(OBJ)/graphics.o

$(OBJ)/gui.o: $(SRC)/clicked_node.h $(SRC)/gui.h $(SRC)/drag_drop.h  $(SRC)/tree_manipulation.h $(SRC)/scite_utils.h $(SRC)/string_utils.h $(SRC)/prefs.h $(SRC)/statusbar.h $(SRC)/graphics.h $(SRC)/about.h $(SRC)/properties_dialog.h $(SRC)/file_utils.h $(SRC)/search.h $(SRC)/clipboard.h $(SRC)/rename.h $(SRC)/remove.h $(SRC)/addfiles.h $(SRC)/recent_files.h $(SRC)/filelist.h $(SRC)/menus.h $(SRC)/search.h $(SRC)/gui.c
	$(CC) $(LOCAL_CFLAGS) $(SRC)/gui.c -o $(OBJ)/gui.o

$(OBJ)/main.o: $(SRC)/clicked_node.h $(SRC)/gui.h $(SRC)/tree_manipulation.h $(SRC)/prefs.h $(SRC)/folder_to_xml.h $(SRC)/graphics.h $(SRC)/scite_utils.h $(SRC)/about.h $(SRC)/file_utils.h $(SRC)/string_utils.h $(SRC)/main.c
	$(CC) $(LOCAL_CFLAGS) $(SRC)/main.c -o $(OBJ)/main.o

$(OBJ)/prefs.o: $(SRC)/prefs.h $(SRC)/string_utils.h $(SRC)/prefs.c
	$(CC) $(LOCAL_CFLAGS) $(SRC)/prefs.c -o $(OBJ)/prefs.o

$(OBJ)/properties_dialog.o: $(SRC)/properties_dialog.h $(SRC)/tree_manipulation.h $(SRC)/clicked_node.h $(SRC)/gui.h $(SRC)/string_utils.h $(SRC)/properties_dialog.c
	$(CC) $(LOCAL_CFLAGS) $(SRC)/properties_dialog.c -o $(OBJ)/properties_dialog.o

$(OBJ)/recent_files.o: $(SRC)/recent_files.h $(SRC)/prefs.h $(SRC)/graphics.h $(SRC)/tree_manipulation.h $(SRC)/string_utils.h $(SRC)/clicked_node.h $(SRC)/file_utils.h $(SRC)/statusbar.h $(SRC)/scite_utils.h $(SRC)/clipboard.h $(SRC)/properties_dialog.h $(SRC)/recent_files.c
	$(CC) $(LOCAL_CFLAGS) $(SRC)/recent_files.c -o $(OBJ)/recent_files.o

$(OBJ)/remove.o: $(SRC)/clicked_node.h $(SRC)/gui.h $(SRC)/gui.h $(SRC)/tree_manipulation.h $(SRC)/remove.h $(SRC)/remove.c
	$(CC) $(LOCAL_CFLAGS) $(SRC)/remove.c -o $(OBJ)/remove.o

$(OBJ)/rename.o: $(SRC)/tree_manipulation.h $(SRC)/clicked_node.h $(SRC)/gui.h $(SRC)/rename.h $(SRC)/rename.c
	$(CC) $(LOCAL_CFLAGS) $(SRC)/rename.c -o $(OBJ)/rename.o

$(OBJ)/scite_utils.o: $(SRC)/scite_utils.h $(SRC)/prefs.h $(SRC)/clicked_node.h $(SRC)/gui.h $(SRC)/graphics.h $(SRC)/string_utils.h $(SRC)/statusbar.h $(SRC)/scite_utils.c
	$(CC) $(LOCAL_CFLAGS) $(SRC)/scite_utils.c -o $(OBJ)/scite_utils.o

$(OBJ)/search.o: $(SRC)/search.h $(SRC)/tree_manipulation.h $(SRC)/graphics.h $(SRC)/string_utils.h $(SRC)/scite_utils.h $(SRC)/statusbar.h $(SRC)/prefs.h $(SRC)/filelist.h $(SRC)/search.c
	$(CC) $(LOCAL_CFLAGS) $(SRC)/search.c -o $(OBJ)/search.o

$(OBJ)/statusbar.o: $(SRC)/statusbar.h $(SRC)/statusbar.c
	$(CC) $(LOCAL_CFLAGS) $(SRC)/statusbar.c -o $(OBJ)/statusbar.o

$(OBJ)/string_utils.o: $(SRC)/string_utils.h $(SRC)/file_utils.h $(SRC)/string_utils.c
	$(CC) $(LOCAL_CFLAGS) $(SRC)/string_utils.c -o $(OBJ)/string_utils.o

$(OBJ)/tree_manipulation.o: $(SRC)/tree_manipulation.h $(SRC)/xml_processing.h $(SRC)/string_utils.h $(SRC)/graphics.h $(SRC)/prefs.h $(SRC)/clicked_node.h $(SRC)/gui.h $(SRC)/filelist.h $(SRC)/tree_manipulation.c
	$(CC) $(LOCAL_CFLAGS) $(SRC)/tree_manipulation.c -o $(OBJ)/tree_manipulation.o

$(OBJ)/xml_processing.o: $(SRC)/xml_processing.h $(SRC)/string_utils.h $(SRC)/tree_manipulation.h $(SRC)/clicked_node.h $(SRC)/gui.h $(SRC)/file_utils.h $(SRC)/prefs.h $(SRC)/xml_processing.c
	$(CC) $(LOCAL_CFLAGS) $(SRC)/xml_processing.c -o $(OBJ)/xml_processing.o

bin/sciteproj: $(OBJECTS)
	$(CC) $(LOCAL_LDFLAGS) $(OBJECTS) -o $(PROG)

clean:
	rm -rf $(OBJECTS) $(PROG)

uninstall:
	rm -f $(DESTDIR)/$(PROG)

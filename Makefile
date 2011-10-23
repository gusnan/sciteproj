CC=gcc
SRC=src
BIN=bin
GRPH=graphics
OBJ=obj

ifdef DEBUG
	STD_CFLAGS=-c -Wall -g3 -ggdb -D_DEBUG
else
	STD_CFLAGS=-c -Wall
endif

PROG=bin/sciteproj
DEPEND=Makefile.dep

OBJECTS=$(OBJ)/about.o $(OBJ)/addfiles.o $(OBJ)/clipboard.o $(OBJ)/drag_drop.o\
$(OBJ)/file_utils.o $(OBJ)/filelist.o $(OBJ)/folder_to_xml.o $(OBJ)/graphics.o\
$(OBJ)/gui.o $(OBJ)/main.o $(OBJ)/prefs.o $(OBJ)/properties_dialog.o \
$(OBJ)/recent_files.o $(OBJ)/remove.o $(OBJ)/rename.o $(OBJ)/scite_utils.o\
$(OBJ)/search.o $(OBJ)/statusbar.o  $(OBJ)/string_utils.o\
$(OBJ)/tree_manipulation.o $(OBJ)/xml_processing.o

GRAPHICS_INCLUDES=$(GRPH)/dir-close.xpm \
$(GRPH)/dir-open.xpm \
$(GRPH)/text-x-cpp.xpm \
$(GRPH)/text-x-h.xpm \
$(GRPH)/text-x-txt.xpm \
$(GRPH)/text-x-java.xpm \
$(GRPH)/text-x-lua.xpm \
$(GRPH)/sciteproj.xpm

ifndef prefix
	ifdef INSTALL_PREFIX
		prefix=$(INSTALL_PREFIX)
	else
		prefix=/usr/local
	endif
endif

ifdef GTK2
	PKG_GTK=gtk+-2.0
	CHECK_GTK3=1
else
	PKG_GTK=gtk+-3.0
endif

LIB_CFLAGS=`pkg-config --cflags $(PKG_GTK)`
STD_LDFLAGS= 

LIBS+=`pkg-config --libs $(PKG_GTK)`

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

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(LOCAL_CFLAGS) -c $< -o $@

$(BIN)/sciteproj: $(OBJECTS)
	$(CC) $(LOCAL_LDFLAGS) $(OBJECTS) -o $(PROG) $(LIBS)

clean:
	rm -rf $(OBJECTS) $(PROG) $(DEPEND)

install:
	install -d $(DESTDIR)$(prefix)/bin
	install -m 755 $(PROG) $(DESTDIR)$(prefix)/bin
	install -d $(DESTDIR)$(prefix)/share/pixmaps
	install -m 644 graphics/sciteproj.xpm $(DESTDIR)$(prefix)/share/pixmaps

uninstall:
	rm -f $(DESTDIR)$(prefix)/$(PROG)
	rm -f $(DESTDIR)$(prefix)/share/pixmaps/sciteproj.xpm

$(DEPEND):
	$(CC) -MM $(SRC)/*.c | sed -e "s/\([A-Za-z+-0._&+-]*:\)/\$(OBJ)\/\1/g" > $(DEPEND)

-include $(DEPEND)

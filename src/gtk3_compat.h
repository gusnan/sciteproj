#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#ifndef __HEADER_GTK3_COMPAT_
#define __HEADER_GTK3_COMPAT_

#ifndef GDK_KEY_Return
	#define GDK_KEY_BackSpace GDK_BackSpace
	#define GDK_KEY_Delete GDK_Delete
	#define GDK_KEY_Insert GDK_Insert
	#define GDK_KEY_Return GDK_Return
	#define GDK_KEY_KP_Enter GDK_KP_Enter
	#define GDK_KEY_Escape GDK_Escape
	#define GDK_KEY_F2 GDK_F2
	#define GDK_KEY_F5 GDK_F5
#endif

#endif /*__HEADER_GTK3_COMPAT_*/


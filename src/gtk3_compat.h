#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#ifndef __HEADER_GTK3_COMPAT_
#define __HEADER_GTK3_COMPAT_

#ifndef GDK_KEY_Return
	#define GDK_KEY_Backspace GDK_BackSpace
#endif

#endif /*__HEADER_GTK3_COMPAT_*/


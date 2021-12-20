/**
 * scite_utils_linux.c - Code for working with Scite (GNU/Linux version)
 *
 *  Copyright 2006 Roy Wood, 2009-2018 Andreas Rönnquist
 *
 * This file is part of SciteProj.
 *
 * SciteProj is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SciteProj is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SciteProj.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <glib.h>
#include <gtk/gtk.h>

#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <glib/gi18n.h>

#include <locale.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/time.h>


#include "scite_utils.h"

// This is for SciteProj :
#include "prefs.h"
#include "clicked_node.h"
#include "gui.h"
#include "graphics.h"
#include "string_utils.h"
#include "statusbar.h"
#include "file_utils.h"

#define APP_SCITEPROJ_ERROR g_quark_from_static_string("APP_SCITEUTILS_ERROR")


// Indicates whether Scite was successfully forked/executed
static gboolean sSciteLaunched = FALSE;

// File descriptor of the request pipe
static int sRequestPipeFD = 0;

// File descriptor of the response pipe
static int sResponsePipeFD = 0;

// GLib GIOChannel of the response pipe
static GIOChannel* sResponsePipeGIOChannel = NULL;

// Scite executable name
static gchar *sSciteExecName0 = (gchar*)"/usr/local/bin/SciTE";
static gchar *sSciteExecName1 = (gchar*)"/usr/local/bin/scite";
static gchar *sSciteExecName2 = (gchar*)"SciTE";
static gchar *sSciteExecName3 = (gchar*)"scite";


// -----------------
// X11 display
Display *sDisplay = NULL;

// Scite X11 window
Window sSciteWin = 0;

gboolean scite_exists = FALSE;

void set_scite_launched(gboolean launched);


/**
 * Determine if Scite is open and ready for communication.
 *
 * @return TRUE if Scite has been forked/executed and we have an open pipe, FALSE otherwise
 */
gboolean scite_ready()
{
   return (sSciteLaunched && sRequestPipeFD != 0);
}



/**
 * Shut down all the Scite pipes and associated GIOChannels.
 */
void shutdown_pipes()
{
   if (sRequestPipeFD) {
      close(sRequestPipeFD);
      sRequestPipeFD = 0;
   }

   if (sResponsePipeGIOChannel) {
      g_io_channel_unref(sResponsePipeGIOChannel);
      g_io_channel_shutdown(sResponsePipeGIOChannel, FALSE, NULL);
      sResponsePipeGIOChannel = NULL;
   }

   if (sResponsePipeFD) {
      close(sResponsePipeFD);
      sResponsePipeFD = 0;
   }


   sSciteWin = 0;
}



/**
 * Callback for data-ready on the pipe we use to read from Scite.  We don't actually do anything with
 * the messages from Scite right now, other than echo them to the console.
 *
 * @param source is the GIOChannel associated with the pipe Scite writes to
 * @param condition is the GIOCondition for the event that triggered the callback
 * @param data is the user data item (not used)
 */
gboolean scite_pipe_read_ready_cb(GIOChannel *source, GIOCondition condition, gpointer data)
{
   g_assert(source == sResponsePipeGIOChannel);


   static gchar buff[1024];
   gboolean finalResult = TRUE;
   gsize bytes_read;
   GError *error = NULL;

#ifdef DEBUG_SCITE
   debug_printf("scite_pipe_read_ready_cb\n");
#endif

   if (condition & G_IO_IN) {
      if (g_io_channel_read_chars(source, buff, sizeof(buff) - 1, &bytes_read, &error) != G_IO_STATUS_NORMAL) {
         g_print("%s: %s = %s\n",
                 __func__,
                 "g_io_channel_read_chars failed",
                 (error != NULL) ? error->message : "<unknown>");
      }
      else {
         if ((bytes_read-1) > 0) {
            buff[bytes_read] = '\0';

            // This is for SciteProj :

            //if(prefs.verbosity>50) {
#ifdef DEBUG_SCITE
            debug_printf("%s: read data '%s'\n", __func__, buff);
#endif
            //}

            // If the string ends with a newline, remove it!
            int len=strlen(buff);

            if (buff[len-1] == '\n') {
               gchar *temp = g_strndup(buff, len-1);
               g_snprintf(buff, 1024, "%s", temp);
               g_free(temp);
            }

            // Is it the response to an "askproperty" command?  Dunno why they are prefixed with "macro:stringinfo:" though....
            static char *askpropertyResponse = (char*)"macro:stringinfo:";

            if (g_str_has_prefix(buff, askpropertyResponse)) {

               char *windowIDStr = buff + strlen(askpropertyResponse);

               sSciteWin = strtol(windowIDStr, NULL, 0);
            }

            if (g_str_has_prefix(buff, "closed:")) {

               gchar *file=get_filename_from_full_path(buff);
               gchar *status_string = g_strdup_printf(_("Closed %s"),file);

               set_statusbar_text(status_string);

               g_free(status_string);
            }

            if (g_str_has_prefix(buff, "closing:")) {

               set_statusbar_text(_("Closed SciTE"));

            }

            if (g_str_has_prefix(buff, "switched:")) {

               gchar *file=get_filename_from_full_path(buff);
               gchar *status_string = g_strdup_printf(_("Switched to %s"),file);

               set_statusbar_text(status_string);

               g_free(status_string);
            }

            if (g_str_has_prefix(buff, "opened:")) {

               gchar *file=get_filename_from_full_path(buff);
               gchar *status_string = g_strdup_printf(_("Opened %s"),file);

               set_statusbar_text(status_string);

               g_free(status_string);
            }
         }
      }
   }

   if (condition & G_IO_ERR) {
      g_print("%s: condition = G_IO_ERR\n", __func__);
      finalResult = FALSE;
   }

   if (condition & G_IO_HUP) {
      // This is for SciteProj :
      if(prefs.verbosity>50) {
         g_print("%s: condition = G_IO_HUP\n", __func__);
      }
      finalResult = FALSE;
   }


   if (!finalResult) {
      // We want to stop monitoring this GIOChannel, so shut it all down

      shutdown_pipes();

      set_scite_launched(FALSE);

//~ 		GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Connection to current instance of Scite has been broken");
//~ 		gtk_dialog_run(GTK_DIALOG(dialog));
//~ 		gtk_widget_destroy(dialog);
   }


   if (error) g_error_free(error);

   return finalResult;
}


/**
 * Callback for the progress dialog shown when opening Scite.  This callback simply sets a boolean
 * pointed to by the userData paramater.
 *
 * @param dialog is the pointer to the dialog
 * @param responseID is the responseID of the cancel button
 * @param userData is a pointer to a boolean that we set to true
 */
static void cancel_button_cb(GtkDialog *dialog, gint responseID, gpointer userData)
{
   if (userData) {
      *((gboolean *) userData) = TRUE;
   }
}



/**
 * Fork and execute an instance of Scite, then connect to its "Director" pipe.  This is uglier than
 * I expected it to be, once I factored in the progress feedback, user cancellation, handling of
 * failure of the fork/exec, etc.
 *
 * @return TRUE on success, FALSE on failure (further details returned in err)
 *
 * @param err returns any errors
 */
gboolean launch_scite(gchar *instring,GError **err)
{

   debug_printf("launch_scite: %s\n", instring);

   gboolean resultCode = FALSE;
   gchar* ipcDirectorName = (gchar*)"ipc.director.name";
   gchar* ipcSciteName = (gchar*)"ipc.scite.name";
   GtkWidget* dialog = NULL;
   int childPipePair[2] = { 0, 0 };

   fd_set readFDS;

   struct timeval timeVal;

   gboolean userClickedCancel = FALSE;
   gchar responsePipePath[256];
   gchar requestPipePath[256];


   // Clean up anything open from previous attempts/activity

   shutdown_pipes();

   set_scite_launched(FALSE);


   if (scite_exists) {
      gchar *scite_command;

      char scite_arg1[256];
      char scite_arg2[256];
      char scite_arg3[256];
      char scite_arg4[256];

      static unsigned long usecsDelta = 100000;
      static unsigned long usecsDialogDelay;

      // We need our process id for use in forming the named pipe filenames
      pid_t childPID = 0;
      gulong usecs = 0;
      int errCode;

      pid_t ourPID = getpid();

      // The response pipe will be used by Scite to send data to us

      g_snprintf(responsePipePath, sizeof(responsePipePath), "/tmp/sciteproj.%ld", (unsigned long) ourPID);

      if (setenv(ipcDirectorName, responsePipePath, TRUE)) {
         errCode = errno;
         g_set_error(err, APP_SCITEPROJ_ERROR, -1,
                     "%s: Could not launch Scite, setenv(\"%s\", \"%s\") failed, error = %s",
                     __func__, ipcDirectorName, responsePipePath, strerror(errCode));
         goto EXITPOINT;
      }

      // The request pipe will be used to send data to Scite

      g_snprintf(requestPipePath, sizeof(requestPipePath), "/tmp/scite.%ld", (unsigned long) ourPID);

      if (setenv(ipcSciteName, requestPipePath, TRUE)) {
         errCode = errno;
         g_set_error(err, APP_SCITEPROJ_ERROR, -1,
                     "%s: Could not launch Scite, setenv(\"%s\", \"%s\") failed, error = %s",
                     __func__, ipcSciteName, requestPipePath, strerror(errCode));
         goto EXITPOINT;
      }


      // Remove any existing files that conflict with the pipe names

      if (remove(responsePipePath) && errno != ENOENT) {
         errCode = errno;
         g_set_error(err, APP_SCITEPROJ_ERROR, -1,
                     "%s: Could not launch Scite, remove(\"%s\") failed, error = %s",
                     __func__, responsePipePath, strerror(errCode));
         goto EXITPOINT;
      }

      if (remove(requestPipePath) && errno != ENOENT) {
         errCode = errno;
         g_set_error(err, APP_SCITEPROJ_ERROR, -1,
                     "%s: Could not launch Scite, remove(\"%s\") failed, error = %s",
                     __func__, requestPipePath, strerror(errCode));
         goto EXITPOINT;
      }


      // Now create our response pipe (Scite creates the request pipe, as long as it doesn't already exist)

      if (mkfifo(responsePipePath, 0777)) {
         errCode = errno;
         g_set_error(err, APP_SCITEPROJ_ERROR, -1,
                     "%s: Could not launch Scite, mkfifo(\"%s\") failed, error = %s",
                     __func__, responsePipePath, strerror(errCode));
         goto EXITPOINT;
      }


      // Open the Scite response pipe

      if ((sResponsePipeFD = open(responsePipePath, O_RDONLY | O_NONBLOCK)) <= 0) {
         errCode = errno;
         g_set_error(err, APP_SCITEPROJ_ERROR, -1,
                     "%s: Could not launch Scite, open(\"%s\") failed, error = %s",
                     __func__, responsePipePath, strerror(errCode));
         goto EXITPOINT;
      }

      // Hook up the Scite response pipe to our Gtk/GLib main loop

      if ((sResponsePipeGIOChannel = g_io_channel_unix_new(sResponsePipeFD)) == NULL) {
         errCode = errno;
         g_set_error(err, APP_SCITEPROJ_ERROR, -1,
                     "%s: Could not launch Scite, g_io_channel_unix_new(\"%s\") failed, error = %s",
                     __func__, responsePipePath, strerror(errCode));
         goto EXITPOINT;
      }

      if ((g_io_channel_set_encoding(sResponsePipeGIOChannel, NULL, err)) != G_IO_STATUS_NORMAL) {
         g_set_error(err, APP_SCITEPROJ_ERROR, -1,
                     "%s: Could not launch Scite, g_io_channel_set_encoding( ) failed, error = %s",
                     __func__, (err != NULL && *err != NULL) ? (*err)->message : "<unknown>");
         goto EXITPOINT;
      }

      g_io_add_watch(sResponsePipeGIOChannel,
                     (GIOCondition) (G_IO_IN | G_IO_ERR | G_IO_HUP),
                     scite_pipe_read_ready_cb,
                     NULL);


      // Also create a pipe pair so our child process can alert us if it fails to execute Scite

      if (pipe(childPipePair)) {
         errCode = errno;
         g_set_error(err, APP_SCITEPROJ_ERROR, -1,
                     "%s: Could not launch Scite, pipe( ) failed, errno = %d = %s",
                     __func__, errCode, strerror(errCode));
         goto EXITPOINT;
      }


      // Fix the instring


      gchar *opt1 = NULL;
      gchar *opt2 = NULL;

      if (instring != NULL) {

         int len = strlen(instring);
         int split = -1;
         gchar *tempstring = instring;
         gchar *place = NULL;
         int co;
         for (co=0; co < len; co++) {

            gchar temp = instring[co];

            if (temp == ' ') {
               split = co;
               co = len;
               place = tempstring;
            }

            tempstring++;
         }

         if (split != -1) {
            opt1 = g_strndup(instring, split);
            opt2 = g_strdup(place);
         } else {
            opt1 = g_strdup(instring);
         }
      }

      debug_printf("opt1:%s\n", opt1);
      debug_printf("opt2:%s\n", opt2);

      // Set up the command line
      if (opt1 != NULL) {

         opt1 = g_strchug(opt1);
         opt1 = g_strchomp(opt1);

         strcpy(scite_arg1, opt1);
      } else {
         strcpy(scite_arg1, "");
      }

      if (opt2 != NULL) {

         opt2 = g_strchug(opt2);
         opt2 = g_strchomp(opt2);

         strcpy(scite_arg2, opt2);
      } else {
         strcpy(scite_arg2, "");
      }
      strcpy(scite_arg3, "");
      strcpy(scite_arg4, "");


      // Fork and (we hope) exec Scite

      childPID = fork();

      if (childPID == -1) {
         errCode = errno;
         g_set_error(err, APP_SCITEPROJ_ERROR, -1,
                     "%s: Could not launch Scite, fork() failed, errno = %d = %s",
                     __func__, errCode, strerror(errCode));
         goto EXITPOINT;
      }
      else if (childPID == 0) {

         debug_printf("Launching:'%s','%s','%s','%s'\n",scite_arg1,scite_arg2,scite_arg3,scite_arg4);

         // We are the child process, so close our end of the read pipe

         close(childPipePair[0]);


         if (prefs.scite_path != NULL) {
            execlp(prefs.scite_path, prefs.scite_path, scite_arg1, scite_arg2, scite_arg3, scite_arg4, (char *) NULL);
         } else {

            // Execute Scite, if we can (Check for SciTE)
            execlp(sSciteExecName0, sSciteExecName0, scite_arg1, scite_arg2, scite_arg3, scite_arg4, (char *) NULL);

            // Apparently the execlp failed, so try the alternative SciTE executable name (scite)
            execlp(sSciteExecName1, sSciteExecName1, scite_arg1, scite_arg2, scite_arg3, scite_arg4, (char *) NULL);


            // (/usr/local/bin/SciTE)
            execlp(sSciteExecName2, sSciteExecName2, scite_arg1, scite_arg2, scite_arg3, scite_arg4, (char *) NULL);

            // (/usr/local/bin/scite)
            execlp(sSciteExecName3, sSciteExecName3, scite_arg1, scite_arg2, scite_arg3, scite_arg4, (char *) NULL);
         }

         // If we get here, the execlp() failed, so tell our parent and exit

         char *message = (gchar*)"Error: Could not execute Scite from child process";
         int messageLength = strlen(message);
         int bytesWritten;

         g_print("%s: %s\n", __func__, message);

         bytesWritten = write(childPipePair[1], message, messageLength);

         if (bytesWritten < messageLength) {
            g_print("%s: Problem sending message to parent: messageLength = %d, bytesWritten = %d\n", __func__, messageLength, bytesWritten);
         }

         close(childPipePair[1]);

         //_exit(0);
      }


      // We are the parent process, and everything looks good so far

      set_scite_launched(TRUE);


      // Close our end of the child write pipe

      close(childPipePair[1]);
      childPipePair[1] = 0;


      // Wait for Scite to create a request pipe, or for our child to report failure, or for the user to cancel

      for (usecs = 0; ; usecs += usecsDelta) {
         // If the pipe is there, stop waiting

         struct stat fileStat;

         if (stat(requestPipePath, &fileStat) == 0 && S_ISFIFO(fileStat.st_mode)) {
            break;
         }


         // Did our child send back an error message?

         FD_ZERO(&readFDS);
         FD_SET(childPipePair[0], &readFDS);
         timeVal.tv_sec = 0;
         timeVal.tv_usec = 0;

         if (select(childPipePair[0] + 1, &readFDS, NULL, NULL, &timeVal) > 0) {
            // If the child sent *any* data, it failed to exec Scite
            set_scite_launched(FALSE);
            g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Could not launch Scite, execlp( ) failed", __func__);
            goto EXITPOINT;
         }


         // No luck yet, so display a progress dialog and wait a bit longer

         if (usecs > usecsDialogDelay && !dialog) {
            dialog = gtk_message_dialog_new(get_main_window(), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_CANCEL, _("Connecting to Scite...."));
            g_signal_connect(dialog, "response", G_CALLBACK(cancel_button_cb), (void *) &userClickedCancel);
            gtk_widget_show_all(dialog);
         }

         while (gtk_events_pending()) {
            gtk_main_iteration();
         }

         if (userClickedCancel) {
            g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Could not connect to Scite", __func__);
            goto EXITPOINT;
         }

         g_usleep(usecsDelta);
      }


      // Try to open the request pipe

      if ((sRequestPipeFD = open(requestPipePath, O_WRONLY | O_NONBLOCK)) <= 0) {
         errCode = errno;
         g_set_error(err, APP_SCITEPROJ_ERROR, -1,
                     "%s: Could not launch Scite, open(\"%s\") failed, error = %s",
                     __func__, requestPipePath, strerror(errCode));
         goto EXITPOINT;
      }


      // Ask SciTE for the x11 window ID

      scite_command = (gchar*)"askproperty:x11.windowid\n";

      if (!send_scite_command(scite_command, err)) {
         goto EXITPOINT;
      }

      //	g_print("%s: Problem sending message to parent: messageLength = %d, bytesWritten = %d\n", __func__, messageLength, bytesWritten);

      // Now, let's resize the new window --
      //	if (!send_scite_command("property:position.left=300\n", err)) {
      //		goto EXITPOINT;
      //	}


      // Wow-- it all actually worked!

      resultCode = TRUE;

      set_statusbar_text(_("Launched SciTE"));
   }

   debug_printf("All done launching SciTE...\n");

EXITPOINT:

   if (dialog) gtk_widget_destroy(dialog);
   if (childPipePair[0]) close(childPipePair[0]);
   if (childPipePair[1]) close(childPipePair[1]);

   if (!resultCode) {
      shutdown_pipes();

      set_scite_launched(FALSE);
   }
   return resultCode;
}



/**
 * Send a command to Scite.
 *
 * @return TRUE on success, FALSE on failure (further details returned in err)
 *
 * @param command is the command to send to Scite
 * @param err returns any errors
 */
gboolean send_scite_command(gchar *command, GError **err)
{
   g_assert(command != NULL);

#ifdef DEBUG_SCITE
   debug_printf("send_scite_command(%s)\n",command);
#endif

   gboolean resultCode = FALSE;
   int commandLength = strlen(command);


   // Ensure we are connected to a running instance of Scite

   if (!scite_ready() && !launch_scite(NULL,err)) {
      goto EXITPOINT;
   }


   // Send the command over the request pipe....

   if ((write(sRequestPipeFD, command, commandLength)) < commandLength) {
      int errCode = errno;
      g_set_error(err, APP_SCITEPROJ_ERROR, -1,
                  "%s: Could not send command to Scite, write(\"%s\") failed, errno = %d = %s",
                  __func__, command, errCode, strerror(errCode));
      goto EXITPOINT;
   }

   resultCode = TRUE;

EXITPOINT:

   return resultCode;
}



/**
 * Activate our child SciTE instance (i.e. bring it to the front)
 *
 * @return TRUE if SciTE could be activated; FALSE otherwise (see err for more information)
 *
 * @param err returns any errors
 */
gboolean activate_scite(GError **err)
{
   gboolean finalResult = FALSE;

   XEvent event;
   Window rootWindow;
   long eventMask = SubstructureRedirectMask | SubstructureNotifyMask;


   // Open the X11 display, if we haven't already done so

   if (sDisplay == NULL) {
      char *displayName = getenv("DISPLAY");

      if (displayName == NULL || *displayName == '\0') {
         displayName = (char*)":0.0";
      }

      sDisplay = XOpenDisplay(displayName);

      if (sDisplay == NULL) {
         g_set_error(err, APP_SCITEPROJ_ERROR, -1,
                     "%s: Could not open X display, XOpenDisplay() = NULL", __func__);
         goto EXITPOINT;
      }
   }

   // Do we actually have a reference to the SciTE window?

   if (sSciteWin == 0) {
      g_set_error(err, APP_SCITEPROJ_ERROR, -1,
                  "%s: Could not activate SciTE window, X11 window ID invalid", __func__);
      goto EXITPOINT;
   }


   // Send the "_NET_ACTIVE_WINDOW" message to the ewmh-compliant window manager (we hope)

   rootWindow = DefaultRootWindow(sDisplay);

   event.xclient.type = ClientMessage;
   event.xclient.window = sSciteWin;
   event.xclient.message_type = XInternAtom(sDisplay, "_NET_ACTIVE_WINDOW", False);
   event.xclient.format = 32;
   event.xclient.data.l[0] = 1;
   event.xclient.data.l[1] = 0;
   event.xclient.data.l[2] = 0;
   event.xclient.data.l[3] = 0;
   event.xclient.data.l[4] = 0;

//~ 	g_print("%s: sDisplay = 0x%lX, message_type = 0x%lX, window = 0x%lX, rootWindow = 0x%lX\n", __func__, (long) sDisplay, (long) event.xclient.message_type, (long) event.xclient.window, (long) rootWindow);

   if (!XSendEvent(sDisplay, rootWindow, False, eventMask, &event)) {
      g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Could not activate SciTE, XSendEvent() = FALSE", __func__);
   }

   XFlush(sDisplay);

   XMapRaised(sDisplay, sSciteWin);


   finalResult = TRUE;

EXITPOINT:

   return finalResult;
}


/**
 *
 */
void set_scite_launched(gboolean launched)
{
   sSciteLaunched=launched;
}


/**
 *	scite_exists
 */
gboolean check_if_scite_exists()
{
   // Check if a config-file exists

   gboolean exists = FALSE;

#ifdef DEBUG_SCITE
   debug_printf("prefs:%s\n", prefs.scite_path);
#endif

   if (prefs.scite_path != NULL) {
      if (g_file_test(prefs.scite_path, G_FILE_TEST_EXISTS)) exists = TRUE;

   } else {
      if (g_file_test(sSciteExecName0, G_FILE_TEST_EXISTS)) exists = TRUE;
      if (g_file_test(sSciteExecName1, G_FILE_TEST_EXISTS)) exists = TRUE;

      // Check both
      gchar *test_filename = g_build_filename("/bin", sSciteExecName2, NULL);
      if (g_file_test(test_filename, G_FILE_TEST_EXISTS)) exists = TRUE;
      if (test_filename != NULL) g_free(test_filename);

      test_filename = g_build_filename("/usr/bin", sSciteExecName2, NULL);
      if (g_file_test(test_filename, G_FILE_TEST_EXISTS)) exists = TRUE;
      if (test_filename != NULL) g_free(test_filename);

      test_filename = g_build_filename("/usr/local/bin", sSciteExecName2, NULL);
      if (g_file_test(test_filename, G_FILE_TEST_EXISTS)) exists = TRUE;
      if (test_filename != NULL) g_free(test_filename);

      test_filename = g_build_filename("/bin", sSciteExecName3, NULL);
      if (g_file_test(test_filename, G_FILE_TEST_EXISTS)) exists = TRUE;
      if (test_filename != NULL) g_free(test_filename);

      test_filename = g_build_filename("/usr/bin", sSciteExecName3, NULL);
      if (g_file_test(test_filename, G_FILE_TEST_EXISTS)) exists = TRUE;
      if (test_filename != NULL) g_free(test_filename);

      test_filename = g_build_filename("/usr/local/bin", sSciteExecName3, NULL);
      if (g_file_test(test_filename, G_FILE_TEST_EXISTS)) exists = TRUE;
      if (test_filename != NULL) g_free(test_filename);
   }

   scite_exists = exists;

   return exists;
}


/**
 *		init_scite_connection
 *		currently doesn't do anything on GNU/Linux
 */
void init_scite_connection()
{

}


/**
 *
 */
gboolean open_filename(gchar *filename, gchar *project_directory, GError **err)
{
   gchar *command = NULL;

   if (!relative_path_to_abs_path(filename, &filename, project_directory, err)) {
      return FALSE;
   }

   // It's a file, so try to open it

   if ((command = g_strdup_printf("open:%s\n", filename)) == NULL) {
      g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Error formatting Scite director command, g_strdup_printf() = NULL", __func__);
      return FALSE;
   }
   else {
      if (send_scite_command(command, err)) {
         // Try to activate SciTE; ignore errors

         activate_scite(NULL);

         if (prefs.give_scite_focus == TRUE) {
            send_scite_command((gchar*)"focus:0", NULL);
         }
      }
   }

   return TRUE;
}

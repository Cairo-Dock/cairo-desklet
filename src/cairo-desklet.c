/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h> 
#include <unistd.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "config.h"
#include <cairo-dock.h>
#include "cairo-desklet-menu.h"
#include "cairo-desklet-user-interaction.h"
#include "cairo-desklet-gui-simple.h"

#define CAIRO_DOCK_THEME_SERVER "http://themes.glx-dock.org"
#define CAIRO_DOCK_CURRENT_THEME_NAME "current_theme"
// Nom du repertoire des themes extras.
#define CAIRO_DOCK_EXTRAS_DIR "extras"
// Nom du repertoire des themes de dock.
#define CAIRO_DOCK_THEMES_DIR "themes"

extern gboolean g_bUseOpenGL;
extern CairoDockDesktopEnv g_iDesktopEnv;
extern gchar *g_cConfFile;  /// en attendant d'avoir le notre...
extern gchar *g_cCurrentThemePath;

/*static void _load_manager (const gchar *cModuleName, GKeyFile *pKeyFile)
{
	CairoDockInternalModule *pInternalModule = cairo_dock_find_internal_module_from_name (cModuleName);
	gboolean r = FALSE;
	if (pInternalModule)
		r = cairo_dock_get_internal_module_config (pInternalModule, pKeyFile);
	else
		cd_warning ("couldn't load %s configuration", cModuleName);
}*/

static int _print_module_name (GldiModule *pModule, gpointer data)
{
	if (pModule->pVisitCard->iContainerType & CAIRO_DOCK_MODULE_CAN_DESKLET)
		g_print (" %s (%s)\n", pModule->pVisitCard->cModuleName, dgettext (pModule->pVisitCard->cGettextDomain, pModule->pVisitCard->cTitle));
	return 1;
}

static gboolean _start_delayed (gpointer *data)
{
	gboolean bListModules = GPOINTER_TO_INT (data[0]);
	gchar **cModulesNames = data[1];
	GldiModule *pModule;
	GError *erreur = NULL;
	
	//\___________________ activate user modules.
	if (!bListModules && !cModulesNames)
	{
		g_print ("You must specify at least 1 module to load\nAvailable modules are:\n");
		gldi_module_foreach_in_alphabetical_order ((GCompareFunc)_print_module_name, NULL);
		exit (1);
	}
	else if (bListModules)
	{
		gldi_module_foreach_in_alphabetical_order ((GCompareFunc)_print_module_name, NULL);
		exit (0);
	}
	
	g_print ("*** %s\n", cModulesNames[0]);
	g_print ("*** %s\n", cModulesNames[1]);
	int i;
	for (i = 0; cModulesNames[i] != NULL; i ++)
	{
		g_print ("+++ %s\n", cModulesNames[i]);
		pModule = gldi_module_get (cModulesNames[i]);
		if (! pModule)
		{
			cd_warning ("no such module (%s)", cModulesNames[i]);
			continue;
		}
		
		pModule->pVisitCard->iContainerType = CAIRO_DOCK_MODULE_CAN_DESKLET;
		gldi_module_activate (pModule);
		
		/**GList *m;
		for (m = pModule->pInstancesList; m != NULL; m = m->next)
		{
			GldiModuleInstance *pInstance = m->data;
			GKeyFile *pKeyFile = cairo_dock_open_key_file (pInstance->cConfFilePath);
			if (pKeyFile != NULL)
			{
				g_key_file_remove_comment (pKeyFile, "Desklet", "initially detached", NULL);
				
				g_key_file_free (pKeyFile);
			}
		}*/
	}
	return FALSE;
}

int main (int argc, char** argv)
{
	#if !defined (GLIB_VERSION_2_36) // no longer needed now... (>= 2.35)
	g_type_init (); // should initialise threads too on new versions of GLIB (>= 2.24)
	#endif
	#if (GLIB_MAJOR_VERSION == 2 && GLIB_MINOR_VERSION < 24)
	if (!g_thread_supported ())
		g_thread_init (NULL);
	#endif
	dbus_g_thread_init ();

	gtk_init (&argc, &argv);
	
	GError *erreur = NULL;
	
	//\___________________ get app's options.
	gboolean bPrintVersion = FALSE, bForceOpenGL = FALSE, bForceCairo = FALSE, bListModules = FALSE;
	gchar *cEnvironment = NULL, *cUserDefinedDataDir = NULL, *cVerbosity = 0, **cModulesNames = NULL, *cThemeServerAdress = NULL;
	GOptionEntry TableDesOptions[] =
	{
		{"modules", 'm', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_STRING_ARRAY,
			&cModulesNames,
			"modules to activate", NULL},
		{"log", 'l', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_STRING,
			&cVerbosity,
			"log verbosity (debug,message,warning,critical,error); default is warning", NULL},
		{"cairo", 'c', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
			&bForceCairo,
			"use Cairo backend", NULL},
		{"opengl", 'o', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
			&bForceOpenGL,
			"use OpenGL backend", NULL},
		{"env", 'e', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_STRING,
			&cEnvironment,
			"force the dock to consider this environnement - use it with care.", NULL},
		{"dir", 'd', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_STRING,
			&cUserDefinedDataDir,
			"force the dock to load from this directory, instead of ~/.config/cairo-dock.", NULL},
		{"version", 'v', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
			&bPrintVersion,
			"print version and quit.", NULL},
		{"server", 'S', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_STRING,
			&cThemeServerAdress,
			"Address of a server containing additional themes. This will overwrite the default server address.", NULL},
		{"List", 'L', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
			&bListModules,
			"list available applets.", NULL},
		{NULL, 0, 0, 0,
			NULL,
			NULL, NULL}
	};

	GOptionContext *context = g_option_context_new ("Cairo-Dock");
	g_option_context_add_main_entries (context, TableDesOptions, NULL);
	g_option_context_parse (context, &argc, &argv, &erreur);
	if (erreur != NULL)
	{
		g_print ("ERROR in options : %s\n", erreur->message);
		return 2;
	}
	
	if (bPrintVersion)
	{
		g_print ("%s\n", CAIRO_DESKLET_VERSION);
		return 0;
	}
	
	if (cVerbosity != NULL)
	{
		cd_log_set_level_from_name (cVerbosity);
		g_free (cVerbosity);
	}
	
	CairoDockDesktopEnv iDesktopEnv = CAIRO_DOCK_UNKNOWN_ENV;
	if (cEnvironment != NULL)
	{
		if (strcmp (cEnvironment, "gnome") == 0)
			iDesktopEnv = CAIRO_DOCK_GNOME;
		else if (strcmp (cEnvironment, "kde") == 0)
			iDesktopEnv = CAIRO_DOCK_KDE;
		else if (strcmp (cEnvironment, "xfce") == 0)
			iDesktopEnv = CAIRO_DOCK_XFCE;
		else if (strcmp (cEnvironment, "none") == 0)
			iDesktopEnv = CAIRO_DOCK_UNKNOWN_ENV;
		else
			cd_warning ("unknown environnment '%s'", cEnvironment);
		g_free (cEnvironment);
	}
	
	gchar *cRootDataDirPath;
	if (cUserDefinedDataDir != NULL)
	{
		cRootDataDirPath = cUserDefinedDataDir;
		cUserDefinedDataDir = NULL;
	}
	else
	{
		cRootDataDirPath = g_strdup_printf ("%s/.config/%s", getenv("HOME"), CAIRO_DESKLET_DATA_DIR);
	}
	
	//\___________________ internationalize the app.
	bindtextdomain (CAIRO_DESKLET_GETTEXT_PACKAGE, CAIRO_DESKLET_LOCALE_DIR);
	bind_textdomain_codeset (CAIRO_DESKLET_GETTEXT_PACKAGE, "UTF-8");
	textdomain (CAIRO_DESKLET_GETTEXT_PACKAGE);
	
	//\___________________ initialize libgldi.
	GldiRenderingMethod iRendering = (bForceOpenGL ? GLDI_OPENGL : bForceCairo ? GLDI_CAIRO : GLDI_DEFAULT);
	gldi_init (iRendering);
	
	//\___________________ set custom user options.
	if (iDesktopEnv != CAIRO_DOCK_UNKNOWN_ENV)
		cairo_dock_fm_force_desktop_env (iDesktopEnv);
	
	gchar *cExtraDirPath = g_strconcat (cRootDataDirPath, "/"CAIRO_DOCK_EXTRAS_DIR, NULL);
	gchar *cThemesDirPath = g_strconcat (cRootDataDirPath, "/"CAIRO_DOCK_THEMES_DIR, NULL);
	gchar *cCurrentThemeDirPath = g_strconcat (cRootDataDirPath, "/"CAIRO_DOCK_CURRENT_THEME_NAME, NULL);
	cairo_dock_set_paths (cRootDataDirPath, cExtraDirPath, cThemesDirPath, cCurrentThemeDirPath, NULL, NULL, cThemeServerAdress ? cThemeServerAdress : g_strdup (CAIRO_DOCK_THEME_SERVER));
	
	g_print ("\n ============================================================================ \n\tCairo-Desklet version: %s\n\tCompiled date:  %s %s\n\tRunning with OpenGL: %d\n ============================================================================\n\n",
		CAIRO_DESKLET_VERSION,
		__DATE__, __TIME__,
		g_bUseOpenGL);
	
	//\___________________ now load modules.
	gldi_modules_new_from_directory (NULL, &erreur);  // NULL <=> default directory
	if (erreur != NULL)
	{
		cd_error ("%s\n  no module available", erreur->message);
		return 1;
	}
	
	//\___________________ define GUI backend.
	cairo_dock_register_simple_gui_backend ();
	
	//\___________________ register to the useful notifications.
	gldi_object_register_notification (&myContainersMgr,
		NOTIFICATION_BUILD_CONTAINER_MENU,
		(GldiNotificationFunc) cairo_dock_notification_build_container_menu,
		GLDI_RUN_FIRST, NULL);
	gldi_object_register_notification (&myContainersMgr,
		NOTIFICATION_BUILD_ICON_MENU,
		(GldiNotificationFunc) cairo_dock_notification_build_icon_menu,
		GLDI_RUN_AFTER, NULL);
	
	gldi_object_register_notification (&myDeskletsMgr,
		NOTIFICATION_CONFIGURE_DESKLET,
		(GldiNotificationFunc) cairo_dock_notification_configure_desklet,
		GLDI_RUN_AFTER, NULL);
	
	//\___________________ set a default config if none.
	if (! g_file_test (g_cConfFile, G_FILE_TEST_EXISTS))
	{
		
		gchar *cCommand = g_strdup_printf ("/bin/cp \"%s\" \"%s\"", CAIRO_DESKLET_SHARE_DATA_DIR"/cairo-desklet.conf", g_cConfFile);
		cd_message (cCommand);
		int r = system (cCommand);
		g_free (cCommand);
	}

	/// _cairo_dock_set_signal_interception (); /// TODO

	//\___________________ initiate a primary container to make a context.
	GldiContainerAttr attr;
	memset (&attr, 0, sizeof (GldiContainerAttr));
	attr.bNoOpengl = g_bUseOpenGL;
	GldiContainer *pInvisible = (GldiContainer*) gldi_object_new (GLDI_MANAGER(&myContainersMgr), &attr);

	gtk_widget_show (pInvisible->pWidget);
	gtk_widget_hide (pInvisible->pWidget);
	
	//\___________________ load managers.
	/*_load_manager ("Desklets", pKeyFile);
	_load_manager ("Dialogs", pKeyFile);
	_load_manager ("Labels", pKeyFile);
	_load_manager ("System", pKeyFile);
	_load_manager ("Icons", pKeyFile);*/
	gldi_get_managers_config (g_cConfFile, GLDI_VERSION);  /// en fait, CAIRO_DESKLET_VERSION ...
	
	gldi_load_managers ();
	
	//\___________________ Start the applications manager.
	cairo_dock_start_applications_manager (g_pMainDock);
	
	//\___________________ activate base modules.
	GldiModule *pModule;
	pModule = gldi_module_get ("desklet rendering");
	if (pModule)
		gldi_module_activate (pModule);
	pModule = gldi_module_get ("dialog rendering");
	if (pModule)
		gldi_module_activate (pModule);
	pModule = gldi_module_get ("Dbus");
	if (pModule)
		gldi_module_activate (pModule);
	
	gpointer data[2] = {GINT_TO_POINTER (bListModules), cModulesNames};
	g_idle_add ((GSourceFunc) _start_delayed, data);
	
	gtk_main ();
	
	gldi_free_all ();
	
	#if (LIBRSVG_MAJOR_VERSION == 2 && LIBRSVG_MINOR_VERSION < 36)
	rsvg_term ();
	#endif
	xmlCleanupParser ();
	
	cd_message ("Bye bye !");
	g_print ("\033[0m\n");

	return 0;
}

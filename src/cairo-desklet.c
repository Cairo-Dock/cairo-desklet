
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h> 
#include <unistd.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include <gtk/gtkgl.h>

#include "config.h"
#include <cairo-dock.h>
#include "cairo-desklet-menu.h"
#include "cairo-desklet-user-interaction.h"
#include "cairo-desklet-gui-simple.h"

#define CAIRO_DOCK_THEME_SERVER "http://themes.glx-dock.org"

extern gboolean g_bUseOpenGL;
extern CairoDockDesktopEnv g_iDesktopEnv;
extern gchar *g_cConfFile;  /// en attendant d'avoir le notre...
extern gchar *g_cCurrentThemePath;

static void _load_internal_module_config (const gchar *cModuleName, GKeyFile *pKeyFile)
{
	CairoDockInternalModule *pInternalModule = cairo_dock_find_internal_module_from_name (cModuleName);
	gboolean r = FALSE;
	if (pInternalModule)
		r = cairo_dock_get_internal_module_config (pInternalModule, pKeyFile);
	else
		cd_warning ("couldn't load %s configuration", cModuleName);
}

static int _print_module_name (CairoDockModule *pModule, gpointer data)
{
	if (pModule->pVisitCard->iContainerType & CAIRO_DOCK_MODULE_CAN_DESKLET)
		g_print ("%s\n", pModule->pVisitCard->cModuleName);
	return 1;
}

int main (int argc, char** argv)
{
	cd_log_init(FALSE);
	//No log
	cd_log_set_level(0);
	
	gtk_init (&argc, &argv);
	
	GError *erreur = NULL;
	
	//\___________________ On recupere quelques options.
	gboolean bSafeMode = FALSE, bMaintenance = FALSE, bNoSticky = FALSE, bNormalHint = FALSE, bCappuccino = FALSE, bPrintVersion = FALSE, bTesting = FALSE, bForceIndirectRendering = FALSE, bForceOpenGL = FALSE, bForceCairo = FALSE, bListModules = FALSE;
	gchar *cEnvironment = NULL, *cUserDefinedDataDir = NULL, *cVerbosity = 0, *cUserDefinedModuleDir = NULL, *cExcludeModule = NULL, **cModulesNames = NULL, *cThemeServerAdress = NULL;
	GOptionEntry TableDesOptions[] =
	{
		{"modules", 'm', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_STRING_ARRAY,
			&cModulesNames,
			"log verbosity (debug,message,warning,critical,error); default is warning", NULL},
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
			"list all the available modules.", NULL},
		{"List", 'L', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
			&bListModules,
			"list of available applets.", NULL},
		{NULL}
	};

	GOptionContext *context = g_option_context_new ("Cairo-Dock");
	g_option_context_add_main_entries (context, TableDesOptions, NULL);
	g_option_context_parse (context, &argc, &argv, &erreur);
	if (erreur != NULL)
	{
		g_print ("ERROR in options : %s\n", erreur->message);
		return 1;
	}
	
	if (bPrintVersion)
	{
		g_print ("%s\n", CAIRO_DESKLET_VERSION);
		return 0;
	}
	
	if (cModulesNames == NULL && !bListModules)
	{
		g_print ("You must specify at least 1 module to load\n");
		/// affcher la liste des modules ...
		exit (1);
	}
	
	cd_log_set_level_from_name (cVerbosity);
	g_free (cVerbosity);
	
	if (cEnvironment != NULL)
	{
		if (strcmp (cEnvironment, "gnome") == 0)
			g_iDesktopEnv = CAIRO_DOCK_GNOME;
		else if (strcmp (cEnvironment, "kde") == 0)
			g_iDesktopEnv = CAIRO_DOCK_KDE;
		else if (strcmp (cEnvironment, "xfce") == 0)
			g_iDesktopEnv = CAIRO_DOCK_XFCE;
		else if (strcmp (cEnvironment, "none") == 0)
			g_iDesktopEnv = CAIRO_DOCK_UNKNOWN_ENV;
		else
			cd_warning ("unknown environnment '%s'", cEnvironment);
		g_free (cEnvironment);
	}
	
	//\___________________ On internationalise l'appli.
	bindtextdomain (CAIRO_DESKLET_GETTEXT_PACKAGE, CAIRO_DESKLET_LOCALE_DIR);
	bind_textdomain_codeset (CAIRO_DESKLET_GETTEXT_PACKAGE, "UTF-8");
	textdomain (CAIRO_DESKLET_GETTEXT_PACKAGE);

	//\___________________ On definit les repertoires des donnees.
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
	gboolean bFirstLaunch = ! g_file_test (cRootDataDirPath, G_FILE_TEST_IS_DIR);
	
	gchar *cExtraDirPath = g_strconcat (cRootDataDirPath, "/"CAIRO_DOCK_EXTRAS_DIR, NULL);
	gchar *cThemesDirPath = g_strconcat (cRootDataDirPath, "/"CAIRO_DOCK_THEMES_DIR, NULL);
	gchar *cCurrentThemeDirPath = g_strconcat (cRootDataDirPath, "/"CAIRO_DOCK_CURRENT_THEME_NAME, NULL);
	cairo_dock_set_paths (cRootDataDirPath, cExtraDirPath, cThemesDirPath, cCurrentThemeDirPath, cThemeServerAdress ? cThemeServerAdress : g_strdup (CAIRO_DOCK_THEME_SERVER));
	
	  /////////////
	 //// LIB ////
	/////////////

	//\___________________ On initialise le gestionnaire de docks (a faire en 1er).
	cairo_dock_init_dock_manager ();
	
	//\___________________ On initialise le gestionnaire de desklets.
	cairo_dock_init_desklet_manager ();
	
	//\___________________ On initialise le gestionnaire de dialogues.
	cairo_dock_init_dialog_manager ();
	
	//\___________________ On initialise le gestionnaire de backends (vues, etc).
	cairo_dock_init_backends_manager ();
	
	//\___________________ On initialise le gestionnaire des indicateurs.
	cairo_dock_init_indicator_manager ();
	
	//\___________________ On initialise le multi-threading.
	if (!g_thread_supported ())
		g_thread_init (NULL);
	
	//\___________________ On demarre le support de X.
	cairo_dock_start_X_manager ();
	
	//\___________________ On initialise le keybinder.
	cd_keybinder_init();
	
	//\___________________ On detecte l'environnement de bureau (apres X et avant les modules).
	if (g_iDesktopEnv == CAIRO_DOCK_UNKNOWN_ENV)
		g_iDesktopEnv = cairo_dock_guess_environment ();
	cd_debug ("environnement de bureau : %d", g_iDesktopEnv);
	
	//\___________________ On enregistre les implementations.
	cairo_dock_register_built_in_data_renderers ();
	
	///cairo_dock_register_hiding_effects ();
	///g_pKeepingBelowBackend = cairo_dock_get_hiding_effect ("Fade out");
	
	///cairo_dock_register_icon_container_renderers ();
	
	//\___________________ On enregistre les notifications de base.
	cairo_dock_register_notification (CAIRO_DOCK_RENDER_ICON,
		(CairoDockNotificationFunc) cairo_dock_render_icon_notification,
		CAIRO_DOCK_RUN_FIRST, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_INSERT_ICON,
		(CairoDockNotificationFunc) cairo_dock_on_insert_remove_icon_notification,
		CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_REMOVE_ICON,
		(CairoDockNotificationFunc) cairo_dock_on_insert_remove_icon_notification,
		CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_UPDATE_ICON,
		(CairoDockNotificationFunc) cairo_dock_update_inserting_removing_icon_notification,
		CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_STOP_ICON,
		(CairoDockNotificationFunc) cairo_dock_stop_inserting_removing_icon_notification,
		CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_UPDATE_FLYING_CONTAINER,
		(CairoDockNotificationFunc) cairo_dock_update_flying_container_notification,
		CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_RENDER_FLYING_CONTAINER,
		(CairoDockNotificationFunc) cairo_dock_render_flying_container_notification,
		CAIRO_DOCK_RUN_AFTER, NULL);
	
	  /////////////
	 //// APP ////
	/////////////
	
	if (bForceOpenGL || ! bForceCairo)
		cairo_dock_initialize_opengl_backend (FALSE, bForceOpenGL);
	
	g_print ("\n ============================================================================ \n\tCairo-Desklet version: %s\n\tCompiled date:  %s %s\n\tRunning with OpenGL: %d\n ============================================================================\n\n",
		CAIRO_DESKLET_VERSION,
		__DATE__, __TIME__,
		g_bUseOpenGL);
	
	//\___________________ On initialise le gestionnaire de modules et on pre-charge les modules existant (il faut le faire apres savoir si on utilise l'OpenGL).
	if (g_module_supported ())
	{
		cairo_dock_initialize_module_manager (cairo_dock_get_modules_dir ());
	}
	else
		cairo_dock_initialize_module_manager (NULL);
	
	if (bListModules)
	{
		cairo_dock_foreach_module_in_alphabetical_order ((GCompareFunc)_print_module_name, NULL);
		if (cModulesNames == NULL)
			exit (1);
	}
	
	//\___________________ On definit le backend des GUI.
	///cairo_dock_load_user_gui_backend ();
	///cairo_dock_register_default_launcher_gui_backend ();
	cairo_dock_register_simple_gui_backend ();
	
	//\___________________ On enregistre la vue par defaut.
	///cairo_dock_register_default_renderer ();
	
	//\___________________ On enregistre nos notifications.
	cairo_dock_register_notification (CAIRO_DOCK_CLICK_ICON,
		(CairoDockNotificationFunc) cairo_dock_notification_click_icon,
		CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_BUILD_CONTAINER_MENU,
		(CairoDockNotificationFunc) cairo_dock_notification_build_container_menu,
		CAIRO_DOCK_RUN_FIRST, NULL);
	cairo_dock_register_notification (CAIRO_DOCK_BUILD_ICON_MENU,
		(CairoDockNotificationFunc) cairo_dock_notification_build_icon_menu,
		CAIRO_DOCK_RUN_AFTER, NULL);
	
	//\___________________ On recupere la config des modules internes.
	if (! g_file_test (g_cConfFile, G_FILE_TEST_EXISTS))
	{
		gchar *cCommand = g_strdup_printf ("/bin/cp \"%s\" \"%s\"", CAIRO_DESKLET_SHARE_DATA_DIR"/cairo-desklet.conf", g_cConfFile);
		cd_message (cCommand);
		int r = system (cCommand);
		g_free (cCommand);
	}
	
	GKeyFile *pKeyFile = cairo_dock_open_key_file (g_cConfFile);
	if (pKeyFile != NULL)
	{
		_load_internal_module_config ("Desklets", pKeyFile);
		
		_load_internal_module_config ("Dialogs", pKeyFile);
		
		_load_internal_module_config ("Labels", pKeyFile);
		
		_load_internal_module_config ("System", pKeyFile);
		
		g_key_file_free (pKeyFile);
	}
	
	//\___________________ On active les modules necessaires.
	CairoDockModule *pModule;
	pModule = cairo_dock_find_module_from_name ("desklet rendering");
	if (pModule)
		cairo_dock_activate_module (pModule, NULL);
	pModule = cairo_dock_find_module_from_name ("dialog rendering");
	if (pModule)
		cairo_dock_activate_module (pModule, NULL);
	pModule = cairo_dock_find_module_from_name ("Dbus");
	if (pModule)
		cairo_dock_activate_module (pModule, NULL);
	
	//\___________________ On active les modules utilisateurs.
	int i;
	for (i = 0; cModulesNames[i] != NULL; i ++)
	{
		pModule = cairo_dock_find_module_from_name (cModulesNames[i]);
		if (! pModule)
		{
			cd_warning ("no such module (%s)", cModulesNames[i]);
			continue;
		}
		
		pModule->pVisitCard->iContainerType = CAIRO_DOCK_MODULE_CAN_DESKLET;
		cairo_dock_activate_module (pModule, &erreur);
		if (erreur != NULL)
		{
			cd_warning (erreur->message);
			g_error_free (erreur);
			erreur = NULL;
			continue;
		}
		
		/**GList *m;
		for (m = pModule->pInstancesList; m != NULL; m = m->next)
		{
			CairoDockModuleInstance *pInstance = m->data;
			GKeyFile *pKeyFile = cairo_dock_open_key_file (pInstance->cConfFilePath);
			if (pKeyFile != NULL)
			{
				g_key_file_remove_comment (pKeyFile, "Desklet", "initially detached", NULL);
				
				g_key_file_free (pKeyFile);
			}
		}*/
	}
	
	gtk_main ();
	
	cairo_dock_free_all ();
	
	rsvg_term ();
	xmlCleanupParser ();
	
	cd_message ("Bye bye !");
	g_print ("\033[0m\n");

	return 0;
}

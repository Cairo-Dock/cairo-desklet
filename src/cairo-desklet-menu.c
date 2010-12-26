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
#define __USE_POSIX 1
#include <time.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <cairo.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <gdk/gdkx.h>

#include "config.h"
#include <cairo-dock.h>
#include "cairo-desklet-user-interaction.h"
#include "cairo-desklet-menu.h"

#define CAIRO_DOCK_CONF_PANEL_WIDTH 800
#define CAIRO_DOCK_CONF_PANEL_HEIGHT 600
#define CAIRO_DOCK_LAUNCHER_PANEL_WIDTH 600
#define CAIRO_DOCK_LAUNCHER_PANEL_HEIGHT 350
#define CAIRO_DOCK_FILE_HOST_URL "https://launchpad.net/cairo-dock"  // https://developer.berlios.de/project/showfiles.php?group_id=8724
#define CAIRO_DOCK_SITE_URL "http://glx-dock.org"  // http://cairo-dock.vef.fr
#define CAIRO_DOCK_FORUM_URL "http://forum.glx-dock.org"  // http://cairo-dock.vef.fr/bg_forumlist.php
#define CAIRO_DOCK_PLUGINS_EXTRAS_URL "http://www.glx-dock.org/mc_album.php?a=4"

extern CairoDockDesktopGeometry g_desktopGeometry;

extern gchar *g_cConfFile;
extern gchar *g_cCurrentLaunchersPath;
extern gchar *g_cCurrentThemePath;

extern gboolean g_bEasterEggs;


static void _cairo_dock_add_about_page (GtkWidget *pNoteBook, const gchar *cPageLabel, const gchar *cAboutText)
{
	GtkWidget *pVBox, *pScrolledWindow;
	GtkWidget *pPageLabel, *pAboutLabel;
	
	pPageLabel = gtk_label_new (cPageLabel);
	pVBox = gtk_vbox_new (FALSE, 0);
	pScrolledWindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (pScrolledWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (pScrolledWindow), pVBox);
	gtk_notebook_append_page (GTK_NOTEBOOK (pNoteBook), pScrolledWindow, pPageLabel);
	
	pAboutLabel = gtk_label_new (NULL);
	gtk_label_set_use_markup (GTK_LABEL (pAboutLabel), TRUE);
	gtk_box_pack_start (GTK_BOX (pVBox),
		pAboutLabel,
		FALSE,
		FALSE,
		0);
	gtk_label_set_markup (GTK_LABEL (pAboutLabel), cAboutText);
}
static void _cairo_dock_about (GtkMenuItem *pMenuItem, CairoContainer *pContainer)
{
	GtkWidget *pDialog = gtk_message_dialog_new (GTK_WINDOW (pContainer->pWidget),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		/*GTK_MESSAGE_INFO*/GTK_MESSAGE_OTHER,
		GTK_BUTTONS_CLOSE,
		NULL/*"\nCairo-Dock (2007-2010)\n version "CAIRO_DESKLET_VERSION*/);
	
#if (GTK_MAJOR_VERSION > 2 || GTK_MINOR_VERSION >= 14)
	GtkWidget *pContentBox = gtk_dialog_get_content_area (GTK_DIALOG(pDialog));
#else
	GtkWidget *pContentBox =  GTK_DIALOG(pDialog)->vbox;
#endif
	GtkWidget *pHBox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (pContentBox), pHBox, FALSE, FALSE, 0);
	
	const gchar *cImagePath = CAIRO_DESKLET_SHARE_DATA_DIR"/"CAIRO_DESKLET_LOGO;
	GtkWidget *pImage = gtk_image_new_from_file (cImagePath);
	gtk_box_pack_start (GTK_BOX (pHBox), pImage, FALSE, FALSE, 0);
	
	GtkWidget *pVBox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (pHBox), pVBox, FALSE, FALSE, 0);
	
	GtkWidget *pLink = gtk_link_button_new_with_label (CAIRO_DOCK_SITE_URL, "Cairo-Dock (2007-2010)\n version "CAIRO_DESKLET_VERSION);
	gtk_box_pack_start (GTK_BOX (pVBox), pLink, FALSE, FALSE, 0);
	
	pLink = gtk_link_button_new_with_label (CAIRO_DOCK_FORUM_URL, _("Community site"));
	gtk_widget_set_tooltip_text (pLink, _("Problems? Suggestions? Just want to talk to us? Come on over!"));
	gtk_box_pack_start (GTK_BOX (pVBox), pLink, FALSE, FALSE, 0);
	
	pLink = gtk_link_button_new_with_label (CAIRO_DOCK_FILE_HOST_URL, _("Development site"));
	gtk_widget_set_tooltip_text (pLink, _("Find the latest version of Cairo-Dock here !"));
	gtk_box_pack_start (GTK_BOX (pVBox), pLink, FALSE, FALSE, 0);
	
	pLink = gtk_link_button_new_with_label (CAIRO_DOCK_PLUGINS_EXTRAS_URL, _("Cairo-Dock-Plug-ins-Extras"));
	gtk_widget_set_tooltip_text (pLink, _("Other applets"));
	gtk_box_pack_start (GTK_BOX (pVBox), pLink, FALSE, FALSE, 0);
	
	GtkWidget *pNoteBook = gtk_notebook_new ();
	gtk_notebook_set_scrollable (GTK_NOTEBOOK (pNoteBook), TRUE);
	gtk_notebook_popup_enable (GTK_NOTEBOOK (pNoteBook));
	gtk_box_pack_start (GTK_BOX (pContentBox), pNoteBook, TRUE, TRUE, 0);
	
	_cairo_dock_add_about_page (pNoteBook,
		_("Development"),
		"<b>Main developer :</b>\n  Fabounet (Fabrice Rey)\n\
<b>Original idea/first development :</b>\n  Mac Slow\n\
<b>Applets :</b>\n  Fabounet\n  Necropotame\n  Ctaf\n  ChAnGFu\n  Tofe\n  Paradoxxx_Zero\n  Mav\n  Nochka85\n  Ours_en_pluche\n  Eduardo Mucelli\n\
<b>Patchs :</b>\n  Special thanks to Augur for his great help with OpenGL\n  Ctaf\n  M.Tasaka\n  Matttbe\n  Necropotame\n  Robrob\n  Smidgey\n  Tshirtman\n");
	_cairo_dock_add_about_page (pNoteBook,
		_("Artwork"),
		"<b>Themes :</b>\n  Fabounet\n  Chilperik\n  Djoole\n  Glattering\n  Vilraleur\n  Lord Northam\n  Paradoxxx_Zero\n  Coz\n  Benoit2600\n  Nochka85\n  Taiebot65\n  Lylambda\n  MastroPino\n\
<b>Translations :</b>\n  Fabounet\n  Ppmt \n  Jiro Kawada (Kawaji)\n  BiAji\n  Mattia Tavernini (Maathias)\n  Peter Thornqvist\n  Yannis Kaskamanidis\n  Eduardo Mucelli\n");
	_cairo_dock_add_about_page (pNoteBook,
		_("Support"),
		"<b>Installation script and web hosting :</b>\n  Mav\n\
<b>Site (glx-dock.org) :</b>\n  Necropotame\n  Matttbe\n  Tdey\n\
<b>LaunchPad :</b>\n  Matttbe\n  Mav\n\
<b>Suggestions/Comments/Beta-Testers :</b>\n  AuraHxC\n  Chilperik\n  Cybergoll\n  Damster\n  Djoole\n  Glattering\n  Franksuse64\n  Mav\n  Necropotame\n  Nochka85\n  Ppmt\n  RavanH\n  Rhinopierroce\n  Rom1\n  Sombrero\n  Vilraleur");
	
	gtk_widget_show_all (pDialog);
	gtk_window_set_position (GTK_WINDOW (pDialog), GTK_WIN_POS_CENTER_ALWAYS);  // un GTK_WIN_POS_CENTER simple ne marche pas, probablement parceque la fenetre n'est pas encore realisee. le 'always' ne pose pas de probleme, puisqu'on ne peut pas redimensionner le dialogue.
	gtk_window_set_keep_above (GTK_WINDOW (pDialog), TRUE);
	gtk_dialog_run (GTK_DIALOG (pDialog));
	gtk_widget_destroy (pDialog);
}

static void _launch_url (const gchar *cURL)
{
	if  (! cairo_dock_fm_launch_uri (cURL))
	{
		gchar *cCommand = g_strdup_printf ("\
which xdg-open > /dev/null && xdg-open %s || \
which firefox > /dev/null && firefox %s || \
which konqueror > /dev/null && konqueror %s || \
which iceweasel > /dev/null && konqueror %s || \
which opera > /dev/null && opera %s ",
			cURL,
			cURL,
			cURL,
			cURL,
			cURL);  // pas super beau mais efficace ^_^
		int r = system (cCommand);
		g_free (cCommand);
	}
}
static void _cairo_dock_show_third_party_applets (GtkMenuItem *pMenuItem, gpointer data)
{
	_launch_url ("http://www.glx-dock.org/mc_album.php?a=4");
}

static void _cairo_dock_quit (GtkMenuItem *pMenuItem, CairoContainer *pContainer)
{
	//cairo_dock_on_delete (pDock->container.pWidget, NULL, pDock);
	Icon *pIcon = NULL;
	if (CAIRO_DOCK_IS_DESKLET (pContainer))
		pIcon = CAIRO_DESKLET (pContainer)->pIcon;
	
	int answer = cairo_dock_ask_question_and_wait (_("Quit Cairo-Desklet?"), pIcon, pContainer);
	cd_debug ("quit : %d (yes:%d)\n", answer, GTK_RESPONSE_YES);
	if (answer == GTK_RESPONSE_YES)
		gtk_main_quit ();
}


gboolean cairo_dock_notification_build_container_menu (gpointer *pUserData, Icon *icon, CairoContainer *pContainer, GtkWidget *menu, gboolean *bDiscardMenu)
{
	//\_________________________ On ajoute le sous-menu Cairo-Dock, toujours present.
	GtkWidget *pMenuItem, *image;
	GdkPixbuf *pixbuf;
	pMenuItem = gtk_image_menu_item_new_with_label ("Cairo-Desklet");
	pixbuf = gdk_pixbuf_new_from_file_at_size (CAIRO_DESKLET_SHARE_DATA_DIR"/"CAIRO_DESKLET_ICON, 32, 32, NULL);
	image = gtk_image_new_from_pixbuf (pixbuf);
	g_object_unref (pixbuf);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (pMenuItem), image);
	gtk_menu_shell_append  (GTK_MENU_SHELL (menu), pMenuItem);
	
	GtkWidget *pSubMenu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (pMenuItem), pSubMenu);
	
	pMenuItem = cairo_dock_add_in_menu_with_stock_and_data (_("Get more applets!"),
		GTK_STOCK_ADD,
		(GFunc)_cairo_dock_show_third_party_applets,
		pSubMenu,
		NULL);
	gtk_widget_set_tooltip_text (pMenuItem, _("Third-party applets provide integration with many programs, like Pidgin"));
	
	cairo_dock_add_in_menu_with_stock_and_data (_("About"),
		GTK_STOCK_ABOUT,
		(GFunc)_cairo_dock_about,
		pSubMenu,
		pContainer);
	
	cairo_dock_add_in_menu_with_stock_and_data (_("Quit"),
		GTK_STOCK_QUIT,
		(GFunc)_cairo_dock_quit,
		pSubMenu,
		pContainer);
}


  ///////////////////////////////////////////////////////////////////
 /////////// LES OPERATIONS SUR LES LANCEURS ///////////////////////
///////////////////////////////////////////////////////////////////

#define _add_entry_in_menu(cLabel, gtkStock, pCallBack, pSubMenu) cairo_dock_add_in_menu_with_stock_and_data (cLabel, gtkStock, (GFunc) (pCallBack), pSubMenu, data)

  //////////////////////////////////////////////////////////////////
 /////////// LES OPERATIONS SUR LES APPLETS ///////////////////////
//////////////////////////////////////////////////////////////////

static void _cairo_dock_initiate_config_module (GtkMenuItem *pMenuItem, gpointer *data)
{
	cd_debug ("%s ()\n", __func__);
	Icon *icon = data[0];
	CairoContainer *pContainer= data[1];
	if (CAIRO_DOCK_IS_DESKLET (pContainer))
		icon = (CAIRO_DESKLET (pContainer))->pIcon;  // l'icone cliquee du desklet n'est pas forcement celle qui contient le module !
	g_return_if_fail (CAIRO_DOCK_IS_APPLET (icon));
	
	cairo_dock_show_module_instance_gui (icon->pModuleInstance, -1);
}

static void _cairo_dock_remove_module_instance (GtkMenuItem *pMenuItem, gpointer *data)
{
	Icon *icon = data[0];
	CairoContainer *pContainer= data[1];
	if (CAIRO_DOCK_IS_DESKLET (pContainer))
		icon = (CAIRO_DESKLET (pContainer))->pIcon;  // l'icone cliquee du desklet n'est pas forcement celle qui contient le module !
	g_return_if_fail (CAIRO_DOCK_IS_APPLET (icon));
	
	gchar *question = g_strdup_printf (_("You're about to remove this applet (%s) from the dock. Are you sure?"), icon->pModuleInstance->pModule->pVisitCard->cModuleName);
	int answer = cairo_dock_ask_question_and_wait (question, icon, CAIRO_CONTAINER (pContainer));
	if (answer == GTK_RESPONSE_YES)
	{
		cairo_dock_remove_module_instance (icon->pModuleInstance);
	}
}

static void _cairo_dock_add_module_instance (GtkMenuItem *pMenuItem, gpointer *data)
{
	Icon *icon = data[0];
	CairoContainer *pContainer= data[1];
	if (CAIRO_DOCK_IS_DESKLET (pContainer))
		icon = (CAIRO_DESKLET (pContainer))->pIcon;  // l'icone cliquee du desklet n'est pas forcement celle qui contient le module !
	g_return_if_fail (CAIRO_DOCK_IS_APPLET (icon));
	
	cairo_dock_add_module_instance (icon->pModuleInstance->pModule);
}

  /////////////////////////////////////////////////////////////////
 /////////// LES OPERATIONS SUR LES APPLIS ///////////////////////
/////////////////////////////////////////////////////////////////

static void _cairo_dock_close_appli (GtkMenuItem *pMenuItem, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	if (CAIRO_DOCK_IS_APPLI (icon))
		cairo_dock_close_xwindow (icon->Xid);
}

static void _cairo_dock_kill_appli (GtkMenuItem *pMenuItem, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	if (CAIRO_DOCK_IS_APPLI (icon))
		cairo_dock_kill_xwindow (icon->Xid);
}

static void _cairo_dock_minimize_appli (GtkMenuItem *pMenuItem, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	if (CAIRO_DOCK_IS_APPLI (icon))
	{
		cairo_dock_minimize_xwindow (icon->Xid);
	}
}

static void _cairo_dock_show_appli (GtkMenuItem *pMenuItem, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	if (CAIRO_DOCK_IS_APPLI (icon))
	{
		cairo_dock_show_xwindow (icon->Xid);
	}
}

static void _cairo_dock_maximize_appli (GtkMenuItem *pMenuItem, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	if (CAIRO_DOCK_IS_APPLI (icon))
	{
		cairo_dock_maximize_xwindow (icon->Xid, ! icon->bIsMaximized);
	}
}

static void _cairo_dock_set_appli_fullscreen (GtkMenuItem *pMenuItem, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	if (CAIRO_DOCK_IS_APPLI (icon))
	{
		cairo_dock_set_xwindow_fullscreen (icon->Xid, ! icon->bIsFullScreen);
	}
}

static void _cairo_dock_move_appli_to_current_desktop (GtkMenuItem *pMenuItem, gpointer *data)
{
	//g_print ("%s ()\n", __func__);
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	if (CAIRO_DOCK_IS_APPLI (icon))
	{
		cairo_dock_move_window_to_current_desktop (icon);
		if (!icon->bIsHidden)
			cairo_dock_show_xwindow (icon->Xid);
	}
}

static void _cairo_dock_move_appli_to_desktop (GtkMenuItem *pMenuItem, gpointer *user_data)
{
	gpointer *data = user_data[0];
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	int iDesktopNumber = GPOINTER_TO_INT (user_data[1]);
	int iViewPortNumberY = GPOINTER_TO_INT (user_data[2]);
	int iViewPortNumberX = GPOINTER_TO_INT (user_data[3]);
	cd_message ("%s (%d;%d;%d)", __func__, iDesktopNumber, iViewPortNumberX, iViewPortNumberY);
	if (CAIRO_DOCK_IS_APPLI (icon))
	{
		cairo_dock_move_window_to_desktop (icon, iDesktopNumber, iViewPortNumberX, iViewPortNumberY);
	}
}

static void _cairo_dock_change_window_above (GtkMenuItem *pMenuItem, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	if (CAIRO_DOCK_IS_APPLI (icon))
	{
		gboolean bIsAbove=FALSE, bIsBelow=FALSE;
		cairo_dock_xwindow_is_above_or_below (icon->Xid, &bIsAbove, &bIsBelow);
		cairo_dock_set_xwindow_above (icon->Xid, ! bIsAbove);
	}
}

  ///////////////////////////////////////////////////////////////////
 ///////////////// LES OPERATIONS SUR LES DESKLETS /////////////////
///////////////////////////////////////////////////////////////////

static inline void _cairo_dock_set_desklet_accessibility (CairoDesklet *pDesklet, CairoDeskletVisibility iVisibility)
{
	cairo_dock_set_desklet_accessibility (pDesklet, iVisibility, TRUE);  // TRUE <=> save state in conf.
	cairo_dock_gui_trigger_update_desklet_visibility (pDesklet);
}
static void _cairo_dock_keep_below (GtkCheckMenuItem *pMenuItem, gpointer *data)
{
	CairoDesklet *pDesklet = data[1];
	if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (pMenuItem)))
		_cairo_dock_set_desklet_accessibility (pDesklet, CAIRO_DESKLET_KEEP_BELOW);
}

static void _cairo_dock_keep_normal (GtkCheckMenuItem *pMenuItem, gpointer *data)
{
	CairoDesklet *pDesklet = data[1];
	if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (pMenuItem)))
		_cairo_dock_set_desklet_accessibility (pDesklet, CAIRO_DESKLET_NORMAL);
}

static void _cairo_dock_keep_above (GtkCheckMenuItem *pMenuItem, gpointer *data)
{
	CairoDesklet *pDesklet = data[1];
	if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (pMenuItem)))
		_cairo_dock_set_desklet_accessibility (pDesklet, CAIRO_DESKLET_KEEP_ABOVE);
}

static void _cairo_dock_keep_on_widget_layer (GtkMenuItem *pMenuItem, gpointer *data)
{
	CairoDesklet *pDesklet = data[1];
	if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (pMenuItem)))
		_cairo_dock_set_desklet_accessibility (pDesklet, CAIRO_DESKLET_ON_WIDGET_LAYER);
}

static void _cairo_dock_keep_space (GtkCheckMenuItem *pMenuItem, gpointer *data)
{
	CairoDesklet *pDesklet = data[1];
	if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (pMenuItem)))
		_cairo_dock_set_desklet_accessibility (pDesklet, CAIRO_DESKLET_RESERVE_SPACE);
}

static void _cairo_dock_set_on_all_desktop (GtkCheckMenuItem *pMenuItem, gpointer *data)
{
	CairoDesklet *pDesklet = data[1];
	gboolean bSticky = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (pMenuItem));
	cairo_dock_set_desklet_sticky (pDesklet, bSticky);
}

static void _cairo_dock_lock_position (GtkMenuItem *pMenuItem, gpointer *data)
{
	CairoDesklet *pDesklet = data[1];
	gboolean bLocked = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (pMenuItem));
	cairo_dock_lock_desklet_position (pDesklet, bLocked);
}


static void _add_desktops_entry (GtkWidget *pMenu, gpointer data)
{
	static gpointer *s_pDesktopData = NULL;
	GtkWidget *pMenuItem, *image;
	
	if (g_desktopGeometry.iNbDesktops > 1 || g_desktopGeometry.iNbViewportX > 1 || g_desktopGeometry.iNbViewportY > 1)
	{
		int i, j, k, iDesktopCode;
		const gchar *cLabel;
		if (g_desktopGeometry.iNbDesktops > 1 && (g_desktopGeometry.iNbViewportX > 1 || g_desktopGeometry.iNbViewportY > 1))
			cLabel = _("Move to desktop %d - face %d");
		else if (g_desktopGeometry.iNbDesktops > 1)
			cLabel = _("Move to desktop %d");
		else
			cLabel = _("Move to face %d");
		GString *sDesktop = g_string_new ("");
		g_free (s_pDesktopData);
		s_pDesktopData = g_new0 (gpointer, 4 * g_desktopGeometry.iNbDesktops * g_desktopGeometry.iNbViewportX * g_desktopGeometry.iNbViewportY);
		gpointer *user_data;
		
		for (i = 0; i < g_desktopGeometry.iNbDesktops; i ++)  // on range par bureau.
		{
			for (j = 0; j < g_desktopGeometry.iNbViewportY; j ++)  // puis par rangee.
			{
				for (k = 0; k < g_desktopGeometry.iNbViewportX; k ++)
				{
					if (g_desktopGeometry.iNbDesktops > 1 && (g_desktopGeometry.iNbViewportX > 1 || g_desktopGeometry.iNbViewportY > 1))
						g_string_printf (sDesktop, cLabel, i+1, j*g_desktopGeometry.iNbViewportX+k+1);
					else if (g_desktopGeometry.iNbDesktops > 1)
						g_string_printf (sDesktop, cLabel, i+1);
					else
						g_string_printf (sDesktop, cLabel, j*g_desktopGeometry.iNbViewportX+k+1);
					iDesktopCode = i * g_desktopGeometry.iNbViewportY * g_desktopGeometry.iNbViewportX + j * g_desktopGeometry.iNbViewportY + k;
					user_data = &s_pDesktopData[4*iDesktopCode];
					user_data[0] = data;
					user_data[1] = GINT_TO_POINTER (i);
					user_data[2] = GINT_TO_POINTER (j);
					user_data[3] = GINT_TO_POINTER (k);
					
					cairo_dock_add_in_menu_with_stock_and_data (sDesktop->str, NULL, (GFunc)(_cairo_dock_move_appli_to_desktop), pMenu, user_data);
				}
			}
		}
		g_string_free (sDesktop, TRUE);
	}
}

gboolean cairo_dock_notification_build_icon_menu (gpointer *pUserData, Icon *icon, CairoContainer *pContainer, GtkWidget *menu)
{
	static gpointer *data = NULL;
	//g_print ("%x;%x;%x\n", icon, pContainer, menu);
	if (data == NULL)
		data = g_new (gpointer, 3);
	data[0] = icon;
	data[1] = pContainer;
	data[2] = menu;
	GtkWidget *pMenuItem, *image;
	
	//\_________________________ On rajoute les actions sur les icones d'applis.
	if (CAIRO_DOCK_IS_APPLI (icon))
	{
		pMenuItem = gtk_separator_menu_item_new ();
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), pMenuItem);
		
		//\_________________________ On rajoute les actions supplementaires sur les icones d'applis.
		pMenuItem = gtk_menu_item_new_with_label (_("Other actions"));
		gtk_menu_shell_append  (GTK_MENU_SHELL (menu), pMenuItem);
		GtkWidget *pSubMenuOtherActions = gtk_menu_new ();
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (pMenuItem), pSubMenuOtherActions);
		
		_add_entry_in_menu (_("Move to this desktop"), GTK_STOCK_JUMP_TO, _cairo_dock_move_appli_to_current_desktop, pSubMenuOtherActions);
		
		Icon *pAppli = cairo_dock_get_icon_with_Xid (icon->Xid);
		if (pAppli)
		{
			icon->bIsMaximized = pAppli->bIsMaximized;
			icon->bIsFullScreen = pAppli->bIsFullScreen;
		}
		else
		{
			icon->bIsMaximized = cairo_dock_xwindow_is_maximized (icon->Xid);
			icon->bIsFullScreen = cairo_dock_xwindow_is_fullscreen (icon->Xid);
		}
		
		_add_entry_in_menu (icon->bIsFullScreen ? _("Not Fullscreen") : _("Fullscreen"), icon->bIsFullScreen ? GTK_STOCK_LEAVE_FULLSCREEN : GTK_STOCK_FULLSCREEN, _cairo_dock_set_appli_fullscreen, pSubMenuOtherActions);
		
		gboolean bIsAbove=FALSE, bIsBelow=FALSE;
		cairo_dock_xwindow_is_above_or_below (icon->Xid, &bIsAbove, &bIsBelow);
		_add_entry_in_menu (bIsAbove ? _("Don't keep above") : _("Keep above"), bIsAbove ? GTK_STOCK_GOTO_BOTTOM : GTK_STOCK_GOTO_TOP, _cairo_dock_change_window_above, pSubMenuOtherActions);
		
		_add_desktops_entry (pSubMenuOtherActions, data);
		
		_add_entry_in_menu (_("Kill"), GTK_STOCK_CANCEL, _cairo_dock_kill_appli, pSubMenuOtherActions);
		
		_add_entry_in_menu (_("Show"), GTK_STOCK_FIND, _cairo_dock_show_appli, menu);
		
		_add_entry_in_menu (icon->bIsMaximized ? _("Unmaximise") : _("Maximise"), icon->bIsMaximized ? CAIRO_DESKLET_SHARE_DATA_DIR"/icon-restore.png" : CAIRO_DESKLET_SHARE_DATA_DIR"/icon-maximize.png", _cairo_dock_maximize_appli, menu);
		
		if (! icon->bIsHidden)
			_add_entry_in_menu (_("Minimise"), CAIRO_DESKLET_SHARE_DATA_DIR"/icon-minimize.png", _cairo_dock_minimize_appli, menu);

		_add_entry_in_menu (_("Close (middle-click)"), CAIRO_DESKLET_SHARE_DATA_DIR"/icon-close.png", _cairo_dock_close_appli, menu);
	}
	
	Icon *pIconModule;
	if (CAIRO_DOCK_IS_APPLET (icon))
		pIconModule = icon;
	else if (CAIRO_DOCK_IS_DESKLET (pContainer))
		pIconModule = CAIRO_DESKLET (pContainer)->pIcon;
	else
		pIconModule = NULL;
	
	//\_________________________ On rajoute les actions propres a un module.
	pMenuItem = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), pMenuItem);
	
	_add_entry_in_menu (_("Configure this applet"), GTK_STOCK_PROPERTIES, _cairo_dock_initiate_config_module, menu);
	
	if (pIconModule->pModuleInstance->pModule->pInstancesList->next != NULL)
		_add_entry_in_menu (_("Remove this applet"), GTK_STOCK_REMOVE, _cairo_dock_remove_module_instance, menu);
	
	if (pIconModule->pModuleInstance->pModule->pVisitCard->bMultiInstance)
	{
		_add_entry_in_menu (_("Launch another instance of this applet"), GTK_STOCK_ADD, _cairo_dock_add_module_instance, menu);
	}

	//\_________________________ On rajoute les actions de positionnement d'un desklet.
	pMenuItem = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), pMenuItem);
	
	pMenuItem = gtk_menu_item_new_with_label (_("Visibility"));
	gtk_menu_shell_append  (GTK_MENU_SHELL (menu), pMenuItem);
	GtkWidget *pSubMenuAccessibility = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (pMenuItem), pSubMenuAccessibility);
	
	GSList *group = NULL;

	gboolean bIsAbove=FALSE, bIsBelow=FALSE;
	Window Xid = GDK_WINDOW_XID (pContainer->pWidget->window);
	cairo_dock_xwindow_is_above_or_below (Xid, &bIsAbove, &bIsBelow);  // gdk_window_get_state bugue.
	gboolean bIsUtility = cairo_dock_window_is_utility (Xid);  // gtk_window_get_type_hint me renvoie toujours 0 !
	gboolean bIsDock = (/*cairo_dock_window_is_dock (Xid) || */CAIRO_DESKLET (pContainer)->bSpaceReserved);
	gboolean bIsNormal = (!bIsAbove && !bIsBelow && !bIsUtility && !bIsDock);
	gboolean bIsSticky = /*(cairo_dock_get_xwindow_desktop (Xid) == -1) || */cairo_dock_xwindow_is_sticky (Xid);
	
	pMenuItem = gtk_radio_menu_item_new_with_label(group, _("Normal"));
	group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(pMenuItem));
	gtk_menu_shell_append(GTK_MENU_SHELL(pSubMenuAccessibility), pMenuItem);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(pMenuItem), bIsNormal);  // on coche celui-ci par defaut, il sera decoche par les suivants eventuellement.
	g_signal_connect(G_OBJECT(pMenuItem), "toggled", G_CALLBACK(_cairo_dock_keep_normal), data);
	
	pMenuItem = gtk_radio_menu_item_new_with_label(group, _("Always on top"));
	group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(pMenuItem));
	gtk_menu_shell_append(GTK_MENU_SHELL(pSubMenuAccessibility), pMenuItem);
	if (bIsAbove)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(pMenuItem), TRUE);
	g_signal_connect(G_OBJECT(pMenuItem), "toggled", G_CALLBACK(_cairo_dock_keep_above), data);
	
	pMenuItem = gtk_radio_menu_item_new_with_label(group, _("Always below"));
	group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(pMenuItem));
	gtk_menu_shell_append(GTK_MENU_SHELL(pSubMenuAccessibility), pMenuItem);
	if (bIsBelow)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(pMenuItem), TRUE);
	g_signal_connect(G_OBJECT(pMenuItem), "toggled", G_CALLBACK(_cairo_dock_keep_below), data);
	
	pMenuItem = gtk_radio_menu_item_new_with_label(group, "Compiz Fusion Widget");
	group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(pMenuItem));
	gtk_menu_shell_append(GTK_MENU_SHELL(pSubMenuAccessibility), pMenuItem);
	if (bIsUtility)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(pMenuItem), TRUE);
	g_signal_connect(G_OBJECT(pMenuItem), "toggled", G_CALLBACK(_cairo_dock_keep_on_widget_layer), data);
	gtk_widget_set_tooltip_text (pMenuItem, _("Set behaviour in Compiz to: (name=cairo-dock & type=utility)"));
	
	pMenuItem = gtk_radio_menu_item_new_with_label(group, _("Reserve space"));
	group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(pMenuItem));
	gtk_menu_shell_append(GTK_MENU_SHELL(pSubMenuAccessibility), pMenuItem);
	if (bIsDock)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(pMenuItem), TRUE);
	g_signal_connect(G_OBJECT(pMenuItem), "toggled", G_CALLBACK(_cairo_dock_keep_space), data);
	
	pMenuItem = gtk_check_menu_item_new_with_label(_("On all desktops"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), pMenuItem);
	if (bIsSticky)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(pMenuItem), TRUE);
	g_signal_connect(G_OBJECT(pMenuItem), "toggled", G_CALLBACK(_cairo_dock_set_on_all_desktop), data);
	
	pMenuItem = gtk_check_menu_item_new_with_label(_("Lock position"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), pMenuItem);
	if (CAIRO_DESKLET (pContainer)->bPositionLocked)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(pMenuItem), TRUE);
	g_signal_connect(G_OBJECT(pMenuItem), "toggled", G_CALLBACK(_cairo_dock_lock_position), data);

	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

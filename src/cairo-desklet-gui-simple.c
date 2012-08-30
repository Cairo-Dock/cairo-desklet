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

#include <string.h>
#include <unistd.h>
#define __USE_XOPEN_EXTENDED
#include <stdlib.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>
#include <gdk/gdkx.h> // GDK_WINDOW_XID

#include "config.h"
#include <cairo-dock.h>
#include "cairo-desklet-user-interaction.h"
#include "cairo-desklet-gui-simple.h"

#define CAIRO_DOCK_PREVIEW_WIDTH 200
#define CAIRO_DOCK_PREVIEW_HEIGHT 250
#define CAIRO_DOCK_SIMPLE_PANEL_WIDTH 1024
#define CAIRO_DOCK_SIMPLE_PANEL_HEIGHT 700
#define ICON_HUGE 60
#define ICON_BIG 56
#define ICON_MEDIUM 48
#define ICON_SMALL 42
#define ICON_TINY 36

#define CAIRO_DOCK_ICON "cairo-dock.svg"

static GtkWidget *s_pSimpleConfigModuleWindow = NULL;
static const  gchar *s_cCurrentModuleName = NULL;

static gboolean on_apply_config_module_simple (gpointer data)
{
	cd_debug ("%s (%s)\n", __func__, s_cCurrentModuleName);
	CairoDockModule *pModule = cairo_dock_find_module_from_name (s_cCurrentModuleName);
	if (pModule != NULL)
	{
		if (pModule->pInstancesList != NULL)
		{
			gchar *cConfFilePath = g_object_get_data (G_OBJECT (s_pSimpleConfigModuleWindow), "conf-file");
			CairoDockModuleInstance *pModuleInstance = NULL;
			GList *pElement;
			for (pElement = pModule->pInstancesList; pElement != NULL; pElement= pElement->next)
			{
				pModuleInstance = pElement->data;
				if (strcmp (pModuleInstance->cConfFilePath, cConfFilePath) == 0)
					break ;
			}
			g_return_val_if_fail (pModuleInstance != NULL, TRUE);
			
			cairo_dock_reload_module_instance (pModuleInstance, TRUE);
		}
	}
	return TRUE;
}

static void on_destroy_config_module_simple (gpointer data)
{
	s_pSimpleConfigModuleWindow = NULL;
}


static inline GtkWidget * _present_module_widget (GtkWidget *pWindow, CairoDockModuleInstance *pInstance, const gchar *cConfFilePath, const gchar *cGettextDomain, const gchar *cOriginalConfFilePath)
{
	//\_____________ On construit l'IHM du fichier de conf.
	GKeyFile* pKeyFile = cairo_dock_open_key_file (cConfFilePath);
	g_return_val_if_fail (pKeyFile != NULL, NULL);
	
	GSList *pWidgetList = NULL;
	GPtrArray *pDataGarbage = g_ptr_array_new ();
	GtkWidget *pNoteBook = cairo_dock_build_key_file_widget (pKeyFile, cGettextDomain, pWindow, &pWidgetList, pDataGarbage, cOriginalConfFilePath);
	
	g_object_set_data (G_OBJECT (pWindow), "conf-file", g_strdup (cConfFilePath));
	g_object_set_data (G_OBJECT (pWindow), "widget-list", pWidgetList);
	g_object_set_data (G_OBJECT (pWindow), "garbage", pDataGarbage);
	
	s_cCurrentModuleName = pInstance->pModule->pVisitCard->cModuleName;
	
	if (pInstance->pModule->pInterface->load_custom_widget != NULL)
		pInstance->pModule->pInterface->load_custom_widget (pInstance, pKeyFile, pWidgetList);
	
	g_key_file_free (pKeyFile);
	
	//\_____________ On l'insere dans la fenetre.
	GtkWidget *pMainVBox = gtk_bin_get_child (GTK_BIN (pWindow));
	gtk_box_pack_start (GTK_BOX (pMainVBox),
		pNoteBook,
		TRUE,
		TRUE,
		0);
	
	return pNoteBook;
}

void cairo_dock_reload_generic_gui (GtkWidget *pWindow)
{
	//g_print ("%s ()", __func__);
	GSList *pWidgetList = g_object_get_data (G_OBJECT (pWindow), "widget-list");
	cairo_dock_free_generated_widget_list (pWidgetList);
	pWidgetList = NULL;
	g_object_set_data (G_OBJECT (pWindow), "widget-list", NULL);
	
	GPtrArray *pDataGarbage = g_object_get_data (G_OBJECT (pWindow), "garbage");
	/// nettoyer...
	g_object_set_data (G_OBJECT (pWindow), "garbage", NULL);
	
	GtkWidget *pMainVBox = gtk_bin_get_child (GTK_BIN (pWindow));
	GList *children = gtk_container_get_children (GTK_CONTAINER (pMainVBox));
	g_return_if_fail (children != NULL);
	GtkWidget *pNoteBook = children->data;
	g_list_free (children);
	gtk_widget_destroy (pNoteBook);
	
	gchar *cConfFilePath = g_object_get_data (G_OBJECT (pWindow), "conf-file");
	GKeyFile* pKeyFile = cairo_dock_open_key_file (cConfFilePath);
	if (pKeyFile != NULL)
	{
		pNoteBook = cairo_dock_build_key_file_widget (pKeyFile,
			NULL,
			pWindow,
			&pWidgetList,
			pDataGarbage,
			NULL);
	}
	
	g_object_set_data (G_OBJECT (pWindow), "widget-list", pWidgetList);
	g_object_set_data (G_OBJECT (pWindow), "garbage", pDataGarbage);
	
	gtk_box_pack_start (GTK_BOX (pMainVBox),
		pNoteBook,
		TRUE,
		TRUE,
		0);
	
	CairoDockLoadCustomWidgetFunc load_custom_widgets = g_object_get_data (G_OBJECT (pWindow), "load-widget");
	if (load_custom_widgets)  // a faire apres avoir mis "widget-list".
		load_custom_widgets (pWindow, pKeyFile, pWidgetList);
	
	g_key_file_free (pKeyFile);
	
	gtk_widget_show_all (pNoteBook);
}

static gboolean on_delete_generic_gui (GtkWidget *pWidget, GdkEvent *event, GMainLoop *pBlockingLoop)
{
	cd_debug ("%s ()\n", __func__);
	if (pBlockingLoop != NULL && g_main_loop_is_running (pBlockingLoop))
	{
		g_main_loop_quit (pBlockingLoop);
	}
	
	gpointer pUserData = g_object_get_data (G_OBJECT (pWidget), "action-data");
	GFreeFunc pFreeUserData = g_object_get_data (G_OBJECT (pWidget), "free-data");
	if (pFreeUserData != NULL)
		pFreeUserData (pUserData);
	
	GSList *pWidgetList = g_object_get_data (G_OBJECT (pWidget), "widget-list");
	cairo_dock_free_generated_widget_list (pWidgetList);
	
	GPtrArray *pDataGarbage = g_object_get_data (G_OBJECT (pWidget), "garbage");
	/// nettoyer.
	
	gchar *cConfFilePath = g_object_get_data (G_OBJECT (pWidget), "conf-file");
	g_free (cConfFilePath);
	
	return (pBlockingLoop != NULL);  // TRUE <=> ne pas detruire la fenetre.
}

static void on_click_generic_apply (GtkButton *button, GtkWidget *pWindow)
{
	//g_print ("%s ()\n", __func__);
	GSList *pWidgetList = g_object_get_data (G_OBJECT (pWindow), "widget-list");
	gchar *cConfFilePath = g_object_get_data (G_OBJECT (pWindow), "conf-file");
	GKeyFile *pKeyFile = cairo_dock_open_key_file (cConfFilePath);
	g_return_if_fail (pKeyFile != NULL);
	
	gchar *cConfModuleName = g_object_get_data (G_OBJECT (pWindow), "module");
	if (cConfModuleName != NULL)
	{
		CairoDockModule *pModule = cairo_dock_find_module_from_name (cConfModuleName);
		if (pModule != NULL)
		{
			CairoDockModuleInstance *pModuleInstance;
			GList *i;
			for (i = pModule->pInstancesList; i != NULL; i = i->next)
			{
				pModuleInstance = i->data;
				if (strcmp (cConfFilePath, pModuleInstance->cConfFilePath) == 0)
					break;
			}
			if (i != NULL)
			{
				if (pModule->pInterface->save_custom_widget != NULL)
					pModule->pInterface->save_custom_widget (pModuleInstance, pKeyFile, pWidgetList);
			}
		}
	}
	else
	{
		CairoDockSaveCustomWidgetFunc save_custom_widgets = g_object_get_data (G_OBJECT (pWindow), "save-widget");
		if (save_custom_widgets)
			save_custom_widgets (pWindow, pKeyFile, pWidgetList);
	}
	
	cairo_dock_update_keyfile_from_widget_list (pKeyFile, pWidgetList);
	cairo_dock_write_keys_to_file (pKeyFile, cConfFilePath);
	g_key_file_free (pKeyFile);
	
	CairoDockApplyConfigFunc pAction = g_object_get_data (G_OBJECT (pWindow), "action");
	gpointer pUserData = g_object_get_data (G_OBJECT (pWindow), "action-data");
	
	if (pAction != NULL)
	{
		gboolean bKeepWindow = pAction (pUserData);
		if (!bKeepWindow)  // on recharge la fenetre.
		{
			cairo_dock_reload_generic_gui (pWindow);
		}
	}
	else
		g_object_set_data (G_OBJECT (pWindow), "result", GINT_TO_POINTER (1));
}

static void on_click_generic_quit (GtkButton *button, GtkWidget *pWindow)
{
	cd_debug ("%s ()\n", __func__);
	GMainLoop *pBlockingLoop = g_object_get_data (G_OBJECT (pWindow), "loop");
	
	gboolean bReturn;
	g_signal_emit_by_name (pWindow, "delete-event", NULL, &bReturn);
	///on_delete_generic_gui (pWindow, NULL, pBlockingLoop);
	
	if (pBlockingLoop == NULL)
		gtk_widget_destroy (pWindow);
}

static void on_click_generic_ok (GtkButton *button, GtkWidget *pWindow)
{
	//g_print ("%s ()\n", __func__);
	
	on_click_generic_apply (button, pWindow);
	on_click_generic_quit (button, pWindow);
}

GtkWidget *cairo_dock_build_generic_gui_window (const gchar *cTitle, int iWidth, int iHeight, CairoDockApplyConfigFunc pAction, gpointer pUserData, GFreeFunc pFreeUserData)
{
	//\_____________ On construit la fenetre.
	GtkWidget *pMainWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_icon_from_file (GTK_WINDOW (pMainWindow), GLDI_SHARE_DATA_DIR"/"CAIRO_DOCK_ICON, NULL);
	if (cTitle != NULL)
		gtk_window_set_title (GTK_WINDOW (pMainWindow), cTitle);

	GtkWidget *pMainVBox = _gtk_vbox_new (CAIRO_DOCK_FRAME_MARGIN);
	gtk_container_add (GTK_CONTAINER (pMainWindow), pMainVBox);
	
	//\_____________ On ajoute les boutons.
	GtkWidget *pButtonsHBox = _gtk_hbox_new (CAIRO_DOCK_FRAME_MARGIN*2);
	gtk_box_pack_end (GTK_BOX (pMainVBox),
		pButtonsHBox,
		FALSE,
		FALSE,
		0);
	
	GtkWidget *pQuitButton = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
	g_signal_connect (G_OBJECT (pQuitButton), "clicked", G_CALLBACK(on_click_generic_quit), pMainWindow);
	gtk_box_pack_end (GTK_BOX (pButtonsHBox),
		pQuitButton,
		FALSE,
		FALSE,
		0);
	
	if (pAction != NULL)
	{
		GtkWidget *pApplyButton = gtk_button_new_from_stock (GTK_STOCK_APPLY);
		g_signal_connect (G_OBJECT (pApplyButton), "clicked", G_CALLBACK(on_click_generic_apply), pMainWindow);
		gtk_box_pack_end (GTK_BOX (pButtonsHBox),
			pApplyButton,
			FALSE,
			FALSE,
			0);
	}
	else
	{
		GtkWidget *pApplyButton = gtk_button_new_from_stock (GTK_STOCK_OK);
		g_signal_connect (G_OBJECT (pApplyButton), "clicked", G_CALLBACK(on_click_generic_ok), pMainWindow);
		gtk_box_pack_end (GTK_BOX (pButtonsHBox),
			pApplyButton,
			FALSE,
			FALSE,
			0);
	}
	
	//\_____________ On ajoute la barre d'etat a la fin.
	GtkWidget *pStatusBar = gtk_statusbar_new ();
	#if (GTK_MAJOR_VERSION < 3)
	gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (pStatusBar), FALSE);  // removed in GTK3 (gtk_window_set_has_resize_grip)
	#endif
	gtk_box_pack_start (GTK_BOX (pButtonsHBox),  // pMainVBox
		pStatusBar,
		FALSE,
		FALSE,
		0);
	g_object_set_data (G_OBJECT (pMainWindow), "status-bar", pStatusBar);
	
	gtk_window_resize (GTK_WINDOW (pMainWindow),
		MIN (iWidth, g_desktopGeometry.iXScreenWidth[CAIRO_DOCK_HORIZONTAL]),
		MIN (iHeight, g_desktopGeometry.iXScreenHeight[CAIRO_DOCK_HORIZONTAL] - (g_pMainDock && g_pMainDock->container.bIsHorizontal ? g_pMainDock->iMaxDockHeight : 0)));
	
	gtk_widget_show_all (pMainWindow);
	
	if (pAction != NULL)
	{
		g_object_set_data (G_OBJECT (pMainWindow), "action", pAction);
		g_object_set_data (G_OBJECT (pMainWindow), "action-data", pUserData);
		g_object_set_data (G_OBJECT (pMainWindow), "free-data", pFreeUserData);
		g_signal_connect (G_OBJECT (pMainWindow),
			"delete-event",
			G_CALLBACK (on_delete_generic_gui),
			NULL);
	}
	return pMainWindow;
}

static int _reset_current_module_widget (void)
{
	if (s_pSimpleConfigModuleWindow == NULL)
		return -1;
	GtkWidget *pMainVBox = gtk_bin_get_child (GTK_BIN (s_pSimpleConfigModuleWindow));
	GList *children = gtk_container_get_children (GTK_CONTAINER (pMainVBox));
	GtkWidget *pGroupWidget = children->data;
	
	int iNotebookPage = 0;
	if (GTK_IS_NOTEBOOK (pGroupWidget))
		iNotebookPage = gtk_notebook_get_current_page (GTK_NOTEBOOK (pGroupWidget));
	
	gtk_widget_destroy (pGroupWidget);
	GSList *pWidgetList = g_object_get_data (G_OBJECT (s_pSimpleConfigModuleWindow), "widget-list");
	cairo_dock_free_generated_widget_list (pWidgetList);
	g_object_set_data (G_OBJECT (s_pSimpleConfigModuleWindow), "widget-list", NULL);
	GPtrArray *pDataGarbage = g_object_get_data (G_OBJECT (s_pSimpleConfigModuleWindow), "garbage");
	if (pDataGarbage != NULL)
		g_ptr_array_free (pDataGarbage, TRUE);
	g_object_set_data (G_OBJECT (s_pSimpleConfigModuleWindow), "garbage", NULL);
	
	return iNotebookPage;
}

static void show_module_instance_gui (CairoDockModuleInstance *pModuleInstance, int iShowPage)
{
	int iNotebookPage = -1;
	if (s_pSimpleConfigModuleWindow == NULL)
	{
		s_pSimpleConfigModuleWindow = cairo_dock_build_generic_gui_window (dgettext (pModuleInstance->pModule->pVisitCard->cGettextDomain, pModuleInstance->pModule->pVisitCard->cTitle),
			CAIRO_DOCK_SIMPLE_PANEL_WIDTH, CAIRO_DOCK_SIMPLE_PANEL_HEIGHT,
			(CairoDockApplyConfigFunc) on_apply_config_module_simple,
			NULL,
			(GFreeFunc) on_destroy_config_module_simple);
		iNotebookPage = iShowPage;
	}
	else
	{
		iNotebookPage = _reset_current_module_widget ();
		if (iShowPage >= 0)
			iNotebookPage = iShowPage;
	}
	
	gchar *cOriginalConfFilePath = g_strdup_printf ("%s/%s", pModuleInstance->pModule->pVisitCard->cShareDataDir, pModuleInstance->pModule->pVisitCard->cConfFileName);
	
	GtkWidget *pGroupWidget = _present_module_widget (s_pSimpleConfigModuleWindow,
		pModuleInstance,
		pModuleInstance->cConfFilePath,
		pModuleInstance->pModule->pVisitCard->cGettextDomain,
		cOriginalConfFilePath);
	
	g_free (cOriginalConfFilePath);
	
	gtk_widget_show_all (s_pSimpleConfigModuleWindow);
	if (iNotebookPage != -1 && GTK_IS_NOTEBOOK (pGroupWidget))
	{
		gtk_notebook_set_current_page (GTK_NOTEBOOK (pGroupWidget), iNotebookPage);
	}
}

static void show_module_gui (const gchar *cModuleName)
{
	CairoDockModule *pModule = cairo_dock_find_module_from_name (cModuleName);
	g_return_if_fail (pModule != NULL);
	
	if (s_pSimpleConfigModuleWindow == NULL)
	{
		s_pSimpleConfigModuleWindow = cairo_dock_build_generic_gui_window (cModuleName,
			CAIRO_DOCK_SIMPLE_PANEL_WIDTH, CAIRO_DOCK_SIMPLE_PANEL_HEIGHT,
			(CairoDockApplyConfigFunc) on_apply_config_module_simple,
			NULL,
			(GFreeFunc) on_destroy_config_module_simple);
	}
	else
	{
		_reset_current_module_widget ();
	}
	
	CairoDockModuleInstance *pModuleInstance = (pModule->pInstancesList != NULL ? pModule->pInstancesList->data : NULL);
	gchar *cConfFilePath = (pModuleInstance != NULL ? pModuleInstance->cConfFilePath : pModule->cConfFilePath);
	gchar *cOriginalConfFilePath = g_strdup_printf ("%s/%s", pModule->pVisitCard->cShareDataDir, pModule->pVisitCard->cConfFileName);
	
	//\_____________ On insere l'IHM du fichier de conf.
	_present_module_widget (s_pSimpleConfigModuleWindow,
		pModuleInstance,
		cConfFilePath,
		pModule->pVisitCard->cGettextDomain,
		cOriginalConfFilePath);
	
	gtk_widget_show_all (s_pSimpleConfigModuleWindow);
	g_free (cOriginalConfFilePath);
}

static void set_status_message_on_gui (const gchar *cMessage)
{
	GtkWidget *pStatusBar = NULL;
	if (s_pSimpleConfigModuleWindow != NULL)
	{
		pStatusBar = g_object_get_data (G_OBJECT (s_pSimpleConfigModuleWindow), "status-bar");
	}
	if (pStatusBar == NULL)
		return ;
	gtk_statusbar_pop (GTK_STATUSBAR (pStatusBar), 0);  // clear any previous message, underflow is allowed.
	gtk_statusbar_push (GTK_STATUSBAR (pStatusBar), 0, cMessage);
}

static gboolean module_is_opened (CairoDockModuleInstance *pModuleInstance)
{
	if (s_pSimpleConfigModuleWindow == NULL || s_cCurrentModuleName == NULL || pModuleInstance == NULL)
		return FALSE;
	
	if (strcmp (s_cCurrentModuleName, pModuleInstance->pModule->pVisitCard->cModuleName) != 0)
		return FALSE;
	
	gchar *cConfFilePath = g_object_get_data (G_OBJECT (s_pSimpleConfigModuleWindow), "conf-file");
	if (cConfFilePath == NULL)
		return FALSE;
	CairoDockModuleInstance *pInstance;
	GList *pElement;
	for (pElement = pModuleInstance->pModule->pInstancesList; pElement != NULL; pElement= pElement->next)
	{
		pInstance = pElement->data;
		if (pInstance->cConfFilePath && strcmp (pModuleInstance->cConfFilePath, pInstance->cConfFilePath) == 0)
			return TRUE;
	}
	return FALSE;
}

static gboolean _test_one_module_name (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer *data)
{
	gchar *cResult = NULL;
	gtk_tree_model_get (model, iter, CAIRO_DOCK_MODEL_RESULT, &cResult, -1);
	cd_debug ("- %s !\n", cResult
	);
	if (cResult && strcmp (data[0], cResult) == 0)
	{
		GtkTreeIter *iter_to_fill = data[1];
		memcpy (iter_to_fill, iter, sizeof (GtkTreeIter));
		gboolean *bFound = data[2];
		*bFound = TRUE;
		g_free (cResult);
		return TRUE;
	}
	g_free (cResult);
	return FALSE;
}

static CairoDockGroupKeyWidget *get_widget_from_name (CairoDockModuleInstance *pInstance, const gchar *cGroupName, const gchar *cKeyName)
{
	g_return_val_if_fail (s_pSimpleConfigModuleWindow != NULL, NULL);
	GSList *pWidgetList = g_object_get_data (G_OBJECT (s_pSimpleConfigModuleWindow), "widget-list");
	g_return_val_if_fail (pWidgetList != NULL, NULL);
	return cairo_dock_gui_find_group_key_widget_in_list (pWidgetList, cGroupName, cKeyName);
}

static void reload_current_widget (CairoDockModuleInstance *pInstance, int iShowPage)
{
	g_return_if_fail (s_pSimpleConfigModuleWindow != NULL && s_cCurrentModuleName != NULL);
	
	CairoDockModule *pModule = cairo_dock_find_module_from_name (s_cCurrentModuleName);
	g_return_if_fail (pModule != NULL && pModule->pInstancesList != NULL);
	
	gchar *cConfFilePath = g_object_get_data (G_OBJECT (s_pSimpleConfigModuleWindow), "conf-file");
	g_return_if_fail (cConfFilePath == NULL);
	
	CairoDockModuleInstance *pModuleInstance;
	GList *pElement;
	for (pElement = pModule->pInstancesList; pElement != NULL; pElement= pElement->next)
	{
		pModuleInstance = pElement->data;
		if (strcmp (pModuleInstance->cConfFilePath, cConfFilePath) == 0)
			break ;
	}
	g_return_if_fail (pElement != NULL);
	
	show_module_instance_gui (pModuleInstance, iShowPage);
}



static void cairo_dock_update_desklet_widgets (CairoDesklet *pDesklet, GSList *pWidgetList)
{
	CairoDockGroupKeyWidget *pGroupKeyWidget;
	GtkWidget *pOneWidget;
	pGroupKeyWidget = cairo_dock_gui_find_group_key_widget_in_list (pWidgetList, "Desklet", "locked");
	g_return_if_fail (pGroupKeyWidget != NULL && pGroupKeyWidget->pSubWidgetList != NULL);
	pOneWidget = pGroupKeyWidget->pSubWidgetList->data;
	gtk_toggle_button_set_active  (GTK_TOGGLE_BUTTON (pOneWidget), pDesklet->bPositionLocked);
	
	pGroupKeyWidget = cairo_dock_gui_find_group_key_widget_in_list (pWidgetList, "Desklet", "size");
	g_return_if_fail (pGroupKeyWidget != NULL && pGroupKeyWidget->pSubWidgetList != NULL);
	g_print ("echo\n");
	pOneWidget = pGroupKeyWidget->pSubWidgetList->data;
	g_signal_handlers_block_matched (pOneWidget,
		(GSignalMatchType) G_SIGNAL_MATCH_FUNC,
		0, 0, NULL, _cairo_dock_set_value_in_pair, NULL);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (pOneWidget), pDesklet->container.iWidth);
	g_signal_handlers_unblock_matched (pOneWidget,
			(GSignalMatchType) G_SIGNAL_MATCH_FUNC,
			0, 0, NULL, _cairo_dock_set_value_in_pair, NULL);
	if (pGroupKeyWidget->pSubWidgetList->next != NULL)
	{
		pOneWidget = pGroupKeyWidget->pSubWidgetList->next->data;
		g_signal_handlers_block_matched (pOneWidget,
			(GSignalMatchType) G_SIGNAL_MATCH_FUNC,
			0, 0, NULL, _cairo_dock_set_value_in_pair, NULL);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (pOneWidget), pDesklet->container.iHeight);
		g_signal_handlers_unblock_matched (pOneWidget,
			(GSignalMatchType) G_SIGNAL_MATCH_FUNC,
			0, 0, NULL, _cairo_dock_set_value_in_pair, NULL);
	}
	
	int iRelativePositionX = (pDesklet->container.iWindowPositionX + pDesklet->container.iWidth/2 <= g_desktopGeometry.iXScreenWidth[CAIRO_DOCK_HORIZONTAL]/2 ? pDesklet->container.iWindowPositionX : pDesklet->container.iWindowPositionX - g_desktopGeometry.iXScreenWidth[CAIRO_DOCK_HORIZONTAL]);
	int iRelativePositionY = (pDesklet->container.iWindowPositionY + pDesklet->container.iHeight/2 <= g_desktopGeometry.iXScreenHeight[CAIRO_DOCK_HORIZONTAL]/2 ? pDesklet->container.iWindowPositionY : pDesklet->container.iWindowPositionY - g_desktopGeometry.iXScreenHeight[CAIRO_DOCK_HORIZONTAL]);
	
	pGroupKeyWidget = cairo_dock_gui_find_group_key_widget_in_list (pWidgetList, "Desklet", "x position");
	g_return_if_fail (pGroupKeyWidget != NULL && pGroupKeyWidget->pSubWidgetList != NULL);
	g_print ("echo\n");
	pOneWidget = pGroupKeyWidget->pSubWidgetList->data;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (pOneWidget), iRelativePositionX);
	
	pGroupKeyWidget = cairo_dock_gui_find_group_key_widget_in_list (pWidgetList, "Desklet", "y position");
	g_return_if_fail (pGroupKeyWidget != NULL && pGroupKeyWidget->pSubWidgetList != NULL);
	g_print ("echo\n");
	pOneWidget = pGroupKeyWidget->pSubWidgetList->data;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (pOneWidget), iRelativePositionY);
	
	pGroupKeyWidget = cairo_dock_gui_find_group_key_widget_in_list (pWidgetList, "Desklet", "rotation");
	g_return_if_fail (pGroupKeyWidget != NULL && pGroupKeyWidget->pSubWidgetList != NULL);
	g_print ("echo\n");
	pOneWidget = pGroupKeyWidget->pSubWidgetList->data;
	gtk_range_set_value (GTK_RANGE (pOneWidget), pDesklet->fRotation/G_PI*180.);
}

void cairo_dock_update_desklet_visibility_widgets (CairoDesklet *pDesklet, GSList *pWidgetList)
{
	CairoDockGroupKeyWidget *pGroupKeyWidget;
	GtkWidget *pOneWidget;
	gboolean bIsSticky = cairo_dock_desklet_is_sticky (pDesklet);
	CairoDeskletVisibility iVisibility = pDesklet->iVisibility;
	
	pGroupKeyWidget = cairo_dock_gui_find_group_key_widget_in_list (pWidgetList, "Desklet", "accessibility");
	g_return_if_fail (pGroupKeyWidget != NULL && pGroupKeyWidget->pSubWidgetList != NULL);
	pOneWidget = pGroupKeyWidget->pSubWidgetList->data;
	gtk_combo_box_set_active (GTK_COMBO_BOX (pOneWidget), iVisibility);
	
	pGroupKeyWidget = cairo_dock_gui_find_group_key_widget_in_list (pWidgetList, "Desklet", "sticky");
	g_return_if_fail (pGroupKeyWidget != NULL && pGroupKeyWidget->pSubWidgetList != NULL);
	pOneWidget = pGroupKeyWidget->pSubWidgetList->data;
	gtk_toggle_button_set_active  (GTK_TOGGLE_BUTTON (pOneWidget), bIsSticky);
	
	pGroupKeyWidget = cairo_dock_gui_find_group_key_widget_in_list (pWidgetList, "Desklet", "locked");
	g_return_if_fail (pGroupKeyWidget != NULL && pGroupKeyWidget->pSubWidgetList != NULL);
	pOneWidget = pGroupKeyWidget->pSubWidgetList->data;
	gtk_toggle_button_set_active  (GTK_TOGGLE_BUTTON (pOneWidget), pDesklet->bPositionLocked);
}

static inline gboolean _module_is_opened (CairoDockModuleInstance *pInstance)
{
	if (s_pSimpleConfigModuleWindow == NULL || s_cCurrentModuleName == NULL || pInstance == NULL || pInstance->cConfFilePath == NULL)
		return FALSE;
	
	if (strcmp (pInstance->pModule->pVisitCard->cModuleName, s_cCurrentModuleName) != 0)  // est-on est en train d'editer ce module dans le panneau de conf.
		return FALSE;
	
	gchar *cConfFilePath = g_object_get_data (G_OBJECT (s_pSimpleConfigModuleWindow), "conf-file");
	g_return_val_if_fail (cConfFilePath != NULL, FALSE);
	
	if (strcmp (pInstance->cConfFilePath, cConfFilePath) != 0)
		return FALSE;  // est-ce cette instance.
	
	return TRUE;
}
static inline gboolean _desklet_is_opened (CairoDesklet *pDesklet)
{
	if (s_pSimpleConfigModuleWindow == NULL || pDesklet == NULL)
		return FALSE;
	Icon *pIcon = pDesklet->pIcon;
	g_return_val_if_fail (pIcon != NULL, FALSE);
	
	CairoDockModuleInstance *pModuleInstance = pIcon->pModuleInstance;
	g_return_val_if_fail (pModuleInstance != NULL, FALSE);
	
	return _module_is_opened (pModuleInstance);
}
static void update_desklet_params (CairoDesklet *pDesklet)
{
	if (! _desklet_is_opened (pDesklet))
		return ;
	
	GSList *pWidgetList = g_object_get_data (G_OBJECT (s_pSimpleConfigModuleWindow), "widget-list");
	g_return_if_fail (pWidgetList != NULL);
	cairo_dock_update_desklet_widgets (pDesklet, pWidgetList);
}

static void update_desklet_visibility_params (CairoDesklet *pDesklet)
{
	if (! _desklet_is_opened (pDesklet))
		return ;
	
	GSList *pWidgetList = g_object_get_data (G_OBJECT (s_pSimpleConfigModuleWindow), "widget-list");
	g_return_if_fail (pWidgetList != NULL);
	cairo_dock_update_desklet_visibility_widgets (pDesklet, pWidgetList);
}

void cairo_dock_register_simple_gui_backend (void)
{
	CairoDockMainGuiBackend *pBackend = g_new0 (CairoDockMainGuiBackend, 1);
	
	//pBackend->show_module_instance_gui 		= show_module_instance_gui;
	pBackend->update_desklet_params 			= update_desklet_params;
	pBackend->update_desklet_visibility_params 	= update_desklet_visibility_params;
	
	cairo_dock_register_config_gui_backend (pBackend);
	
	CairoDockGuiBackend *pConfigBackend = g_new0 (CairoDockGuiBackend, 1);
	
	pConfigBackend->set_status_message_on_gui 	= set_status_message_on_gui;
	pConfigBackend->reload_current_widget 	= reload_current_widget;
	pConfigBackend->show_module_instance_gui 	= show_module_instance_gui;
	pConfigBackend->get_widget_from_name 		= get_widget_from_name;
	
	cairo_dock_register_gui_backend (pConfigBackend);
}

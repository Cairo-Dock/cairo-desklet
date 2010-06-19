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

#include "config.h"
#include <cairo-dock.h>
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
	if (pInstance != NULL)
		g_object_set_data (G_OBJECT (pWindow), "module", (gpointer)pInstance->pModule->pVisitCard->cModuleName);
	
	if (pInstance != NULL && pInstance->pModule->pInterface->load_custom_widget != NULL)
		pInstance->pModule->pInterface->load_custom_widget (pInstance, pKeyFile);
	
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
	s_cCurrentModuleName = pModuleInstance->pModule->pVisitCard->cModuleName;
	
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
	s_cCurrentModuleName = pModule->pVisitCard->cModuleName;
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

static CairoDockGroupKeyWidget *get_widget_from_name (const gchar *cGroupName, const gchar *cKeyName)
{
	g_return_val_if_fail (s_pSimpleConfigModuleWindow != NULL, NULL);
	return cairo_dock_gui_find_group_key_widget (s_pSimpleConfigModuleWindow, cGroupName, cKeyName);
}

static void close_gui (void)
{
	if (s_pSimpleConfigModuleWindow != NULL)
	{
		GtkWidget *pMainVBox = gtk_bin_get_child (GTK_BIN (s_pSimpleConfigModuleWindow));
		GList *children = gtk_container_get_children (GTK_CONTAINER (pMainVBox));
		GList *w = g_list_last (children);
		GtkWidget *pButtonsHBox = w->data;
		
		children = gtk_container_get_children (GTK_CONTAINER (pButtonsHBox));
		w = g_list_last (children);
		GtkWidget *pQuitButton = w->data;
		
		gboolean bReturn;
		g_signal_emit_by_name (pQuitButton, "clicked", NULL, &bReturn);
	}
}

void cairo_dock_register_simple_gui_backend (void)
{
	CairoDockGuiBackend *pBackend = g_new0 (CairoDockGuiBackend, 1);
	
	pBackend->show_main_gui 			= NULL;
	pBackend->show_module_instance_gui 	= show_module_instance_gui;
	pBackend->show_module_gui 			= show_module_gui;
	pBackend->set_status_message_on_gui = set_status_message_on_gui;
	pBackend->module_is_opened 			= module_is_opened;
	pBackend->deactivate_module_in_gui 	= NULL;
	pBackend->get_widget_from_name 		= get_widget_from_name;
	pBackend->close_gui 				= close_gui;
	
	cairo_dock_register_gui_backend (pBackend);
}

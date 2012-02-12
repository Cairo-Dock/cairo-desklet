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
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <cairo.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <cairo-dock.h>
#include "cairo-desklet-user-interaction.h"

extern gchar *g_cConfFile;
extern gchar *g_cCurrentIconsPath;

static CairoDockMainGuiBackend *s_pMainGuiBackend = NULL;

void cairo_dock_register_config_gui_backend (CairoDockMainGuiBackend *pBackend)
{
	g_free (s_pMainGuiBackend);
	s_pMainGuiBackend = pBackend;
}


static guint s_iSidUpdateDesklet = 0;
static CairoDesklet *s_DeskletToUpdate = NULL;
static gboolean _update_desklet_params (gpointer data)
{
	if (s_DeskletToUpdate != NULL)
	{
		if (s_pMainGuiBackend && s_pMainGuiBackend->update_desklet_params)
			s_pMainGuiBackend->update_desklet_params (s_DeskletToUpdate);
		s_DeskletToUpdate = NULL;
	}
	s_iSidUpdateDesklet = 0;
	return FALSE;
}
static gboolean _on_stop_desklet (gpointer pUserData, CairoDesklet *pDesklet)
{
	if (s_DeskletToUpdate == pDesklet)  // the desklet we were about to update has been destroyed, cancel.
	{
		if (s_iSidUpdateDesklet != 0)
		{
			g_source_remove (s_iSidUpdateDesklet);
			s_iSidUpdateDesklet = 0;
		}
		s_DeskletToUpdate = NULL;
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}
static void cairo_dock_gui_trigger_update_desklet_params (CairoDesklet *pDesklet)
{
	g_return_if_fail (pDesklet != NULL);
	if (s_DeskletToUpdate != pDesklet)  // new desklet to update, let's forget the previous one.
	{
		if (s_iSidUpdateDesklet != 0)
		{
			g_source_remove (s_iSidUpdateDesklet);
			s_iSidUpdateDesklet = 0;
		}
		s_DeskletToUpdate = NULL;
	}
	if (s_iSidUpdateDesklet == 0)  // no update scheduled yet, let's schedule it.
	{
		s_iSidUpdateDesklet = g_idle_add_full (G_PRIORITY_LOW,
			(GSourceFunc) _update_desklet_params,
			NULL,
			NULL);
		s_DeskletToUpdate = pDesklet;
		cairo_dock_register_notification_on_object (pDesklet,
			NOTIFICATION_STOP_DESKLET_DEPRECATED,
			(CairoDockNotificationFunc) _on_stop_desklet,
			CAIRO_DOCK_RUN_AFTER, NULL);
	}
}

gboolean cairo_dock_notification_configure_desklet (gpointer pUserData, CairoDesklet *pDesklet)
{
	cairo_dock_gui_trigger_update_desklet_params (pDesklet);
	
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


static guint s_iSidUpdateVisiDesklet = 0;
static gboolean _update_desklet_visibility_params (gpointer data)
{
	if (s_DeskletToUpdate != NULL)
	{
		if (s_pMainGuiBackend && s_pMainGuiBackend->update_desklet_visibility_params)
			s_pMainGuiBackend->update_desklet_visibility_params (s_DeskletToUpdate);
		s_DeskletToUpdate = NULL;
	}
	s_iSidUpdateVisiDesklet = 0;
	return FALSE;
}
void cairo_dock_gui_trigger_update_desklet_visibility (CairoDesklet *pDesklet)
{
	g_return_if_fail (pDesklet != NULL);
	if (s_DeskletToUpdate != pDesklet)  // new desklet to update, let's forget the previous one.
	{
		if (s_iSidUpdateVisiDesklet != 0)
		{
			g_source_remove (s_iSidUpdateVisiDesklet);
			s_iSidUpdateVisiDesklet = 0;
		}
		s_DeskletToUpdate = NULL;
	}
	if (s_iSidUpdateVisiDesklet == 0)  // no update scheduled yet, let's schedule it.
	{
		s_iSidUpdateVisiDesklet = g_idle_add_full (G_PRIORITY_LOW,
			(GSourceFunc) _update_desklet_visibility_params,
			NULL,
			NULL);
		s_DeskletToUpdate = pDesklet;
		cairo_dock_register_notification_on_object (pDesklet,
			NOTIFICATION_STOP_DESKLET_DEPRECATED,
			(CairoDockNotificationFunc) _on_stop_desklet,
			CAIRO_DOCK_RUN_AFTER, NULL);
	}
}

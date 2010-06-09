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

gboolean cairo_dock_notification_click_icon (gpointer pUserData, Icon *icon, CairoDock *pDock, guint iButtonState)
{
	//g_print ("+ %s (%s)\n", __func__, icon ? icon->cName : "no icon");
	if (icon == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	if (CAIRO_DOCK_IS_URI_LAUNCHER (icon))  // URI : on lance ou on monte.
	{
		cd_debug (" uri launcher");
		gboolean bIsMounted = FALSE;
		if (icon->iVolumeID > 0)
		{
			gchar *cActivationURI = cairo_dock_fm_is_mounted (icon->cBaseURI, &bIsMounted);
			g_free (cActivationURI);
		}
		if (icon->iVolumeID > 0 && ! bIsMounted)
		{
			int answer = cairo_dock_ask_question_and_wait (_("Do you want to mount this device?"), icon, CAIRO_CONTAINER (pDock));
			if (answer != GTK_RESPONSE_YES)
			{
				return CAIRO_DOCK_LET_PASS_NOTIFICATION;
			}
			cairo_dock_fm_mount (icon, CAIRO_CONTAINER (pDock));
		}
		else
			cairo_dock_fm_launch_uri (icon->cCommand);
		return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
	}
	else if (CAIRO_DOCK_IS_LAUNCHER (icon))
	{
		if (icon->cCommand != NULL && strcmp (icon->cCommand, "none") != 0)  // finalement, on lance la commande.
		{
			if (pDock->iRefCount != 0)
			{
				Icon *pMainIcon = cairo_dock_search_icon_pointing_on_dock (pDock, NULL);
				if (CAIRO_DOCK_IS_APPLET (pMainIcon))
					return CAIRO_DOCK_LET_PASS_NOTIFICATION;
			}
			
			gboolean bSuccess = FALSE;
			if (*icon->cCommand == '<')
			{
				bSuccess = cairo_dock_simulate_key_sequence (icon->cCommand);
				if (!bSuccess)
					bSuccess = cairo_dock_launch_command_full (icon->cCommand, icon->cWorkingDirectory);
			}
			else
			{
				bSuccess = cairo_dock_launch_command_full (icon->cCommand, icon->cWorkingDirectory);
				if (! bSuccess)
					bSuccess = cairo_dock_simulate_key_sequence (icon->cCommand);
			}
			if (! bSuccess)
			{
				cairo_dock_request_icon_animation (icon, pDock, "blink", 1);  // 1 clignotement si echec
			}
			return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
		}
	}
	else
		cd_debug ("no action here");
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

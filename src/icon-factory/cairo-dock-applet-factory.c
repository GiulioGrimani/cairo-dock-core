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
#include <cairo.h>

#include "cairo-dock-icons.h"
#include "cairo-dock-modules.h"
#include "cairo-dock-log.h"
#include "cairo-dock-applet-factory.h"


Icon *cairo_dock_new_applet_icon (CairoDockMinimalAppletConfig *pMinimalConfig, CairoDockModuleInstance *pModuleInstance)
{
	//\____________ On cree l'icone.
	Icon *icon = g_new0 (Icon, 1);
	icon->iType = CAIRO_DOCK_APPLET;
	icon->pModuleInstance = pModuleInstance;
	
	//\____________ On recupere les infos de sa config.
	icon->cName = g_strdup (pMinimalConfig->cLabel);
	icon->cFileName = g_strdup (pMinimalConfig->cIconFileName);
	
	icon->fOrder = pMinimalConfig->fOrder;
	
	if (! pMinimalConfig->bIsDetached)
	{
		icon->fWidth = pMinimalConfig->iDesiredIconWidth;
		icon->fHeight = pMinimalConfig->iDesiredIconHeight;
		icon->cParentDockName = g_strdup (pMinimalConfig->cDockName != NULL ? pMinimalConfig->cDockName : CAIRO_DOCK_MAIN_DOCK_NAME);
	}
	else  // l'applet creera la surface elle-meme, car on ne sait ni la taille qu'elle voudra lui donner, ni meme si elle l'utilisera !
	{
		icon->fWidth = -1;
		icon->fHeight = -1;
	}
	icon->fScale = 1;
	icon->fGlideScale = 1;
	icon->fWidthFactor = 1.;
	icon->fHeightFactor = 1.;
	
	return icon;
}
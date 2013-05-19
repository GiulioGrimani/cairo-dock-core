/*
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

#ifndef __CAIRO_DOCK_CALLBACKS__
#define  __CAIRO_DOCK_CALLBACKS__

#include <gtk/gtk.h>
#include "cairo-dock-struct.h"
G_BEGIN_DECLS


void cairo_dock_freeze_docks (gboolean bFreeze);

void cairo_dock_on_change_icon (Icon *pLastPointedIcon, Icon *pPointedIcon, CairoDock *pDock);

gboolean cairo_dock_on_leave_dock_notification (G_GNUC_UNUSED gpointer data, CairoDock *pDock, G_GNUC_UNUSED gboolean *bStartAnimation);

void gldi_dock_init_internals (CairoDock *pDock);

G_END_DECLS
#endif

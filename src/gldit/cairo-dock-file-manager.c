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
#include <sys/stat.h>

#include "cairo-dock-dock-factory.h"
#include "cairo-dock-dock-facility.h"
#include "cairo-dock-icons.h"
#include "cairo-dock-load.h"
#include "cairo-dock-draw.h"
#include "cairo-dock-dialog-manager.h"
#include "cairo-dock-log.h"
#include "cairo-dock-dock-manager.h"
#include "cairo-dock-desktop-file-factory.h"
#include "cairo-dock-container.h"
#include "cairo-dock-internal-system.h"
#include "cairo-dock-launcher-manager.h"  // cairo_dock_launch_command_sync
#include "cairo-dock-X-utilities.h"  // cairo_dock_property_is_present_on_root
#include "cairo-dock-file-manager.h"

CairoDockDesktopEnv g_iDesktopEnv = CAIRO_DOCK_UNKNOWN_ENV;

static CairoDockDesktopEnvBackend *s_pEnvBackend = NULL;

void cairo_dock_fm_register_vfs_backend (CairoDockDesktopEnvBackend *pVFSBackend)
{
	g_free (s_pEnvBackend);
	s_pEnvBackend = pVFSBackend;
}


GList * cairo_dock_fm_list_directory (const gchar *cURI, CairoDockFMSortType g_fm_iSortType, int iNewIconsType, gboolean bListHiddenFiles, int iNbMaxFiles, gchar **cFullURI)
{
	if (s_pEnvBackend != NULL && s_pEnvBackend->list_directory != NULL)
	{
		return s_pEnvBackend->list_directory (cURI, g_fm_iSortType, iNewIconsType, bListHiddenFiles, iNbMaxFiles, cFullURI);
	}
	else
	{
		cFullURI = NULL;
		return NULL;
	}
}

gboolean cairo_dock_fm_get_file_info (const gchar *cBaseURI, gchar **cName, gchar **cURI, gchar **cIconName, gboolean *bIsDirectory, int *iVolumeID, double *fOrder, CairoDockFMSortType iSortType)
{
	if (s_pEnvBackend != NULL && s_pEnvBackend->get_file_info != NULL)
	{
		s_pEnvBackend->get_file_info (cBaseURI, cName, cURI, cIconName, bIsDirectory, iVolumeID, fOrder, iSortType);
		return TRUE;
	}
	else
		return FALSE;
}

gboolean cairo_dock_fm_get_file_properties (const gchar *cURI, guint64 *iSize, time_t *iLastModificationTime, gchar **cMimeType, int *iUID, int *iGID, int *iPermissionsMask)
{
	if (s_pEnvBackend != NULL && s_pEnvBackend->get_file_properties != NULL)
	{
		s_pEnvBackend->get_file_properties (cURI, iSize, iLastModificationTime, cMimeType, iUID, iGID, iPermissionsMask);
		return TRUE;
	}
	else
		return FALSE;
}

static gpointer _cairo_dock_fm_launch_uri_threaded (gchar *cURI)
{
	cd_debug ("%s (%s)", __func__, cURI);
	s_pEnvBackend->launch_uri (cURI);
	g_free (cURI);
}
gboolean cairo_dock_fm_launch_uri (const gchar *cURI)
{
	if (s_pEnvBackend != NULL && s_pEnvBackend->launch_uri != NULL && cURI != NULL)
	{
		//s_pEnvBackend->launch_uri (cURI);
		GError *erreur = NULL;
		gchar *cThreadURI = g_strdup (cURI);
		GThread* pThread = g_thread_create ((GThreadFunc) _cairo_dock_fm_launch_uri_threaded, (gpointer) cThreadURI, FALSE, &erreur);
		if (erreur != NULL)
		{
			cd_warning (erreur->message);
			g_error_free (erreur);
		}
		
		GtkRecentManager *rm = gtk_recent_manager_get_default () ;
		gtk_recent_manager_add_item (rm, cURI);
		
		return TRUE;
	}
	else
		return FALSE;
}

gboolean cairo_dock_fm_add_monitor_full (const gchar *cURI, gboolean bDirectory, const gchar *cMountedURI, CairoDockFMMonitorCallback pCallback, gpointer data)
{
	g_return_val_if_fail (cURI != NULL, FALSE);
	if (s_pEnvBackend != NULL && s_pEnvBackend->add_monitor != NULL)
	{
		if (cMountedURI != NULL && strcmp (cMountedURI, cURI) != 0)
		{
			s_pEnvBackend->add_monitor (cURI, FALSE, pCallback, data);
			if (bDirectory)
				s_pEnvBackend->add_monitor (cMountedURI, TRUE, pCallback, data);
		}
		else
		{
			s_pEnvBackend->add_monitor (cURI, bDirectory, pCallback, data);
		}
		return TRUE;
	}
	else
		return FALSE;
}

gboolean cairo_dock_fm_remove_monitor_full (const gchar *cURI, gboolean bDirectory, const gchar *cMountedURI)
{
	g_return_val_if_fail (cURI != NULL, FALSE);
	if (s_pEnvBackend != NULL && s_pEnvBackend->remove_monitor != NULL)
	{
		s_pEnvBackend->remove_monitor (cURI);
		if (cMountedURI != NULL && strcmp (cMountedURI, cURI) != 0 && bDirectory)
		{
			s_pEnvBackend->remove_monitor (cMountedURI);
		}
		return TRUE;
	}
	else
		return FALSE;
}



gboolean cairo_dock_fm_mount_full (const gchar *cURI, int iVolumeID, CairoDockFMMountCallback pCallback, Icon *icon, CairoContainer *pContainer)
{
	if (s_pEnvBackend != NULL && s_pEnvBackend->mount != NULL && iVolumeID > 0 && cURI != NULL)
	{
		s_pEnvBackend->mount (cURI, iVolumeID, pCallback, icon, pContainer);
		return TRUE;
	}
	else
		return FALSE;
}

gboolean cairo_dock_fm_unmount_full (const gchar *cURI, int iVolumeID, CairoDockFMMountCallback pCallback, Icon *icon, CairoContainer *pContainer)
{
	if (s_pEnvBackend != NULL && s_pEnvBackend->unmount != NULL && iVolumeID > 0 && cURI != NULL)
	{
		s_pEnvBackend->unmount (cURI, iVolumeID, pCallback, icon, pContainer);
		return TRUE;
	}
	else
		return FALSE;
}

gchar *cairo_dock_fm_is_mounted (const gchar *cURI, gboolean *bIsMounted)
{
	if (s_pEnvBackend != NULL && s_pEnvBackend->is_mounted != NULL)
		return s_pEnvBackend->is_mounted (cURI, bIsMounted);
	else
		return NULL;
}

gboolean cairo_dock_fm_can_eject (const gchar *cURI)
{
	if (s_pEnvBackend != NULL && s_pEnvBackend->can_eject != NULL)
		return s_pEnvBackend->can_eject (cURI);
	else
		return FALSE;
}

gboolean cairo_dock_fm_eject_drive (const gchar *cURI)
{
	if (s_pEnvBackend != NULL && s_pEnvBackend->eject != NULL)
		return s_pEnvBackend->eject (cURI);
	else
		return FALSE;
}


gboolean cairo_dock_fm_delete_file (const gchar *cURI)
{
	if (s_pEnvBackend != NULL && s_pEnvBackend->delete_file != NULL)
	{
		return s_pEnvBackend->delete_file (cURI);
	}
	else
		return FALSE;
}

gboolean cairo_dock_fm_rename_file (const gchar *cOldURI, const gchar *cNewName)
{
	if (s_pEnvBackend != NULL && s_pEnvBackend->rename != NULL)
	{
		return s_pEnvBackend->rename (cOldURI, cNewName);
	}
	else
		return FALSE;
}

gboolean cairo_dock_fm_move_file (const gchar *cURI, const gchar *cDirectoryURI)
{
	if (s_pEnvBackend != NULL && s_pEnvBackend->move != NULL)
	{
		return s_pEnvBackend->move (cURI, cDirectoryURI);
	}
	else
		return FALSE;
}


gchar *cairo_dock_fm_get_trash_path (const gchar *cNearURI, gchar **cFileInfoPath)
{
	if (s_pEnvBackend != NULL && s_pEnvBackend->get_trash_path != NULL)
	{
		return s_pEnvBackend->get_trash_path (cNearURI, cFileInfoPath);
	}
	else
		return NULL;
}

gchar *cairo_dock_fm_get_desktop_path (void)
{
	if (s_pEnvBackend != NULL && s_pEnvBackend->get_desktop_path != NULL)
	{
		return s_pEnvBackend->get_desktop_path ();
	}
	else
		return NULL;
}

gboolean cairo_dock_fm_logout (void)
{
	if (s_pEnvBackend != NULL && s_pEnvBackend->logout!= NULL)
	{
		s_pEnvBackend->logout ();
		return TRUE;
	}
	else
		return FALSE;
}

gboolean cairo_dock_fm_shutdown (void)
{
	if (s_pEnvBackend != NULL && s_pEnvBackend->shutdown!= NULL)
	{
		s_pEnvBackend->shutdown ();
		return TRUE;
	}
	else
		return FALSE;
}

gboolean cairo_dock_fm_lock_screen (void)
{
	if (s_pEnvBackend != NULL && s_pEnvBackend->lock_screen != NULL)
	{
		s_pEnvBackend->lock_screen ();
		return TRUE;
	}
	else
		return FALSE;
}

gboolean cairo_dock_fm_setup_time (void)
{
	if (s_pEnvBackend != NULL && s_pEnvBackend->setup_time!= NULL)
	{
		s_pEnvBackend->setup_time ();
		return TRUE;
	}
	else
		return FALSE;
}

gboolean cairo_dock_fm_show_system_monitor (void)
{
	if (s_pEnvBackend != NULL && s_pEnvBackend->show_system_monitor!= NULL)
	{
		s_pEnvBackend->show_system_monitor ();
		return TRUE;
	}
	else
		return FALSE;
}

Icon *cairo_dock_fm_create_icon_from_URI (const gchar *cURI, CairoContainer *pContainer, CairoDockFMSortType iFileSortType)
{
	if (s_pEnvBackend == NULL || s_pEnvBackend->get_file_info == NULL)
		return NULL;
	Icon *pNewIcon = cairo_dock_create_dummy_launcher (NULL, NULL, NULL, NULL, 0);
	pNewIcon->cBaseURI = g_strdup (cURI);
	gboolean bIsDirectory;
	s_pEnvBackend->get_file_info (cURI, &pNewIcon->cName, &pNewIcon->cCommand, &pNewIcon->cFileName, &bIsDirectory, &pNewIcon->iVolumeID, &pNewIcon->fOrder, iFileSortType);
	if (pNewIcon->cName == NULL)
	{
		cairo_dock_free_icon (pNewIcon);
		return NULL;
	}
	//g_print ("%s -> %s\n", cURI, pNewIcon->cFileName);

	if (bIsDirectory)
	{
		cd_message ("  c'est un sous-repertoire");
	}

	if (iFileSortType == CAIRO_DOCK_FM_SORT_BY_NAME)
	{
		GList *pList = (CAIRO_DOCK_IS_DOCK (pContainer) ? CAIRO_DOCK (pContainer)->icons : CAIRO_DESKLET (pContainer)->icons);
		GList *ic;
		Icon *icon;
		for (ic = pList; ic != NULL; ic = ic->next)
		{
			icon = ic->data;
			if (icon->cName != NULL && strcmp (pNewIcon->cName, icon->cName) < 0)
			{
				if (ic->prev != NULL)
				{
					Icon *prev_icon = ic->prev->data;
					pNewIcon->fOrder = (icon->fOrder + prev_icon->fOrder) / 2;
				}
				else
					pNewIcon->fOrder = icon->fOrder - 1;
				break ;
			}
			else if (ic->next == NULL)
			{
				pNewIcon->fOrder = icon->fOrder + 1;
			}
		}
	}
	cairo_dock_load_icon_buffers (pNewIcon, pContainer);

	return pNewIcon;
}

void cairo_dock_fm_create_dock_from_directory (Icon *pIcon, CairoDock *pParentDock)
{
	if (s_pEnvBackend == NULL)
		return;
	cd_message ("");
	g_free (pIcon->cCommand);
	pIcon->cCommand = NULL;
	GList *pIconList = cairo_dock_fm_list_directory (pIcon->cBaseURI, pIcon->iSortSubIcons, CAIRO_DOCK_LAUNCHER, mySystem.bShowHiddenFiles, pIcon->iNbSubIcons, &pIcon->cCommand);
	pIcon->pSubDock = cairo_dock_create_subdock_from_scratch (pIconList, pIcon->cName, pParentDock);

	cairo_dock_update_dock_size (pIcon->pSubDock);  // le 'load_buffer' ne le fait pas.

	cairo_dock_fm_add_monitor (pIcon);
}



static Icon *cairo_dock_fm_alter_icon_if_necessary (Icon *pIcon, CairoContainer *pContainer)
{
	if (s_pEnvBackend == NULL)
		return NULL;
	cd_debug ("%s (%s)", __func__, pIcon->cBaseURI);
	Icon *pNewIcon = cairo_dock_fm_create_icon_from_URI (pIcon->cBaseURI, pContainer, 0);  /// voir comment remonter a l'info iFileSortType ...
	g_return_val_if_fail (pNewIcon != NULL && pNewIcon->cName != NULL, NULL);

	//g_print ("%s <-> %s (%s <-> <%s)\n", pIcon->cName, pNewIcon->cName, pIcon->cFileName, pNewIcon->cFileName);
	if (pIcon->cName == NULL || strcmp (pIcon->cName, pNewIcon->cName) != 0 || pNewIcon->cFileName == NULL || strcmp (pIcon->cFileName, pNewIcon->cFileName) != 0 || pIcon->fOrder != pNewIcon->fOrder)
	{
		cd_message ("  on remplace %s", pIcon->cName);
		if (CAIRO_DOCK_IS_DOCK (pContainer))
		{
			pNewIcon->cParentDockName = g_strdup (pIcon->cParentDockName);
			cairo_dock_remove_one_icon_from_dock (CAIRO_DOCK (pContainer), pIcon);
		}
		else
		{
			CAIRO_DESKLET (pContainer)->icons = g_list_remove (CAIRO_DESKLET (pContainer)->icons, pIcon);
		}
		if (pIcon->cDesktopFileName != NULL)
			cairo_dock_fm_remove_monitor (pIcon);

		pNewIcon->cDesktopFileName = g_strdup (pIcon->cDesktopFileName);
		if (pIcon->pSubDock != NULL)
		{
			pNewIcon->pSubDock == pIcon->pSubDock;
			pIcon->pSubDock = NULL;

			if (pNewIcon->cName != NULL && strcmp (pIcon->cName, pNewIcon->cName) != 0)
			{
				cairo_dock_rename_dock (pIcon->cName, pNewIcon->pSubDock, pNewIcon->cName);
			}  // else : detruire le sous-dock.
		}
		pNewIcon->fX = pIcon->fX;
		pNewIcon->fXAtRest = pIcon->fXAtRest;
		pNewIcon->fDrawX = pIcon->fDrawX;
		pNewIcon->iType = pIcon->iType;
		
		if (CAIRO_DOCK_IS_DOCK (pContainer))
			cairo_dock_insert_icon_in_dock_full (pNewIcon, CAIRO_DOCK (pContainer), CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, ! CAIRO_DOCK_INSERT_SEPARATOR, NULL);  // on met a jour la taille du dock pour le fXMin/fXMax, et eventuellement la taille de l'icone peut aussi avoir change.
		else
			CAIRO_DESKLET (pContainer)->icons = g_list_insert_sorted (CAIRO_DESKLET (pContainer)->icons,
				pIcon,
				(GCompareFunc) cairo_dock_compare_icons_order);  // on n'utilise pas le pDesklet->pRenderer->load_icons, car on remplace juste une icone par une autre quasi identique, et on ne sait pas si load_icons a ete utilisee.
		cairo_dock_redraw_icon (pNewIcon, pContainer);

		if (pNewIcon->cDesktopFileName != NULL)
			cairo_dock_fm_add_monitor (pNewIcon);

		cairo_dock_free_icon (pIcon);
		return pNewIcon;
	}
	else
	{
		cairo_dock_free_icon (pNewIcon);
		return pIcon;
	}
}
void cairo_dock_fm_manage_event_on_file (CairoDockFMEventType iEventType, const gchar *cBaseURI, Icon *pIcon, CairoDockIconType iTypeOnCreation)
{
	g_return_if_fail (cBaseURI != NULL && pIcon != NULL);
	gchar *cURI = (g_strdup (cBaseURI));
	cairo_dock_remove_html_spaces (cURI);
	cd_message ("%s (%d sur %s)", __func__, iEventType, cURI);

	switch (iEventType)
	{
		case CAIRO_DOCK_FILE_DELETED :
		{
			Icon *pConcernedIcon;
			CairoContainer *pParentContainer;
			if (pIcon->cBaseURI != NULL && strcmp (cURI, pIcon->cBaseURI) == 0)
			{
				pConcernedIcon = pIcon;
				pParentContainer = cairo_dock_search_container_from_icon (pIcon);
			}
			else if (pIcon->pSubDock != NULL)
			{
				pConcernedIcon = cairo_dock_get_icon_with_base_uri (pIcon->pSubDock->icons, cURI);
				if (pConcernedIcon == NULL)  // on cherche par nom.
				{
					pConcernedIcon = cairo_dock_get_icon_with_name (pIcon->pSubDock->icons, cURI);
				}
				if (pConcernedIcon == NULL)
					return ;
				pParentContainer = CAIRO_CONTAINER (pIcon->pSubDock);
			}
			else
			{
				cd_warning ("  on n'aurait pas du recevoir cet evenement !");
				return ;
			}
			cd_message ("  %s sera supprimee", pConcernedIcon->cName);
			
			if (CAIRO_DOCK_IS_DOCK (pParentContainer))
			{
				cairo_dock_remove_one_icon_from_dock (CAIRO_DOCK (pParentContainer), pConcernedIcon);  // enleve aussi son moniteur.
				cairo_dock_update_dock_size (CAIRO_DOCK (pParentContainer));
			}
			else if (pConcernedIcon->cDesktopFileName != NULL)  // alors elle a un moniteur.
				cairo_dock_fm_remove_monitor (pConcernedIcon);
			
			cairo_dock_free_icon (pConcernedIcon);
		}
		break ;
		
		case CAIRO_DOCK_FILE_CREATED :
		{
			if ((pIcon->cBaseURI == NULL || strcmp (cURI, pIcon->cBaseURI) != 0) && pIcon->pSubDock != NULL)  // dans des cas foirreux, il se peut que le fichier soit cree alors qu'il existait deja dans le dock.
			{
				CairoContainer *pParentContainer = cairo_dock_search_container_from_icon (pIcon);
				
				if (pIcon->pSubDock != NULL)  // cas d'un signal CREATED sur un fichier deja existant, merci GFVS :-/
				{
					Icon *pSameIcon = cairo_dock_get_icon_with_base_uri (pIcon->pSubDock->icons, cURI);
					if (pSameIcon != NULL)
					{
						cd_message ("ce fichier (%s) existait deja !", pSameIcon->cName);
						return;  // on decide de ne rien faire, c'est surement un signal inutile.
						//cairo_dock_remove_one_icon_from_dock (pIcon->pSubDock, pSameIcon);
						//cairo_dock_free_icon (pSameIcon);
					}
				}
				Icon *pNewIcon = cairo_dock_fm_create_icon_from_URI (cURI, (CAIRO_DOCK_IS_DOCK (pParentContainer) ? CAIRO_CONTAINER (pIcon->pSubDock) : pParentContainer), 0);  /// voir comment remonter a l'info iFileSortType ...
				if (pNewIcon == NULL)
					return ;
				pNewIcon->iType = iTypeOnCreation;

				if (CAIRO_DOCK_IS_DOCK (pParentContainer))
				{
					cairo_dock_insert_icon_in_dock_full (pNewIcon, pIcon->pSubDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, ! CAIRO_DOCK_INSERT_SEPARATOR, NULL);
					pNewIcon->cParentDockName = g_strdup (pIcon->cName);
				}
				else
					CAIRO_DESKLET (pParentContainer)->icons = g_list_insert_sorted (CAIRO_DESKLET (pParentContainer)->icons,
						pIcon,
						(GCompareFunc) cairo_dock_compare_icons_order);
				cd_message ("  %s a ete insere(e)", (pNewIcon != NULL ? pNewIcon->cName : "aucune icone n'"));
				
				if (pNewIcon->iVolumeID > 0)
				{
					gboolean bIsMounted;
					gchar *cUri = cairo_dock_fm_is_mounted (pNewIcon->cBaseURI, &bIsMounted);
					g_free (cUri);
					if (bIsMounted)
					{
						cd_message (" c'est un volume, on considere qu'il vient de se faire monter");
						cairo_dock_remove_dialog_if_any (pNewIcon);  // on empeche la multiplication des dialogues de (de)montage.
						cairo_dock_show_temporary_dialog_with_icon_printf (_("%s is now mounted"), pNewIcon, CAIRO_DOCK_IS_DOCK (pParentContainer) ? CAIRO_CONTAINER (pIcon->pSubDock) : pParentContainer, 4000, "same icon", pNewIcon->cName);
					}
				}
			}
		}
		break ;
		
		case CAIRO_DOCK_FILE_MODIFIED :
		{
			Icon *pConcernedIcon;
			CairoContainer *pParentContainer;
			if (pIcon->cBaseURI != NULL && strcmp (pIcon->cBaseURI, cURI) == 0)  // c'est l'icone elle-meme.
			{
				pConcernedIcon = pIcon;
				pParentContainer = cairo_dock_search_container_from_icon (pIcon);
				g_return_if_fail (pParentContainer != NULL);
			}
			else if (pIcon->pSubDock != NULL)  // c'est a l'interieur du repertoire qu'elle represente.
			{
				pConcernedIcon = cairo_dock_get_icon_with_base_uri (pIcon->pSubDock->icons, cURI);
				//g_print ("cURI in sub-dock: %s\n", cURI);
				if (pConcernedIcon == NULL)  // on cherche par nom.
				{
					pConcernedIcon = cairo_dock_get_icon_with_name (pIcon->pSubDock->icons, cURI);
				}
				g_return_if_fail (pConcernedIcon != NULL);
				pParentContainer = CAIRO_CONTAINER (pIcon->pSubDock);
			}
			else
			{
				cd_warning ("  a file has been modified but we couldn't find which one.");
				return ;
			}
			cd_message ("  %s est modifiee", pConcernedIcon->cName);
			
			if (pConcernedIcon->iVolumeID > 0)
				cairo_dock_remove_dialog_if_any (pConcernedIcon);  // on empeche la multiplication des dialogues de (de)montage.
			Icon *pNewIcon = cairo_dock_fm_alter_icon_if_necessary (pConcernedIcon, pParentContainer);  // pConcernedIcon a ete remplacee et n'est donc peut-etre plus valide.
			
			if (pNewIcon != NULL && pNewIcon->iVolumeID > 0)
			{
				cd_message ("ce volume a change");
				gboolean bIsMounted = FALSE;
				if (s_pEnvBackend->is_mounted != NULL)
				{
					gchar *cActivationURI = s_pEnvBackend->is_mounted (pNewIcon->cBaseURI, &bIsMounted);
					g_free (cActivationURI);
				}
				cairo_dock_show_temporary_dialog_with_icon_printf (bIsMounted ? _("%s is now mounted") : _("%s is now unmounted"), pNewIcon, pParentContainer, 4000, "same icon", pNewIcon->cName);
			}
		}
		break ;
	}
	g_free (cURI);
}

void cairo_dock_fm_action_on_file_event (CairoDockFMEventType iEventType, const gchar *cURI, Icon *pIcon)
{
	cairo_dock_fm_manage_event_on_file (iEventType, cURI, pIcon, CAIRO_DOCK_LAUNCHER);
}

void cairo_dock_fm_action_after_mounting (gboolean bMounting, gboolean bSuccess, const gchar *cName, Icon *icon, CairoContainer *pContainer)
{
	cd_message ("%s (%s) : %d\n", __func__, (bMounting ? "mount" : "unmount"), bSuccess);  // en cas de demontage effectif, l'icone n'est plus valide !
	if ((! bSuccess && pContainer != NULL) || icon == NULL)  // dans l'autre cas (succes), l'icone peut ne plus etre valide ! mais on s'en fout, puisqu'en cas de succes, il y'aura rechargement de l'icone, et donc on pourra balancer le message a ce moment-la.
	{
		///if (icon != NULL)
			cairo_dock_show_temporary_dialog_with_icon_printf (bMounting ? _("failed to mount %s") : _("Failed to unmount %s"), icon, pContainer, 4000, "same icon", cName);
		///else
		///	cairo_dock_show_general_message (cMessage, 4000);
	}
}



gboolean cairo_dock_fm_move_into_directory (const gchar *cURI, Icon *icon, CairoContainer *pContainer)
{
	g_return_val_if_fail (cURI != NULL && icon != NULL, FALSE);
	cd_message (" -> copie de %s dans %s", cURI, icon->cBaseURI);
	gboolean bSuccess = cairo_dock_fm_move_file (cURI, icon->cBaseURI);
	if (! bSuccess)
	{
		cd_warning ("couldn't copy this file.\nCheck that you have writing rights, and that the new does not already exist.");
		gchar *cMessage = g_strdup_printf ("Attention : couldn't copy %s into %s.\nCheck that you have writing rights, and that the name does not already exist.", cURI, icon->cBaseURI);
		cairo_dock_show_temporary_dialog (cMessage, icon, pContainer, 4000);
		g_free (cMessage);
	}
	return bSuccess;
}


CairoDockDesktopEnv cairo_dock_guess_environment (void)
{
	const gchar * cEnv = g_getenv ("GNOME_DESKTOP_SESSION_ID");
	if (cEnv != NULL && *cEnv != '\0')
		return CAIRO_DOCK_GNOME;
	
	cEnv = g_getenv ("KDE_FULL_SESSION");
	if (cEnv != NULL && *cEnv != '\0')
		return CAIRO_DOCK_KDE;
	
	cEnv = g_getenv ("KDE_SESSION_UID");
	if (cEnv != NULL && *cEnv != '\0')
		return CAIRO_DOCK_KDE;
	
	if (cairo_dock_property_is_present_on_root ("_DT_SAVE_MODE"))
		return CAIRO_DOCK_XFCE;
	
	gchar *cKWin = cairo_dock_launch_command_sync ("pgrep kwin");
	if (cKWin != NULL && *cKWin != '\0')
	{
		g_free (cKWin);
		return CAIRO_DOCK_KDE;
	}
	g_free (cKWin);
	
	return CAIRO_DOCK_UNKNOWN_ENV;
	
}

int cairo_dock_get_file_size (const gchar *cFilePath)
{
	struct stat buf;
	if (cFilePath == NULL)
		return 0;
	buf.st_size = 0;
	if (stat (cFilePath, &buf) != -1)
	{
		return buf.st_size;
	}
	else
		return 0;
}
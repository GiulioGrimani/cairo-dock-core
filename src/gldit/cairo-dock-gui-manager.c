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

#include "../../config.h"
#include "cairo-dock-struct.h"
#include "cairo-dock-modules.h"
#include "cairo-dock-log.h"
#include "cairo-dock-gui-factory.h"
#include "cairo-dock-keyfile-utilities.h"
#include "cairo-dock-animations.h"
#include "cairo-dock-draw.h"
#include "cairo-dock-dialog-manager.h"
#include "cairo-dock-dock-manager.h"
#include "cairo-dock-container.h"
#include "cairo-dock-applications-manager.h"
#include "cairo-dock-launcher-factory.h"
#include "cairo-dock-load.h"
#include "cairo-dock-internal-accessibility.h"
#include "cairo-dock-desktop-file-factory.h"
#include "cairo-dock-gui-manager.h"

#define CAIRO_DOCK_GUI_MARGIN 6

extern CairoDock *g_pMainDock;
extern gchar *g_cCairoDockDataDir;

static CairoDockGuiBackend *s_pGuiBackend = NULL;
static CairoDockLauncherGuiBackend *s_pLauncherGuiBackend = NULL;

  //////////////////////////
 // MAIN DOCK VISIBILITY //
//////////////////////////

static int iNbConfigDialogs = 0;
int cairo_dock_get_nb_dialog_windows (void)
{
	return iNbConfigDialogs;
}

void cairo_dock_dialog_window_destroyed (void)
{
	/**iNbConfigDialogs --;
	if (iNbConfigDialogs <= 0)
	{
		iNbConfigDialogs = 0;
		if (g_pMainDock != NULL)  // peut arriver au 1er lancement.
			cairo_dock_pop_down (g_pMainDock);
	}*/
	//g_print ("iNbConfigDialogs <- %d\n", iNbConfigDialogs);
}

void cairo_dock_dialog_window_created (void)
{
	/**iNbConfigDialogs ++;
	//g_print ("iNbConfigDialogs <- %d\n", iNbConfigDialogs);
	if (g_pMainDock != NULL && cairo_dock_search_window_covering_dock (g_pMainDock, FALSE, TRUE) == NULL)  // peut etre NULL au 1er lancement.
		cairo_dock_pop_up (g_pMainDock);*/
}


  //////////////////////////
 // DYNAMIC CONFIG PANEL //
//////////////////////////

GtkWidget *cairo_dock_get_widget_from_name (const gchar *cGroupName, const gchar *cKeyName)
{
	if (s_pGuiBackend && s_pGuiBackend->get_widgets_from_name)
	{
		GSList *pList = s_pGuiBackend->get_widgets_from_name (cGroupName, cKeyName);
		if (pList != NULL)
			return pList->data;
	}
	return NULL;
}

void cairo_dock_reload_current_module_widget_full (CairoDockModuleInstance *pInstance, int iShowPage)
{
	g_return_if_fail (pInstance != NULL);
	if (pInstance->pModule->pVisitCard->cInternalModule != NULL)
	{
		cairo_dock_show_module_gui (pInstance->pModule->pVisitCard->cInternalModule);
	}
	else
	{
		cairo_dock_show_module_instance_gui (pInstance, iShowPage);
	}
	/**g_return_if_fail (s_pCurrentGroupWidget != NULL && s_pCurrentGroup != NULL && cairo_dock_get_current_widget_list () != NULL);
	
	int iNotebookPage = (GTK_IS_NOTEBOOK (s_pCurrentGroupWidget) ? 
		(iShowPage >= 0 ?
			iShowPage :
			gtk_notebook_get_current_page (GTK_NOTEBOOK (s_pCurrentGroupWidget))) :
		-1);
	
	gtk_widget_destroy (s_pCurrentGroupWidget);
	s_pCurrentGroupWidget = NULL;
	
	cairo_dock_reset_current_widget_list ();
	
	CairoDockModule *pModule = cairo_dock_find_module_from_name (s_pCurrentGroup->cGroupName);
	GtkWidget *pWidget;
	if (pModule != NULL)
	{
		pWidget = cairo_dock_present_group_widget (pModule->cConfFilePath, s_pCurrentGroup, FALSE, pInstance);
	}
	else
	{
		pWidget = cairo_dock_present_group_widget (g_cConfFile, s_pCurrentGroup, TRUE, NULL);
	}
	if (iNotebookPage != -1)
	{
		gtk_notebook_set_current_page (GTK_NOTEBOOK (s_pCurrentGroupWidget), iNotebookPage);
	}*/
}


void cairo_dock_deactivate_module_in_gui (const gchar *cModuleName)
{
	if (s_pGuiBackend && s_pGuiBackend->deactivate_module_in_gui)
		s_pGuiBackend->deactivate_module_in_gui (cModuleName);
}

static gboolean _cairo_dock_module_is_opened (CairoDockModuleInstance *pModuleInstance)
{
	if (s_pGuiBackend && s_pGuiBackend->module_is_opened)
		return s_pGuiBackend->module_is_opened (pModuleInstance);
	else
		return FALSE;
}

void cairo_dock_update_desklet_size_in_gui (CairoDockModuleInstance *pModuleInstance, int iWidth, int iHeight)
{
	if (_cairo_dock_module_is_opened (pModuleInstance))  // on est en train d'editer ce module dans le panneau de conf.
	{
		GSList *pSubWidgetList = NULL;
		GtkWidget *pOneWidget;
		if (s_pGuiBackend && s_pGuiBackend->get_widgets_from_name)
			pSubWidgetList = s_pGuiBackend->get_widgets_from_name ("Desklet", "size");
		
		if (pSubWidgetList != NULL)
		{
			pOneWidget = pSubWidgetList->data;
			g_signal_handlers_block_matched (pOneWidget,
				(GSignalMatchType) G_SIGNAL_MATCH_FUNC,
				0, 0, NULL, _cairo_dock_set_value_in_pair, NULL);
			gtk_spin_button_set_value (GTK_SPIN_BUTTON (pOneWidget), iWidth);
			g_signal_handlers_unblock_matched (pOneWidget,
				(GSignalMatchType) G_SIGNAL_MATCH_FUNC,
				0, 0, NULL, _cairo_dock_set_value_in_pair, NULL);
			
			if (pSubWidgetList->next != NULL)
			{
				pOneWidget = pSubWidgetList->next->data;
				g_signal_handlers_block_matched (pOneWidget,
					(GSignalMatchType) G_SIGNAL_MATCH_FUNC,
					0, 0, NULL, _cairo_dock_set_value_in_pair, NULL);
				gtk_spin_button_set_value (GTK_SPIN_BUTTON (pOneWidget), iHeight);
				g_signal_handlers_unblock_matched (pOneWidget,
					(GSignalMatchType) G_SIGNAL_MATCH_FUNC,
					0, 0, NULL, _cairo_dock_set_value_in_pair, NULL);
			}
		}
		
	}
}

void cairo_dock_update_desklet_position_in_gui (CairoDockModuleInstance *pModuleInstance, int x, int y)
{
	if (_cairo_dock_module_is_opened (pModuleInstance))  // on est en train d'editer ce module dans le panneau de conf.
	{
		GtkWidget *pOneWidget;
		pOneWidget = cairo_dock_get_widget_from_name ("Desklet", "x position");
		if (pOneWidget != NULL)
			gtk_spin_button_set_value (GTK_SPIN_BUTTON (pOneWidget), x);
		pOneWidget = cairo_dock_get_widget_from_name ("Desklet", "y position");
		if (pOneWidget != NULL)
			gtk_spin_button_set_value (GTK_SPIN_BUTTON (pOneWidget), y);
	}
}

void cairo_dock_update_desklet_detached_state_in_gui (CairoDockModuleInstance *pModuleInstance, gboolean bIsDetached)
{
	if (_cairo_dock_module_is_opened (pModuleInstance))  // on est en train d'editer ce module dans le panneau de conf.
	{
		GtkWidget *pOneWidget = cairo_dock_get_widget_from_name ("Desklet", "initially detached");
		if (pOneWidget != NULL)
			gtk_toggle_button_set_active  (GTK_TOGGLE_BUTTON (pOneWidget), bIsDetached);
	}
}


void cairo_dock_set_status_message (GtkWidget *pWindow, const gchar *cMessage)
{
	cd_debug ("%s (%s)\n", __func__, cMessage);
	GtkWidget *pStatusBar;
	if (pWindow != NULL)
	{
		pStatusBar = g_object_get_data (G_OBJECT (pWindow), "status-bar");
		if (pStatusBar == NULL)
			return ;
		//g_print ("%s (%s sur %x/%x)\n", __func__, cMessage, pWindow, pStatusBar);
		gtk_statusbar_pop (GTK_STATUSBAR (pStatusBar), 0);  // clear any previous message, underflow is allowed.
		gtk_statusbar_push (GTK_STATUSBAR (pStatusBar), 0, cMessage);
	}
	else
	{
		if (s_pGuiBackend && s_pGuiBackend->set_status_message_on_gui)
			s_pGuiBackend->set_status_message_on_gui (cMessage);
	}
}

void cairo_dock_set_status_message_printf (GtkWidget *pWindow, const gchar *cFormat, ...)
{
	g_return_if_fail (cFormat != NULL);
	va_list args;
	va_start (args, cFormat);
	gchar *cMessage = g_strdup_vprintf (cFormat, args);
	cairo_dock_set_status_message (pWindow, cMessage);
	g_free (cMessage);
	va_end (args);
}


  /////////////////
 // GUI BACKEND //
/////////////////

void cairo_dock_register_gui_backend (CairoDockGuiBackend *pBackend)
{
	g_free (s_pGuiBackend);
	s_pGuiBackend = pBackend;
}

GtkWidget *cairo_dock_show_main_gui (void)
{
	GtkWidget *pWindow = NULL;
	if (s_pGuiBackend && s_pGuiBackend->show_main_gui)
		pWindow = s_pGuiBackend->show_main_gui ();
	if (pWindow && g_pMainDock != NULL)  // evitons d'empieter sur le main dock.
	{
		if (g_pMainDock->container.bIsHorizontal)
		{
			if (g_pMainDock->container.bDirectionUp)
				gtk_window_move (GTK_WINDOW (pWindow), 0, 0);
			else
				gtk_window_move (GTK_WINDOW (pWindow), 0, g_pMainDock->iMinDockHeight+10);
		}
		else
		{
			if (g_pMainDock->container.bDirectionUp)
				gtk_window_move (GTK_WINDOW (pWindow), 0, 0);
			else
				gtk_window_move (GTK_WINDOW (pWindow), g_pMainDock->iMinDockHeight+10, 0);
		}
	}
	
	return pWindow;
}

void cairo_dock_show_module_instance_gui (CairoDockModuleInstance *pModuleInstance, int iShowPage)
{
	if (s_pGuiBackend && s_pGuiBackend->show_module_instance_gui)
		s_pGuiBackend->show_module_instance_gui (pModuleInstance, iShowPage);
}

void cairo_dock_show_module_gui (const gchar *cModuleName)
{
	if (s_pGuiBackend && s_pGuiBackend->show_module_gui)
		s_pGuiBackend->show_module_gui (cModuleName);
}

void cairo_dock_close_gui (void)
{
	if (s_pGuiBackend && s_pGuiBackend->close_gui)
		s_pGuiBackend->close_gui ();
}


  ////////////////
 // NORMAL GUI //
////////////////

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
	
	cairo_dock_dialog_window_destroyed ();
	
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
					pModule->pInterface->save_custom_widget (pModuleInstance, pKeyFile);
			}
		}
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
	gtk_window_set_icon_from_file (GTK_WINDOW (pMainWindow), CAIRO_DOCK_SHARE_DATA_DIR"/"CAIRO_DOCK_ICON, NULL);
	if (cTitle != NULL)
		gtk_window_set_title (GTK_WINDOW (pMainWindow), cTitle);
	
	GtkWidget *pMainVBox = gtk_vbox_new (FALSE, CAIRO_DOCK_GUI_MARGIN);
	gtk_container_add (GTK_CONTAINER (pMainWindow), pMainVBox);
	
	//\_____________ On ajoute les boutons.
	GtkWidget *pButtonsHBox = gtk_hbox_new (FALSE, CAIRO_DOCK_GUI_MARGIN*2);
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
	gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (pStatusBar), FALSE);
	gtk_box_pack_start (GTK_BOX (pButtonsHBox),  // pMainVBox
		pStatusBar,
		FALSE,
		FALSE,
		0);
	g_object_set_data (G_OBJECT (pMainWindow), "status-bar", pStatusBar);
	
	gtk_window_resize (GTK_WINDOW (pMainWindow), iWidth, iHeight);
	
	gtk_widget_show_all (pMainWindow);
	cairo_dock_dialog_window_created ();
	
	int iResult = 0;
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

gboolean cairo_dock_build_generic_gui (const gchar *cConfFilePath, const gchar *cGettextDomain, const gchar *cTitle, int iWidth, int iHeight, CairoDockApplyConfigFunc pAction, gpointer pUserData, GFreeFunc pFreeUserData, GtkWidget **pWindow)
{
	//\_____________ On construit la fenetre.
	GtkWidget *pMainWindow = cairo_dock_build_generic_gui_window (cTitle, iWidth, iHeight, pAction, pUserData, pFreeUserData);
	
	//\_____________ On construit l'IHM du fichier de conf.
	GSList *pWidgetList = NULL;
	GPtrArray *pDataGarbage = g_ptr_array_new ();
	GtkWidget *pNoteBook = cairo_dock_build_conf_file_widget (cConfFilePath,
		cGettextDomain,
		pMainWindow,
		&pWidgetList,
		pDataGarbage,
		NULL);
	
	//\_____________ On l'insere dans la fenetre.
	GtkWidget *pMainVBox = gtk_bin_get_child (GTK_BIN (pMainWindow));
	gtk_box_pack_start (GTK_BOX (pMainVBox),
		pNoteBook,
		TRUE,
		TRUE,
		0);
	gtk_widget_show_all (pMainWindow);
	
	g_object_set_data (G_OBJECT (pMainWindow), "conf-file", g_strdup (cConfFilePath));
	g_object_set_data (G_OBJECT (pMainWindow), "widget-list", pWidgetList);
	g_object_set_data (G_OBJECT (pMainWindow), "garbage", pDataGarbage);
	
	int iResult = 0;
	if (pAction != NULL)
	{
		if (pWindow)
			*pWindow = pMainWindow;
	}
	else  // on bloque.
	{
		GList *children = gtk_container_get_children (GTK_CONTAINER (pMainVBox));
		GList *w = g_list_last (children);
		g_return_val_if_fail (w != NULL, FALSE);
		GtkWidget *pButtonsHBox = w->data;
		
		GtkWidget *pOkButton = gtk_button_new_from_stock (GTK_STOCK_OK);
		g_signal_connect (G_OBJECT (pOkButton), "clicked", G_CALLBACK(on_click_generic_ok), pMainWindow);
		gtk_box_pack_end (GTK_BOX (pButtonsHBox),
			pOkButton,
			FALSE,
			FALSE,
			0);
		
		gtk_window_set_modal (GTK_WINDOW (pMainWindow), TRUE);
		GMainLoop *pBlockingLoop = g_main_loop_new (NULL, FALSE);
		g_object_set_data (G_OBJECT (pMainWindow), "loop", pBlockingLoop);
		g_signal_connect (G_OBJECT (pMainWindow),
			"delete-event",
			G_CALLBACK (on_delete_generic_gui),
			pBlockingLoop);

		cd_debug ("debut de boucle bloquante ...\n");
		GDK_THREADS_LEAVE ();
		g_main_loop_run (pBlockingLoop);
		GDK_THREADS_ENTER ();
		cd_debug ("fin de boucle bloquante\n");
		
		g_main_loop_unref (pBlockingLoop);
		g_object_set_data (G_OBJECT (pMainWindow), "loop", NULL);
		
		iResult = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (pMainWindow), "result"));
		cd_debug ("iResult : %d", iResult);
		gtk_widget_destroy (pMainWindow);
		if (pWindow)
			*pWindow = NULL;
	}
	
	return iResult;
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
	gtk_widget_destroy (pNoteBook);
	
	gchar *cConfFilePath = g_object_get_data (G_OBJECT (pWindow), "conf-file");
	pNoteBook = cairo_dock_build_conf_file_widget (cConfFilePath, NULL, pWindow, &pWidgetList, pDataGarbage, cConfFilePath);
	gtk_box_pack_start (GTK_BOX (pMainVBox),
		pNoteBook,
		TRUE,
		TRUE,
		0);
	gtk_widget_show_all (pNoteBook);
	g_object_set_data (G_OBJECT (pWindow), "widget-list", pWidgetList);
	g_object_set_data (G_OBJECT (pWindow), "garbage", pDataGarbage);
}


  //////////////////////////
 // LAUNCHER GUI BACKEND //
//////////////////////////

void cairo_dock_register_launcher_gui_backend (CairoDockLauncherGuiBackend *pBackend)
{
	g_free (s_pLauncherGuiBackend);
	s_pLauncherGuiBackend = pBackend;
}

GtkWidget *cairo_dock_build_launcher_gui (Icon *pIcon)
{
	if (s_pLauncherGuiBackend && s_pLauncherGuiBackend->show_gui)
		s_pLauncherGuiBackend->show_gui (pIcon);
}

static guint s_iSidRefreshGUI = 0;
static gboolean _refresh_launcher_gui (gpointer data)
{
	if (s_pLauncherGuiBackend && s_pLauncherGuiBackend->refresh_gui)
		s_pLauncherGuiBackend->refresh_gui ();
	
	s_iSidRefreshGUI = 0;
	return FALSE;
}
void cairo_dock_trigger_refresh_launcher_gui (void)
{
	if (s_iSidRefreshGUI != 0)
		return;
	
	s_iSidRefreshGUI = g_idle_add ((GSourceFunc) _refresh_launcher_gui, NULL);
}
#include <dconf.h>

#include "quickpad_window.h"


struct _QuickpadAppWindow {
	GtkApplicationWindow parent;
	
	GtkWidget * btnTlbrNew;
	GtkWidget * btnTlbrImport;
	GtkWidget * btnTlbrExport;
	GtkWidget * btnTlbrMnuConfig;
	
	GtkCheckMenuItem * mnuLineNumbers;
	GtkCheckMenuItem * mnuHighlightCurrentLine;
	
	GtkNotebook * ntbContent;
	
	guint iTabCounter;
        gboolean bInitTab;
};

typedef struct _QuickpadTab {
	QuickpadAppWindow *pWindow;
	GtkWidget * pPageChild;
	GtkWidget * pHBoxShow;
	GtkWidget * pHBoxEdit;
	GtkWidget * pLabel;
	GtkWidget * pTxtLabel;
	GtkWidget * pTextView;
	GSettings * pTabSettings;
	gchar * pcTabId;
	gint iTabPos;
} QuickpadTab;

G_DEFINE_TYPE(QuickpadAppWindow, quickpad_app_window, GTK_TYPE_APPLICATION_WINDOW);

enum {
	PROP_TAB_COUNTER = 1,
	N_PROPERTIES
};

static void quickpad_app_window_set_property (GObject * object, guint property_id, const GValue * value, GParamSpec * pspec) {
	QuickpadAppWindow *self = QUICKPAD_APP_WINDOW(object);

	switch (property_id) {
		case PROP_TAB_COUNTER:
			self->iTabCounter = g_value_get_int(value);
			break;
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
}

static void quickpad_app_window_get_property (GObject * object, guint property_id, GValue * value, GParamSpec *pspec) {
	QuickpadAppWindow *self = QUICKPAD_APP_WINDOW(object);

	switch (property_id) {
		case PROP_TAB_COUNTER:
			g_value_set_int(value, self->iTabCounter);
			break;
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
}

static GParamSpec * arObjectProperties[N_PROPERTIES] = { NULL, };

QuickpadTab * quickpad_app_window_add_tab(QuickpadAppWindow * pWindow, gchar * pcTabId, gchar * pcTitle, gchar * pcContent);
gchar * quickpad_app_window_select_name(QuickpadAppWindow * pWindow, GtkFileChooserAction action, gchar * pcFilename);

void quickpad_clbk_btn_edit(GtkMenuItem *menuitem, gpointer user_data);
void quickpad_clbk_btn_save(GtkMenuItem *menuitem, gpointer user_data);
void quickpad_clbk_btn_cancel(GtkMenuItem *menuitem, gpointer user_data);
void quickpad_clbk_btn_close(GtkMenuItem *menuitem, gpointer user_data);
gboolean quickpad_clbk_entry_evt_keyrelease (GtkWidget *widget, GdkEventKey *event, gpointer user_data);

static void quickpad_app_window_init (QuickpadAppWindow *pWindow) {
	GtkBuilder * pBuilder;
	GtkWidget * pMenuConfig;
	
	gtk_widget_init_template(GTK_WIDGET(pWindow));
	
	pWindow->bInitTab = FALSE;
}

static void quickpad_app_window_class_init (QuickpadAppWindowClass *pClass) {
	GObjectClass * pObjectClass = G_OBJECT_CLASS (pClass);

	pObjectClass->set_property = quickpad_app_window_set_property;
	pObjectClass->get_property = quickpad_app_window_get_property;

	arObjectProperties[PROP_TAB_COUNTER] = g_param_spec_int("tab-counter", "tab-counter", "Id of the next tab", 0, INT_MAX, 1, G_PARAM_READWRITE);
	g_object_class_install_properties (pObjectClass, N_PROPERTIES, arObjectProperties);
	
	gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(pClass), "/net/thepozer/quickpad/quickpad.wnd-main.glade");
	
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(pClass), QuickpadAppWindow, btnTlbrNew);
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(pClass), QuickpadAppWindow, btnTlbrImport);
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(pClass), QuickpadAppWindow, btnTlbrExport);
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(pClass), QuickpadAppWindow, btnTlbrMnuConfig);

	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(pClass), QuickpadAppWindow, mnuLineNumbers);
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(pClass), QuickpadAppWindow, mnuHighlightCurrentLine);
	
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(pClass), QuickpadAppWindow, ntbContent);
}

gboolean quickpad_clbk_delete_event (GtkWidget * widget, GdkEvent * event, gpointer user_data) {
	QuickpadAppWindow * pWindow = QUICKPAD_APP_WINDOW(widget);
	
	gtk_widget_destroy(widget);
	
	return TRUE;
}

QuickpadAppWindow * quickpad_app_window_new (QuickpadApp * pApp) {
	QuickpadAppWindow * pWindow = g_object_new (QUICKPAD_TYPE_APP_WINDOW, "application", pApp, NULL);
	GtkWidget * pPageChild = NULL;
	QuickpadTab * pTabInfo = NULL;
	DConfClient * pDcnfClient;
	GSettings * pSettings = quickpad_app_get_settings(pApp);
	gchar ** pArElements, * pcTabId;
	gint iNbElements, iIdx;
	
	g_settings_bind(pSettings, "tab-counter", pWindow, "tab-counter", G_SETTINGS_BIND_DEFAULT);
	g_settings_bind(pSettings, "line-numbers", pWindow->mnuLineNumbers, "active", G_SETTINGS_BIND_DEFAULT);
	g_settings_bind(pSettings, "highlight-current-line", pWindow->mnuHighlightCurrentLine, "active", G_SETTINGS_BIND_DEFAULT);
	
	g_signal_connect(pWindow, "delete-event", G_CALLBACK(quickpad_clbk_delete_event), pWindow);
	
	pWindow->bInitTab = TRUE;
        
	pDcnfClient = dconf_client_new();
	pArElements = dconf_client_list(pDcnfClient, "/net/thepozer/quickpad/tabs/", &iNbElements);
	for (iIdx = 0; iIdx < iNbElements; iIdx ++) {
		pcTabId = g_strndup(pArElements[iIdx], strlen(pArElements[iIdx]) - 1);
		pTabInfo = quickpad_app_window_add_tab(pWindow, pcTabId, NULL, NULL);
		pTabInfo->iTabPos = g_settings_get_int(pTabInfo->pTabSettings, "order");
	}
        
	iNbElements = gtk_notebook_get_n_pages(pWindow->ntbContent);
	for (iIdx = 0; iIdx < iNbElements; iIdx ++) {
		pPageChild = gtk_notebook_get_nth_page(pWindow->ntbContent, iIdx);
		if (pPageChild != NULL) {
			pTabInfo = g_object_get_data(G_OBJECT(pPageChild), "tab_info");
			if (pTabInfo != NULL) {
				gtk_notebook_reorder_child(pWindow->ntbContent, pPageChild, pTabInfo->iTabPos);
			}
		}
	}
	
	pWindow->bInitTab = FALSE;
        
	return pWindow;
}

void quickpad_clbk_mnu_about (GtkMenuItem *menuitem, gpointer user_data) {
	QuickpadAppWindow * pWindow = QUICKPAD_APP_WINDOW(user_data);
	const gchar * pcAuthors[] = {
		PACKAGE_BUGREPORT,
		NULL
	};
	
	gtk_show_about_dialog (GTK_WINDOW(pWindow),
		"authors", pcAuthors,
		"comments", _("Small/simple notes editor.\nDon't have to save them manualy"),
		"copyright", _("(c) 2018 Didier Prolhac"),
		"license", _("MIT"),
		"license-type", GTK_LICENSE_MIT_X11,
		"program-name", PACKAGE_NAME,
		"version", PACKAGE_VERSION,
		"website", "https://quickpad.thepozer.net/",
		"website-label", _("QuickPad"),
		NULL);
}

void quickpad_clbk_mnu_quit (GtkMenuItem *menuitem, gpointer user_data) {
	QuickpadAppWindow * pWindow = QUICKPAD_APP_WINDOW(user_data);
	
	quickpad_app_quit(QUICKPAD_APP(gtk_window_get_application(GTK_WINDOW(pWindow))));
}

void quickpad_clbk_btn_new (GtkMenuItem *menuitem, gpointer user_data) {
	QuickpadAppWindow * pWindow = QUICKPAD_APP_WINDOW(user_data);
	
	quickpad_app_window_add_tab(pWindow, NULL, _("New pad"), "");
}

void quickpad_clbk_btn_import (GtkMenuItem *menuitem, gpointer user_data) {
	QuickpadAppWindow * pWindow = QUICKPAD_APP_WINDOW(user_data);
	GFile * pFile = NULL;
	GError * pErr = NULL;
	gchar * pcFilename = NULL, * pcTitle = NULL, * pcContent = NULL;
	
	pcFilename = quickpad_app_window_select_name(pWindow, GTK_FILE_CHOOSER_ACTION_OPEN, NULL);
	if (pcFilename != NULL) {
	    pFile = g_file_new_for_path(pcFilename);
	    pcTitle = g_file_get_basename(pFile);
	    
	    if (g_file_load_contents(pFile, NULL, &pcContent, NULL, NULL, &pErr)) {
		    quickpad_app_window_add_tab(pWindow, NULL, pcTitle, pcContent);
		    g_free(pcContent);
	    } else {
		    g_printerr(_("Error importing file '%s' : (%i) %s"), pcFilename, pErr->code, pErr->message);
	    }
	    
	    g_free(pcTitle);
    	g_free(pcFilename);
	}
}

void quickpad_clbk_btn_export_cb_async (GObject *source_object, GAsyncResult *res, gpointer user_data) {
	GtkSourceFileSaver * pSrcFileSaver = GTK_SOURCE_FILE_SAVER(source_object);
	gchar * pcFilename = user_data;
	GError * pErr = NULL;
	gboolean success = FALSE;

	success = gtk_source_file_saver_save_finish(pSrcFileSaver, res, &pErr);

	if (!success) {
		g_printerr(_("Error writing export '%s' : (%i) %s"), pcFilename, pErr->code, pErr->message);
	}
	
	g_free(pcFilename);
}

void quickpad_clbk_btn_export (GtkMenuItem *menuitem, gpointer user_data) {
	QuickpadAppWindow * pWindow = QUICKPAD_APP_WINDOW(user_data);
	GtkTextBuffer * pTextBuffer = NULL;
	GtkSourceFileSaver * pSrcFileSaver = NULL;
	GtkSourceFile * pSrcFile = NULL;
	GtkWidget * pPageChild = NULL;
	QuickpadTab * pTabInfo = NULL;
	GFile * pFile = NULL;
	gchar * pcFilename = NULL;
	gint iNumPage = -1;
	
	iNumPage = gtk_notebook_get_current_page(pWindow->ntbContent);
	if (iNumPage >= 0) {
		pPageChild = gtk_notebook_get_nth_page(pWindow->ntbContent, iNumPage);
		if (pPageChild != NULL) {
			pTabInfo = g_object_get_data(G_OBJECT(pPageChild), "tab_info");
			if (pTabInfo != NULL) {
				pcFilename = quickpad_app_window_select_name(pWindow, GTK_FILE_CHOOSER_ACTION_SAVE, NULL);
				
    				if (pcFilename != NULL) {
					g_print("Export file : '%s'.\n", pcFilename);
					
					pTextBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pTabInfo->pTextView));
					pFile = g_file_new_for_path(pcFilename);
					pSrcFile = gtk_source_file_new();
					gtk_source_file_set_location(pSrcFile, pFile);
					    
					pSrcFileSaver = gtk_source_file_saver_new(GTK_SOURCE_BUFFER(pTextBuffer), pSrcFile);
					    
					gtk_source_file_saver_save_async(pSrcFileSaver, G_PRIORITY_DEFAULT, NULL, NULL, NULL, NULL, quickpad_clbk_btn_export_cb_async, pcFilename);
				}
			}
		}
	}
}

gchar * quickpad_app_window_select_name(QuickpadAppWindow * pWindow, GtkFileChooserAction action, gchar * pcFilename) {
    GtkWidget * pDlgFile = NULL;
    gchar * pcNewFilename = NULL;
    gint iResult = 0;
    gboolean bSelectName = FALSE;
    
    if (action == GTK_FILE_CHOOSER_ACTION_OPEN) {
        pDlgFile = gtk_file_chooser_dialog_new (_("Open File"), GTK_WINDOW(pWindow), GTK_FILE_CHOOSER_ACTION_OPEN, 
                    _("_Cancel"), GTK_RESPONSE_CANCEL, _("_Open"), GTK_RESPONSE_ACCEPT, NULL);
    } else if (action == GTK_FILE_CHOOSER_ACTION_SAVE) {
        pDlgFile = gtk_file_chooser_dialog_new (_("Save File"), GTK_WINDOW(pWindow), GTK_FILE_CHOOSER_ACTION_SAVE,
                    _("_Cancel"), GTK_RESPONSE_CANCEL, _("_Save"), GTK_RESPONSE_ACCEPT, NULL);
		gtk_file_chooser_set_create_folders(GTK_FILE_CHOOSER(pDlgFile), TRUE);
	}
    
    if (pDlgFile) {
        if (pcFilename) {
            gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(pDlgFile), pcFilename);
        } else if (action == GTK_FILE_CHOOSER_ACTION_SAVE) {
            gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(pDlgFile), _("New file"));
        }
        
		iResult = gtk_dialog_run (GTK_DIALOG (pDlgFile));
	    
        if (iResult == GTK_RESPONSE_ACCEPT) {
			pcNewFilename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(pDlgFile));
        }
        gtk_widget_destroy (pDlgFile);
    }
    
    return pcNewFilename;
}


void quickpad_clbk_btn_config (GtkMenuItem *menuitem, gpointer user_data) {
	QuickpadAppWindow * pWindow = QUICKPAD_APP_WINDOW(user_data);
	
	
}

void quickpad_clbk_btn_edit(GtkMenuItem *menuitem, gpointer user_data) {
	QuickpadTab * pTabInfo = user_data;
	
	gtk_entry_set_text(GTK_ENTRY(pTabInfo->pTxtLabel), gtk_label_get_label(GTK_LABEL(pTabInfo->pLabel)));
	gtk_widget_hide(pTabInfo->pHBoxShow);
	gtk_widget_show(pTabInfo->pHBoxEdit);
	gtk_widget_grab_focus(pTabInfo->pTxtLabel);
}

void quickpad_clbk_btn_save (GtkMenuItem *menuitem, gpointer user_data) {
	QuickpadTab * pTabInfo = user_data;
	
	gtk_label_set_label(GTK_LABEL(pTabInfo->pLabel), gtk_entry_get_text(GTK_ENTRY(pTabInfo->pTxtLabel)));
	quickpad_clbk_btn_cancel(menuitem, user_data);
}

void quickpad_clbk_btn_cancel (GtkMenuItem *menuitem, gpointer user_data) {
	QuickpadTab * pTabInfo = user_data;
	
	gtk_widget_hide(pTabInfo->pHBoxEdit);
	gtk_widget_show(pTabInfo->pHBoxShow);
}

void quickpad_clbk_btn_close (GtkMenuItem *menuitem, gpointer user_data) {
	QuickpadTab * pTabInfo = user_data;
	DConfClient * pDcnfClient;
	gchar * pcPath;
	gint iPos = -1;
	
	iPos = gtk_notebook_page_num(pTabInfo->pWindow->ntbContent, pTabInfo->pPageChild);
	g_print("quickpad_clbk_btn_close - pcTabId : '%s' - iPos : %d\n", pTabInfo->pcTabId, iPos);
	if (iPos >= 0) {
		gtk_notebook_remove_page(pTabInfo->pWindow->ntbContent, iPos);
		pDcnfClient = dconf_client_new();
		
		pcPath = g_strdup_printf("/net/thepozer/quickpad/tabs/%s/", pTabInfo->pcTabId);
		dconf_client_write_fast(pDcnfClient, pcPath, NULL, NULL);
                
		g_free(pcPath);
		g_free(pTabInfo);
	}
}

gboolean quickpad_clbk_entry_evt_keyrelease (GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
	QuickpadTab * pTabInfo = user_data;
	
	if (event->keyval == GDK_KEY_Return) {
		quickpad_clbk_btn_save(NULL, user_data);
		return TRUE;
	} else if (event->keyval == GDK_KEY_Escape) {
		quickpad_clbk_btn_cancel(NULL, user_data);
		return TRUE;
	}
	
	return FALSE;
}

void quickpad_clbk_reorder_page (GtkNotebook *ntbContent, GtkWidget *child, guint page_num, gpointer user_data) {
	QuickpadAppWindow * pWindow = QUICKPAD_APP_WINDOW(user_data);
	GtkWidget *pPageChild = NULL;
	QuickpadTab * pTabInfo = NULL;
	int iIdx, iSize;
	
	if (!pWindow->bInitTab) {
		iSize = gtk_notebook_get_n_pages(ntbContent);
		for (iIdx = 0; iIdx < iSize; iIdx ++) {
			pPageChild = gtk_notebook_get_nth_page(pWindow->ntbContent, iIdx);
			if (pPageChild != NULL) {
				pTabInfo = g_object_get_data(G_OBJECT(pPageChild), "tab_info");
				if (pTabInfo != NULL) {
					pTabInfo->iTabPos = iIdx;
					g_settings_set_int(pTabInfo->pTabSettings, "order", pTabInfo->iTabPos);
				}
			}
		}
	}
}

QuickpadTab * quickpad_app_window_add_tab(QuickpadAppWindow * pWindow, gchar * pcTabId, gchar * pcTitle, gchar * pcContent) {
	GtkWidget * pScrolled, * pHBox, * pBtnEdit, * pBtnClose, * pBtnSave, * pBtnCancel;
	GtkTextBuffer * pTextBuffer;
	GSettings * pSettings, * pTabSettings;
	QuickpadTab * pTabInfo = NULL;
	gchar * pcPath = NULL;
	gint iPos = -1;
	gboolean bNewId = (pcTabId == NULL);
	
	pTabInfo = g_new0(QuickpadTab, 1);
	pTabInfo->pWindow = pWindow;
	pTabInfo->iTabPos = -1;

	pScrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_show(pScrolled);
	gtk_widget_set_hexpand(pScrolled, TRUE);
	gtk_widget_set_vexpand(pScrolled, TRUE);

	pTabInfo->pTextView = gtk_source_view_new();
	gtk_text_view_set_monospace(GTK_TEXT_VIEW (pTabInfo->pTextView), TRUE);
	gtk_widget_show(GTK_WIDGET(pTabInfo->pTextView));
	gtk_container_add(GTK_CONTAINER(pScrolled), GTK_WIDGET(pTabInfo->pTextView));
	
	pTextBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pTabInfo->pTextView));
	
	pHBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_show(pHBox);
	
	pTabInfo->pHBoxShow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_show(pTabInfo->pHBoxShow);
	gtk_box_pack_start(GTK_BOX(pHBox), pTabInfo->pHBoxShow, TRUE, TRUE, 0);

	pBtnEdit = gtk_button_new_from_icon_name("gtk-edit", GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_button_set_relief(GTK_BUTTON(pBtnEdit), GTK_RELIEF_NONE);
	gtk_widget_show(pBtnEdit);
	gtk_box_pack_start(GTK_BOX(pTabInfo->pHBoxShow), pBtnEdit, FALSE, FALSE, 0);
	
	pTabInfo->pLabel = gtk_label_new("");
	gtk_widget_show(pTabInfo->pLabel);
	gtk_box_pack_start(GTK_BOX(pTabInfo->pHBoxShow), pTabInfo->pLabel, TRUE, TRUE, 0);
	
	pTabInfo->pHBoxEdit = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_hide(pTabInfo->pHBoxEdit);
	gtk_box_pack_start(GTK_BOX(pHBox), pTabInfo->pHBoxEdit, TRUE, TRUE, 0);

	pTabInfo->pTxtLabel = gtk_entry_new();
	gtk_widget_show(pTabInfo->pTxtLabel);
	gtk_box_pack_start(GTK_BOX(pTabInfo->pHBoxEdit), pTabInfo->pTxtLabel, TRUE, TRUE, 0);

	pBtnSave = gtk_button_new_from_icon_name("gtk-apply", GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_button_set_relief(GTK_BUTTON(pBtnSave), GTK_RELIEF_NONE);
	gtk_widget_show(pBtnSave);
	gtk_box_pack_start(GTK_BOX(pTabInfo->pHBoxEdit), pBtnSave, FALSE, FALSE, 0);
	
	pBtnCancel = gtk_button_new_from_icon_name("gtk-cancel", GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_button_set_relief(GTK_BUTTON(pBtnCancel), GTK_RELIEF_NONE);
	gtk_widget_show(pBtnCancel);
	gtk_box_pack_start(GTK_BOX(pTabInfo->pHBoxEdit), pBtnCancel, FALSE, FALSE, 0);

	pBtnClose = gtk_button_new_from_icon_name("window-close", GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_button_set_relief(GTK_BUTTON(pBtnClose), GTK_RELIEF_NONE);
	gtk_widget_show(pBtnClose);
	gtk_box_pack_start(GTK_BOX(pHBox), pBtnClose, FALSE, FALSE, 0);
	
	pTabInfo->iTabPos = gtk_notebook_append_page(pWindow->ntbContent, pScrolled, pHBox);
	pTabInfo->pPageChild = gtk_notebook_get_nth_page(pWindow->ntbContent, pTabInfo->iTabPos);
	gtk_notebook_set_tab_reorderable(pWindow->ntbContent, pTabInfo->pPageChild, TRUE);
	
	if (bNewId) {
		pcTabId = g_strdup_printf("%u", pWindow->iTabCounter);
		g_object_set(pWindow, "tab-counter", pWindow->iTabCounter + 1, NULL);
	} else {
		pcTabId = g_strdup(pcTabId);
	}
	
	pTabInfo->pcTabId = pcTabId;
	
	g_signal_connect(pBtnEdit,   "clicked", G_CALLBACK(quickpad_clbk_btn_edit),   pTabInfo);
	g_signal_connect(pBtnSave,   "clicked", G_CALLBACK(quickpad_clbk_btn_save),   pTabInfo);
	g_signal_connect(pBtnCancel, "clicked", G_CALLBACK(quickpad_clbk_btn_cancel), pTabInfo);
	g_signal_connect(pBtnClose,  "clicked", G_CALLBACK(quickpad_clbk_btn_close),  pTabInfo);
	g_signal_connect(pTabInfo->pTxtLabel, "key-release-event",  G_CALLBACK(quickpad_clbk_entry_evt_keyrelease), pTabInfo);
	g_object_set_data(G_OBJECT(pTabInfo->pPageChild), "tab_info", pTabInfo);
	
	pcPath = g_strdup_printf("/net/thepozer/quickpad/tabs/%s/", pcTabId);
	pTabInfo->pTabSettings = g_settings_new_with_path("net.thepozer.quickpad.tab", pcPath);

	g_settings_bind(pTabInfo->pTabSettings, "title",   pTabInfo->pLabel,     "label",     G_SETTINGS_BIND_DEFAULT);
	g_settings_bind(pTabInfo->pTabSettings, "content", pTextBuffer,          "text",      G_SETTINGS_BIND_DEFAULT);
	//g_settings_bind(pTabInfo->pTabSettings, "order",   pTabInfo->pPageChild, "position",  G_SETTINGS_BIND_DEFAULT);

	pSettings = quickpad_app_get_settings(QUICKPAD_APP(gtk_window_get_application(GTK_WINDOW(pWindow))));
	
	g_settings_bind (pSettings, "line-numbers", pTabInfo->pTextView, "show-line-numbers", G_SETTINGS_BIND_DEFAULT);
	g_settings_bind (pSettings, "highlight-current-line", pTabInfo->pTextView, "highlight-current-line", G_SETTINGS_BIND_DEFAULT);

	if (bNewId) {
		g_settings_set_string(pTabInfo->pTabSettings, "id",      pcTabId);
		g_settings_set_string(pTabInfo->pTabSettings, "title",   pcTitle);
		g_settings_set_string(pTabInfo->pTabSettings, "content", pcContent);
	}
	
	gtk_notebook_set_current_page(pWindow->ntbContent, pTabInfo->iTabPos);
	
	g_free(pcPath);
	
	return pTabInfo;
}

#include <dconf.h>

#include "quickpad_window.h"


struct _QuickpadAppWindow {
	GtkApplicationWindow parent;
	
	GtkWidget * btnTlbrNew;
	GtkWidget * btnTlbrImport;
	GtkWidget * btnTlbrExport;
	
	GtkNotebook * ntbContent;
	
	guint iTabCounter;
	GHashTable * pHTabsIds;
};

typedef struct _QuickpadTab {
	QuickpadAppWindow *pWindow;
	GtkWidget * pPageChild;
	GtkWidget * pHBoxShow;
	GtkWidget * pHBoxEdit;
	GtkWidget * pLabel;
	GtkWidget * pTxtLabel;
	gchar * pcTabId;
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

void quickpad_app_window_add_tab(QuickpadAppWindow * pWindow, gchar * pcTabId, gchar * pcTitle, gchar * pcContent);
void quickpad_app_window_update_tab_list (QuickpadAppWindow * pWindow);

void quickpad_clbk_btn_edit(GtkMenuItem *menuitem, gpointer user_data);
void quickpad_clbk_btn_save(GtkMenuItem *menuitem, gpointer user_data);
void quickpad_clbk_btn_cancel(GtkMenuItem *menuitem, gpointer user_data);
void quickpad_clbk_btn_close(GtkMenuItem *menuitem, gpointer user_data);
gboolean quickpad_clbk_entry_evt_keyrelease (GtkWidget *widget, GdkEventKey *event, gpointer user_data);

static void quickpad_app_window_init (QuickpadAppWindow *pWindow) {
	gtk_widget_init_template(GTK_WIDGET(pWindow));
	
	pWindow->pHTabsIds = g_hash_table_new(g_str_hash, g_str_equal);
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

	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(pClass), QuickpadAppWindow, ntbContent);
}

gboolean quickpad_clbk_delete_event (GtkWidget * widget, GdkEvent * event, gpointer user_data) {
	QuickpadAppWindow * pWindow = QUICKPAD_APP_WINDOW(widget);
	
	gtk_widget_destroy(widget);
	
	return TRUE;
}

QuickpadAppWindow * quickpad_app_window_new (QuickpadApp * pApp) {
	QuickpadAppWindow * pWindow = g_object_new (QUICKPAD_TYPE_APP_WINDOW, "application", pApp, NULL);
	GSettings * pSettings = quickpad_app_get_settings(pApp);
	
	g_settings_bind(pSettings, "tab-counter", pWindow, "tab-counter", G_SETTINGS_BIND_DEFAULT);
	
	g_signal_connect(pWindow, "delete-event", G_CALLBACK(quickpad_clbk_delete_event), pWindow);
	
	DConfClient * pDcnfClient;
	gchar ** pArElements, * pcTabId;
	gint iNbElements, iIdx;
	
	pDcnfClient = dconf_client_new();
	pArElements = dconf_client_list(pDcnfClient, "/net/thepozer/quickpad/tabs/", &iNbElements);
	for (iIdx = 0; iIdx < iNbElements; iIdx ++) {
		pcTabId = g_strndup(pArElements[iIdx], strlen(pArElements[iIdx]) - 1);
		g_print ("name tabs : %s\n", pcTabId);
		g_hash_table_add(pWindow->pHTabsIds, pcTabId);
		quickpad_app_window_add_tab(pWindow, pcTabId, NULL, NULL);
	}

	return pWindow;
}

void quickpad_clbk_btn_new (GtkMenuItem *menuitem, gpointer user_data) {
	QuickpadAppWindow * pWindow = QUICKPAD_APP_WINDOW(user_data);
	
	quickpad_app_window_add_tab(pWindow, NULL, _("New pad"), "");
}

void quickpad_clbk_btn_import (GtkMenuItem *menuitem, gpointer user_data) {
	QuickpadAppWindow * pWindow = QUICKPAD_APP_WINDOW(user_data);
	
	
}

void quickpad_clbk_btn_export (GtkMenuItem *menuitem, gpointer user_data) {
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
		if (g_hash_table_remove(pTabInfo->pWindow->pHTabsIds, pTabInfo->pcTabId)) {

			pDcnfClient = dconf_client_new();

			pcPath = g_strdup_printf("/net/thepozer/quickpad/tabs/%s/", pTabInfo->pcTabId);
			dconf_client_write_fast(pDcnfClient, pcPath, NULL, NULL);
			g_free(pcPath);
			
			g_free(pTabInfo);
		} else {
			g_printerr("quickpad_clbk_btn_close - pcTabId : '%s' - Not found in HashTable ... \n", pTabInfo->pcTabId);
		}
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

void quickpad_app_window_add_tab(QuickpadAppWindow * pWindow, gchar * pcTabId, gchar * pcTitle, gchar * pcContent) {
	GtkWidget * pScrolled, * pTextView, * pHBox, * pBtnEdit, * pBtnClose, * pBtnSave, * pBtnCancel;
	GtkTextBuffer * pTextBuffer;
	GSettings * pTabSettings;
	QuickpadTab * pTabInfo = NULL;
	gchar * pcPath = NULL;
	gint iPos = -1;
	gboolean bNewId = (pcTabId == NULL);
	
	pTabInfo = g_new0(QuickpadTab, 1);
	pTabInfo->pWindow = pWindow;

	pScrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_show(pScrolled);
	gtk_widget_set_hexpand(pScrolled, TRUE);
	gtk_widget_set_vexpand(pScrolled, TRUE);

	pTextView = gtk_text_view_new();
	gtk_text_view_set_monospace(GTK_TEXT_VIEW (pTextView), TRUE);
	gtk_widget_show(GTK_WIDGET(pTextView));
	gtk_container_add(GTK_CONTAINER(pScrolled), GTK_WIDGET(pTextView));
	
	pTextBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pTextView));
	
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
	
	iPos = gtk_notebook_append_page(pWindow->ntbContent, pScrolled, pHBox);
	pTabInfo->pPageChild = gtk_notebook_get_nth_page(pWindow->ntbContent, iPos);
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
	g_object_set_data(G_OBJECT(pTabInfo->pPageChild), "tab_id", pcTabId);
	
	pcPath = g_strdup_printf("/net/thepozer/quickpad/tabs/%s/", pcTabId);
	pTabSettings = g_settings_new_with_path("net.thepozer.quickpad.tab", pcPath);

	g_settings_bind(pTabSettings, "title",   pTabInfo->pLabel, "label", G_SETTINGS_BIND_DEFAULT);
	g_settings_bind(pTabSettings, "content", pTextBuffer,      "text",  G_SETTINGS_BIND_DEFAULT);

	if (bNewId) {
		g_hash_table_add(pWindow->pHTabsIds, pcTabId);
		
		g_settings_set_string(pTabSettings, "id",      pcTabId);
		g_settings_set_string(pTabSettings, "title",   pcTitle);
		g_settings_set_string(pTabSettings, "content", pcContent);
	}
	
	gtk_notebook_set_current_page(pWindow->ntbContent, iPos);
	
	g_free(pcPath);
}

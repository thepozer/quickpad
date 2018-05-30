#include "quickpad_window.h"

struct _QuickpadAppWindow {
	GtkApplicationWindow parent;
	
	GtkWidget * btnTlbrNew;
	GtkWidget * btnTlbrImport;
	GtkWidget * btnTlbrExport;
	
	GtkNotebook * ntbContent;
	
	guint iTabCounter;
	GPtrArray * pPaTabsIds;
};

typedef struct _QuickpadTab {
	QuickpadAppWindow *pWindow;
	GtkWidget * pPageChild;
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

static void quickpad_app_window_init (QuickpadAppWindow *pWindow) {
	gtk_widget_init_template(GTK_WIDGET(pWindow));
	
	pWindow->pPaTabsIds = g_ptr_array_new();
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

	GVariant * pvTabs;
	GVariantIter * pTabsIter;
	gchar * pcTabId = NULL;
	
	pvTabs = g_settings_get_value(pSettings, "tabs");
	
	g_variant_get (pvTabs, "as", &pTabsIter);
	while (g_variant_iter_loop (pTabsIter, "s", &pcTabId)) {
		g_print ("name : %s\n", pcTabId);
		g_ptr_array_add(pWindow->pPaTabsIds, g_strdup(pcTabId));
		quickpad_app_window_add_tab(pWindow, pcTabId, NULL, NULL);
	}
	
	g_variant_iter_free (pTabsIter);
		
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

void quickpad_clbk_btn_close (GtkMenuItem *menuitem, gpointer user_data) {
	QuickpadTab * pTabInfo = user_data;
	gint iPos = -1;
	
	iPos = gtk_notebook_page_num(pTabInfo->pWindow->ntbContent, pTabInfo->pPageChild);
	g_print("quickpad_clbk_btn_close - pcTabId : '%s' - iPos : %d\n", pTabInfo->pcTabId, iPos);
	if (iPos >= 0) {
		gtk_notebook_remove_page(pTabInfo->pWindow->ntbContent, iPos);
		if (g_ptr_array_remove(pTabInfo->pWindow->pPaTabsIds, pTabInfo->pcTabId)) {
			quickpad_app_window_update_tab_list(pTabInfo->pWindow);
		
			g_free(pTabInfo);
		} else {
			g_printerr("quickpad_clbk_btn_close - pcTabId : '%s' - Not found in PtrArray ... \n", pTabInfo->pcTabId);
		}
	}
}

void quickpad_app_window_add_tab(QuickpadAppWindow * pWindow, gchar * pcTabId, gchar * pcTitle, gchar * pcContent) {
	GtkWidget * pScrolled, * pTextView, * pPageChild, * pHBox, * pLabel, * pBtnClose;
	GtkTextBuffer * pTextBuffer;
	GSettings * pTabSettings;
	QuickpadTab * pTabInfo = NULL;
	gchar * pcPath = NULL;
	gint iPos = -1;
	gboolean bNewId = (pcTabId == NULL);
	
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
	
	pLabel = gtk_label_new("");
	gtk_widget_show(pLabel);
	gtk_box_pack_start(GTK_BOX(pHBox), pLabel, TRUE, TRUE, 0);
	
	pBtnClose = gtk_button_new_from_icon_name("window-close", GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_button_set_relief(GTK_BUTTON(pBtnClose), GTK_RELIEF_NONE);
	gtk_widget_show(pBtnClose);
	gtk_box_pack_start(GTK_BOX(pHBox), pBtnClose, FALSE, FALSE, 0);
	
	iPos = gtk_notebook_append_page(pWindow->ntbContent, pScrolled, pHBox);
	pPageChild = gtk_notebook_get_nth_page(pWindow->ntbContent, iPos);
	gtk_notebook_set_tab_reorderable(pWindow->ntbContent, pPageChild, TRUE);
	
	if (bNewId) {
		pcTabId = g_strdup_printf("%u", pWindow->iTabCounter);
		g_object_set(pWindow, "tab-counter", pWindow->iTabCounter + 1, NULL);
	} else {
		pcTabId = g_strdup(pcTabId);
	}
	
	pTabInfo = g_new0(QuickpadTab, 1);
	pTabInfo->pWindow = pWindow;
	pTabInfo->pPageChild = pPageChild;
	pTabInfo->pcTabId = pcTabId;
	
	g_signal_connect(pBtnClose, "clicked", G_CALLBACK(quickpad_clbk_btn_close), pTabInfo);
	g_object_set_data(G_OBJECT(pPageChild), "tab_id", pcTabId);
	
	pcPath = g_strdup_printf("/net/thepozer/quickpad/tabs/%s/", pcTabId);
	pTabSettings = g_settings_new_with_path("net.thepozer.quickpad.tab", pcPath);

	g_settings_bind(pTabSettings, "title",   pLabel,      "label", G_SETTINGS_BIND_DEFAULT);
	g_settings_bind(pTabSettings, "content", pTextBuffer, "text",  G_SETTINGS_BIND_DEFAULT);

	if (bNewId) {
		g_ptr_array_add(pWindow->pPaTabsIds, pcTabId);
		quickpad_app_window_update_tab_list(pWindow);
		
		g_settings_set_string(pTabSettings, "id",      pcTabId);
		g_settings_set_string(pTabSettings, "title",   pcTitle);
		g_settings_set_string(pTabSettings, "content", pcContent);
	}
	
	gtk_notebook_set_current_page(pWindow->ntbContent, iPos);
	
	g_free(pcPath);
}

void quickpad_app_window_update_tab_list (QuickpadAppWindow * pWindow) {
	QuickpadApp * pApp = QUICKPAD_APP(gtk_window_get_application(GTK_WINDOW(pWindow)));
	GSettings * pSettings = quickpad_app_get_settings(pApp);
	GVariantBuilder * pvTabsBuilder = NULL;
	GVariant * pvTabs = NULL;
	
	pvTabsBuilder = g_variant_builder_new (G_VARIANT_TYPE ("as"));
	for(gint iIdx = 0; pWindow->pPaTabsIds->len > iIdx; iIdx++) {
		g_print("quickpad_app_window_update_tab_list - TabId : '%s' - iIdx : %d\n", g_ptr_array_index(pWindow->pPaTabsIds, iIdx), iIdx);
		g_variant_builder_add(pvTabsBuilder, "s", g_ptr_array_index(pWindow->pPaTabsIds, iIdx));
	}
	
	pvTabs = g_variant_builder_end(pvTabsBuilder);
	g_settings_set_value(pSettings, "tabs", pvTabs);
	
	g_variant_unref(pvTabs);
	g_variant_builder_unref(pvTabsBuilder);
}

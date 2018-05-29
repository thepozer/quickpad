#include "quickpad_window.h"

struct _QuickpadAppWindow {
	GtkApplicationWindow parent;
	
	GtkWidget * btnTlbrNew;
	GtkWidget * btnTlbrImport;
	GtkWidget * btnTlbrExport;
	
	GtkNotebook * ntbContent;
	
	guint iTabCounter;
	GVariantBuilder * pvTabsBuilder;
};

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

static void quickpad_app_window_init (QuickpadAppWindow *pWindow) {
	gtk_widget_init_template(GTK_WIDGET(pWindow));
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

void quickpad_add_tab(QuickpadAppWindow * pWindow, gchar * pcTabId, gchar * pcTitle, gchar * pcContent) {
	GSettings * pTabSettings;
	GtkTextBuffer * pTextBuffer;
	GtkWidget * pScrolled, * pTextView, * pLabel, * pPageChild;
	gchar * pcPath = NULL;
	gint iPos = -1;
	
	pScrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_show(pScrolled);
	gtk_widget_set_hexpand(pScrolled, TRUE);
	gtk_widget_set_vexpand(pScrolled, TRUE);

	pTextView = gtk_text_view_new();
	gtk_text_view_set_monospace(GTK_TEXT_VIEW (pTextView), TRUE);
	gtk_widget_show(GTK_WIDGET(pTextView));
	gtk_container_add(GTK_CONTAINER(pScrolled), GTK_WIDGET(pTextView));
	
	pTextBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pTextView));
	
	pLabel = gtk_label_new("");
	gtk_widget_show(pLabel);
	
	iPos = gtk_notebook_append_page(pWindow->ntbContent, pScrolled, pLabel);
	
	pPageChild = gtk_notebook_get_nth_page(pWindow->ntbContent, iPos);
	
	if (pcTabId) {
		pcPath = g_strdup_printf("/net/thepozer/quickpad/tabs/%s/", pcTabId);
	} else {
		pcPath = g_strdup_printf("/net/thepozer/quickpad/tabs/%u/", pWindow->iTabCounter);
	}
	
	pTabSettings = g_settings_new_with_path("net.thepozer.quickpad.tab", pcPath);
	g_settings_bind(pTabSettings, "title",   pLabel,      "label", G_SETTINGS_BIND_DEFAULT);
	g_settings_bind(pTabSettings, "content", pTextBuffer, "text",  G_SETTINGS_BIND_DEFAULT);

	if (!pcTabId) {
		GVariant * pvTabs;
		QuickpadApp * pApp = QUICKPAD_APP(gtk_window_get_application(GTK_WINDOW(pWindow)));
		GSettings * pSettings = quickpad_app_get_settings(pApp);
		
		g_variant_builder_add(pWindow->pvTabsBuilder, "s", g_strdup_printf("%u", pWindow->iTabCounter));
		pvTabs = g_variant_new ("as", pWindow->pvTabsBuilder);
		g_settings_set_value(pSettings, "tabs", pvTabs);
		g_variant_unref(pvTabs);
		
		g_settings_set_uint(pTabSettings, "id", pWindow->iTabCounter);
		g_settings_set_string(pTabSettings, "title",   pcTitle);
		g_settings_set_string(pTabSettings, "content", pcContent);
		
		g_object_set(pWindow, "tab-counter", pWindow->iTabCounter + 1, NULL);
	}
	
	g_free(pcPath);
}

QuickpadAppWindow * quickpad_app_window_new (QuickpadApp * pApp) {
	QuickpadAppWindow * pWindow = g_object_new (QUICKPAD_TYPE_APP_WINDOW, "application", pApp, NULL);
	GSettings * pSettings = quickpad_app_get_settings(pApp);
	
	g_settings_bind(pSettings, "tab-counter", pWindow, "tab-counter", G_SETTINGS_BIND_DEFAULT);
	
	g_signal_connect(pWindow, "delete-event", G_CALLBACK(quickpad_clbk_delete_event), pWindow);

	GVariant * pvTabs;
	GVariantIter * pTabsIter;
	GSettings * pTabSettings;
	gchar * pcTabId = NULL;
	
	pWindow->pvTabsBuilder = g_variant_builder_new (G_VARIANT_TYPE ("as"));
	
/*
	g_variant_builder_add (builder, "s", "when");
	value = g_variant_new ("as", builder);
	g_variant_builder_unref (builder);
*/
	
	pvTabs = g_settings_get_value(pSettings, "tabs");
	
	g_variant_get (pvTabs, "as", &pTabsIter);
	while (g_variant_iter_loop (pTabsIter, "s", &pcTabId)) {
		g_print ("name : %s\n", pcTabId);
		g_variant_builder_add(pWindow->pvTabsBuilder, "s", pcTabId);
		quickpad_add_tab(pWindow, pcTabId, NULL, NULL);
	}
	g_variant_iter_free (pTabsIter);
		
	return pWindow;
}

void quickpad_clbk_btn_new (GtkMenuItem *menuitem, gpointer user_data) {
	QuickpadAppWindow * pWindow = QUICKPAD_APP_WINDOW(user_data);
	
	quickpad_add_tab(pWindow, NULL, _("New pad"), "");
}

void quickpad_clbk_btn_import (GtkMenuItem *menuitem, gpointer user_data) {
	QuickpadAppWindow * pWindow = QUICKPAD_APP_WINDOW(user_data);
	
	
}

void quickpad_clbk_btn_export (GtkMenuItem *menuitem, gpointer user_data) {
	QuickpadAppWindow * pWindow = QUICKPAD_APP_WINDOW(user_data);
	
	
}

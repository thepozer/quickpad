#include "quickpad_window.h"

struct _QuickpadAppWindow {
	GtkApplicationWindow parent;

	GtkNotebook * ntbContent;
};

G_DEFINE_TYPE(QuickpadAppWindow, quickpad_app_window, GTK_TYPE_APPLICATION_WINDOW);

static void quickpad_app_window_init (QuickpadAppWindow *pWindow) {
	gtk_widget_init_template(GTK_WIDGET(pWindow));
}

static void quickpad_app_window_class_init (QuickpadAppWindowClass *pClass) {
	gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(pClass), "/net/thepozer/quickpad/quickpad.wnd-main.glade");
	
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(pClass), QuickpadAppWindow, ntbContent);
}

gboolean smpldt_clbk_delete_event (GtkWidget * widget, GdkEvent * event, gpointer user_data) {
	QuickpadAppWindow * pWindow = QUICKPAD_APP_WINDOW(widget);
	
	gtk_widget_destroy(widget);
	
	return TRUE;
}

QuickpadAppWindow * quickpad_app_window_new (QuickpadApp *pApp) {
	QuickpadAppWindow * pWindow = g_object_new (QUICKPAD_TYPE_APP_WINDOW, "application", pApp, NULL);
	
//	pWindow->pEditData = NULL;
	
	g_signal_connect(pWindow, "delete-event", G_CALLBACK(smpldt_clbk_delete_event), pWindow);
	
	return pWindow;
}

void quickpad_clbk_notebook_switch_page (GtkNotebook * ntbContent, GtkWidget * pChild, guint page_num, gpointer user_data) {
	QuickpadAppWindow * pWindow = QUICKPAD_APP_WINDOW(user_data);
	
/*
	pTmpEditData = g_object_get_data(G_OBJECT(pChild), "content_data");
	if (pTmpEditData != NULL) {
		pWindow->pEditData = pTmpEditData;
	}
*/	
}

void quickpad_add_tab(QuickpadAppWindow * pWindow, gchar * pcTitle, gchar * pcContent) {
	GtkWidget * pScrolled, * pTextView, * pLabel, * pPageChild;
	gint iPos = -1;
	
	pScrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_show(pScrolled);
	gtk_widget_set_hexpand(pScrolled, TRUE);
	gtk_widget_set_vexpand(pScrolled, TRUE);

	pTextView = gtk_text_view_new();
	gtk_text_view_set_monospace(GTK_TEXT_VIEW (pTextView), TRUE);
	gtk_widget_show(GTK_WIDGET(pTextView));
	gtk_container_add(GTK_CONTAINER(pScrolled), GTK_WIDGET(pTextView));
	
	pLabel = gtk_label_new((pcTitle != NULL) ? pcTitle : _("New pad"));
	gtk_widget_show(pLabel);
	
	iPos = gtk_notebook_append_page(pWindow->ntbContent, pScrolled, pLabel);
	
	pPageChild = gtk_notebook_get_nth_page(pWindow->ntbContent, iPos);

}

void quickpad_clbk_btn_new (GtkMenuItem *menuitem, gpointer user_data) {
	QuickpadAppWindow * pWindow = QUICKPAD_APP_WINDOW(user_data);
	
	quickpad_add_tab(pWindow, NULL, NULL);
}

void quickpad_clbk_btn_import (GtkMenuItem *menuitem, gpointer user_data) {
	QuickpadAppWindow * pWindow = QUICKPAD_APP_WINDOW(user_data);
	
	
}

void quickpad_clbk_btn_export (GtkMenuItem *menuitem, gpointer user_data) {
	QuickpadAppWindow * pWindow = QUICKPAD_APP_WINDOW(user_data);
	
	
}

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



#include "quickpad_app.h"

struct _QuickpadApp {
	GtkApplication parent;
  
	GSettings * pSettings;
};

G_DEFINE_TYPE(QuickpadApp, quickpad_app, GTK_TYPE_APPLICATION);

static void quickpad_app_init (QuickpadApp * pApp) {}

static void quickpad_app_activate (GApplication * pApp) {

}

static void quickpad_app_class_init(QuickpadAppClass *class)
{
	G_APPLICATION_CLASS (class)->activate = quickpad_app_activate;
}

QuickpadApp * quickpad_app_new (void) {
	QuickpadApp * pApp = NULL;
	
	pApp = g_object_new (QUICKPAD_TYPE_APP, "application-id", "net.thepozer.quickpad", "flags", G_APPLICATION_HANDLES_OPEN, NULL);
			
	pApp->pSettings = g_settings_new ("net.thepozer.quickpad");
	
	return pApp;
}

GSettings * quickpad_app_get_settings (QuickpadApp * pApp) {
	return pApp->pSettings;
}

void quickpad_app_quit (QuickpadApp * pApp) {
	
	g_application_quit(G_APPLICATION(pApp));
}

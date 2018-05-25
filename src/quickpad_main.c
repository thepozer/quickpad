#define __QUICKPAD_MAIN_PART__
#include "quickpad_main.h"

#include "quickpad_app.h"

int main (int argc, char *argv []) {
	/* Set up internationalization */
	setlocale (LC_ALL, "");
	bindtextdomain (PACKAGE, LOCALEDIR);
	textdomain (PACKAGE);

	g_resources_register(quickpad_get_resource());
	
	return g_application_run(G_APPLICATION(quickpad_app_new()), argc, argv);
}

#ifndef __QUICKPAD_APP_H__
#define __QUICKPAD_APP_H__

#include "quickpad_main.h"


#define QUICKPAD_TYPE_APP (quickpad_app_get_type ())
G_DECLARE_FINAL_TYPE (QuickpadApp, quickpad_app, QUICKPAD, APP, GtkApplication)

QuickpadApp * quickpad_app_new (void);
void          quickpad_app_quit (QuickpadApp * pApp);

GSettings *   quickpad_app_get_settings (QuickpadApp * pApp);

#endif /* __QUICKPAD_APP_H__ */


#ifndef __QUICKPAD_WINDOW_H__
#define __QUICKPAD_WINDOW_H__

#include <string.h>

#include "quickpad_main.h"
#include "quickpad_app.h"

#define QUICKPAD_TYPE_APP_WINDOW (quickpad_app_window_get_type ())
G_DECLARE_FINAL_TYPE (QuickpadAppWindow, quickpad_app_window, QUICKPAD, APP_WINDOW, GtkApplicationWindow)


QuickpadAppWindow * quickpad_app_window_new       (QuickpadApp * pApp);


#endif /* __QUICKPAD_WINDOW_H__ */

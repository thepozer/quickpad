AM_CFLAGS = $(QUICKPAD_CFLAGS)
AM_CPPFLAGS = -DLOCALEDIR=\""$(localedir)"\"
bin_PROGRAMS = quickpad

quickpad_LDADD = $(QUICKPAD_LIBS)
quickpad_SOURCES = quickpad_main.c \
quickpad_app.c \
quickpad.gresources.h \
quickpad.gresources.c


quickpad.gresources.c: quickpad.gresources.xml quickpad.wnd-main.glade
	$(AM_V_GEN) glib-compile-resources --generate-source $<

quickpad.gresources.h: quickpad.gresources.xml quickpad.wnd-main.glade
	$(AM_V_GEN) glib-compile-resources --generate-header $<

gsettings_SCHEMAS = net.thepozer.quickpad.gschema.xml
EXTRA_DIST = $(gsettings_SCHEMAS)

@GSETTINGS_RULES@


AC_INIT([quickpad], [0.1.0], [Didier prolhac <dev@thepozer.net>])
AC_CONFIG_SRCDIR([src/quickpad_main.c])
AM_INIT_AUTOMAKE([-Wall foreign])
AM_SILENT_RULES([yes])

AC_PROG_CC
PKG_PROG_PKG_CONFIG	
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.18.1])
IT_PROG_INTLTOOL([0.40])

GLIB_GSETTINGS

PKG_CHECK_MODULES([QUICKPAD], [glib-2.0 gtk+-3.0 gtksourceview-3.0], , exit)

# Needed by intltool
GETTEXT_PACKAGE=${PACKAGE_TARNAME}
AC_SUBST([GETTEXT_PACKAGE])


AC_CONFIG_FILES([
    Makefile
    src/Makefile
    data/Makefile
    po/Makefile.in
])
AC_OUTPUT

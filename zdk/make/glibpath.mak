GLIB_PREFIX=/usr
GTK_PREFIX=/usr
SIGC_PREFIX=/usr
GTKMM_PREFIX=/usr

ifneq ($(shell ls /usr/local/include/gtk-- 2>/dev/null),)
 GTKMM_PREFIX=/usr/local
 SIGC_PREFIX=/usr/local
else
 #
 # GTKMM_2=1
 # configure should pickup the correct location
 #
endif

bin_PROGRAMS = pacfinder

pacfinder_SOURCES = \
	aboutdialog.c \
	aboutdialog.h \
	database.c \
	database.h \
	interface.c \
	interface.h \
	main.c \
	main.h \
	util.c \
	util.h \
	window.c \
	window.h

pacfinder_CFLAGS = \
	-Wall \
	-Wextra \
	-Wno-unused-parameter \
	-DLOCALEDIR=\"$(localedir)\" \
	@GTK_CFLAGS@

pacfinder_LDADD = \
	@GTK_LIBS@

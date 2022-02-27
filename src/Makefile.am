bin_PROGRAMS = pacfinder

BUILT_SOURCES = \
	pacfinder-resources.c

pacfinder_SOURCES = \
	aboutdialog.c \
	aboutdialog.h \
	database.c \
	database.h \
	interface.c \
	interface.h \
	main.c \
	main.h \
	settings.c \
	settings.h \
	util.c \
	util.h \
	window.c \
	window.h \
	$(BUILT_SOURCES)

pacfinder_CFLAGS = \
	-Wall \
	-Wextra \
	-Wno-unused-parameter \
	-DLOCALEDIR=\"$(localedir)\" \
	@GTK_CFLAGS@

pacfinder_LDADD = \
	@GTK_LIBS@

resource_dir = ${abs_top_builddir}/data/resources

pacfinder-resources.c:
	$(GLIB_COMPILE_RESOURCES) \
		--sourcedir=$(resource_dir) \
		--generate-source \
		--target=$@ \
		$(resource_dir)/com.stevenbenner.pacfinder.gresource.xml

clean-local:
	$(RM) -f $(BUILT_SOURCES)

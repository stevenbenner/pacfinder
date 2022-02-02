gsettings_SCHEMAS = com.stevenbenner.pacfinder.gschema.xml

@GSETTINGS_RULES@

EXTRA_DIST = \
	$(gsettings_SCHEMAS)

CLEANFILES = \
	gschemas.compiled

gschemas.compiled: $(gsettings_SCHEMAS) Makefile
	$(AM_V_GEN) $(GLIB_COMPILE_SCHEMAS) $(builddir)

all-local: gschemas.compiled

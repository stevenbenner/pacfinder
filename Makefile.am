SUBDIRS = \
	data \
	src

if USE_NLS
	SUBDIRS += po
endif

DIST_SUBDIRS = $(SUBDIRS)

EXTRA_DIST = \
	autogen.sh

ACLOCAL_AMFLAGS = --install -I m4

.PHONY: AUTHORS
AUTHORS:
	$(AM_V_GEN) if test -d "$(top_srcdir)/.git"; then \
		git -C "$(top_srcdir)/.git" log --format='%aN <%aE>' | sort | uniq > $@; \
	fi

dist-hook: AUTHORS

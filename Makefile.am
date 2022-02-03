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

.PHONY: ChangeLog
ChangeLog:
	$(AM_V_GEN) if test -d "$(top_srcdir)/.git"; then \
		git -C "$(top_srcdir)/.git" log --date=short --format='%cd %an:%n%x09* %s' | \
			awk '/^[^\t]/{if(previous==$$0)next;previous=$$0}{print}' | \
			sed '/^[^\t]/s/\([^ ]*\) \(.*\)/\n\1\n\2/' > $@; \
	fi

dist-hook: AUTHORS ChangeLog

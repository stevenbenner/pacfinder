check_PROGRAMS = test_suite

test_suite_SOURCES = \
	$(top_srcdir)/src/util.c \
	$(top_srcdir)/src/util.h \
	main.c \
	test_util.c \
	test_util.h

test_suite_CPPFLAGS = \
	-I$(top_srcdir)/src

test_suite_CFLAGS = \
	-Wall \
	-Wextra \
	-Wno-unused-parameter \
	-DLOCALEDIR=\"$(localedir)\" \
	@GTK_CFLAGS@

test_suite_LDADD = \
	@GTK_LIBS@

TEST_LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) $(top_srcdir)/build-aux/tap-driver.sh

TESTS = test_suite

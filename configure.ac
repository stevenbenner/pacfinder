#
# Copyright 2022 Steven Benner
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

dnl Initialize autoconf
AC_COPYRIGHT([Copyright 2022 Steven Benner])
AC_PREREQ([2.71])
AC_INIT(
	[PacFinder],
	[1.0],
	[https://github.com/stevenbenner/pacfinder/issues],
	[pacfinder],
	[https://github.com/stevenbenner/pacfinder]
)
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_FILES([
	data/desktop/Makefile
	data/gsettings/Makefile
	data/icons/scalable/Makefile
	data/icons/Makefile
	data/resources/Makefile
	data/Makefile
	doc/Makefile
	po/Makefile.in
	src/Makefile
	test/Makefile
	Makefile
])

dnl Check programs
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_SED

dnl Check types
AC_TYPE_OFF_T
AC_TYPE_SIZE_T

dnl Check headers
AC_CHECK_HEADERS([unistd.h])

dnl Check functions
AC_CHECK_FUNCS([setlocale])

dnl Check libraries
AC_CHECK_LIB([alpm], [alpm_version], , AC_MSG_ERROR([pacman is required to compile pacfinder]))

dnl Check modules
PKG_CHECK_MODULES([GLIB],[glib-2.0 >= 2.56])
PKG_CHECK_MODULES([GTK], [gtk+-3.0 >= 3.22.0])
PKG_CHECK_MODULES([LIBALPM], [libalpm >= 13.0.0])

dnl Test suite
AC_REQUIRE_AUX_FILE([tap-driver.sh])

dnl Initialize automake
AM_INIT_AUTOMAKE([dist-zstd no-dist-gzip subdir-objects])
AM_SILENT_RULES([yes])

dnl GSettings
AX_REQUIRE_DEFINED([GLIB_GSETTINGS])
GLIB_GSETTINGS

dnl GResource
GLIB_COMPILE_RESOURCES=`$PKG_CONFIG --variable glib_compile_resources gio-2.0`
AC_SUBST(GLIB_COMPILE_RESOURCES)

dnl Generate LINGUAS file
rm -f $srcdir/po/LINGUAS
for po_file in `ls $srcdir/po/*.po | sort`; do
	lang=`echo "$po_file" | ${SED} "s|.*/po/\(.*\)\.po|\1|g"`
	echo $lang >> $srcdir/po/LINGUAS
done

dnl Internationalization
AM_GNU_GETTEXT([external], [need-ngettext])
AM_GNU_GETTEXT_VERSION([0.21])
AM_CONDITIONAL([USE_NLS], [test "x${USE_NLS}" = "xyes"])
AC_SUBST([GETTEXT_PACKAGE], [$PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$PACKAGE"], [Gettext package.])

dnl Generate output
AC_OUTPUT

dnl Configuration summary
echo "
  ${PACKAGE_NAME} - version ${PACKAGE_VERSION}

  Build information:
    source code dir : ${srcdir}
    prefix          : ${prefix}
    use NLS         : ${USE_NLS}
    compiler        : ${CC}
    compiler flags  : ${CFLAGS}

  Install paths:
    binaries        : $(eval echo $(eval echo ${bindir}))
    data            : $(eval echo $(eval echo ${datadir}))
      desktop       : $(eval echo $(eval echo ${datadir}))/applications
      schemas       : $(eval echo $(eval echo ${gsettingsschemadir}))
      icons         : $(eval echo $(eval echo ${datadir}))/icons/hicolor
      locales       : $(eval echo $(eval echo ${localedir}))
      man pages     : $(eval echo $(eval echo ${mandir}))
"

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
	[0.1],
	[https://github.com/stevenbenner/pacfinder/issues],
	[pacfinder],
	[https://github.com/stevenbenner/pacfinder]
)
AC_PREFIX_DEFAULT([/usr]) dnl Prefer /usr over /usr/local on Arch Linux
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_FILES([
	src/Makefile
	Makefile
])
AC_PROG_CC

dnl Check libraries
AC_CHECK_LIB([alpm], [alpm_version], , AC_MSG_ERROR([pacman is required to compile pacfinder]))

dnl Check modules
PKG_CHECK_MODULES([GLIB],[glib-2.0 >= 2.56])
PKG_CHECK_MODULES([GTK], [gtk+-3.0 >= 3.22.0])
PKG_CHECK_MODULES([LIBALPM], [libalpm >= 13.0.0])

dnl Initialize automake
AM_INIT_AUTOMAKE
AM_SILENT_RULES([yes])

dnl Generate output
AC_OUTPUT

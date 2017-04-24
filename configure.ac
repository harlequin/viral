# Copyright (c) 2017 harlequin
# https://github.com/harlequin/viral
#
# This file is part of viral.
#
# viral is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([viral], [0.1], [test@test.com])
AC_CONFIG_AUX_DIR(posix)
AC_CANONICAL_TARGET
AC_CONFIG_SRCDIR([src/viral.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([subdir-objects foreign])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h limits.h memory.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h sys/param.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_FSEEKO
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([dup2 gethostbyname gettimeofday inet_ntoa memchr memmove memset mkdir rmdir select socket strcasecmp strchr strcspn strdup strerror strrchr strspn strstr strtol strtoul])

AC_CANONICAL_HOST
case $host_os in
        mingw*)
                LIBS="-lws2_32"
        ;;
        *)
        ;;
esac




AC_MSG_CHECKING([whether to use ssl])
enableSSL=no
AC_ARG_ENABLE(ssl,
   [  --enable-ssl		Enable SSL support)],
   [SSL="$enableval"]
   enableSSL=yes,
   [SSL="no"]
)
AC_MSG_RESULT([$SSL])
AC_SUBST([SSL])


dnl
dnl Check for libs
dnl
AC_SEARCH_LIBS([pthread_create], [pthread])

dnl
dnl check for OpenSSL-SSL library
dnl

if test "$SSL" = "yes"; then
   AC_CHECKING([for OpenSSL SSL Library and Header files])
   AC_CHECK_HEADER(openssl/ssl.h,, [AC_MSG_ERROR([OpenSSL SSL headers not found.])])	
   AC_CHECK_LIB(ssl, SSL_accept, , [AC_MSG_ERROR([sipgrep requires but cannot find ssl])])
   AC_CHECK_LIB([crypto],[CRYPTO_new_ex_data], [], [AC_MSG_ERROR([OpenSSL libraries required])])

   AC_DEFINE(USE_SSL, 1, [Use OpenSSL SSL library])
   AC_DEFINE(MG_ENABLE_SSL, 1, [Use OpenSSL SSL library])
fi



echo
echo $PACKAGE $VERSION
echo
echo Installation prefix......... : $prefix
echo SSL/TLS..................... : $enableSSL
echo


AC_CONFIG_FILES([Makefile])
AC_OUTPUT

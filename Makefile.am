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

bin_PROGRAMS = viral

viral_SOURCES = \
	src/downloads.c\
	src/http_client.c \
	src/log.c \
	src/networks.c \
	src/options.c \
	src/portable.c \
	src/postprocess.c \
	src/rpc.c \
	src/sig.c \
	src/utils.c \
	src/viral.c \
	lib/frozen/frozen.c \
	lib/slre/slre.c \
	lib/mongoose/mongoose.c

webui_FILES = \
	webui/favicon.ico \
	webui/favicon-32x32.png \
	webui/index.html \
	webui/css/bootstrap.min.css \
	webui/css/bootstrap-theme.css \
	webui/css/bootstrap-theme.min.css \
	webui/css/jquery.dynatable.css \
	webui/css/viral.css \
	webui/fonts/glyphicons-halflings-regular.eot \
	webui/fonts/glyphicons-halflings-regular.svg \
	webui/fonts/glyphicons-halflings-regular.ttf \
	webui/fonts/glyphicons-halflings-regular.woff \
	webui/fonts/glyphicons-halflings-regular.woff2 \
	webui/js/bootstrap.min.js \
	webui/js/downloads.js \
	webui/js/index.js \
	webui/js/jquery.dynatable.js \
	webui/js/jquery.min.js \
	webui/js/logs.js \
	webui/js/rpc.js \
	webui/js/search.js \
	webui/js/series.js

exampleconf_FILES = \
	viral.conf
	

# INSTALL
exampleconfdir = $(sysconfdir)
dist_exampleconf_DATA = $(exampleconf_FILES)
webuidir = $(datadir)/viral
nobase_dist_webui_DATA = $(webui_FILES)
#sysconfdir
# Install configuration files into /etc
# (only if they do not exist there to prevent override by update)
#install-conf:
#	$(mkinstalldirs) "$(DESTDIR)$(sysconfdir)" ; \
#	cp "$(DESTDIR)$(exampleconfdir)/viral.conf" "$(DESTDIR)$(sysconfdir)/viral.conf" ;


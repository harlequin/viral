/**
 * Copyright (c) 2015 harlequin
 * https://github.com/harlequin/viral
 *
 * This file is part of viral.
 *
 * viral is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef NETWORKS_H_
#define NETWORKS_H_

#include "../lib/mongoose/mongoose.h"
#include "portable.h"

#define IRC_CALLBACK(funcname)			void funcname (irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
#define IRC_NUMERIC_CALLBACK(funcname)	void funcname (irc_session_t * session, unsigned int event, const char * origin, const char ** params, unsigned int count)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)  (sizeof(x) / sizeof(x[0]))
#endif

void networks_init ( struct mg_mgr *mgr );
void networks_shutdown ( void );
void network_connect ( struct mg_mgr *mgr, irc_server_t *server );
void network_disconnect ( irc_server_t *server );

#endif /* NETWORKS_H_ */

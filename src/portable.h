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


#ifndef PORTABLE_H_
#define PORTABLE_H_

#include <stdarg.h>

#ifdef WIN32
	int vasprintf( char **sptr, char *fmt, va_list argv );
	int asprintf( char **sptr, char *fmt, ... );
	//int vsscanf(const char *s, const char *fmt, va_list ap);

#include <process.h>
#include <winsock.h>
#include <windows.h>

	#define sleep(x) Sleep((x) *1000)
	#define CREATE_THREAD(id,func,param)	(CreateThread(0, 0, func, param, 0, (PDWORD) id) == 0)
	#define THREAD_FUNCTION(funcname)		DWORD WINAPI funcname (LPVOID arg)
	#define thread_id_t 	DWORD
	
	#ifndef sleep
	#define sleep(x) Sleep((x) *1000)
	#endif

	//#define mkdir(x,y) _mkdir(x)
	#define S_DIRMODE 0
	
#else
	#define CREATE_THREAD(id,func,param)	(pthread_create (id, 0, func, (void *) param) != 0)
	#define THREAD_FUNCTION(funcname)		void * funcname (void * arg)
	#define thread_id_t		pthread_t

	#include <error.h>

	#define S_DIRMODE (S_IRWXU | S_IRWXG | S_IRWXO)
#endif
#include "downloads.h"




typedef enum {
	IRC_SERVER_STATE_IDLE = 0,
	IRC_SERVER_STATE_CONNECTING = 1,
	IRC_SERVER_STATE_CONNECTED = 2
} irc_server_state_e;

typedef struct irc_channel_s {
	char *name;
	uint8_t joined;
	struct irc_channel_s *next;
} irc_channel_t;

typedef struct irc_server_s{
	int id;
	char *host;
	irc_channel_t *channels;
	uint16_t port;
	uint8_t encryption;
	irc_download_t *item;
	irc_server_state_e state;
	int connections_errors;
	struct mg_connection *nc;
	struct irc_server_s *next;
} irc_server_t;



#endif /* PORTABLE_H_ */

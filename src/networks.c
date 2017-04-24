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

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include <stdio.h>
#include <inttypes.h>
#include <errno.h>

#include "networks.h"
#include "log.h"
#include "portable.h"
#include "options.h"
#include "postprocess.h"
#include "utlist.h"
#include "../lib/mongoose/mongoose.h"
#include "../lib/slre/slre.h"
#include "viral.h"

static uint32_t own_addr;

void dump_irc(struct mg_connection *nc, struct mg_str * event, struct mg_str * origin, struct mg_str *params, unsigned int count);

void foundip(irc_server_t *server, char *ip) {
	LOG(E_DEBUG, "*** Found your IP: %s\n", ip);
	struct hostent *addr;
	addr = gethostbyname(ip);
	if ( addr ) {
		own_addr = ((struct in_addr *) addr->h_addr)->s_addr;
		memset(ip_address, 0, 255);
		sprintf(ip_address, "%s", inet_ntoa (*((struct in_addr *) addr->h_addr)));
		LOG(E_DEBUG, "*** Resolved IP: %s\n", ip_address);
	}
}

int strip_irc_colors(struct mg_str *data ) {
	const char *p;
	char  *d = 0;
	int len = 0;

	d = malloc( data->len );
	memset(d, 0, data->len);

	for ( p = data->p; p != data->p + data->len; p++ ) {
		switch (*p) {
		case 0x02:
		case 0x1F:
		case 0x16:
		case 0x0F:
			continue;
		case 0x03:
			if ( isdigit(p[1]) ) {
				p++;
				if ( isdigit(p[1]) ) {
					p++;
				}
				if ( p[1] == ',' && isdigit (p[2]) ) {
					p += 2;
					if ( isdigit (p[1]) ) {
						p++;
					}
				}
				continue;
			}
			//break;
			continue;
		default:
			d[len++] = *p;
		}
	}

	data->p = d;
	data->len = len;

	return len;
}

void numeric_handler(struct mg_connection *nc, int code, struct mg_str * origin, struct mg_str *params, unsigned int count) {
	if ( code == 401 ) {
		char *bot = malloc(params[1].len);
		sprintf(bot, "%.*s", params[1].len, params[1].p);
		irc_download_t *d = download_search_by_bot(bot);
		if ( d ) {
			if ( mg_vcasecmp(&params[2], "No such nick/channel") == 0) {
				d->status = DOWNLOAD_ERROR;
				d->error = 3;
				//d->error_text = malloc(params[2].len + 1);
				//strncpy(d->error_text, params[2].p, params[2].len);
				//d->error_text[params[2].len] = '\0';
				downloads_queue_save();
			}
		}
	}
}


void notice_handler(struct mg_connection *nc, struct mg_str * event, struct mg_str * origin, struct mg_str *params, unsigned int count) {
	int i;
	int res;
	struct slre_cap cap[6] = {};

	irc_server_t* server = (irc_server_t*) nc->user_data;

	strip_irc_colors(&params[1]);

	char *regexes[] = {
		"(?i)\\*\\* All Slots Full, Added you to the main queue for pack ([0-9]*) \\(\"([^\"]*)\"\\) in position ([\\d]*)",
		"(?i)Queued ([\\S]*) for \"([^\"]*)\", in position ([\\d]*) of ([\\d]*). ([\\S]*) or less remaining. \\(at ([0-9:]*)\\)",
		"(?i)\\*\\* All Slots Full, Denied, You already have that item queued."
	};

	char *error_regexes[] = {
		"\\*\\* XDCC SEND denied, you must have voice on a known channel to request a pack",
		"\\*\\* You already requested that pack",
		"XDCC SEND denied, you must be on a known channel to request a pack",
		"\\*\\* Invalid Pack Number"
	};

	for(i = 0; i < ARRAY_SIZE(regexes); i++) {
		res = slre_match(regexes[i], params[1].p, params[1].len, cap, ARRAY_SIZE(cap), SLRE_IGNORE_CASE);
		if (res >= 0 ) {
			server->item->status = DOWNLOAD_QUEUED;
			return;
		}
	}

	for(i = 0; i < ARRAY_SIZE(error_regexes); i++) {
		res = slre_match(error_regexes[i], params[1].p, params[1].len, cap, ARRAY_SIZE(cap), SLRE_IGNORE_CASE);
		if (res >= 0 ) {
			/* So my item is in queue */
			/* all values can be queried when len > 0 */
			/* LOG(E_DEBUG, "%.*s\n", cap[0].len, cap[0].ptr ); */
			server->item->status = DOWNLOAD_ERROR;
			server->item->error= 2;
			//sprintf(server->item->error_text, "You already requested that pack");


			return;
			//strcpy(server->item->message,params[1]);
		}
	}
}

void privmsg_handler(struct mg_connection *nc, struct mg_str * event, struct mg_str * origin, struct mg_str *params, unsigned int count) {
	struct slre_cap cap[1];
	int res;
	char channel[255] = {0};
	int i;

	strip_irc_colors(&params[1]);


	if ( count > 1 ) {

		char *queue_dcc[] = {
			"You can only have 1 transfer at a time, Added you to the main queue for pack \\S+ in position (\\d+)"
		};

		char *error_dcc[] = {
			"Closing Connection: DCC Timeout ([\\S]+ Sec Timeout)",
			"DCC Send Denied, I don't accept transfers from [\\S]+",
			"You already requested that pack",
			"XDCC SEND denied, you must be on a known channel to request a pack"
		};

		char *bot = malloc(origin->len);
		sprintf(bot, "%.*s", origin->len, origin->p);
		irc_download_t *d = download_search_by_bot(bot);
		if ( d ) {

			for ( i = 0; i < ARRAY_SIZE(error_dcc); i++) {
				res = slre_match(error_dcc[i], params[1].p, params[1].len, cap, 1, SLRE_IGNORE_CASE);
				if ( res < 0 ) {
					continue;
				} else {
					d->status = DOWNLOAD_ERROR;
					//d->error_text = malloc(params[1].len);
					//strncpy(d->error_text, params[1].p, params[1].len - 1);

					d->error = 99;

					downloads_queue_save();
					return;
				}
			}


			for (i = 0; i < ARRAY_SIZE(queue_dcc); i++) {
				res = slre_match(queue_dcc[i], params[1].p, params[1].len, cap, 1, SLRE_IGNORE_CASE);
				if ( res > -1 ) {
					d->status = DOWNLOAD_QUEUED;
					//d->error_text = malloc(params[1].len);
					//sprintf(d->error_text, "Queue position %.*s", cap[0].len, cap[0].ptr);

					d->error = 1;

					downloads_queue_save();
					return;
				}
			}
		}
	}



	char *force_join_regexes[] = {
		"(?i)Du Musst Auch Den Raum ([\\S]+) Betreten",
		"(?i)You must join ([\\S]+) in order to remain downloading in this channel.",
		"(?i)You Must /JOIN ([\\S]+) As Well To Download",
		"(?i)Hello! /join ([\\S]+) for help"
	};

	//channel = malloc(255);
	for(i = 0; i < ARRAY_SIZE(force_join_regexes); i++) {
		res = slre_match(force_join_regexes[i], params[1].p /*notice*/, params[1].len /*strlen(notice)*/, cap, 1, SLRE_IGNORE_CASE);
		if(res < 0) {
			continue;
		} else {
			sprintf(channel, "%.*s", cap[0].len, cap[0].ptr );
			LOG(E_INFO, "==> Somebody told us to join channel %s\n", channel);
			mg_printf(nc, "JOIN %s\r\n", channel);
			return;
		}
	}
}

static void dcc_handler(struct mg_connection *nc, int ev, void *p) {
	struct mbuf *io = &nc->recv_mbuf;
	struct irc_download *session = (struct irc_download *)nc->user_data;

	switch (ev) {
		case MG_EV_POLL:
			if ( io->len > 0 ) {

				if ( session->start_time == 0 ) {
					session->start_time = time(0);
				}

				session->current_size += io->len;
				fwrite(io->buf, io->len, 1, session->fp);
				mbuf_remove(io, io->len);
				unsigned long g = htonl( (unsigned long) session->current_size );
				//mg_printf(nc, "%d", (unsigned char*)&g);
				mg_printf(nc, "%u", (unsigned int)&g);
				LOG(E_DEBUG, "*** current size %u\n",(unsigned int)&g);
			}
			if ( session->current_size >= session->total_size ) {
				nc->flags |=  MG_F_CLOSE_IMMEDIATELY;
			}
			break;
		case MG_EV_CONNECT:
			LOG(E_DEBUG, "*** DCC Transfer established\n");
			session->status = DOWNLOAD_DOWNLOADING;
			break;
		case MG_EV_RECV:

			if ( session->start_time == 0 ) {
				session->start_time = time(0);
			}

			session->current_size = session->current_size + nc->recv_mbuf.len;
			fwrite(nc->recv_mbuf.buf, nc->recv_mbuf.len, 1, session->fp);
			mbuf_remove(&nc->recv_mbuf, nc->recv_mbuf.len);
			break;
		case MG_EV_CLOSE:

			fclose(session->fp);

			if ( session->current_size >= session->total_size  ) {
				LOG(E_INFO, "DCC Transfer completed\n");
				session->status = DOWNLOAD_POSTPROCESSING;
				LOG(E_INFO, "Starting post process for file %s\n", session->name);
				/*This blocks everything */
				/*Make in another thread */
				thread_id_t id;
				if (CREATE_THREAD(&id, &postprocess, session)){

				}
				//postprocess(session->name,0);
			} else {
				LOG(E_WARN, "DCC Transfer closed with error\n");
				LOG(E_DEBUG, "%s - %"INT64_FMT"/%"INT64_FMT" (%d)\n",session->file_location, session->total_size, session->current_size, nc->recv_mbuf.len);
				session->status = DOWNLOAD_ERROR;
				downloads_queue_save();
			}

			break;
		default:
			LOG(E_DEBUG, ">DCC>EV: %d\n", ev);
			break;
	}
}

/*TODO: Rework dcc_request method for cleaner code */

#define	UC(b)	(((int)b) & 0xff)

void dcc_request(struct mg_connection *nc, struct mg_str * event, struct mg_str * origin, struct mg_str *params, unsigned int count) {
	struct mg_str command, action, filename, address, port, size, token;

	/* This is the item from the download queue */
	irc_server_t* server = (irc_server_t*) nc->user_data;
	irc_download_t* dcc_session = (irc_download_t*) server->item;
	struct mg_connection *c;
	//char *connect_address;
	char b[18];
	char *sz;
	int passive = 0;
	int i;
	const char *e = params[1].p + params[1].len, *s;


	for(i = 0; i < count; i++) {
		LOG(E_DEBUG, "*** Param %i: %.*s\n", i, params[i].len, params[i].p);
	}



	s = mg_skip(params[1].p++, e, " ", &command);
	s = mg_skip(s++, e, " ", &action);


	if ( mg_vcasecmp(&command , "\1VERSION\1") == 0) {
		mg_printf(nc, "NOTICE %.*s :\x01%s\x01\r\n", origin->len, origin->p, "VERSION viral by harlequin ver. alpha");
		return;
	} else if ( mg_vcasecmp(&action , "ACCEPT") == 0) {

		c = mg_connect( nc->mgr , dcc_session->remote_address, dcc_handler);
		c->user_data = dcc_session;
		if ( dcc_session->ssl ) {
#ifdef USE_SSL
			//mg_set_ssl(c, NULL, NULL);
#else
			LOG(E_FATAL, "SSL not support ... abort now\n");
#endif
		}

		if (c == NULL) {
			LOG(E_DEBUG, "Unable to establish a dcc transfer to %s\n", dcc_session->remote_address);
		}

		return;
	} else {}


	s = mg_skip(s++, e, " ", &filename);
	s = mg_skip(s++, e, " ", &address);
	s = mg_skip(s++, e, " ", &port);
	if ( mg_ncasecmp(port.p, "0", 1) == 0) {
		/* Reverse DCC detected */
		/* now we have to find the size and token */
		s = mg_skip(s++, e, " ", &size);
		s = mg_skip(s++, e, "\001", &token);
		passive = 1;
	} else {
		/* DCC conncetion */
		/* Parse size */
		s = mg_skip(s++, e, "\001", &size);
	}



	uint64_t n = to64(address.p);
	char *p = (char *)&n;
	(void)snprintf(b, sizeof(b), "%d.%d.%d.%d", UC(p[3]), UC(p[2]), UC(p[1]), UC(p[0]));


	sprintf(dcc_session->remote_address, "%s:%.*s", b, port.len, port.p);


	sprintf(dcc_session->file_location, "%s/%.*s", viral_options._incoming_directory, filename.len, filename.p);

	sz = malloc(size.len);
	sprintf(sz, "%.*s",size.len, size.p);
	dcc_session->total_size = to64(size.p);
	dcc_session->current_size = 0;
	dcc_session->error = 0;
	dcc_session->start_time = 0;
	sprintf(dcc_session->name, "%.*s", filename.len, filename.p);

	cs_stat_t st;

	if ( !passive ) {

		int stat = mg_stat(dcc_session->file_location, &st);
		dcc_session->fp = fopen( dcc_session->file_location, "ab" );
		if ( stat == 0 ) {

			if ( st.st_size > 0 ) {
				if ( st.st_size >= dcc_session->total_size) {
					LOG(E_WARN, "*** DCC Transfer seems to be completed\n");
					mg_printf(nc, "PRIVMSG %.*s :xdcc remove\r\n", origin->len, origin->p);
					dcc_session->status = DOWNLOAD_POSTPROCESSING;
					downloads_queue_save();
					LOG(E_INFO, "Starting post process for file %s\n", dcc_session->name);
					thread_id_t id;
					if (CREATE_THREAD(&id, &postprocess, dcc_session->name)) {}
					return;
				}

				dcc_session->current_size = st.st_size;
				LOG(E_DEBUG, "*** Sending DCC resume request\n");
				mg_printf( nc, "PRIVMSG %.*s :\1DCC RESUME %s %.*s %" INT64_FMT "\1\r\n", origin->len, origin->p, dcc_session->name, port.len, port.p, st.st_size );
				return;
			}
		}
	}

	dcc_session->fp = fopen( dcc_session->file_location, "ab" );

	/* Save information of download into the queue */
	downloads_queue_save();


	if ( passive ) {


		if ( (c = mg_bind(nc->mgr, "15156", dcc_handler)) != NULL ) {
			dcc_session->ssl = 0;
			LOG(E_DEBUG, "PRIVMSG %.*s :\1DCC SEND %s %u %s %" INT64_FMT " %.*s\1\r\n",
					origin->len, origin->p,
					dcc_session->name,
					own_addr /* ip_address*/,
					"15156",
					dcc_session->total_size,
					token.len, token.p);
			mg_printf( nc, "PRIVMSG %.*s :\1DCC SEND %s %u %s %" INT64_FMT " %.*s\1\r\n",
					origin->len, origin->p,
					dcc_session->name,
					own_addr /* ip_address*/,
					"15156",
					dcc_session->total_size,
					token.len, token.p);
		} else {
			LOG(E_ERROR, "*** Can't bind to port 15156\n");
		}

		//OPEN LISTENING
		//DCC SEND <filename> <ip> <port> <filesize> <token>
		//LOG(E_DEBUG, "*** Passive transfer not supported!\n");
		//dcc_session->status = DOWNLOAD_ERROR;
		//downloads_queue_save();
		//return;
	} else {
		LOG(E_DEBUG, "*** Connecting to %s\n", dcc_session->remote_address);
		LOG(E_DEBUG, "*** Download title %s\n", dcc_session->name);
		LOG(E_DEBUG, "*** Download bot %s\n", dcc_session->bot);
		c = mg_connect( nc->mgr , dcc_session->remote_address, dcc_handler);
		//mg_enable_multithreading(c);
		if(!c) {
			LOG(E_DEBUG, "Unable to establish a dcc transfer to %s\n", dcc_session->remote_address);
			return;
		}

/*TODO: Rework SSL conditionals */
//		int cmp_res = mg_vcasecmp(&action, "SSEND");
//		//if ( mg_ncasecmp(action.p, "SSEND", 5) == 0) {
//		if ( cmp_res == 0 ) {
//			dcc_session->ssl = 1;
//#ifdef VIRAL_ENABLE_SSL
//			mg_set_ssl(c, NULL, NULL);
//#else
//			LOG(E_FATAL, "SSL not support ... abort now\n");
//#endif
//		} else {
//			dcc_session->ssl = 0;
//		}
	}

	/*TODO: somehow a crash was happen here - has something to do with a dirty queue*/
	c->user_data = dcc_session;
	/*END CRASH*/

//	if (c == NULL) {
//		LOG(E_DEBUG, "Unable to establish a dcc transfer to %s\n", dcc_session->remote_address);
//	    return;
//	}
}

void dump_irc(struct mg_connection *nc, struct mg_str * event, struct mg_str * origin, struct mg_str *params, unsigned int count) {
	char buf[512];
	int cnt;
	buf[0] = '\0';
	for ( cnt = 0; cnt < count; cnt++ )	{
		if ( cnt )
			strcat (buf, "|");

		strip_irc_colors( &params[cnt] );

		sprintf(buf, "%s%.*s",buf, params[cnt].len, params[cnt].p);
	}
	LOG (E_DEBUG ,"[DUMP] Event \"%.*s\", origin: \"%.*s\", params: %d [%s]\n",event->len, event->p, origin->len, origin->p , cnt, buf);
}


/*********************** PUBLIC FUNCTIONS ************************************/
void irc_parser (struct mg_connection *nc, struct mg_str *d) {
	irc_server_t *session = (irc_server_t *) nc->user_data;
	const char *end = d->p + d->len;
	const char *s = d->p;
	const char *ip;

	#define MAX_PARAMS_ALLOWED 20
	int code = 0, paramindex = 0;
	struct mg_str prefix;
	struct mg_str command;
	struct mg_str params[MAX_PARAMS_ALLOWED+1];
	struct mg_str tmp;

	memset ((char *)params, 0, sizeof(params));
	ip = malloc(18);

	// Parse <prefix>
	if ( d->p[0] == ':' ) {
		s = mg_skip(++d->p, end, " ", &prefix );
		//strip nicks
		mg_skip(prefix.p, prefix.p + prefix.len, "!@", &prefix);
	}

	// Parse <command>
	s = mg_skip(s, end, " ", &command);
	if ( isdigit (command.p[0]) && isdigit (command.p[1]) && isdigit (command.p[2]) ) {
		code = atoi(command.p);
	}

	// Parse middle/params
	if ( s[0] != ':' ) {
		while( ((s = mg_skip(s, end, " \r\n",  &params[paramindex++])) != end) ) {
			if ( s[0] == ':' ) {
				/* found last param */
				break;
			}
		}
	}
	if ( s != end) {
		mg_skip(++s, end, "\r",  &params[paramindex++]);
	}

	/* Event handling */
	if ( code ) {
		if ( code == 4 /* code == 376 || code == 422 */) {
			/*TODO: throw connected event */
			/* We are connected */
			session->state = IRC_SERVER_STATE_CONNECTED;
			LOG(E_DEBUG, "*** Connected, joining all channels\n");
			irc_channel_t *channel = NULL, *tmp = NULL;
			LL_FOREACH_SAFE(session->channels, channel, tmp) {
				LOG(E_DEBUG, "*** Join channel %s\n", channel->name);
				mg_printf(nc, "JOIN %s\r\n", channel->name);
			}
			//list_foreach(session->channels, nc, join_channel_handler);
		} else if ( code == 1 ) {
			if ( paramindex > 1 && strlen(ip_address) == 0 ) {
				LOG(E_DEBUG, "*** 001 scan for ip address: `%.*s`\n", params[1].len, params[1].p);
				ip = mg_skip( params[1].p, params[1].p + params[1].len, "@", &tmp);
				mg_skip( ip, params[1].p + params[1].len, "\n", &tmp);
				memcpy(ip_address, tmp.p, tmp.len);
				foundip(session, ip_address);
			}
		}

		numeric_handler(nc, code, &prefix, params, paramindex );

		if ( code > 400 )
			dump_irc(nc, &command, &prefix, params, paramindex );
		/*TODO throw numeric event */
	} else if ( mg_vcasecmp( &command, "PING" ) == 0  ) {
		/* Reply directly to PING */
		mg_printf(nc, "PONG %.*s\r\n", params[0].len, params[0].p);
	} else if ( mg_vcasecmp( &command, "QUIT" ) == 0  ) {
	} else if ( mg_vcasecmp( &command, "JOIN" ) == 0  ) {
		if ( mg_vcasecmp(&prefix, viral_options._user_name) == 0) {

			irc_channel_t *channel = NULL, *tmp = NULL;

			LL_FOREACH_SAFE(session->channels, channel, tmp) {
				if( mg_vcasecmp(&params[0], channel->name) == 0) {
					channel->joined = 1;
					LOG(E_DEBUG, "Channel %s marked as connected\n", channel->name);
					break;
				}
			}

			//list_foreach(session->channels, &params[0], joined_channel_handler);
		}
	} else if ( mg_vcasecmp( &command, "PART") == 0 ) {
	} else if ( mg_vcasecmp( &command, "TOPIC") == 0 ) {
		dump_irc(nc, &command, &prefix, params, paramindex );
	} else if ( mg_vcasecmp( &command, "KICK") == 0 ) {
		dump_irc(nc, &command, &prefix, params, paramindex );
	} else if ( mg_vcasecmp( &command, "INVITE") == 0 ) {
		dump_irc(nc, &command, &prefix, params, paramindex );
	} else if ( mg_vcasecmp( &command, "KILL") == 0 ) {
		/* Ignore */
	} else if ( mg_vcasecmp( &command, "NICK") == 0 ) {
		dump_irc(nc, &command, &prefix, params, paramindex );
		/*TODO: has somebody changed our nick? */
	} else if ( mg_vcasecmp( &command, "MODE") == 0 ) {
		if ( mg_vcasecmp( &params[0], viral_options._user_name ) == 0) {
			dump_irc(nc, &command, &prefix, params, paramindex );
		}
	} else if ( mg_vcasecmp(&command, "NOTICE") == 0) {
		dump_irc(nc, &command, &prefix, params, paramindex );
		//LOG(E_DEBUG, "%.*s", d->len, d->p);
		/*TODO: Channel or real private ?*/
		/*TODO: DCC request ?*/
		if ( mg_vcasecmp( &params[0], viral_options._user_name ) == 0) {
			privmsg_handler(nc, &command, &prefix, params, paramindex );
		}


	} else if ( mg_vcasecmp(&command, "PRIVMSG") == 0) {
		/*TODO: Channel or real private ?*/
		/*TODO: DCC request ?*/
		if ( mg_vcasecmp( &params[0], viral_options._user_name ) == 0) {
			/* Private message */
			dump_irc(nc, &command, &prefix, params, paramindex );

			if ( params[1].p[0] == 0x01 && params[1].p[params[1].len - 1] == 0x01) {
				LOG(E_DEBUG, "Found a dcc request\n");
				dcc_request(nc, &command, &prefix, params, paramindex );
			} else {
				privmsg_handler(nc, &command, &prefix, params, paramindex );
			}



		} else {
			/* Channel message */
		}
	} else {
		/* Unknown */
		dump_irc(nc, &command, &prefix, params, paramindex );
	}
}

void irc_handler(struct mg_connection *nc, int ev, void *p) {
	struct mg_str f;
	struct mbuf *io = &nc->recv_mbuf;
	int len = 0;
	irc_server_t * server = (irc_server_t *) nc->user_data;

	switch (ev) {
		case MG_EV_POLL:
			break;
		case MG_EV_CONNECT:

			if ( (nc->flags & MG_F_CLOSE_IMMEDIATELY) == MG_F_CLOSE_IMMEDIATELY) {
				server->connections_errors++;
				server->state = IRC_SERVER_STATE_IDLE;
				LOG(E_ERROR, "Connection refused ... flags:0x%04x, error:0x%04x,\n ", nc->flags, (int) *(int*)p);
				break;
			}

			LOG(E_DEBUG, "Connection established\n");
			len = mg_printf(nc, "NICK %s\r\n\r\n", viral_options._user_name);
			len = mg_printf(nc, "USER %s 8 * :%s\r\n", viral_options._user_name, viral_options._user_name);
			server->connections_errors = 0;

			break;
		case MG_EV_RECV:
			/*TODO: not really god ... rework */
			while ( len <= io->len ) {
				mg_skip(io->buf, io->buf + io->len, "\n", &f);
				irc_parser(nc, &f);
				len += f.len + 2;
				mbuf_remove(io, f.len + 1);
			}
			break;
		case MG_EV_CLOSE:
			server->state = IRC_SERVER_STATE_IDLE;

			if ( nc->err != 0) {
				server->connections_errors++;
			}
			LOG(E_DEBUG, "Connection closed with flags:0x%04x (%d)\n", nc->flags, nc->err);
			break;
		default:
			break;
	}
}

void network_disconnect ( irc_server_t *server ) {
	irc_channel_t *channel = NULL, *tmp = NULL;
	
	LOG(E_INFO, "Disconnecting from server %s\n", server->host);
	
	LL_FOREACH_SAFE(server->channels, channel, tmp) {
		LOG(E_DEBUG, "*** Part channel %s\n", channel->name);
		mg_printf(server->nc, "PART %s Bye Bye\r\n", channel->name);
		channel->joined = 0;
	}
	
	server->nc->flags = MG_F_SEND_AND_CLOSE;
	server->state = IRC_SERVER_STATE_IDLE;
}

void network_connect ( struct mg_mgr *mgr, irc_server_t *server ) {
	struct mg_connection *nc;
	char *address;
	const char *ssl_result;

	address = malloc(255);
	sprintf(address, "%s:%d", server->host, server->port);
	LOG(E_INFO, "Starting network thread for %s\n", address);


	if ( server->encryption ) {
#ifndef USE_SSL
		LOG(E_ERROR, "ssl conection requested but not compiled!\n");
		return;
#endif
		LOG(E_INFO, "Connection is secured (ssl) ...\n");
		struct mg_connect_opts opts;
		memset(&opts, 0, sizeof(opts));
		opts.ssl_ca_cert = "*";
		nc = mg_connect_opt(mgr, address, irc_handler, opts);
	} else {
		nc = mg_connect(mgr, address, irc_handler);
	}

	if(!nc) {
		LOG(E_WARN, "Unable to connect to %s\n", address);
		return;
	}

	/* Add user data to nc */
	nc->user_data = server;
	server->nc = nc;
}

void networks_shutdown ( void ) {
	LOG(E_INFO, "Closing network connections\n");
}

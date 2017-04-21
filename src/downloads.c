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


#include <string.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdarg.h>

#include "../lib/mongoose/mongoose.h"

#include "log.h"
#include "downloads.h"
#include "options.h"
#include "portable.h"
#include "networks.h"
#include "utlist.h"

#define QUEUE_VERSION 1

irc_download_t *download_queue = NULL;
irc_download_t *history_queue = NULL;
uint8_t paused = 0;
int max_id = 0;

#ifndef max
#define max(a,b) \
  ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b; })
#endif

char *_strncpy(const char *data) {
	char *res;
	int size;

	if(data == NULL) {
		res = malloc(1);
		res[0] = '\0';
		return res;
	}

	size = strlen(data);

	res = malloc(size + 1);
	memcpy(res, data, size);
	res[size] = '\0';

	return res;
}

int _fscanf(FILE* infile, const char* Format, ...)
{
	char szLine[1024];
	if (!fgets(szLine, sizeof(szLine), infile))	{
		return 0;
	}

	LOG(E_DEBUG, " %s\n", szLine);

	va_list ap;
	va_start(ap, Format);
	int res = vsscanf(szLine, Format, ap);
	va_end(ap);

	return res;
}




//int download_compare (irc_download_t *a, irc_download_t *b) {
//	return strcmp(a->bot, b->bot);
//}

irc_download_t *download_search_by_bot(const char *name) {
	irc_download_t *res = NULL;
	LL_SEARCH_SCALAR_STR(download_queue, res, bot, name);
	return res;
}


irc_download_t *download_create(void) {
	irc_download_t *item;
	item = malloc(sizeof(irc_download_t));

	if(!item) {
		LOG(E_ERROR, "malloc fails to create a download item!");
		return NULL;
	}

	item->id = ++max_id;
	item->packet = 0;
	item->total_size = 0;
	item->current_size = 0;
	item->status = DOWNLOAD_READY;
	item->error = 0;

	item->server[0] = '\0';
	item->channel[0] = '\0';
	item->bot[0] = '\0';
	item->name[0] = '\0';

	return item;
}


void downloads_queue_load ( const char *file ) {
	FILE *f;
	char *p;
	int version;
	unsigned timestamp;
	int c;
	int i;
	irc_download_t *el;

	LOG(E_INFO, "Loading queue ...\n");

	p = malloc(strlen(viral_options._incoming_directory) + strlen(file) + 1);
	p = (char *) malloc(MG_MAX_PATH);
	if (!p) {
		LOG(E_FATAL, "Queue file can't be allocated!\n");
	}
	memset(p, 0, MG_MAX_PATH);
	sprintf(p, "%s/%s", viral_options._incoming_directory, file);

	f = fopen(p, "rb");

	if(!f){ return; }

	fscanf(f, "%d\n", &version);
	if(version != QUEUE_VERSION) {
		printf("VERSION ERROR\n");
		return;
	}
	fscanf(f, "%d\n", &c);
	fscanf(f, "%u\n", &timestamp);

	LOG(E_INFO, "Reading %d item from queue\n", c);

	for(i = 0; i < c; i++) {
		el = download_create();

		fscanf(f, "%d\n", &el->id);
		fscanf(f, "%s\n", el->server);
		fscanf(f, "%s\n", el->channel);
		fscanf(f, "%s\n", el->bot);
		fscanf(f, "%d\n", &el->packet);
		fscanf(f, "%s\n", el->name);
		fscanf(f, "%"INT64_FMT"\n", &el->total_size);
		fscanf(f, "%d\n", &el->status);
		fscanf(f, "%d\n", &el->error);

		max_id = max(max_id, el->id);
		LOG(E_DEBUG, "[*] %d %s %s %s %d\n",el->id,  el->server, el->channel, el->bot, el->packet);
		/* If status is downloading than reset it */
		if (el->status == DOWNLOAD_DOWNLOADING || el->status == DOWNLOAD_REQUESTED) {
			el->status = DOWNLOAD_READY;
		}
		LL_APPEND(download_queue, el);
	}

	if(p) {
		free(p);
	}
	fclose(f);
}

void downloads_queue_init( void ) {
	download_queue = NULL;
	history_queue = NULL;
	downloads_queue_load( "downloads");
	//downloads_queue_load( history_queue, "history");
}


inline int save_item(irc_download_t *el, void *param) {
	FILE *f = (FILE *) param;

	fprintf(f, "%d\n", el->id);
	fprintf(f, "%s\n", el->server);
	fprintf(f, "%s\n", el->channel);
	fprintf(f, "%s\n", el->bot);
	fprintf(f, "%d\n", el->packet);
	fprintf(f, "%s\n", el->name);
	fprintf(f, "%"INT64_FMT"\n", el->total_size);
	fprintf(f, "%d\n", el->status);
	fprintf(f, "%d\n", el->error);
	//fprintf(f, "%s\n", el->error_text);

	return 0;
}

void downloads_queue_save ( void ) {
	FILE *f;
	char *p;
	int count;
	irc_download_t *el, *tmp;
	p = malloc(strlen(viral_options._incoming_directory) + 11);
	sprintf(p, "%s/%s", viral_options._incoming_directory, "downloads");

	LL_COUNT(download_queue, el, count);

	f = fopen(p, "wb");
	fprintf(f, "%d\n", QUEUE_VERSION);
	fprintf(f, "%d\n", count);
	fprintf(f, "%u\n", (unsigned)time(NULL));
	LL_FOREACH_SAFE(download_queue, el, tmp) {
		save_item(el, f);
	}
	fclose(f);
}

void history_queue_save( void ) {
//	FILE *f;
//	char *p;
//
//	p = malloc(strlen(viral_options._incoming_directory) + 11);
//	sprintf(p, "%s/%s", viral_options._incoming_directory, "history");
//
//	f = fopen(p, "ab");
//	fprintf(f, "%d\n", QUEUE_VERSION);
//	fprintf(f, "%d\n", download_queue->count);
//	fprintf(f, "%u\n", (unsigned)time(NULL));
//	list_foreach( history_queue, f, save_item);
//	fclose(f);
}


void downloads_add ( const char *server, const char *channel, const char *bot, uint16_t packet, const char *name  ) {
	irc_download_t *item;
	item = download_create();

	sprintf(item->server,"%s", server);
	sprintf(item->channel,"%s", channel);
	sprintf(item->bot,"%s", bot);
	sprintf(item->name,"%s", name);

	//item->server = _strncpy(server);
	//item->channel = _strncpy(channel);
	//item->bot = _strncpy(bot);
	item->packet = packet;
	//item->name =  _strncpy(name);

	LL_APPEND(download_queue, item);
	downloads_queue_save();
}

int downloads_queue_handler( irc_download_t  *el, void *param ) {
	struct mg_mgr *mgr = (struct mg_mgr *) param;
	irc_channel_t *tmp = NULL;
	irc_channel_t *channel = NULL;
	irc_server_t *s = NULL;
	irc_download_t *look_up, *look_up_tmp;


	/* Another item is currently active skip looping*/
	if ( el->status == DOWNLOAD_COMPLETED) {
		/*TODO: Add item to history */
		LL_DELETE(download_queue, el);
		downloads_queue_save();
		return 1;
	}

	/* Download queue has pause state */
	if ( paused == 1) {
		return 1;
	}

	if (el->status > DOWNLOAD_READY && el->status < DOWNLOAD_DOWNLOADED) {
		return 1;
	}

	/* Start requesting */
	if ( el->status == DOWNLOAD_READY ) {

		LL_FOREACH_SAFE(download_queue, look_up, look_up_tmp ) {
			if ( look_up->status > DOWNLOAD_READY && look_up->status < DOWNLOAD_COMPLETED) {
				return 1;
			}
		}

		LL_SEARCH_SCALAR_STR(viral_options.servers, s,host, el->server);

		if( s != NULL ) {

			if ( s->state == IRC_SERVER_STATE_IDLE ) {
				LOG(E_INFO, "*** Connecting to network ...\n");

				if ( s->connections_errors >= 10 ) {
					LOG(E_ERROR, "Max count of connection errors, set download to invalid.\n");
					/*TODO: Set the complete server to invalid? */
					el->status = DOWNLOAD_ERROR;
					return 0;
				}

				s->state = IRC_SERVER_STATE_CONNECTING;
				network_connect( mgr, s);
				return 0;
			}

			LL_SEARCH_SCALAR_STR(s->channels, channel, name, el->channel);
			if ( channel ) {
				if (channel->joined == 1) {

					el->status = DOWNLOAD_REQUESTED;
					LOG(E_DEBUG, "Start requesting for download in channel %s\n", channel->name);
					s->item = el;

					LOG(E_DEBUG, "Sent message in %s to %s\n", s->host, el->bot);
					mg_printf(s->nc, "PRIVMSG %s :xdcc send #%d\r\n", el->bot,	el->packet);

				}
			} else {
				/* Channel not in list, add this automatically */
				tmp = malloc(sizeof(irc_channel_t *));
				tmp->name = malloc(strlen(el->channel));
				tmp->joined = 0;
				strcpy(tmp->name, el->channel);
				LL_APPEND(s->channels, tmp);
			}
		}
		return 0; /* was before 1 */
	}
	return 0;
}

void downloads_queue_poll( void *mgr ) {
	irc_download_t *e, *t;
	irc_server_t *server, *tmp_server;

	struct ns_mgr *m = ( struct ns_mgr *) mgr;


	if ( download_queue == NULL ) {
		/*Queue is empty, disconnect from all irc server*/
		LL_FOREACH_SAFE( viral_options.servers, server, tmp_server ) {
			if ( server->state == IRC_SERVER_STATE_CONNECTED ) {
				network_disconnect(server);
			}
		}
	} else {
		LL_FOREACH_SAFE(download_queue, e, t) {
			if ( downloads_queue_handler(e, m) == 1 ) {
				break;
			}
		}
	}
}

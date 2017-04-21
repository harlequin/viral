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


#ifndef DOWNLOADS_H_
#define DOWNLOADS_H_

#include <stdint.h>
#include <time.h>

typedef enum {
	DOWNLOAD_READY = 0,
	DOWNLOAD_REQUESTED,
	DOWNLOAD_QUEUED,
	DOWNLOAD_DOWNLOADING,
	DOWNLOAD_DOWNLOADED,		/* a new job can be activated */
	DOWNLOAD_POSTPROCESSING, /* a new job can be activated */
	DOWNLOAD_COMPLETED, 		/* a new job can be activated */
	DOWNLOAD_HISTORY, 		/* a new job can be activated */
	DOWNLOAD_PAUSED,			/* a new job can be activated */
	DOWNLOAD_ERROR
} e_download_status;


//#define __USE_MINGW_ANSI_STDIO 1 /* So mingw uses its printf not msvcrt */
#define _FILE_OFFSET_BITS 64

#include <stdint.h>   /* For uint64_t */
#include <inttypes.h> /* For PRIu64 */
#include <stdlib.h>   /* For exit status */

typedef struct irc_download {
	uint16_t id;
	char server[255];
	char channel[51];
	char bot[255];
	uint16_t packet;
	char name[255];
	uint64_t total_size;
	uint64_t current_size;
	e_download_status status;


	char file_location[255];
	char remote_address[255];
	int ssl;
	//char *error_text;

	uint16_t error;
	FILE *fp;

	unsigned int dccid;




	time_t start_time;

	struct irc_download *next;

} irc_download_t;


irc_download_t *download_create(void);
irc_download_t *download_search_by_bot(const char *bot);
irc_download_t *download_queue;
void downloads_queue_init( void );
void downloads_add ( const char *server, const char *channel, const char *bot, uint16_t packet, const char *name  );
void downloads_queue_save ( void );
void downloads_queue_poll( void *mgr );



#endif

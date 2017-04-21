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
#include <stdio.h>
#include "log.h"
#include "../lib/slre/slre.h"
#include "sig.h"
#include "downloads.h"
#include "utlist.h"
#include "../lib/mongoose/mongoose.h"

#include "../lib/frozen/frozen.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#endif

//char *jtoa ( struct json_token *tokens, const char *path ) {
//	char *result;
//	struct json_token *tmp;
//
//	tmp = find_json_token(tokens, path);
//	if (!tmp) {
//		LOG(E_ERROR, "Token not found\n");
//		return NULL;
//	}
//	result = malloc(tmp->len + 1);
//	sprintf(result, "%.*s", tmp->len, tmp->ptr);
//
//	return result;
//}

size_t viral_strlen(const char *s) {
	if (!s) {
		return 0;
	} else {
		return strlen(s);
	}

}

char *slre_error(int v) {
	switch (v) {
		case SLRE_NO_MATCH:
			return "No match";
		case SLRE_UNEXPECTED_QUANTIFIER:
			return "Unexpected quantifier";
		case SLRE_UNBALANCED_BRACKETS:
			return "UNBALANCED_BRACKETS";
		case SLRE_INTERNAL_ERROR:
			return "SLRE_INTERNAL_ERROR";
		case SLRE_INVALID_CHARACTER_SET:
			return "SLRE_INVALID_CHARACTER_SET";
		case SLRE_INVALID_METACHARACTER:
			return "SLRE_INVALID_METACHARACTER";
		case SLRE_CAPS_ARRAY_TOO_SMALL:
			return "SLRE_CAPS_ARRAY_TOO_SMALL";
		case SLRE_TOO_MANY_BRANCHES:
			return "SLRE_TOO_MANY_BRANCHES";
		case SLRE_TOO_MANY_BRACKETS:
			return "SLRE_TOO_MANY_BRACKETS";
		default:
			return "";
	}
}

void notice_parser (const char *notice, int len) {
	int i;
	int res;
	struct slre_cap cap[11] = {};

	LOG(E_DEBUG, "%s\n", notice);

	//const char *r = "^(?:[:](\S+) )?(\S+)(?: (?!:)(.+?))?(?: [:](.+))?$";
	const char *r = "^([:](\\S+) )?(\\S+)( ([^:]*)(.+?))?( [:](.+))?$";
	res = slre_match(r, notice, len, cap, ARRAY_SIZE(cap), SLRE_IGNORE_CASE);
	if(res <0) {
		LOG(E_DEBUG, "%s\n", slre_error(res));
	} else {
		for(i = 0; i < ARRAY_SIZE(cap); i++) {
			LOG(E_DEBUG, "Param %d: %.*s\n", i, cap[i].len, cap[i].ptr);
		}
	}


//	const char *regexes[] = {
//		"(?i)\\*\\* All Slots Full, Added you to the main queue for pack ([0-9]*) \\(\"([^\"]*)\"\\) in position ([\\d]*)",
//		"(?i)Queued ([\\S]*) for \"([^\"]*)\", in position ([\\d]*) of ([\\d]*). ([\\S]*) or less remaining. \\(at ([0-9:]*)\\)",
//		"(?i)\\*\\* All Slots Full, Denied, You already have that item queued."
//	};
//
//	const char *error_regexes[] = {
//		"\\*\\* XDCC SEND denied, you must have voice on a known channel to request a pack",
//		"\\*\\* You already requested that pack",
//		"XDCC SEND denied, you must be on a known channel to request a pack"
//	};


}




int parse_irc_msg ( char *data, irc_download_t *item ) {
	struct slre_cap cap[2];
	int res;
	char *tmp;
	res = slre_match("/msg (\\S+) xdcc send #(\\d+)", data, strlen(data), cap, 2, SLRE_IGNORE_CASE);

	//item->bot = malloc(cap[0].len);
	tmp = malloc(cap[1].len);

	sprintf(item->bot, "%.*s", cap[0].len, cap[0].ptr );
	sprintf(tmp, "%.*s", cap[1].len, cap[1].ptr );
	item->packet = atoi(tmp);

	/* When a negativ result was received -> error */
	if( res < 0 ) {
		return res;
	} else {
		return 0;
	}
}

/**
 * Add a new download to queue
 * Format: xdcc://irc.server.net/servername/#channel/bot/#0030/Test.avi/
 */
void add_xdcc_link ( char *uri ) {
	struct slre_cap cap[7] = {{0}};
	irc_download_t *item;
	int res;
	int i;

	LOG(E_DEBUG, "XDCC: [%s]\n", uri);

	res = slre_match("xdcc://([^/]+)/([^/]+)/([^/]+)/([^/]+)/#?([^/]+)/([^/]+)", uri, strlen(uri), cap, 7, SLRE_IGNORE_CASE);

	if ( res < 0 ) {
		LOG(E_ERROR, "Error %d\n", res);
		return;
	}

	item = download_create();

	LOG(E_DEBUG, "Download slot created ...\n");
	LOG(E_DEBUG, "Item values\n");
	for(i = 0; i < 6; i++) {
		LOG(E_DEBUG, "=> %d: %.*s\n", i, cap[i].len, cap[i].ptr);
	}

	sprintf(item->bot, "%.*s", cap[3].len, cap[3].ptr );
	sprintf(item->channel, "%.*s", cap[2].len, cap[2].ptr );
	sprintf(item->server, "%.*s", cap[0].len, cap[0].ptr );

	LOG(E_DEBUG, "ptr[5] len: %d\n", cap[5].len);

	if ( cap[5].len != 0 ) {
		sprintf(item->name, "%.*s", cap[5].len, cap[5].ptr);
	} else {
		item->name[0] = '\0';
	}

	item->packet = atoi(cap[4].ptr);

	LOG(E_INFO, "Add download [%s] [%s] [%s] [%d] [%s]\n",
			item->server,
			item->channel,
			item->bot,
			item->packet,
			item->name
	);

	//LL_SEARCH_SCALAR(viral_options.servers, )

	//if( server_get( item->server ) == NULL ) {
		/* ADD DEFAULT PORT AND NO SSL */
		//server_add(item->server, item->server, 0, 6667);
		//channel_add(item->server, item->channel);
		//server_write_db();
	//}

	LL_APPEND(download_queue, item);
	downloads_queue_save();
	//downloads_add(item->server, item->channel, item->bot, item->packet, "");
}

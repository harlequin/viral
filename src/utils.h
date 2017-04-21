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

#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include "downloads.h"
#include "../lib/mongoose/mongoose.h"
#include "log.h"

char *jtoa ( struct json_token *tokens, const char *path );

#define FIND_JSON_TOKEN_SAFE(token, tokens, path, x) \
	token = find_json_token(tokens, path); \
	if (!token ) { \
		LOG(E_ERROR, "Token not found\n"); \
		token = NULL; \
	} else {\
		if ( token->type != x ) { \
			LOG(E_ERROR, "Token isn't in expecting format\n");\
			token = NULL; \
		} \
	}

#define JSON_TOKEN_TO_NUMBER(token,r, i) \
		r = malloc(255); \
		sprintf(r, "%.*s", token->len, token->ptr); \
		i = (int) atoi((const char *)r);

#define JSON_TOKEN_TO_STRING(token,r) \
		sprintf(r, "%.*s", token->len, token->ptr);
		/*//r = malloc(255); \*/


#define FIND_JSON_TOKEN_TO_STRING(token, tokens, path, res) \
	FIND_JSON_TOKEN_SAFE(token, tokens, path, JSON_TYPE_STRING) \
	if(!token) {\
		/*res = NULL;*/ } else { \
		JSON_TOKEN_TO_STRING(token, res) \
	}

#define FIND_JSON_TOKEN_TO_NUMBER(token, tokens, path, res, i) \
	FIND_JSON_TOKEN_SAFE(token, tokens, path, JSON_TYPE_NUMBER) \
	if(!token) {\
		res = NULL; } else { \
			JSON_TOKEN_TO_NUMBER(token, res, i) \
	}

size_t viral_strlen(const char *s);
void notice_parser (const char *notice, int len);
int http_download(const char *uri);
int parse_irc_msg ( char *data, irc_download_t *item );
void add_xdcc_link ( char *uri );

#define strlen(x) viral_strlen(x)

#endif /* UTILS_H_ */

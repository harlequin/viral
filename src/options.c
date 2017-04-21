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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "options.h"
#include "log.h"
#include "utlist.h"

void dump_options(void *fp) {
	#undef C
	#define C(x,y) fprintf(fp, "%s = %s\n", #x, #y);
	X
}

int namecmp(irc_server_t *a, irc_server_t *b) {
    return !(a->id == b->id);
}

char *trim_blanks (char *s)  {
    char *t=s, *u=s;
    while ((*t)&&((*t==' ')||(*t=='\t'))) t++;
    while (*t) *(u++)=*(t++);
    while ((u--!=s)&&((*u==' ')||(*u=='\t')||(*u=='\n')));
    *(++u)='\0';
    return s;
}

char** str_split(char* a_str, const char a_delim) {
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp) {
        if (a_delim == *tmp) {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result) {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token) {
            *(result + idx++) = trim_blanks(strdup(token));
            //printf("%s\n", *(result + idx));
            token = strtok(0, delim);
        }
        *(result + idx) = 0;
    }

    return result;
}

void add_server_attribute(irc_server_t *item, const char *attribute, const char *value) {
	//char **channels;
	int i;
	char *v = NULL;

	if(strcmp(attribute, "host") == 0) {
		item->host = strdup(value);
		//item->host = malloc(strlen(value));
		//strcpy(item->host, value);
	}else if(strcmp(attribute, "channels") == 0) {

		if(strlen(value) > 0) {
			//list_item *e;
			irc_channel_t *channel;
			char **channel_array;
			channel_array = str_split((char*)value, ',');
			if(channel_array != NULL) {
				for (i = 0; (v = *(channel_array + i)); i++){
					//e = malloc(sizeof(list_item));
					//e->value = malloc(sizeof(irc_server_config_t));

					channel = malloc(sizeof(irc_channel_t));
					channel->joined = 0;
					channel->name = strdup(v);

					//e->value = channel;
					//list_append( item->channels, e );
					LL_APPEND(item->channels, channel);
					free(v);
				}
				free(channel_array);
			}


			//item->channels = str_split((char*)value, ',');
		} else {
			item->channels = NULL;
		}

	}else if(strcmp(attribute, "encryption") == 0) {
		item->encryption = !strcmp(value, "yes");
	}else if(strcmp(attribute, "port") == 0) {
		item->port = atoi(value);
	} else {

		/*FIX: Do not use LOG here */
		//LOG(E_WARN, "Wrong config option\n");
	}

}

int read_server_config(const char *name, const char *value) {
	char key[255];
	int c;
	int j;
	irc_server_t *item;
	irc_server_t tmp;

	LL_COUNT( viral_options.servers, item, j );

	if(sscanf(name, "server%d.%s", &c, key) == 2) {
//		LOG(E_DEBUG, "=====================\n");
//		LOG(E_DEBUG, "Count:       %d\n", c);
//		LOG(E_DEBUG, "Attribute:   %s\n", key);
//		LOG(E_DEBUG, "Struct Size: %d\n", j );
	}

	if ( c == j ) {
		item = malloc(sizeof(irc_server_t));
		if(!item) {
			LOG(E_FATAL, "malloc failed!\n");
		}
		item->id = c;
		//item->thread = NULL;
		item->state = IRC_SERVER_STATE_IDLE;
		item->nc = NULL;
		item->connections_errors = 0;
		item->channels = NULL;// list_create();
		LL_APPEND(viral_options.servers, item);
	} else {
		tmp.id = c;
		LL_SEARCH(viral_options.servers, item, &tmp,namecmp );

		if(!item) {
			LOG(E_INFO, "NO ELEMENT FOUND\n");
		} else {
		}
	}

	add_server_attribute(item, key, value);

	return 0;
}

int options_init(const char *option_file) {
	FILE *f;
	char buffer[1024];
	char *equals;
	char *name;
	char *value;
	char *t;
	int linenum = 0;

	viral_options.servers = NULL;

	if(!option_file || *option_file == '\0')
		return -1;

	f = fopen(option_file, "r");
	if(!f) {
		fprintf(stderr, "options file not existing [%s]\n", option_file);
		f = fopen(option_file, "w+");
		dump_options(f);
		/* revert file pointer to read the whole data */
		fseek(f, 0, SEEK_SET);
	}


	while(fgets(buffer, sizeof(buffer), f)) {
		linenum++;
		t = strchr(buffer, '\n');
		if(t) {
			*t = '\0';
			t--;
			while((t >= buffer) && isspace(*t)) {
				*t = '\0';
				t--;
			}
		}

		/* skip leading whitespaces */
		name = buffer;
		while(isspace(*name))
			name++;

		/* check for comments or empty lines */
		if(name[0] == '#' || name[0] == '\0') continue;

		if(!(equals = strchr(name, '='))) {
			fprintf(stderr, "parsing error file %s line %d : %s\n", option_file, linenum, name);
			continue;
		}

		/* remove ending whitespaces */
		for(t=equals-1; t>name && isspace(*t); t--)
			*t = '\0';

		*equals = '\0';
		value = equals+1;

		/* skip leading whitespaces */
		while(isspace(*value))
			value++;

		//Server struct found
		if(strncmp(name, "server", 6) == 0) {
			read_server_config(name, value);
			continue;
		}

		#undef C
		#define C(x,y) if (strcmp(name, #x)==0 ) viral_options._##x = strdup(value);
		X

	}
	fclose(f);
	return 0;
}


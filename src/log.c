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

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include "utlist.h"

//#include "../lib/fossa/fossa.h"
#include "../lib/mongoose/mongoose.h"
#include "log.h"

static FILE *log_fp = NULL;
static int log_level = E_DEBUG;

log_message_t *log_messages = NULL;

char *level_name[] = {
	"OFF",					/* E_OFF */
	"FATAL",				/* E_FATAL */
	"ERROR",				/* E_ERROR */
	"WARNING",				/* E_WARN -- warn */
	"INFO",					/* E_INFO */
	"DEBUG",				/* E_DEBUG */
	0
};

int log_init(const char *fname, unsigned int level) {
	FILE *fp;
	log_level = level;

	if (!fname)					/* use default i.e. stdout */
		return 0;

	if (!(fp = fopen(fname, "a")))
		return 1;
	log_fp = fp;
	return 0;
}



void log_err(int level, char *fname, int lineno, char *fmt, ...) {
	char * errbuf;
	va_list ap;
	time_t t;
	struct tm *tm;
	int len;
	log_message_t *m;

	if (level && level > log_level && level>E_FATAL)
		return;

	va_start(ap, fmt);

	errbuf = malloc(1024);

	if ( (len = mg_avprintf(&errbuf,1024, fmt, ap)) == -1) {
		va_end(ap);
		return;
	}
	va_end(ap);

	t = time(NULL);
	tm = localtime(&t);


	fprintf(stdout, "[%04d/%02d/%02d %02d:%02d:%02d] ",
		tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
	    tm->tm_hour, tm->tm_min, tm->tm_sec);

	/* print on stdout */
	fprintf(stdout, "[%s] %s:%d: %s", level_name[level], fname, lineno, errbuf);
	fflush(stdout);

	/* print to fp */
	if(log_fp) {
		fprintf(log_fp, "[%04d/%02d/%02d %02d:%02d:%02d] ",
				tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
			    tm->tm_hour, tm->tm_min, tm->tm_sec);

		fprintf(log_fp, "[%s] %s:%d: %s", level_name[level], fname, lineno, errbuf);
		fflush(log_fp);
	}



//	m = malloc(sizeof(log_message_t));
//	if ( !m ) {
//		LOG(E_FATAL, "log message can't created!\n");
//	}
//	m->data = calloc(1, len);
//	sprintf(m->data, "%.*s",len, errbuf);
//	m->level = level;
//	DL_PREPEND(log_messages, m);




	free(errbuf);

	if (level==E_FATAL)
		exit(-1);

	return;
}

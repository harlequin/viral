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

#ifndef __LOG_H__
#define __LOG_H__

#define E_OFF		0
#define E_FATAL     1
#define E_ERROR		2
#define E_WARN      3
#define E_INFO      4
#define E_DEBUG		5

int log_init(const char *fname, unsigned int level);
void log_err(int level, char *fname, int lineno, char *fmt, ...);

typedef struct LOG_MESSAGE {
	int level;
	char *data;
	struct LOG_MESSAGE *prev, *next;
} log_message_t;

#define LOG(level, fmt, arg...) { log_err(level, __FILE__, __LINE__, fmt, ##arg); }

#endif

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
#include <stdarg.h>
#include <stdlib.h>

#if WIN32

int vasprintf( char **sptr, char *fmt, va_list argv ) {
    int wanted = vsnprintf( *sptr = NULL, 0, fmt, argv );
    if( (wanted > 0) && ((*sptr = malloc( 1 + wanted )) != NULL) )
        return vsprintf( *sptr, fmt, argv );

    return wanted;
}

int asprintf( char **sptr, char *fmt, ... ) {
    int retval;
    va_list argv;
    va_start( argv, fmt );
    retval = vasprintf( sptr, fmt, argv );
    va_end( argv );
    return retval;
}

// Hack from http://stackoverflow.com/questions/2457331/replacement-for-vsscanf-on-msvc
/*
int vsscanf(const char *s, const char *fmt, va_list ap)
{
	int i;
	// up to max 10 arguments
	void *a[10];
	for (i = 0; i < sizeof(a)/sizeof(a[0]); i++)
	{
		a[i] = va_arg(ap, void*);
	}
	return sscanf(s, fmt, a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8], a[9]);
}
*/

#endif

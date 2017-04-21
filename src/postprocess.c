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
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>  /* Many POSIX functions (but not all, by a large margin) */

#include "postprocess.h"
#include "log.h"
#include "portable.h"
#include "../lib/mongoose/mongoose.h"
#include "options.h"

#ifndef MAX_PATH
#define MAX_PATH 255
#endif



//void list_dir(const char *name, int level) {
//	DIR *dir;
//	struct dirent *entry;
//	struct stat st_buf;
//	int status;
//	char path[1024];
//	int len;
//
//
//	char **result;
//		char *query;
//		int cols, rows;
//		int i;
//
//
//	if (!(dir = opendir(name)))
//		return;
//	if (!(entry = readdir(dir)))
//		return;
//
//	do {
//		len = snprintf(path, sizeof(path) - 1, "%s/%s", name, entry->d_name);
//		path[len] = 0;
//
//		status = stat (path, &st_buf);
//		if (status != 0) {
//			LOG(E_ERROR, "Can't determinate file system object '%s'\n", entry->d_name);
//			return;
//		}
//
//		if ( S_ISDIR(st_buf.st_mode )) {
//			/* Skip dots */
//			if (strcmp(entry->d_name, ".") == 0	|| strcmp(entry->d_name, "..") == 0) { continue; }
//
//			printf("%*s[%s]\n", level * 2, "", entry->d_name);
//			list_dir(path, level + 1);
//
//		} else {
//			printf("%*s- %s\n", level * 2, "", entry->d_name);
//			viral_media_object_t *object = guess_media_object( entry->d_name );
//
//
//			if( object ) {
//
//
//				query = sqlite3_mprintf("SELECT * FROM tv_serie WHERE title REGEXP %s", object->name);
//
//				if ((sql_get_table(query, &result, &rows, &cols) == SQLITE_OK) && rows) {
//					for (i = cols; i < rows * cols + cols; i += cols) {
//						LOG(E_INFO, "Show found in database: %s\n", result[i+1]);
//					}
//				}
//			} else {
//				/*NOT PARSED LOG SOMEWHERE FOR DEBUGGING*/
//			}
//
//
//		}
//
//	} while( (entry = readdir(dir)) );
//	closedir(dir);
//}



char *remove_ext (char* mystr, char dot, char sep) {
    char *retstr, *lastdot, *lastsep;

    // Error checks and allocate string.

    if (mystr == NULL)
        return NULL;
    if ((retstr = malloc (strlen (mystr) + 1)) == NULL)
        return NULL;

    // Make a copy and find the relevant characters.

    strcpy (retstr, mystr);
    lastdot = strrchr (retstr, dot);
    lastsep = (sep == 0) ? NULL : strrchr (retstr, sep);

    // If it has an extension separator.

    if (lastdot != NULL) {
        // and it's before the extenstion separator.

        if (lastsep != NULL) {
            if (lastsep < lastdot) {
                // then remove it.

                *lastdot = '\0';
            }
        } else {
            // Has extension separator with no path separator.

            *lastdot = '\0';
        }
    }

    // Return the modified string.

    return retstr;
}

int ends_with(const char * haystack, const char * needle) {
	const char * end;
	int nlen = strlen(needle);
	int hlen = strlen(haystack);

	if( nlen > hlen )
		return 0;
 	end = haystack + hlen - nlen;

 	//TODO: FIX THAT THIS IS STRCASECMP
	//return (strcasecmp(end, needle) ? 0 : 1);
 	return (strcmp(end, needle) ? 0 : 1);
}



/* Takes care of moving a file from a temporary download location to a completed location. Now in UTF-8. */
void
move_file_utf8 (char *src_dir, char *dst_dir, char *fname, int dccpermissions)
{
	char src[4096];
	char dst[4096];
	//int res;
	//, i;
	//char *src_fs;	/* FileSystem encoding */
	//char *dst_fs;

	/* if dcc_dir and dcc_completed_dir are the same then we are done */
	if (0 == strcmp (src_dir, dst_dir) || 0 == dst_dir[0])
		return;			/* Already in "completed dir" */

	snprintf (src, sizeof (src), "%s/%s", src_dir, fname);
	snprintf (dst, sizeof (dst), "%s/%s", dst_dir, fname);

	LOG(E_DEBUG, "%s - %s\n", src, dst);

	/* already exists in completed dir? Append a number */
//	if (file_exists_utf8 (dst))
//	{
//		for (i = 0; ; i++)
//		{
//			snprintf (dst, sizeof (dst), "%s/%s.%d", dst_dir, fname, i);
//			if (!file_exists_utf8 (dst))
//				break;
//		}
//	}

	/* convert UTF-8 to filesystem encoding */
//	src_fs = xchat_filename_from_utf8 (src, -1, 0, 0, 0);
//	if (!src_fs)
//		return;
//	dst_fs = xchat_filename_from_utf8 (dst, -1, 0, 0, 0);
//	if (!dst_fs)
//	{
//		free (src_fs);
//		return;
//	}

	/* first try a simple rename move */
	//res =
	rename (src, dst);

	//if (res == -1 && (errno == EXDEV || errno == EPERM))
	//{
		/* link failed because either the two paths aren't on the */
		/* same filesystem or the filesystem doesn't support hard */
		/* links, so we have to do a copy. */
	//	if (copy_file (src, dst, dccpermissions))
	//		unlink (src);
	//}

	//free (dst);
	//free (src);
}


THREAD_FUNCTION(postprocess) {
//void postprocess(const char *name, int post_process_flags) {
	irc_download_t *session = (irc_download_t *) arg;
	const char *name = (const char*) session->name;

	char extract_directory[MAX_PATH];
	char download_location[MAX_PATH];
	char download_name[MAX_PATH];
	char complete_location[MAX_PATH];
	//char command[1024];
	//char output[256];
	//FILE *process;
	struct stat st_buf;
	int status;
	//int ln;
	int res;

	LOG(E_INFO, "Post process for '%s' started\n", name);

	sprintf(extract_directory, "%s/%s",viral_options._incoming_directory, POSTPROCESS_EXTRACT_DIRECTORY);
	sprintf(download_location, "%s/%s",viral_options._incoming_directory, name);
	sprintf(download_name, "%s", remove_ext((char*)name, '.', '/'));
	sprintf(complete_location, "%s/%s", viral_options._complete_directory, name);


	LOG(E_DEBUG, "Post process information\n");
	LOG(E_DEBUG, "Download object     :  %s\n", name);
	LOG(E_DEBUG, "Object name         :  %s\n", download_name);
	LOG(E_DEBUG, "Download location   :  %s\n", download_location);
	LOG(E_DEBUG, "Extraction directory:  %s\n", extract_directory);

	status = stat (download_location, &st_buf);
	if (status != 0) {
		LOG(E_ERROR, "Can't determinate file system object, exit post process\n");
		return 0;
	}

	if (S_ISDIR(st_buf.st_mode)) {
		LOG(E_ERROR, "File object is a directory, exit post process\n");
		return 0;
	}

	if (access(download_location, R_OK) != 0) {
		LOG(E_ERROR, "File is not readable, exit post process\n");
		return 0;
	}

	/*TODO: implement a list of files extensions which are moved direclty */
	/* from the incoming directory to completed */

	LOG(E_DEBUG, "Starting functional post-process for object\n");

	/* only when there is an archive we have to create the extrace_directory */
	//if ( ends_with(name, ".tar") || ends_with(name, ".rar")) {
		/* TODO IMPLEMENT UMASK MODE FOR LINUX */
	//	mkdir(extract_directory, 0755);
	//} else {
		LOG(E_INFO, "Moving to destination folder\n");
		res = rename (download_location, complete_location);
		LOG(E_DEBUG, "Move operation ends with %d - %d\n", res, errno);
		if (res == -1 && (errno == EXDEV || errno == EPERM)) {
			LOG(E_ERROR, "Move file was not possible, exit post process\n");
		}

		LOG(E_DEBUG, "Post process finished\n");
		session->status = DOWNLOAD_COMPLETED;
		downloads_queue_save();

		return 0;
	//}

//	if( ends_with(name, ".tar")) {
//		LOG(E_INFO, "Extracting tar archive ...\n");
//		sprintf(command, "tar xvf \"%s\" -C \"%s\"", download_location, extract_directory);
//		LOG(E_DEBUG, "%s\n", command);
//		process = popen(command, "r");
//		if( process == NULL ) {
//			LOG(E_ERROR, "tar command failed\n");
//			return 0;
//		}
//		while (fgets(output, sizeof(output), process) != NULL) {
//			ln = strlen(output) - 1;
//			if(output[ln] == '\n')
//				output[ln] = '\0';
//			LOG(E_INFO, "%s\n", output);
//		}
//		pclose(process);
//	}

	//return 0;
}



//static unsigned char
//copy_file (char *dl_src, char *dl_dest, int permissions)	/* FS encoding */
//{
//	int tmp_src, tmp_dest;
//	unsigned char ok = FALSE;
//	char dl_tmp[4096];
//	ssize_t return_tmp, return_tmp2;
//
//	if ((tmp_src = open (dl_src, O_RDONLY /*| OFLAGS**/)) == -1)
//	{
//		fprintf (stderr, "Unable to open() file '%s' (%s) !", dl_src,
//				  strerror (errno));
//		return FALSE;
//	}
//
//	if ((tmp_dest =
//		 open (dl_dest, O_WRONLY | O_CREAT | O_TRUNC /*| OFLAGS*/, permissions)) < 0)
//	{
//		close (tmp_src);
//		fprintf (stderr, "Unable to create file '%s' (%s) !", dl_src,
//				  strerror (errno));
//		return FALSE;
//	}
//
//	for (;;)
//	{
//		return_tmp = read (tmp_src, dl_tmp, sizeof (dl_tmp));
//
//		if (!return_tmp)
//		{
//			ok = TRUE;
//			break;
//		}
//
//		if (return_tmp < 0)
//		{
//			fprintf (stderr, "download_move_to_completed_dir(): "
//				"error reading while moving file to save directory (%s)",
//				 strerror (errno));
//			break;
//		}
//
//		return_tmp2 = write (tmp_dest, dl_tmp, return_tmp);
//
//		if (return_tmp2 < 0)
//		{
//			fprintf (stderr, "download_move_to_completed_dir(): "
//				"error writing while moving file to save directory (%s)",
//				 strerror (errno));
//			break;
//		}
//
//		if (return_tmp < sizeof (dl_tmp))
//		{
//			ok = TRUE;
//			break;
//		}
//	}
//
//	close (tmp_dest);
//	close (tmp_src);
//	return ok;
//}



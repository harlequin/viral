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
#include <fcntl.h>
#include "viral.h"
#include "portable.h"
#include "rpc.h"
#include "log.h"
#include "options.h"
#include "downloads.h"
#include "networks.h"
#include "sig.h"
#include "http_client.h"
#include "../version.h"

volatile int exit_flag = 0;
unsigned int daemonize = 0;
char ip_address[255];
char *configuration_file;


void cmd_processing(int argc, char **argv) {
	int i;

	for (i = 1; i < argc; i++) {
		//LOG(E_DEBUG, "Option %s\n", argv[i]);
	
		if (argv[i][0] != '-') {
			LOG(E_ERROR, "Unknown option: %s\n", argv[i]);
		} else {
			switch (argv[i][1]) {
				case 'd':
					daemonize = 1;
					break;
				case 'c':
					configuration_file = malloc(255);
					strncpy(configuration_file, argv[++i], 255);
					break;
				default:
					LOG(E_ERROR, "Unknown option: %s\n", argv[i]);
					break;
			}
		}
	}
}

#include "http_client.h"

void download_success (struct mg_str data) {
	LOG(E_DEBUG, "Download success\n");
}


int main(int argc, char **argv) {
	
	struct mg_mgr mgr;

	LOG(E_WARN, "viral %s / %s\n", BUILD_GIT_SHA, BUILD_GIT_TIME);
	v_sig_init();
	cmd_processing(argc, argv);

#ifdef HAVE_FORK
	int i;
	if (daemonize) {
		int pid = fork();
		if (pid > 0)
			exit(0);
		else if (pid < 0) {
			printf("Unable to daemonize.\n");
			exit(-1);
		} else {
			setsid();
			for (i = getdtablesize(); i >= 0; i--)
				close(i);
			i = open("/dev/null", O_RDWR);
			dup(i);
			dup(i);
			signal(SIGCHLD, SIG_IGN);
			signal(SIGTSTP, SIG_IGN);
			signal(SIGTTOU, SIG_IGN);
			signal(SIGTTIN, SIG_IGN);
		}
	}
#endif

	if ( !configuration_file ) {
		printf("No config file found for application.\n");
		exit(-1);
	}
	
	options_init(configuration_file /*"./viral.conf"*/);
	
//#ifdef WIN32
	log_init("./viral.log", atoi( viral_options._log_level));
//#else
//	log_init("/var/log/viral.log", atoi( viral_options._log_level));
//#endif

#ifdef WIN32
	mkdir(viral_options._complete_directory /*, S_DIRMODE*/);
	mkdir(viral_options._incoming_directory /*, S_DIRMODE*/ );
#else
	mkdir(viral_options._complete_directory , S_DIRMODE);
	mkdir(viral_options._incoming_directory , S_DIRMODE );
#endif
	downloads_queue_init();
	mg_mgr_init(&mgr, NULL);

	rpc_init(&mgr);

	while(exit_flag == 0) {
		mg_mgr_poll(&mgr, 1000);
		downloads_queue_poll(&mgr);
	}

	mg_mgr_free(&mgr);

	LOG(E_INFO, "Shutdown viral\n");
	return 0;
}

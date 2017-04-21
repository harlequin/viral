/**
* Copyright (C) 2015 harlequin
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
*    documentation  and/or other materials provided with the distribution.
* 3. Neither the names of the copyright holders nor the names of any
*    contributors may be used to endorse or promote products derived from this
*    software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include <string.h>
#include "portable.h"

#define X \
	C(webui, ./webui) \
	C(complete_directory, ./downloads) \
	C(incoming_directory , ./tmp) \
    C(pid_file , ./download/uirc.pid) \
    C(listening_port , 9999) \
	C(encryption, no) \
    C(user_name , darknet) \
	C(log_level, 4) \
	C(log_buffer_size, 1000)


typedef struct {
    #define C(x, y) char *_##x;
	X
	#undef C
	irc_server_t *servers;
} options_t;

int options_init(const char *option_file);
options_t viral_options;


#define VIRAL_IP_SCRIPT "ip1.dynupdate.no-ip.com/"


#endif

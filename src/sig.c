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
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

//#define DEBUG
#define PATH_SEPARATOR "\\"

#include "log.h"
#include "sig.h"
#include "viral.h" //cross reference!!

#ifdef WIN32
#include <windows.h>
#include <imagehlp.h>
#endif

#ifdef HAVE_BACKTRACE
	#include <execinfo.h>
#endif



void addr2line(char const * const program_name, void const * const addr) {
	char addr2line_cmd[512] = {0};
	sprintf(addr2line_cmd,"addr2line -f -p -e %.256s %p", program_name, addr);
	return (void) system(addr2line_cmd);
}
/*
#ifdef WIN32

	void print_backtrace( PCONTEXT context ) {
		char application[MAX_PATH + 1];
		GetModuleFileName(NULL, application, sizeof(application));

		SymInitialize(GetCurrentProcess(), 0, TRUE);
		STACKFRAME frame = { };

		// setup initial stack frame 
		frame.AddrPC.Offset         = context->Eip;
		frame.AddrPC.Mode           = AddrModeFlat;
		frame.AddrStack.Offset      = context->Esp;
		frame.AddrStack.Mode        = AddrModeFlat;
		frame.AddrFrame.Offset      = context->Ebp;
		frame.AddrFrame.Mode        = AddrModeFlat;

		while (StackWalk(IMAGE_FILE_MACHINE_I386, GetCurrentProcess(), GetCurrentThread(), &frame, context, 0, SymFunctionTableAccess, SymGetModuleBase, 0 ) ) {
			addr2line(application, (void*)frame.AddrPC.Offset);
		}

		SymCleanup( GetCurrentProcess() );
	}


#else
*/

	void print_backtrace() {
	#if defined(HAVE_BACKTRACE)
		printf("Segmentation fault, tracing...\n");

			void *array[100];
			size_t size;
			char **strings;
			size_t i;

			size = backtrace(array, 100);
			strings = backtrace_symbols(array, size);

			// first trace to screen
			printf("Obtained %zd stack frames\n", size);
			for (i = 0; i < size; i++)
			{
				printf("\t%s\n", strings[i]);
			}
			free(strings);
	#else
		printf("Segmentation fault %s\n", __FUNCTION__);
	#endif
	}
//#endif




#ifdef WIN32
LONG __stdcall ExceptionFilter(EXCEPTION_POINTERS* pExPtrs) {
	LOG(E_ERROR,"Unhandled Exception: code: 0x%8.8X, flags: %d, address: 0x%8.8X\n",
		pExPtrs->ExceptionRecord->ExceptionCode,
		pExPtrs->ExceptionRecord->ExceptionFlags,
		pExPtrs->ExceptionRecord->ExceptionAddress);

//#ifdef DEBUG
		print_backtrace(pExPtrs->ContextRecord);
//#else
	 //info("Detailed exception information can be printed by debug version of NZBGet (available from download page)");
//#endif

	ExitProcess(-1);
	return EXCEPTION_CONTINUE_SEARCH;
}
#endif






static void v_sigterm(int sig) {

	switch(sig) {
		case SIGSEGV:
			signal(SIGSEGV, SIG_DFL);
#ifdef WIN32
			SetUnhandledExceptionFilter(ExceptionFilter);
#else
			print_backtrace();
			raise (SIGABRT);
#endif
			break;
		default:
			signal(sig, SIG_DFL);
			LOG(E_INFO,"Application %d got signal %d\n", getpid(), sig);
			exit_flag = sig;
			break;
		}
}

void v_sig_init() {
	LOG(E_DEBUG, "Installing signal handler\n");
	signal(SIGINT, v_sigterm);
	signal(SIGSEGV, v_sigterm);
}

void v_test_stacktrace ( ) {
	LOG(E_WARN, "*** STACKTRACE TEST ACTIVE ***\n");
	char* N = NULL;
	strcpy(N, "");
}


/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2012 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/
#include "SDL_config.h"

#if SDL_THREAD_3DS

/* Thread management routines for SDL */

#include <3ds.h>
#include "SDL_thread.h"
#include "../SDL_systhread.h"
#include "../SDL_thread_c.h"

#define STACKSIZE (4 * 1024)

int SDL_SYS_CreateThread(SDL_Thread *thread, void *args)
{
	// The priority of these child threads must be higher (aka the value is lower) than that
	// of the main thread, otherwise there is thread starvation due to stdio being locked.
	s32 priority = 0;
	svcGetThreadPriority(&priority, CUR_THREAD_HANDLE);

	thread->handle = threadCreate(SDL_RunThread, args, STACKSIZE, priority-1, -2, false);
	if (!thread->handle) {
		SDL_SetError("threadCreate() failed");
		return -1;
	}
	return 0;
}

void SDL_SYS_SetupThread(void)
{
	return;
}

Uint32 SDL_ThreadID(void)
{
	return (Uint32)threadGetCurrent();
}

void SDL_SYS_WaitThread(SDL_Thread *thread)
{
	if (!thread || !thread->handle) {
		return;
	}
	threadJoin(thread->handle, U64_MAX);
}

void SDL_SYS_KillThread(SDL_Thread *thread)
{
	if (!thread || !thread->handle) {
		return;
	}
	threadJoin(thread->handle, U64_MAX);
	threadFree(thread->handle);
}

#endif /* SDL_THREAD_3DS */


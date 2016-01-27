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

/* An implementation of semaphores using mutexes and condition variables */

#include <3ds.h>
#include "SDL_timer.h"
#include "SDL_thread.h"
#include "SDL_systhread_c.h"

struct SDL_semaphore
{
//	Handle handle;
	Handle event;
};

SDL_sem *SDL_CreateSemaphore(Uint32 initial_value)
{
	SDL_sem *sem;

	sem = (SDL_sem *)SDL_malloc(sizeof(*sem));
	if ( ! sem ) {
		SDL_OutOfMemory();
		return NULL;
	}
	
	/*
	// TODO: implement proper mutex/semaphore
	Result res = svcCreateSemaphore(&sem->handle, initial_value, 1);
	if(res) {
		SDL_SetError("svcCreateSemaphore failed: %i", res);
		free(sem);
		sem = NULL;
	}
	*/

	svcCreateEvent(&sem->event, 0);

	return sem;
}

/* WARNING:
   You cannot call this function when another thread is using the semaphore.
*/
void SDL_DestroySemaphore(SDL_sem *sem)
{
	/*
	// TODO: implement proper mutex/semaphore
	if (sem != NULL) {
		if (sem->handle) {
			s32 count = 0;
			Result res = svcReleaseSemaphore(&count, sem->handle, 1);
			if(res != 0) {
				SDL_SetError("svcReleaseSemaphore failed: %i", res);
			}
			svcCloseHandle(sem->handle);
		}
		free(sem);
		sem = NULL;
	}
	*/
	if (sem != NULL) {
		if (sem->event) {
			svcClearEvent(sem->event);
			svcCloseHandle(sem->event);
		}
		free(sem);
		sem = NULL;
	}
}

int SDL_SemWaitTimeout(SDL_sem *sem, Uint32 timeout)
{
	if (!sem) {
		SDL_SetError("Passed a NULL semaphore");
		return -1;
	}
	Result res = svcWaitSynchronization(sem->event, (u64)timeout*1000000LL);
	if(res) {
		SDL_SetError("svcWaitSynchronization failed: %i", res);
		return -1;
	}
	return 0;
}

int SDL_SemTryWait(SDL_sem *sem)
{
	return SDL_SemWaitTimeout(sem, 0);
}


int SDL_SemWait(SDL_sem *sem)
{
	return SDL_SemWaitTimeout(sem, SDL_MUTEX_MAXWAIT);
}

Uint32 SDL_SemValue(SDL_sem *sem)
{
	return 1;
}

int SDL_SemPost(SDL_sem *sem)
{
	if (!sem) {
		SDL_SetError("Passed a NULL semaphore");
		return -1;
	}
	Result res = svcSignalEvent(sem->event);
	if(res) {
		SDL_SetError("svcSignalEvent failed: %li", res);
		return -1;
	}
	return 0;
}

#endif /* SDL_THREAD_3DS */


/* Simple test of the SDL threading code */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "SDL.h"
#include "SDL_thread.h"

static int alive = 0;

/* Call this instead of exit(), so we can clean up SDL: atexit() is evil. */
static void quit(int rc)
{
	SDL_Quit();
	exit(rc);
}

int SDLCALL _ThreadFunc(void *data)
{
	printf("Started thread %s: My thread id is %lu\n",
				(char *)data, SDL_ThreadID());
	while ( alive ) {
		printf("Thread '%s' is alive!\n", (char *)data);
		SDL_Delay(1*100);
	}
	printf("Thread '%s' exiting!\n", (char *)data);
	return(0);
}

int main(int argc, char *argv[])
{
	gfxInitDefault();
	consoleInit(GFX_BOTTOM, NULL);

	SDL_Thread *thread;

	printf("SDL_Init\n");
	/* Load the SDL library */
	if ( SDL_Init(0) < 0 ) {
		printf("Couldn't initialize SDL: %s\n",SDL_GetError());
		return(1);
	}

	alive = 1;
	printf("SDL_CreateThread\n");
	thread = SDL_CreateThread(_ThreadFunc, "#1");
	if ( thread == NULL ) {
		printf("Couldn't create thread: %s\n", SDL_GetError());
		quit(1);
	}

	SDL_Delay(5*100);
	printf("Waiting for thread #1\n");
	alive = 0;
	SDL_WaitThread(thread, NULL); // SDL_WaitThread == threadJoin (memory leak?)

	alive = 1;
	printf("SDL_CreateThread\n");
	thread = SDL_CreateThread(_ThreadFunc, "#2");
	if ( thread == NULL ) {
		fprintf(stderr, "Couldn't create thread: %s\n", SDL_GetError());
		quit(1);
	}

	SDL_Delay(5*100);
	printf("Killing thread #2\n");
	alive = 0;
	SDL_KillThread(thread);

	SDL_Quit();	/* Never reached */
	return(0);	/* Never reached */
}

#include "SDL_config.h"

#include <math.h>
#include <string.h>

#include "SDL.h"
#include "SDL_error.h"
#include "SDL_video.h"
#include "SDL_mouse.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"

#include <3ds.h>

#include "SDL_3dsvideo.h"
#include "SDL_3dsevents_c.h"

#define DSVID_DRIVER_NAME "3ds"

static int DS_VideoInit (_THIS, SDL_PixelFormat *vformat);
static SDL_Rect **DS_ListModes (_THIS, SDL_PixelFormat *format, Uint32 flags);
static SDL_Surface *DS_SetVideoMode (_THIS, SDL_Surface *current, int width, int height, int bpp, Uint32 flags);
static int DS_SetColors (_THIS, int firstcolor, int ncolors, SDL_Color *colors);
static void DS_VideoQuit (_THIS);

static int DS_AllocHWSurface (_THIS, SDL_Surface *surface);
static int DS_LockHWSurface (_THIS, SDL_Surface *surface);
static int DS_FlipHWSurface (_THIS, SDL_Surface *surface);
static void DS_UnlockHWSurface (_THIS, SDL_Surface *surface);
static void DS_FreeHWSurface (_THIS, SDL_Surface *surface);

static void DS_UpdateRects (_THIS, int numrects, SDL_Rect *rects);

SDL_Rect *mode;

static SDL_Rect mode_400x240;
static SDL_Rect mode_240x400;
static SDL_Rect mode_320x240;
static SDL_Rect mode_240x320;

static SDL_Rect* modes_descending[] = {
	&mode_400x240, &mode_240x400,
	&mode_320x240, &mode_240x320,
	NULL
};

static int DS_Available (void) {
	return (1);
}

static void DS_DeleteDevice (SDL_VideoDevice *device) {
	SDL_free (device->hidden);
	SDL_free (device);
}

static int HWAccelBlit(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
	return 0;
}
 
static int CheckHWBlit (_THIS, SDL_Surface *src, SDL_Surface *dst) {
	if (src->flags & SDL_SRCALPHA)
		return false;
	if (src->flags & SDL_SRCCOLORKEY)
		return false;
	if (src->flags & SDL_HWPALETTE)
		return false;
	if (dst->flags & SDL_SRCALPHA)
		return false;
	if (dst->flags & SDL_SRCCOLORKEY)
		return false;
	if (dst->flags & SDL_HWPALETTE)
		return false;

	if (src->format->BitsPerPixel != dst->format->BitsPerPixel)
		return false;
	if (src->format->BytesPerPixel != dst->format->BytesPerPixel)
		return false;

	src->map->hw_blit = HWAccelBlit;

	return true;
}

static SDL_VideoDevice *DS_CreateDevice(int devindex) {
	SDL_VideoDevice *device = 0;

	device = (SDL_VideoDevice *)SDL_malloc(sizeof(SDL_VideoDevice));
	if (device) {
		SDL_memset (device, 0, (sizeof *device));
		device->hidden = (struct SDL_PrivateVideoData *)
		SDL_malloc((sizeof *device->hidden));
	}
	if ((device == NULL) || (device->hidden == NULL)) {
		SDL_OutOfMemory();
		if (device) {
			SDL_free(device);
		}

		return 0;
	} 

	SDL_memset (device->hidden, 0, (sizeof *device->hidden));

	device->VideoInit = DS_VideoInit;
	device->ListModes = DS_ListModes;
	device->SetVideoMode = DS_SetVideoMode;
	device->CreateYUVOverlay = NULL;
	device->SetColors = DS_SetColors;
	device->UpdateRects = DS_UpdateRects;
	device->VideoQuit = DS_VideoQuit;
	device->AllocHWSurface = DS_AllocHWSurface;
	device->CheckHWBlit = CheckHWBlit;
	device->FillHWRect = NULL;
	device->SetHWColorKey = NULL;
	device->SetHWAlpha = NULL;
	device->LockHWSurface = DS_LockHWSurface;
	device->UnlockHWSurface = DS_UnlockHWSurface;
	device->FlipHWSurface = DS_FlipHWSurface;
	device->FreeHWSurface = DS_FreeHWSurface;
	device->SetCaption = NULL;
	device->SetIcon = NULL;
	device->IconifyWindow = NULL;
	device->GrabInput = NULL;
	device->GetWMInfo = NULL;
	device->InitOSKeymap = DS_InitOSKeymap;
	device->PumpEvents = DS_PumpEvents;
	device->info.blit_hw = 1;

	device->free = DS_DeleteDevice;
	return device;
}

VideoBootStrap TDS_bootstrap = {
	DSVID_DRIVER_NAME, "3DS Video", DS_Available, DS_CreateDevice
};

int DS_VideoInit (_THIS, SDL_PixelFormat *vformat) {

	gfxInit(GSP_RGB565_OES, GSP_RGB565_OES, false);

	mode_400x240.w = 400, mode_400x240.h = 240;
	mode_240x400.w = 240, mode_240x400.h = 400;
	mode_320x240.w = 320, mode_320x240.h = 240;
	mode_240x320.w = 240, mode_240x320.h = 320;

	return 0;
}

SDL_Rect **DS_ListModes (_THIS, SDL_PixelFormat *format, Uint32 flags) {
	return &modes_descending[0];
}

SDL_Surface *DS_SetVideoMode(_THIS, SDL_Surface *current, int width, int height, int bpp, Uint32 flags) {

	Uint32 Rmask, Gmask, Bmask, Amask; 
	
	mode = modes_descending[0];
	while (mode) {
		if (mode->w == width && mode->h == height)
			break;
		else
			++mode;
	}
	
	if (!mode) {
		SDL_SetError ("Display mode (%dx%d) is unsupported.", width, height);
		return NULL;
	}

	int gfxScreen = GFX_TOP;
	if ((mode->w == 320) || (mode->h == 320)) {
		this->hidden->buffer = gfxGetFramebuffer (GFX_BOTTOM, GFX_LEFT, NULL, NULL);
		consoleInit (GFX_TOP, NULL);
		gfxScreen = GFX_BOTTOM;
	} else {
		this->hidden->buffer = gfxGetFramebuffer (GFX_TOP, GFX_LEFT, NULL, NULL);
		consoleInit (GFX_BOTTOM, NULL);
	}

	if(bpp > 16) {
		bpp=24;
 		Rmask = 0xff0000;
		Gmask = 0xff00;
		Bmask = 0xff;
		Amask = 0;
		gfxSetScreenFormat(gfxScreen, GSP_BGR8_OES);
	} else {
		bpp=16;
 		Rmask = 0xF800;
		Gmask = 0x07E0;
		Bmask = 0x001F;
		Amask = 0;
		gfxSetScreenFormat(gfxScreen, GSP_RGB565_OES);
	}

	SDL_memset (this->hidden->buffer, 0x0, 240*400*(bpp/8));
	if (!SDL_ReallocFormat (current, bpp, Rmask, Gmask, Bmask, Amask)) {
		this->hidden->buffer = NULL;
		SDL_SetError ("Couldn't allocate new pixel format for requested mode");

		return (NULL);
	}

	current->flags = flags | SDL_FULLSCREEN | SDL_HWSURFACE | SDL_DOUBLEBUF;
	this->hidden->w = current->w = width;
	this->hidden->h = current->h = height;
	current->pixels = this->hidden->buffer;

	if (flags & SDL_DOUBLEBUF) { 
		gfxSetDoubleBuffering(gfxScreen, true);
		this->hidden->secondbufferallocd = 1;
		current->pixels = this->hidden->buffer;
	} else {
		gfxSetDoubleBuffering(gfxScreen, false);
	}

	current->pitch = current->w * (bpp / 8);

	return current;
}

static int DS_AllocHWSurface (_THIS, SDL_Surface *surface) {
	if (this->hidden->secondbufferallocd)
		return -1;

	return (0);
}

static void DS_FreeHWSurface (_THIS, SDL_Surface *surface) {
	this->hidden->secondbufferallocd = 0;
}

static int DS_LockHWSurface (_THIS, SDL_Surface *surface) {
	return (0);
}

static void DS_UnlockHWSurface (_THIS, SDL_Surface *surface) {
	return;
}

static int DS_FlipHWSurface (_THIS, SDL_Surface *surface) {
	// Flush and swap framebuffers
	gfxFlushBuffers();
	gfxSwapBuffers();
	//Wait for VBlank
	gspWaitForVBlank();
	return (0);
}

static void DS_UpdateRects (_THIS, int numrects, SDL_Rect *rects) {
	// Flush and swap framebuffers
	gfxFlushBuffers();
	gfxSwapBuffers();
	//Wait for VBlank
	gspWaitForVBlank();
}

int DS_SetColors (_THIS, int firstcolor, int ncolors, SDL_Color *colors) {
	int i, j;

	if (SDL_VideoSurface && SDL_VideoSurface->format && SDL_VideoSurface->format->palette) {
		for (i = 0; i < 256; i += 4) {
			for (j = 0; j < 4; j++) {
				SDL_VideoSurface->format->palette->colors[i + j].r = 85 * j;
				SDL_VideoSurface->format->palette->colors[i + j].g = 85 * j;
				SDL_VideoSurface->format->palette->colors[i + j].b = 85 * j;
			}
		}
	}

	return (0);
}

void DS_VideoQuit (_THIS) {
	gfxExit();
}


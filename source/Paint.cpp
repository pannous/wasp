#include <SDL.h>
#include <stdio.h>
#include "Paint.h"
#include "wasm_helpers.h"

SDL_Surface *surface;
SDL_Window *window;
SDL_Renderer *renderer;

//const int SCREEN_WIDTH = 3840;// works but fucks up osx windows for a few seconds after
//const int SCREEN_HEIGHT = 2160;
const int SCREEN_WIDTH = 3840 / 2;
const int SCREEN_HEIGHT = 2160 / 2;

// const int SCREEN_WIDTH = 1600;
// const int SCREEN_HEIGHT = 1024;
// const int SCREEN_WIDTH = 1920;
// const int SCREEN_HEIGHT = 1280;
//int SCREEN_WIDTH = 800;
//int SCREEN_HEIGHT = 800;

const int BYTES_PER_PIXEL = 4;
int pitch = SCREEN_WIDTH;
int iteration = 0;

const Uint8 *state;

void dot(int x, int y, Uint32 pixel) {
	Uint32 *p = (Uint32 *) surface->pixels + y * SCREEN_WIDTH + x; // *BYTES_PER_PIXEL;
	*p = pixel;
}

int get_safe(int x, int y) {
	if (x < 0)x = SCREEN_WIDTH - 1;// wrap around
	if (y < 0)y = SCREEN_HEIGHT - 1;
	Uint32 *p = (Uint32 *) surface->pixels + (y % SCREEN_HEIGHT) * SCREEN_WIDTH + (x % SCREEN_WIDTH) * BYTES_PER_PIXEL;
	return *p;
}

uint get(int x, int y) {
	return get_safe(x, y);
	// Uint32 *p=surface->pixels + y * pitch + x *BYTES_PER_PIXEL;
	// return *p;
}

Uint8 get_r(int x, int y) {
	Uint8 *p = (Uint8 *) surface->pixels + y * pitch + x * BYTES_PER_PIXEL;
	return *(p + 2);
}

Uint8 get_g(int x, int y) {
	Uint8 *p = (Uint8 *) surface->pixels + y * pitch + x * BYTES_PER_PIXEL;
	return *(p + 1);
}

Uint8 get_b(int x, int y) {
	Uint8 *p = (Uint8 *) surface->pixels + y * pitch + x * BYTES_PER_PIXEL;
	return *p;
}

void surface_fill_random() {
	for (int x = 0; x < SCREEN_WIDTH; x++) {
		for (int y = 0; y < SCREEN_HEIGHT; y++) {
			//			unsigned int c=0x00FFFFFF;//rand ();
//			0x001122FF;//rand ();
			unsigned int c = rand();// cstdlib › rand NOT a true random!
			dot(x, y, c);
		}
	}
}

void init_sdl() {
	SDL_Init(SDL_INIT_VIDEO);              // Initialize SDL2
	// Create an application window with the following settings:
	window = SDL_CreateWindow(
			__FILE_NAME__,                  // window title
			0,           // initial x position
			0,           // initial y position
			SCREEN_WIDTH,                               // width, in pixels
			SCREEN_HEIGHT,                               // height, in pixels
			SDL_WINDOW_OPENGL //|SDL_GL_DOUBLEBUFFER
	);
//	SDL_SetWindowFullscreen(window,SDL_WINDOW_FULLSCREEN);
//    SDL_SetWindowFullscreen(window,SDL_WINDOW_FULLSCREEN_DESKTOP);// faked, slow

	surface = SDL_GetWindowSurface(window);// SDL_GetVideoSurface();
	pitch = surface->pitch;
	surface_fill_random();
	paint(-1);
}

void cleanUp() {
	SDL_Delay(1000);  // Pause execution for 1000 milliseconds, for example
	// Close and destroy the window
	SDL_DestroyWindow(window);
	// Clean up
	SDL_Quit();
}

struct Size {
	int width;
	int height;
};

// returns image.data.length (of RGBA array)
int64 init_graphics() {
    init_sdl();
    return (int64) surface;// surface->w * surface->h * 4; //  (int *) surface->pixels;
}
//int* init_graphics(int width, int height){
//	if(width>0)SCREEN_WIDTH=width;
//	if(height>0)SCREEN_HEIGHT=height;
//	init_sdl();
//	return (int*)surface->pixels;
//}

void checkInput() {
	SDL_Event test_event;
	SDL_KeyboardEvent key;
	while (SDL_PollEvent(&test_event)) {
		switch (test_event.type) {
//			 case SDL_MOUSEMOTION:
            case SDL_MOUSEBUTTONDOWN:
                cleanUp();
                proc_exit(0);
            case SDL_KEYDOWN:
                key = test_event.key;
                if (key.keysym.sym == SDLK_ESCAPE) {
                    cleanUp();
                    proc_exit(0);
                }
		}
	}
}

int paint(int wasm_offset) {// ready to paint!
//	if changed
//	char *wasm_memory=0;//getWasmMemory();
	int nr_bytes = surface->w * surface->h * 4;
	// if nr_bytes changed: skip frame!
	if (wasm_memory and wasm_offset >= 0)
		memcpy(surface->pixels, ((char *) wasm_memory + wasm_offset), nr_bytes);
	SDL_UpdateWindowSurface(window);
	SDL_PumpEvents();
	checkInput();
	state = SDL_GetKeyboardState(NULL);
	return nr_bytes;// number of bytes required for next frame todo return struct!
#ifdef DEBUG
	if (state[SDL_SCANCODE_RETURN]||state[SDL_SCANCODE_ESCAPE]) exit(0);
#endif
}

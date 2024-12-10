/* badXpi
 *
 * GPLv3
 * 
 */

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <linux/fb.h>

#include <SDL.h>

#include <fcntl.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mem.h"
#include "../include/gpu_calls.h"

// Video defines
// visible area we're drawing

#define SCREEN_RAM_OFFSET 0x00000

#define LSHORTCUT_KEY SDL_SCANCODE_LCTRL
#define RSHORTCUT_KEY SDL_SCANCODE_RCTRL

// When rendering a layer line, we can amortize some of the cost by calculating multiple pixels at a time.
#define LAYER_PIXELS_PER_ITERATION 8


static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *sdlTexture;
static char *scale_quality = "best";

static uint16_t width, height;
static uint16_t window_scale = 1;

int gpu_init(uint16_t addr)
{ 
    uint16_t sys = *(uint16_t*)mem_base(addr+2);
    uint16_t gpu_width=*(uint16_t*)mem_base(sys);
    uint16_t gpu_height=*(uint16_t*)mem_base(sys+2);
    uint16_t gpu_scale=*(uint16_t*)mem_base(sys+4);

    uint32_t window_flags = SDL_WINDOW_ALLOW_HIGHDPI;

    width=gpu_width;
    height=gpu_height;
    window_scale=gpu_scale;

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, quality);
    SDL_CreateWindowAndRenderer(width * window_scale, height * window_scale, window_flags, &window, &renderer);
    SDL_SetWindowResizable(window, 1);
    SDL_RenderSetLogicalSize(renderer, width, height);

    sdlTexture = SDL_CreateTexture(renderer,
                    SDL_PIXELFORMAT_RGB888,
                    SDL_TEXTUREACCESS_STREAMING,
                    SCREEN_WIDTH, SCREEN_HEIGHT);

    SDL_SetWindowTitle(window, "bad65C02");

    SDL_ShowCursor(SDL_DISABLE);

    if (sdlTexture) {
        printf("Video setup success!\n");
        return 0;
    }
    return 1;
}

int gpu_cleanup()
{
    printf("VIDEO cleanup\n");
    SDL_DestroyTexture( sdlTexture );
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    SDL_Quit();

    return 0;
}

uint8_t sync_gpu_assist()
{    
    uint16_t addr=*(uint16_t*)mem_base(0xFFF8);
    uint16_t data=*(uint16_t*)mem_base(addr);

    switch (data)
    {
    case PI_GPU_INIT:
        return gpu_init(addr);

    case PI_GPU_CLEANUP:
        return gpu_cleanup(addr);


    default:
        return 255;
    }

    return 0;
}
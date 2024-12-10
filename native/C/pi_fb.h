
#include <stdio.h>
#include "../../include/fb_calls.h"

unsigned char call_buff[40];

int send_for_pi(unsigned int _callid, void *data);

int fb_init(void)
{
    void *sys;
    
    return send_for_pi(PI_FB_INIT, sys);
}

typedef struct {
	unsigned int    x;
	unsigned int    y;
	unsigned char    col;
} syscall_fb_point_t;

int fb_point(unsigned int x, unsigned int y, unsigned char col)
{
    syscall_fb_point_t *sys;
    sys=(void*)call_buff;

    sys->x =        (unsigned int)x;
    sys->y =        (unsigned int)y;
    sys->col =      (unsigned char)col;

    return send_for_pi(PI_FB_POINT, sys);
}

typedef struct {
	unsigned int    x;
	unsigned int    y;
	unsigned char   r;
	unsigned char   g;
	unsigned char   b;
} syscall_fb_pointRGB_t;

int fb_pointRGB(unsigned int x, unsigned int y, unsigned char r, unsigned char g, unsigned char b)
{
    syscall_fb_pointRGB_t *sys;
    sys=(void*)call_buff;

    sys->x =        (unsigned int)x;
    sys->y =        (unsigned int)y;
    sys->r =        (unsigned char)r;
    sys->g =        (unsigned char)g;
    sys->b =        (unsigned char)b;

    return send_for_pi(PI_FB_POINTRGB, sys);
}

typedef struct {
	unsigned int    x;
	unsigned int    y;
	unsigned int    x1;
	unsigned int    y1;
	unsigned char   col;
} syscall_fb_line_t;

int fb_line(unsigned int x, unsigned int y, unsigned int x1, unsigned int y1, unsigned char col)
{
    syscall_fb_line_t *sys;
    sys=(void*)call_buff;

    sys->x =        (unsigned int)x;
    sys->y =        (unsigned int)y;
    sys->x1 =       (unsigned int)x1;
    sys->y1 =       (unsigned int)y1;
    sys->col =      (unsigned char)col;

    return send_for_pi(PI_FB_LINE, sys);
}

typedef struct {
	unsigned int    x;
	unsigned int    y;
	unsigned int    r;
	unsigned char    col;
} syscall_fb_circle_t;

int fb_circle(unsigned int x, unsigned int y, unsigned int r, unsigned char col)
{
    syscall_fb_circle_t *sys;
    sys=(void*)call_buff;

    sys->x =        (unsigned int)x;
    sys->y =        (unsigned int)y;
    sys->r =        (unsigned int)r;
    sys->col =      (unsigned char)col;

    return send_for_pi(PI_FB_CIRCLE, sys);
}

int fb_wait_for_vsync(void)
{
    void *sys;
    
    return send_for_pi(PI_FB_WAITFORVSYNC, sys);
}

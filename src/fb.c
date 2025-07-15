/* badXpi
 *
 * GPLv3
 * 
 */

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <linux/fb.h>

#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mem.h"
#include "../include/fb_calls.h"


uint16_t palette[] = {0x0000,0x7bcf,0x0008,0x73ca,0x610c,0x2b00,0x5000,0x3b8e,0x2a0d,0x0106,0x39cf,0x18c3,0x39c7,0x33ca,0x7a00,0x5acb,0x0000,0x0841,0x1082,0x18c3,0x2104,0x2945,0x3186,0x39c7,0x4208,0x4a49,0x528a,0x5acb,0x630c,0x6b4d,0x738e,0x7bcf,0x0842,0x18c4,0x2106,0x3188,0x420a,0x4a4c,0x5acf,0x0842,0x1084,0x18c6,0x2108,0x294a,0x318c,0x39cf,0x0002,0x0844,0x0846,0x1088,0x108a,0x18cc,0x18cf,0x0002,0x0004,0x0006,0x0008,0x000a,0x000c,0x000f,0x0882,0x1904,0x2186,0x3208,0x428a,0x4b0c,0x5b8f,0x0842,0x10c4,0x1946,0x21c8,0x2a4a,0x32cc,0x3b4f,0x0042,0x08c4,0x0946,0x1188,0x120a,0x1a8c,0x1b0f,0x0042,0x00c4,0x0106,0x0188,0x020a,0x024c,0x02cf,0x0881,0x1903,0x2185,0x3207,0x4289,0x4b0b,0x5bcd,0x0881,0x1103,0x1984,0x2206,0x2a88,0x3309,0x3bcb,0x0081,0x0902,0x0984,0x1205,0x1286,0x1b08,0x1bc9,0x0081,0x0102,0x0183,0x0204,0x0285,0x0306,0x03c7,0x0881,0x1903,0x2984,0x3206,0x4288,0x5309,0x63cb,0x0881,0x1102,0x2183,0x2a04,0x3285,0x4306,0x4bc7,0x0080,0x0901,0x1181,0x1a02,0x2282,0x2b03,0x33c3,0x0080,0x0900,0x0980,0x1200,0x1280,0x1b00,0x1bc0,0x1081,0x2103,0x3184,0x4206,0x5288,0x6309,0x7bcb,0x1081,0x2102,0x3183,0x4204,0x5285,0x6306,0x7bc7,0x1080,0x2101,0x3181,0x4202,0x5282,0x6303,0x7bc3,0x1080,0x2100,0x3180,0x4200,0x5280,0x6300,0x7bc0,0x1041,0x20c3,0x3144,0x4186,0x5208,0x6289,0x7b0b,0x1041,0x2082,0x3103,0x4144,0x5185,0x6206,0x7a47,0x1000,0x2041,0x3081,0x40c2,0x5102,0x6143,0x7983,0x1000,0x2040,0x3040,0x4080,0x5080,0x60c0,0x78c0,0x1041,0x20c3,0x3105,0x4187,0x5209,0x624b,0x7acd,0x1041,0x2083,0x30c4,0x4106,0x5148,0x6189,0x79cb,0x1001,0x2042,0x3044,0x4085,0x5086,0x60c8,0x78c9,0x1001,0x2002,0x3003,0x4004,0x5005,0x6006,0x7807,0x1042,0x20c4,0x3106,0x4188,0x520a,0x624c,0x72cf,0x0842,0x1884,0x28c6,0x3908,0x494a,0x598c,0x69cf,0x0802,0x1844,0x2846,0x3088,0x408a,0x50cc,0x60cf,0x0802,0x1804,0x2006,0x3008,0x400a,0x480c};

struct fb_var_screeninfo screen_info;
struct fb_fix_screeninfo fixed_info;
int fbfd = -1;
char *fbbuffer = NULL;
size_t fbbuflen;

// stencil buffer 
char* stencil_buf = NULL;

// stamp buffer
char* stamp_buffer = NULL;

uint8_t  brush_color = 1;
uint8_t  background_color = 0;

// internal functions, not exposed directly
//
uint8_t _cls()
{
    bzero((void*)fbbuffer,fbbuflen);
    return 0;
}

void _block(uint16_t x, uint16_t y, uint16_t x2, uint16_t y2, unsigned char col) {
    uint16_t tmp;

    if ( x == x2 || y == y2)
        return;

    if (x > x2) {
        tmp=x2;
        x2=x;
        x=tmp;
    }
    
    if (y > y2) {
        tmp=y2;
        y2=y;
        y=tmp;
    }
    
    uint16_t pixel = palette[col];

    while (y < y2) {
        uint32_t location = x*screen_info.bits_per_pixel/8 + 
                                y*fixed_info.line_length;
        uint16_t* px = (void*)fbbuffer+location;
        tmp=x;
        while (tmp < x2) {
            *px++ = pixel;
            tmp++;
        }
        y++;
    }
}

static inline void _point(uint16_t x, uint16_t y, uint8_t col)
{
    uint16_t pixel = palette[col];

    uint32_t location = x*screen_info.bits_per_pixel/8 + 
                                y*fixed_info.line_length;
    *((uint16_t*) (fbbuffer + location)) = pixel;
}

static void _pointRGB(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b )
{
    uint16_t pixel = (r << screen_info.red.offset)|
                     (g << screen_info.green.offset)|
                     (b << screen_info.blue.offset);

    uint32_t location = x*screen_info.bits_per_pixel/8 + 
                                y*fixed_info.line_length;
    *((uint16_t*) (fbbuffer + location)) = pixel;
}

#define sgn(x) ((x) < 0 ? -1 : ((x) > 0 ? 1 : 0))

void _line(int x, int y, int x2, int y2, unsigned char col) {
	bool yLonger=false;
	int incrementVal, endVal;
	int shortLen=y2-y;
	int longLen=x2-x;
	if (abs(shortLen)>abs(longLen)) {
		int swap=shortLen;
		shortLen=longLen;
		longLen=swap;
		yLonger=true;
	}
	endVal=longLen;
	if (longLen<0) {
		incrementVal=-1;
		longLen=-longLen;
	} else incrementVal=1;
	int decInc;
	if (longLen==0) decInc=0;
	else decInc = (shortLen << 16) / longLen;
	int j=0;
	if (yLonger) {	
		for (int i=0;i!=endVal;i+=incrementVal) {
			_point(x+(j >> 16),y+i,col);	
			j+=decInc;
		}
	} else {
		for (int i=0;i!=endVal;i+=incrementVal) {
			_point(x+i,y+(j >> 16),col);
			j+=decInc;
		}
	}
}

/*
// Draw a line from (x1, y1) to (x2, y2)
void _line(int x1, int y1, int x2, int y2, unsigned char attr) {
    int16_t dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int16_t dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int16_t err = dx + dy, e2;

    while (1) {
        _point(x1, y1, attr);
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}
*/
/*
static void _line(uint16_t x, uint16_t y, uint16_t x1, uint16_t y1, uint8_t col)
{
    int16_t deltax=x1 - x;
    int16_t deltay=y1 - y;

    int16_t drawx=x;
    int16_t drawy=y;
 
   if (abs(deltax) >= abs(deltay)) {
    for (int n = 0; n < abs(deltax); n++){
      y += abs(deltay);
      if(y >= abs(deltax)){
        y -= abs(deltax);
        drawy += sgn(deltay);
      }
      drawx += sgn(deltax);
      
      _point(drawx,drawy,col);
    }
    return;
   }
   else {
    for (int n = 0; n < abs(deltay); n++){
      x += abs(deltax);
      if(x >= abs(deltay)){
        x -= abs(deltay);
        drawx += sgn(deltax);
      }
      drawy += sgn(deltay);
      
      _point(drawx,drawy,col);
    }
   }
}
*/
static void _pan(uint8_t direction)
{
    uint32_t location;

    //printf("pan: direction=0x%02X\n",direction);
    switch (direction)
    {
    case 0x01:
        /* to the right */
            memmove( (void*)fbbuffer+2,
                     (void*)fbbuffer,
                    screen_info.yres*fixed_info.line_length-2);
            _line(0,0,0,screen_info.yres-1,background_color);
            break;
    case 0x09:
        /* to the left */
            memmove( (void*)fbbuffer,
                     (void*)fbbuffer+2,
                    screen_info.yres*fixed_info.line_length-0);
            _line(screen_info.xres-1,0,screen_info.xres-1,screen_info.yres-1,background_color);
            break;
    case 0x10:
        /* down */
            location = (uint32_t)fbbuffer+fixed_info.line_length;
            memmove( (void*)location,
                     (void*) fbbuffer,
                    (screen_info.yres-1)*fixed_info.line_length);
            _line(0,0,screen_info.xres-1,0,background_color);
        break;
    case 0x90:
        /* up */
            location = (uint32_t)fbbuffer+fixed_info.line_length;
            memmove( (void*) fbbuffer,
                     (void*) location,
                    (screen_info.yres-1)*fixed_info.line_length);
            _line(0,screen_info.yres-1,screen_info.xres-1,screen_info.yres-1,background_color);
        break;

    default:
        break;
    }
}

uint8_t fb_wait_vsync()
{
    int zero = 0;
    if (ioctl(fbfd, FBIO_WAITFORVSYNC, &zero) == -1)
        printf("fb ioctl failed: %s\n", strerror(errno));
  return zero & 0xff;
}

/* Unsupported 
uint8_t fb_in_vblank()
{
    struct fb_vblank info;
    if (ioctl(fbfd, FBIOGET_VBLANK, &info) == -1)
        printf("fb ioctl failed: %s\n", strerror(errno));

    if (info.flags & FB_VBLANK_VBLANKING )
      return 1;
    
    return 0;
}
*/

// The memory mapped functions, no callbacks, this needs to be fast
//

/* fb mem io overlay drawing with a pixel cursor
*
* 0xFF00, 0xFF01   PIXEL CURSOR BASE X
* 0xFF02, 0xFF03                BASE Y       WRITING CURSOR UPDATES VIRTUAL X/Y
* 0xFF04           PIXEL at X/Y              WRITES PALLETE VAL
* 0xFF05           PIXEL at X+1/Y            WRITES PALLETE VAL, MODIFIES VIRTUAL X/Y
* 0xFF06           PIXEL at X/Y+1
* 0xFF07           PIXEL at X-1/Y
* 0xFF08           PIXEL at X/Y-1
* 0xFF09           PIXEL at (CUSRSOR X/Y+1)  WRITES PALLETE VAL, MODIFIES VIRTUAL X/Y, CURSOR
* 0xFF0A           PIXEL at (CUSRSOR X+1/Y)
* 0xFF0B           PAN SCREEN(direction)
* ...           
* 0xFF0E           BRUSH COLOR          
* 0xFF0F           BACKGROUND COLOR
*/

uint16_t p_cursor_base_x;
uint16_t p_cursor_base_y;
uint16_t p_cursor_x;
uint16_t p_cursor_y;


void fb_mem_io_write(uint16_t address, uint8_t data)
{
    //printf("addr: 0x%04X, data: 0x%02X\n", address,data);
    switch (address & 0xF)
    {
    case 0x00:
        p_cursor_base_x =  (p_cursor_base_x & 0xFF00) | data;
        p_cursor_x = p_cursor_base_x;
        return;
    case 0x01:
        p_cursor_base_x =  (p_cursor_base_x & 0x00FF) | data<<8;
        p_cursor_x = p_cursor_base_x;
        return;
    case 0x02:
        p_cursor_base_y =  (p_cursor_base_y & 0xFF00) | data;
        p_cursor_y = p_cursor_base_y;
        return;
    case 0x03:
        p_cursor_base_y =  (p_cursor_base_y & 0x00FF) | data<<8;
        p_cursor_y = p_cursor_base_y;
        return;
    case 0x04: 
        _point(p_cursor_x, p_cursor_y, data);
        return;
    case 0x05: 
        _point(p_cursor_x++, p_cursor_y, data);
        return;
    case 0x06: 
        _point(p_cursor_x, p_cursor_y++, data);
        return;
    case 0x07: 
        _point(p_cursor_x--, p_cursor_y, data);
        return;
    case 0x08: 
        _point(p_cursor_x, p_cursor_y--, data);
        return;
    case 0x09:
        p_cursor_base_y++;
        p_cursor_x=p_cursor_base_x;
        p_cursor_y=p_cursor_base_y;
        _point(p_cursor_x, p_cursor_y, data);
        return;
    case 0x0A:
        p_cursor_base_x++;
        p_cursor_x=p_cursor_base_x;
        p_cursor_y=p_cursor_base_y;
        _point(p_cursor_x, p_cursor_y, data);
        return;
    case 0x0B:
        _pan(data);
        return;
    case 0x0E:
        brush_color=data;
        return;
    case 0x0F:
        background_color=data;
        return;

    default:
        break;
    }
}

void fb_mem_io_write_1(uint16_t address, uint8_t data)
{
    // Sprites & blitter


}

uint8_t fb_mem_io_read(uint16_t address)
{
    //printf("addr: 0x%04X\n", address);

    // Note: since the highest byte of the long is read first
    // the actual setting of the time is there
    //
    switch (address & 0xF)
    {
    case 0x00:
        return screen_info.xres & 0xFF;
    case 0x01:
        return (screen_info.xres>>8) & 0xFF;
    case 0x02:
        return screen_info.yres & 0xFF;
    case 0x03:
        return (screen_info.yres>>8) & 0xFF;
//    case 0x0C:
//        return fb_in_vblank();

    default:
        return 255;
    }
}

int fb_init()
{
   int r = 1;

   //printf("fb_init()");

   fbfd = open("/dev/fb0", O_RDWR);
   if (fbfd >= 0)
   {
      if (!ioctl(fbfd, FBIOGET_VSCREENINFO, &screen_info) &&
          !ioctl(fbfd, FBIOGET_FSCREENINFO, &fixed_info)) {
         fbbuflen = screen_info.yres_virtual * fixed_info.line_length;
         fbbuffer = mmap(NULL,
                       fbbuflen,
                       PROT_READ|PROT_WRITE,
                       MAP_SHARED,
                       fbfd,
                       0);
         if (fbbuffer != MAP_FAILED) {
            /*
            printf("fb0 opnened\n");
            printf("  fb0   x: %i\n",screen_info.xres);
            printf("  fb0   y: %i\n",screen_info.yres);
            printf("  fb0 bpp: %i\n",screen_info.bits_per_pixel);

            printf("  fb0 red offset  : %i\n",screen_info.red.offset);
            printf("  fb0 green offset: %i\n",screen_info.green.offset);
            printf("  fb0 blue offset : %i\n",screen_info.blue.offset);

            printf("  fb0 red length  : %i\n",screen_info.red.length);
            printf("  fb0 green length: %i\n",screen_info.green.length);
            printf("  fb0 blue length : %i\n",screen_info.blue.length);
            */

            io_write_top_handler[0]=&fb_mem_io_write; // PIXEL OPS
            io_write_top_handler[1]=&fb_mem_io_write_1; // SPRITES & BLITTER OPS
            io_read_top_handler[0]=&fb_mem_io_read;

            return 0;   /* Indicate success */
         }
         else {
            fprintf(stderr,"mmap failed");
         }
      }
        else {
            fprintf(stderr,"ioctl failed");
        }
    }
    else {
        fprintf(stderr,"open failed");
    }
    if (fbbuffer && fbbuffer != MAP_FAILED)
      munmap(fbbuffer, fbbuflen);
    if (fbfd >= 0)
      close(fbfd);

    return 1;
}

uint8_t fb_pointRGB(uint16_t addr)
{ 
    uint16_t sys = *(uint16_t*)mem_base(addr+2);
    uint16_t x=*(uint16_t*)mem_base(sys);
    uint16_t y=*(uint16_t*)mem_base(sys+2);
    uint16_t r=*(uint8_t*)mem_base(sys+4);
    uint16_t g=*(uint8_t*)mem_base(sys+6);
    uint16_t b=*(uint8_t*)mem_base(sys+8);

    //printf("fb_point x=%i, y=%i, col=%i\n",x,y,col);

    _pointRGB(x, y, r, g, b);

    return 0;
} 

uint8_t fb_point(uint16_t addr)
{ 
    uint16_t sys = *(uint16_t*)mem_base(addr+2);
    uint16_t x=*(uint16_t*)mem_base(sys);
    uint16_t y=*(uint16_t*)mem_base(sys+2);
    uint16_t col=*(uint8_t*)mem_base(sys+4);

    //printf("fb_point x=%i, y=%i, col=%i\n",x,y,col);

    _point(x, y, col);

    return 0;
} 

uint8_t fb_line(uint16_t addr)
{ 
    uint16_t sys = *(uint16_t*)mem_base(addr+2);
    uint16_t x=*(uint16_t*)mem_base(sys);
    uint16_t y=*(uint16_t*)mem_base(sys+2);
    uint16_t x1=*(uint16_t*)mem_base(sys+4);
    uint16_t y1=*(uint16_t*)mem_base(sys+6);
    uint16_t col=*(uint8_t*)mem_base(sys+8);

    //printf("fb_line x=%i, y=%i, x1=%i, y1=%i, col=%i\n",x,y,x1,y1,col);

    _line(x, y, x1, y1, col);

    return 0;
}

uint8_t fb_circle(uint16_t addr)
{
    uint16_t sys = *(uint16_t*)mem_base(addr+2);
    uint16_t x=*(uint16_t*)mem_base(sys);
    uint16_t y=*(uint16_t*)mem_base(sys+2);
    uint16_t radius=*(uint16_t*)mem_base(sys+4);
    uint16_t col=*(uint8_t*)mem_base(sys+6);

    //printf("fb_circle x=%i, y=%i, radius=%i, col=%i\n",x,y,radius,col);

    int16_t xpos,ypos;

    ypos = 0;
    int16_t t1 = radius >> 4;
    int16_t t2;
    xpos = radius;
    while ( xpos >= ypos ) {
        _point(x+xpos,y+ypos,col); 
        _point(x-xpos,y+ypos,col);
        _point(x+xpos,y-ypos,col);
        _point(x-xpos,y-ypos,col);
        _point(x+ypos,y+xpos,col); 
        _point(x-ypos,y+xpos,col);
        _point(x+ypos,y-xpos,col);
        _point(x-ypos,y-xpos,col);
        ypos++; 
        t1 += ypos;
        t2 = t1 - xpos;
        if ( t2 >= 0 ) { 
            t1 = t2;
            xpos--;
        }
    }

    return 0;
}

uint8_t fb_cls(uint16_t addr)
{ 
    uint16_t sys = *(uint16_t*)mem_base(addr+2);

    //printf("fb_cls()\n");

    _cls();

    return 0;
}

uint8_t fb_block(uint16_t addr)
{ 
    uint16_t sys = *(uint16_t*)mem_base(addr+2);
    uint16_t x=*(uint16_t*)mem_base(sys);
    uint16_t y=*(uint16_t*)mem_base(sys+2);
    uint16_t x1=*(uint16_t*)mem_base(sys+4);
    uint16_t y1=*(uint16_t*)mem_base(sys+6);
    uint16_t col=*(uint8_t*)mem_base(sys+8);

    //printf("fb_block x=%i, y=%i, x1=%i, y1=%i, col=%i\n",x,y,x1,y1,col);

    _block(x, y, x1, y1, col);

    return 0;
}

uint8_t fb_malloc_sbuf(uint16_t addr)
{
    uint16_t sys = *(uint16_t*)mem_base(addr+2);
    uint16_t width=*(uint16_t*)mem_base(sys);
    uint16_t height=*(uint16_t*)mem_base(sys+2);

    //printf("fb_malloc_sbuf width=%i, height=%i\n",width,height);

    stencil_buf=malloc(width*height);

    if (!stencil_buf) 
        return 0xff;

    return 0;
}

uint8_t sync_fb_assist()
{    
    uint16_t addr=*(uint16_t*)mem_base(0xFFF8);
    uint16_t data=*(uint16_t*)mem_base(addr);

    switch (data)
    {
    case PI_FB_INIT:
        return fb_init();
    case PI_FB_POINT:
        return fb_point(addr);
    case PI_FB_POINTRGB:
        return fb_pointRGB(addr);
    case PI_FB_LINE:
        return fb_line(addr);
    case PI_FB_CIRCLE:
        return fb_circle(addr);
    case PI_FB_WAITFORVSYNC:
        return fb_wait_vsync();
    case PI_FB_CLS:
        return fb_cls(addr);
    case PI_FB_BLOCK:
        return fb_block(addr);
    case PI_FB_MALLOC_SBUF:
        return fb_malloc_sbuf(addr);
    default:
        return 255;
    }

    return 0;
}

void async_fb_assist(uint16_t addr)
{    
    uint16_t data=*(uint16_t*)mem_base(addr);

    switch (data)
    {
    case PI_FB_INIT:
        fb_init();
        return;
    default:
        return;
    }

    return;
}

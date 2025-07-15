/*
*  This is the framebuffer test program fb callbacks
*  to the Raspberry Pi.
*/

#include "pi_libc.h"
#include "pi_fb.h"
#include "pi_time.h"

int main()
{
    unsigned char argc = *(unsigned char*)0x0900;
    unsigned int *argv = (unsigned int*)0x0901;

    int j;

    int xsize,ysize;

    // clear the screen
    puts(CHAR_CLS);
    puts(CHAR_CURSOR_HIDE);

    // turn off cursor blink (when running remote)
    system("echo -e \"\\e[?25l\" > /dev/console");

    printf("Framebuffer tests starting\n");

    fb_init();
    // Grab the values from memory
    xsize = PI_FB_SIZE_X;
    ysize = PI_FB_SIZE_Y;

    printf("Graphics resolution x=%i, y=%i\n", xsize, ysize);
    
    //set the background color
    PI_FB_COLOR_BACKGROUND=0x04;

    //fill the entire screen
    fb_block(0,0,*(unsigned int*)0xFF00, *(unsigned int*)0xFF02,1);

    //init the virtual timer hardware
    time_init();

    // draw a line pattern
    for (j=0;j<254;j++) {
        fb_line(106+j*2,0,360,240,j);
        //msleep(100);
    }

    // draw a circle
    fb_circle(360,240,30,0x1f);

    // move the screen around
    for (j=0; j<100; j++) {
        fb_wait_for_vsync();
        PI_FB_CMD_PAN=0x01;
    }

    for (j=0; j<200; j++) {
        fb_wait_for_vsync();
        PI_FB_CMD_PAN=0x09;
    }

    for (j=0; j<100; j++) {
        fb_wait_for_vsync();
        PI_FB_CMD_PAN=0x01;
    }

    for (j=0; j<100; j++) {
        fb_wait_for_vsync();
        PI_FB_CMD_PAN=0x10;
    }

    for (j=0; j<200; j++) {
        fb_wait_for_vsync();
        PI_FB_CMD_PAN=0x90;
    }

    for (j=0; j<100; j++) {
        fb_wait_for_vsync();
        PI_FB_CMD_PAN=0x10;
    }

    // show the cursor again
    puts(CHAR_CURSOR_SHOW);

    printf("Test end !\n");
    // Exit the runtime - with the 
    // right exit code
    sysexit(2);

    return 0;
}

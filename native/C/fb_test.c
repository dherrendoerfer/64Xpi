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

    // clear the screen
    puts(CHAR_CLS);
    puts(CHAR_CURSOR_HIDE);

    printf("Framebuffer tests starting\n");

    fb_init();
    printf("Graphics resolution x=%i, y=%i\n",*(unsigned int*)0xFF00, *(unsigned int*)0xFF02);

    time_init();

    for (j=0;j<254;j++) {
        fb_line(106+j*2,0,360,240,j);
        //fb_line(719 -106-j,0,360,240,j);
        msleep(100);
    }

    fb_circle(360,240,30,0x1f);

    for (j=0; j<100; j++) {
        fb_wait_for_vsync();
        *(unsigned int*)0xFF0B=0x01;
    }

    for (j=0; j<200; j++) {
        fb_wait_for_vsync();
        *(unsigned int*)0xFF0B=0x09;
    }

    for (j=0; j<100; j++) {
        fb_wait_for_vsync();
        *(unsigned int*)0xFF0B=0x01;
    }

    for (j=0; j<100; j++) {
        fb_wait_for_vsync();
        *(unsigned int*)0xFF0B=0x10;
    }

    for (j=0; j<200; j++) {
        fb_wait_for_vsync();
        *(unsigned int*)0xFF0B=0x90;
    }

    for (j=0; j<100; j++) {
        fb_wait_for_vsync();
        *(unsigned int*)0xFF0B=0x10;
    }

    // show the cursor again
    puts(CHAR_CURSOR_SHOW);

    printf("Test end !\n");
    // Exit the runtime - with the 
    // right exit code
    sysexit(2);

    return 0;
}

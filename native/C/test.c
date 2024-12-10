/*
*  This is the callback test program it tests synchronous callbacks
*  to the Raspberry Pi.
*/

#include "pi_libc.h"
#include "pi_fb.h"
#include "pi_time.h"

// our message buffer
static char msg[]="Hello                       \n";

int main()
{
    unsigned char argc = *(unsigned char*)0x0900;
    unsigned int *argv = (unsigned int*)0x0901;

    int fd, ret, errno, j;
    unsigned char i,g;

    long time_ns;
    long time_s;
    unsigned long time_ms;

    //sprintf(msg,"Hello World\n");

    // clear the screen
    puts(CHAR_CLS);

    // test some puts() uses
    if (!puts(msg)) {
        puts("World\n");
    }

    printf("Test starting");

    putchar('.');
    putchar('.');
    putchar('.');
    putchar('.');
    putchar('\n');

    printf("Press any key to continue\n");
    i=getchar();

    printf("you pressed = 0x%02X\n",i);

    // query the time unit
    time_init();

    time_ns=*(long *)0xFF20;
    time_s=*(long *)0xFF24;
    printf("time is: %li.%05li\n",time_s,time_ns);
    system("sleep 1");

    time_ns=*(long *)0xFF20;
    time_s=*(long *)0xFF24;
    printf("time is: %li.%05li\n",time_s,time_ns);
    system("sleep 1");

    time_ns=*(long *)0xFF20;
    time_s=*(long *)0xFF24;
    printf("time is: %li.%05li\n",time_s,time_ns);
    system("sleep 10");

    // query rough ms counter
    time_ms=*(unsigned long *)0xFF28;
    printf(" time_ms: %lu\n",time_ms);
    system("sleep 1");

    time_ms=*(unsigned long *)0xFF28;
    printf(" time_ms: %lu\n",time_ms);

    *(unsigned int *)0xFF2C = 1000;
    while ( *(unsigned char *)0xFF2C );

    time_ms=*(unsigned long *)0xFF28;
    printf(" time_ms: %lu\n",time_ms);
    msleep(1000);

    printf("Press any key to continue\n");
    i=getchar();

    //get argc, argv
    sprintf(msg,"argc=%i\n",argc);
    puts(msg);

    for (i=0; i<argc; i++) {
      sprintf(msg,"argv[%i]=%s\n",i ,argv[i]);
      puts(msg);
    }

    // open a file for writing
    fd=open("/tmp/test",O_WRONLY | O_APPEND | O_CREAT);

    if (fd==255) {
        puts("open() failed\n");
        return 0;
    }

    sprintf(msg,"fd=%i\n",fd);
    puts(msg);

    // write something to the file
    ret=write(fd,"the quick brown fox jumps over the lazy dog\n",44);
    sprintf(msg,"wrote %i bytes\n",ret);
    puts(msg);

    // close the test file
    if (close(fd)) {
        puts("close() failed\n");
        return 0;
    }
    
    // open the file again
    fd=open("/tmp/test",0);

    lseek(fd, 4, SEEK_SET);

    // read 16 bytes
    ret=read(fd,msg,16);
    errno=*(unsigned char*)0xFFF9;

    //if fail, print detailed error
    if (ret==255) {
      sprintf(msg,"error: %i\nerrno: %i\n",ret,errno);
      puts(msg);
      return 0;
    }
 
    // patch line end and end of string
    msg[16]=10;
    msg[17]=0;
    puts(msg);

    // read some more
    sprintf(msg,"read %i bytes\n",ret);
    puts(msg);

    ret=read(fd,msg,23);
    msg[23]=10;
    msg[24]=0;
    puts(msg);

    sprintf(msg,"read %i bytes\n",ret);
    puts(msg);

    //close the fd
    if (close(fd)) {
        puts("close() failed\n");
        return 0;
    }

    // change the working dir
    chdir("/tmp");

    puts(CHAR_CLS);

    // read the working dir
    getcwd(msg,30);
    puts(msg);  
    puts("\n");  

    // run a command on the runtime host
    system("/bin/ls -l /tmp/test");

    unlink("/tmp/test");

    puts("Type something:\n");

    i=0;
    while(1) {
        read(STDIN,&msg[i],1 );
        if (msg[i] == 10)
            break;
        i++;
        if (i == sizeof(msg)-1)
            break;
    }
    msg[++i] = 0;

    puts("You entered:");
    puts(msg);
    puts("\n");

    system("sleep 10");

    // Graphics test
    
    // hide the cursor
    puts(CHAR_CURSOR_HIDE);
    puts(CHAR_CLS);

    puts(CHAR_VPOS(10));
    printf("Graphics tests starting\n");

    fb_init();
    printf("Graphics resolution x=%i, y=%i\n",*(unsigned int*)0xFF00, *(unsigned int*)0xFF02);
    
    for (g=0; g<20; g++) {   
      for (i=0; i<255; i++) {
          fb_point(i, 10+g, i);
      }
    }

    for (g=0; g<20; g++) {   
          fb_line(0,35+g,100,35+g,0x01);
    }

    fb_circle(100,100,20,0x1f);

    system("sleep 10");
    puts(CHAR_CLS);

    system("sleep 10");

    // show the cursor again
    puts(CHAR_CURSOR_SHOW);

    puts("Test end !\n");
    // Exit the runtime - with the 
    // right exit code
    sysexit(2);

    return 0;
}


/* badXpi
 *
 * GPLv3
 * 
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>

#include "cpu.h"
#include "mem.h"

//#include "../native/C/entry.h"

//
// Test program for now

//#define DEBUG 1

int main(int argc, char *argv[])
{
  //printf("64Xpi\n-----\n\n");

  //printf("Tests\n");

  //printf("mem_init()\n");
  mem_init();

  //printf("ARGC=%i\n",argc);
  //printf("ARGV[0]=%s\n",argv[0]);
  //printf("ARGV[1]=%s\n",argv[1]);


  
  //mem_write(0xFFFE,0x00);
  //mem_write(0xFFFF,0x10);
  himem2[himem2_index[0]+0xFFFE] = 0x00;
  himem2[himem2_index[0]+0xFFFF] = 0x10;

  // load test program starting from 0x1000
//  for (int g=0; g<test_len; g++)
//    mem_write(0x1000+g,test[g]);

  printf("loading %s\n", argv[1]);

  int fd = open(argv[1],0);
  if (fd == -1) {
    fprintf(stderr,"could not open file\n");
    exit(1);
  }

  //skip magic and rom header for now
  lseek(fd, 86, SEEK_SET);

  char buf;
  int count;
  while (read(fd,&buf,1)) {
    mem_write(0x1000+count,buf);
    count++;
  }
  close(fd);

  //copy argc, the argv contents and the argv strings to 0x0900 skip the 1st argument!!! 
  int strptr = 0x901 + (argc-1)*2;

  mem_write(0x900,argc-1);
  for (int g=0; g<argc-1; g++) {
    mem_write(0x901+(g*2), strptr & 0xff);
    mem_write(0x901+(g*2)+1, 0x09);
    char* arg = argv[g+1];
    for(int i=0; i<20; i++) {
      if (arg[i] == 0)
        break;
      mem_write(strptr, arg[i]);
      strptr++;
    }
    mem_write(strptr++, 0);
  }
  
  //printf("reset()\n");
  reset65C02();

  // watch execution until reset vector is hit
  for (int g=0; g<80; g++) {
    step65C02();
    
    #ifdef DEBUG
    printf("ADDR:%04X,",bus_addr);
    if (bus_rw) {
      printf("r,");
    }
    else {
      printf("w,");
    }
    printf(" %02X",mem_read(bus_addr));
    printf("\n");
    #endif


    if (bus_addr == 0xFFFF) {
        // hit reset vector.
        break;
    }
    if (g==79) {
      printf("6502 reset failed.\n");
      return 1;
    }
  }

   printf("6502.\n");


  // continue to run (for 1000 clocks) 
  for (long g=0; g<2000000; ) {
    // test for async assist call 

    #ifdef DEBUG
    step65C02();
    printf("ADDR:%04X,",bus_addr);
    if (bus_rw) {
      printf("r,");
    }
    else {
      printf("w,");
    }
    printf(" %02X, (0x%02X,0x%02X)",mem_read(bus_addr),himem1_page,himem2_page);
    printf("\n");

    usleep(10000);
    #else

    if (step65C02()) {
      printf("\nSTEP BREAK\n");
    }
    
    #endif

  }

  return 0;
}
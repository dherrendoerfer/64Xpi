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
#include <sys/mman.h>

#include "64xpi.h"
#include "cpu.h"
#include "mem.h"

//#include "../native/C/entry.h"

//
// Test program for now

//#define DEBUG 1


volatile uint64_t timer_now;

uint32_t *st;
volatile uint64_t *timer;

void setup_timer()
{
  int mem_fd;
  void *st_map;

   /* open /dev/mem */
   if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
      printf("can't open /dev/mem \n");
      exit(-1);
   }

   /* mmap GPIO */
   st_map = mmap(
      NULL, 
      4096,
      PROT_READ|PROT_WRITE,
      MAP_SHARED,
      mem_fd,
      ST_BASE
   );
   /* close fd */
   close(mem_fd); 

   if (st_map == MAP_FAILED) {
      printf("mmap error %d\n", (int)st_map);//errno also set!
      exit(-1);
   }

  /* store map pointer */
  st = st_map;
  timer=(volatile void*)st+4;
}

int main(int argc, char *argv[])
{
  setup_timer();
  mem_init();
  
  // set the start address to 0x1000
  himem2[himem2_index[0]+0xFFFE] = 0x00;
  himem2[himem2_index[0]+0xFFFF] = 0x10;

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
    usleep(100);
    
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
    loop:
      timer_now=*timer;
      //if (step65C02()) {
      //  printf("\nSTEP BREAK\n");
      //}
      step65C02();
      step65C02();

      while(timer_now == *timer);
    goto loop;
    
    #endif
  }

  return 0;
}

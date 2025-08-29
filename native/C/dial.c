/*
*  This is nc wrapper to the Raspberry Pi.
*/

#include "pi_libc.h"
#include "pi_fb.h"
#include "pi_mbox.h"
#include "pi_time.h"

static char msg[48];

void usage()
{
  printf("usage: dial <HOST> <PORT>\n");
  sysexit(1);
}

int main()
{
  unsigned char argc = *(unsigned char*)0x0900;
  unsigned int *argv = (unsigned int*)0x0901;

  if (argc < 2)
    usage();


  sprintf(msg,"nc %s %s", argv[1],argv[2]);

  system(msg);

  sysexit(2);
  return 0;
} 
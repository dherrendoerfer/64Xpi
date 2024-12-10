
#include <stdint.h>

volatile uint16_t mbox_stopped = 1;



void mbox_thread()
{
    mbox_stopped = 0;
     

     
    mbox_stopped = 1;
    return;
}
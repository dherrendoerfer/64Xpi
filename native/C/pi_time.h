#include <stdint.h>
#include "../../include/time_calls.h"

unsigned char call_buff[40];

int send_for_pi(unsigned int _callid, void *data);

int time_init(void)
{
    void *sys;
    
    return send_for_pi(PI_TIME_INIT, sys);
}

void msleep(uint16_t msecs) 
{
    *(unsigned int *)0xFF2C = msecs;
    while ( *(unsigned char *)0xFF2C );
    return;
}
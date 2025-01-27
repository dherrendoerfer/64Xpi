#include <stdint.h>
#include "../../include/mbox_calls.h"

unsigned char call_buff[40];

int send_for_pi(unsigned int _callid, void *data);

int mbox_init(void)
{
    void *sys;
    
    return send_for_pi(PI_MBOX_INIT, sys);
}
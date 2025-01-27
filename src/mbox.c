
#include <stdint.h>
#include <pthread.h>

volatile uint16_t mbox_stopped = 1;



void mbox_thread()
{
    mbox_stopped = 0;
     

     
    mbox_stopped = 1;
    return;
}

pthread_t mboxThread;

int mbox_init()
{
    //printf("time_init()");

    if (pthread_create(&mboxThread, NULL, mbox_thread, NULL)) {
        printf("thread create failed\n");
        return -1;
    }
    
    while(mbox_stopped);

    return 0;
}

uint8_t sync_mbox_assist()
{    
    uint16_t addr=*(uint16_t*)mem_base(0xFFF8);
    uint16_t data=*(uint16_t*)mem_base(addr);

    switch (data)
    {
    case PI_MBOX_INIT:
        return mbox_init();
    default:
        return 255;
    }

    return 0;
}
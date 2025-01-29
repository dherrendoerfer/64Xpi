#include <pthread.h>

#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "mem.h"
#include "mbox.h"
#include "../include/mbox_calls.h"

#include "fb.h"
#include "mmu.h"
#include "../include/fb_calls.h"
#include "../include/mmu_calls.h"

// Threads start out running, then stop and go to sleep
// a write to it's mem-address wakes the thread
volatile uint16_t mbox0_stopped = 0;
volatile uint16_t mbox0_addr;
volatile uint16_t mbox1_stopped = 0;
volatile uint16_t mbox1_addr;
volatile uint16_t mbox2_stopped = 0;
volatile uint16_t mbox2_addr;
volatile uint16_t mbox3_stopped = 0;
volatile uint16_t mbox3_addr;

sigset_t fSigSet;

pthread_t mboxThread0;
pthread_t mboxThread1;
pthread_t mboxThread2;
pthread_t mboxThread3;


void mbox_mem_io_write(uint16_t address, uint8_t data)
{
    //printf("addr: 0x%04X, data: 0x%02X\n", address,data);
    switch (address & 0xF)
    {
    case 0x00:
        if (mbox0_stopped) {
            mbox0_addr = (mbox0_addr & 0xff00) | data;
        }
        return;
    case 0x01:
        if (mbox0_stopped) {
            mbox0_addr = (mbox0_addr & 0x00ff) | data<<8;

            //wake mbox0           
            pthread_kill(mboxThread0, SIGUSR1);
        }
        return;
    case 0x02:
        if (mbox1_stopped) {
            mbox1_addr = (mbox1_addr & 0xff00) | data;
        }
        return;
    case 0x03:
        if (mbox1_stopped) {
            mbox1_addr = (mbox1_addr & 0x00ff) | data<<8;
            //wake mbox1
            pthread_kill(mboxThread1, SIGUSR1);
        }
        return;
    case 0x04: 
        if (mbox2_stopped) {
            mbox2_addr = (mbox2_addr & 0xff00) | data;
        }
        return;
    case 0x05: 
        if (mbox2_stopped) {
            mbox2_addr = (mbox2_addr & 0x00ff) | data<<8;
            //wake mbox2
            pthread_kill(mboxThread2, SIGUSR1);
        }
        return;
    case 0x06: 
        if (mbox3_stopped) {
            mbox3_addr = (mbox3_addr & 0xff00) | data;
        }
        return;
    case 0x07: 
        if (mbox3_stopped) {
            mbox3_addr = (mbox3_addr & 0x00ff) | data<<8;
            //wake mbox3
            pthread_kill(mboxThread3, SIGUSR1);
        }
        return;

    default:
        break;
    }
}

uint8_t mbox_mem_io_read(uint16_t address)
{
    //printf("addr: 0x%04X\n", address);

    switch (address & 0xF)
    {
    case 0x00:
        return mbox0_addr & 0xFF;
    case 0x01:
        return mbox0_addr>>8;
    case 0x02:
        return mbox1_addr & 0xFF;
    case 0x03:
        return mbox1_addr>>8;
    case 0x04:
        return mbox2_addr & 0xFF;
    case 0x05:
        return mbox2_addr>>8;
    case 0x06:
        return mbox3_addr & 0xFF;
    case 0x07:
        return mbox3_addr>>8;
    default:
        return 255;
    }
}

void async_call_dispatcher(uint16_t addr)
{
    uint16_t data=*(uint16_t*)mem_base(addr);

    //printf("\nCALLBACK %04X\n",data);

    switch(data & 0xFF00)
    {
      case PI_FB:
        return async_fb_assist(addr);
      case PI_MMU:
        return async_mmu_assist(addr);
    }
} 

void *mbox_thread0()
{
    int nSig;

    while (1==1) {
        mbox0_stopped = 1;
        sigwait(&fSigSet, &nSig);

        fprintf(stdout, "thread0 woke up.\n");
    }
    //mbox0_stopped = 1;
}

void *mbox_thread1()
{
    int nSig;

    while (1==1) {
        mbox1_stopped = 1;
        sigwait(&fSigSet, &nSig);

        fprintf(stdout, "thread1 woke up.\n");
    }
    //mbox1_stopped = 1;
}

void *mbox_thread2()
{
    int nSig;

    while (1==1) {
        mbox2_stopped = 1;
        sigwait(&fSigSet, &nSig);

        fprintf(stdout, "thread2 woke up.\n");
    }
    //mbox2_stopped = 1;
}

void *mbox_thread3()
{
    int nSig;

    while (1==1) {
        mbox3_stopped = 1;
        sigwait(&fSigSet, &nSig);

        fprintf(stdout, "thread3 woke up.\n");
    }
    //mbox3_stopped = 1;
}


int mbox_init()
{
    printf("mbox_init()\n");

    sigemptyset(&fSigSet);
    sigaddset(&fSigSet, SIGUSR1);
    sigaddset(&fSigSet, SIGSEGV);

    pthread_sigmask(SIG_BLOCK, &fSigSet, NULL);

    io_write_top_handler[0xF]=&mbox_mem_io_write; // PIXEL OPS
    io_read_top_handler[0xF]=&mbox_mem_io_read;

    if (pthread_create(&mboxThread0, NULL, mbox_thread0, NULL)) {
        printf("thread 0 create failed\n");
        return -1;
    }

    if (pthread_create(&mboxThread1, NULL, mbox_thread1, NULL)) {
        printf("thread 1 create failed\n");
        return -1;
    }

    if (pthread_create(&mboxThread2, NULL, mbox_thread2, NULL)) {
        printf("thread 2 create failed\n");
        return -1;
    }

    if (pthread_create(&mboxThread3, NULL, mbox_thread3, NULL)) {
        printf("thread 3 create failed\n");
        return -1;
    }

    while(!mbox0_stopped);
    while(!mbox1_stopped);
    while(!mbox2_stopped);
    while(!mbox3_stopped);

    printf("mbox threads created\n");

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
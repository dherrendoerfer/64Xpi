/* badXpi
 *
 * GPLv3
 * 
 */

#include <sys/time.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "mem.h"
#include "../include/time_calls.h"


static struct timeval now;
static struct timeval ms;
static uint32_t time_ms = 0;

static uint32_t deadline = 0;

// The memory mapped functions, no callbacks, this needs to be fast
//

/* time unit mem io
*                                           READ                WRITE
* 0xFF20, 0xFF21, 0xFF22, 0xFF23            timeval_usec
* 0xFF24, 0xFF25, 0xFF26, 0xFF27            timeval_sec
* 0xFF28, 0xFF29, 0xFF2A, 0xFF2B            milliseconds
* 0xFF2C, 0xFF2D                                                deadline in ms
* 0xFF2C                                    deadline passed?
* 0xFF2E, 0xFF2F                            random() numbers
*/

void time_mem_io_write(uint16_t address, uint8_t data)
{
    //printf("addr: 0x%04X, data: 0x%02X\n", address,data);
    switch (address & 0xF)
    {
    case 0x00:
        return;
 
    case 0x0C:
        gettimeofday (&ms, NULL);
        deadline = data;
        return;
    case 0x0D:
        deadline = deadline | (data << 8);
        deadline += (ms.tv_sec % 1000000)*1000 + ms.tv_usec/1000;
        return;

    default:
        break;
    }
}

uint8_t time_mem_io_read(uint16_t address)
{
    //printf("addr: 0x%04X\n", address);

    // Note: since the highest byte of the long is read first
    // the actual setting of the time is there
    //
    switch (address & 0xF)
    {
    case 0x00:
        return  now.tv_usec & 0xff;
    case 0x01:
        return (now.tv_usec >> 8) & 0xff;
    case 0x02:
        return (now.tv_usec >> 16) & 0xff;
    case 0x03:
        gettimeofday (&now, NULL);
        return (now.tv_usec >> 24) & 0xff;

    case 0x04:
        return  now.tv_sec & 0xff;
    case 0x05:
        return (now.tv_sec >> 8) & 0xff;
    case 0x06:
        return (now.tv_sec >> 16) & 0xff;
    case 0x07:
        return (now.tv_sec >> 24) & 0xff;

    case 0x08:
        return  time_ms & 0xff;
    case 0x09:
        return (time_ms >> 8) & 0xff;
    case 0x0A:
        return (time_ms >> 16) & 0xff;
    case 0x0B:
        gettimeofday (&ms, NULL);
        time_ms=(ms.tv_sec % 1000000)*1000 + ms.tv_usec/1000;
        return (time_ms >> 24) & 0xff;

    case 0x0C:
        gettimeofday (&ms, NULL);
        if (deadline > (ms.tv_sec % 1000000)*1000 + ms.tv_usec/1000)
            return 255;
        else
            return 0;
    case 0x0E:
        return random() & 0xff;
    case 0x0F:
        return random() & 0xff;

    default:
        return 255;
    }
}

int time_init()
{
    //printf("time_init()");

    // initialise some defaults
    gettimeofday (&now, NULL);
    gettimeofday (&ms, NULL);

    io_write_top_handler[2]=&time_mem_io_write; // Clock
    io_read_top_handler[2]=&time_mem_io_read;  // Clock

    return 0;
}

uint8_t sync_time_assist()
{    
    uint16_t addr=*(uint16_t*)mem_base(0xFFF8);
    uint16_t data=*(uint16_t*)mem_base(addr);

    switch (data)
    {
    case PI_TIME_INIT:
        return time_init();
    default:
        return 255;
    }

    return 0;
}
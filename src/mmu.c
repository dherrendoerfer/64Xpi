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
#include "../include/mmu_calls.h"


// The memory mapped functions, no callbacks, this needs to be fast
//

/* time unit mem io
*                                           READ                WRITE
* 0xFFE0, 0xFFE1                                 himem pages 0 & 1
*
*/

void mmu_mem_io_write(uint16_t address, uint8_t data)
{
    //printf("addr: 0x%04X, data: 0x%02X\n", address,data);
    switch (address & 0xF)
    {
    case 0x00:
        himem2[himem2_index[0]+address] = data;
        himem1_page=data;
        return;
    case 0x01:
        himem2[himem2_index[0]+address] = data;
        himem2_page=data;
        return;

    default:
        break;
    }
}

uint8_t mmu_mem_io_read(uint16_t address)
{
    //printf("addr: 0x%04X\n", address);

    // Note: since the highest byte of the long is read first
    // the actual setting of the time is there
    //
    switch (address & 0xF)
    {
    case 0x00:
        return  himem1_page;
    case 0x01:
        return  himem2_page;
 
    default:
        return 255;
    }
}

int mmu_init()
{
    //printf("time_init()");

    io_write_top_handler[0xe]=&mmu_mem_io_write; // MMU
    io_read_top_handler[0xe]=&mmu_mem_io_read;   // MMU

    return 0;
}

uint8_t sync_mmu_assist()
{    
    uint16_t addr=*(uint16_t*)mem_base(0xFFF8);
    uint16_t data=*(uint16_t*)mem_base(addr);

    switch (data)
    {
    case PI_MMU_INIT:
        return mmu_init();
    default:
        return 255;
    }

    return 0;
}

void async_mmu_assist(uint16_t addr)
{    
    uint16_t data=*(uint16_t*)mem_base(addr);

    switch (data)
    {
    case PI_MMU_INIT:
        mmu_init();
        return;
    default:
        return;
    }

    return ;
}
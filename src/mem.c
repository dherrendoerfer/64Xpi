/* badXpi
 *
 * GPLv3
 * 
 */


#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>

#include "mem.h"
#include "libC_assist.h"

// Main memory
uint8_t *mem = 0;
uint8_t* lowmem;
uint8_t* himem1;  // Start of HIMEM1 in mem
uint8_t* himem2;  // Start of HIMEM2 in mem

//volatile uint8_t mem_type = 0;
volatile uint16_t mem_io = 0;
volatile uint8_t mem_rw = 0;

int32_t himem1_index[256];
int32_t himem2_index[256];

uint8_t himem1_page = 0;
uint8_t himem2_page = 0;

uint8_t mem_init()
{
  // pre-calculate the page index offsets for himem areas
   for(int g=0; g<256; g++)
   { 
      himem1_index[g] = g*0x2000 - HIMEM1_START;
      himem2_index[g] = g*0x4000 - HIMEM2_START;
   }

  mem = malloc(LOMEM_SIZE + HIMEM1_SIZE + HIMEM2_SIZE);

  if (!mem) {
    return 1;
  }

  lowmem = &mem[0];
  himem1 = &mem[HIMEM1_START_IN_MEM];
  himem2 = &mem[HIMEM2_START_IN_MEM];

  // make sure to commit
  asm volatile ("dsb st" : : : "memory");
  
  return 0;
}

//
// write
//

static void write_lomem_zeropage(uint16_t address, uint8_t val)
{
  lowmem[address] = val;
}

static void write_lomem_noio(uint16_t address, uint8_t val)
{
  lowmem[address] = val;
}

/*
static void write_lomem_io(uint16_t address, uint8_t val)
{
  if (address < IO_START) {
    lowmem[address] = val;
    return;
  }

  mem_io=address;

  switch(address>>4) {
        case 0x9f0:
          via1_write(address & 0xf, val);
        case 0x9f1:
          via2_write(address & 0xf, val);
      case 0x9f2:
      case 0x9f3:
        video_write(address & 0x1f, val);
      default:
        break;
    }
  return;
}
*/

static void write_himem1(uint16_t address, uint8_t val)
{
//  mem_type=MEM_TYPE_HIMEM1;
  himem1[himem1_index[himem1_page]+address] = val;
}

static void write_himem2(uint16_t address, uint8_t val)
{
//  mem_type=MEM_TYPE_HIROM;
  if (himem2_page < EXRAM_START) {
    return;
  }
  himem2[himem2_index[himem2_page]+address] = val;
}

void* io_write_top_handler[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
void(*iowritetop)(uint16_t, uint8_t);

static void write_himem2_top(uint16_t address, uint8_t val)
{
  // Top 255 bytes is always pinned to bank0
  //

  // Function (library) callback (deposits a pointer to the call)
  if (address == 0xFFF8) {
      himem2[himem2_index[0]+address] = val;
      return;
  }

  if (address == 0xFFF9) {
    uint8_t ret;
    errno = 0;
    himem2[himem2_index[0]+address] = val;

    ret = sync_libC_assist();
    
    himem2[himem2_index[0]+0xFFF8] = ret;
    himem2[himem2_index[0]+0xFFF9] = errno;
    return;
  }
 
  // Memory Page switching  (FFE0 -> memory management unit) 
  if (address==0xFFE0) {
    himem2[himem2_index[0]+address] = val;
    himem1_page=val;
    return;
  }
  if (address==0xFFE1) {
    himem2[himem2_index[0]+address] = val;
    himem2_page=val;
    return;
  }

  // The NMI vector 
  if (address==0xFFFA) {
    himem2[himem2_index[0]+address] = val;
    return;
  }
  if (address==0xFFFB) {
    himem2[himem2_index[0]+address] = val;
    return;
  }

  // The interrupt vector 
  if (address==0xFFFE) {
    himem2[himem2_index[0]+address] = val;
    return;
  }
  if (address==0xFFFF) {
    himem2[himem2_index[0]+address] = val;
    return;
  }

  uint8_t handler = (address>>4) & 0xf;
  if ( io_write_top_handler[handler]) {
    iowritetop=io_write_top_handler[handler];
    (*iowritetop)(address, val);

    return;
  }

  himem2[himem2_index[himem2_page]+address] = val;
}

static const void* mem_write_handler[16] = { &write_lomem_zeropage,
                                      &write_lomem_noio,
                                      &write_lomem_noio,
                                      &write_lomem_noio,
                                      &write_lomem_noio,
                                      &write_lomem_noio,
                                      &write_lomem_noio,
                                      &write_lomem_noio,
                                      &write_lomem_noio,
                                      &write_lomem_noio,
                                      &write_himem1,
                                      &write_himem1,
                                      &write_himem2,
                                      &write_himem2,
                                      &write_himem2,
                                      &write_himem2_top,
                                    };


static void(*writemem)(uint16_t, uint8_t);
inline void mem_write(uint16_t address, uint8_t val)
{
  writemem=mem_write_handler[address >> 12];
  (*writemem)(address, val);
}

//
// read
//

static uint8_t read_lomem_zeropage(uint16_t address)
{
  return (volatile uint8_t)lowmem[address];
}

static uint8_t read_lomem_noio(uint16_t address)
{
  return (volatile uint8_t)lowmem[address];
}
/*
static uint8_t read_lomem_io(uint16_t address)
{
  if (address < IO_START) {
    return (volatile uint8_t)lowmem[address];
  }

  // Handle IO
  switch(address>>4) {
        case 0x9f0:
          return via1_read(address & 0xf);
        case 0x9f1:
          return via2_read(address & 0xf);
      case 0x9f2:
      case 0x9f3:
        return video_read(address & 0x1f);
      default:
        break;
    }

  return 0;
}
*/
static uint8_t read_himem1(uint16_t address)
{
  return (volatile uint8_t)himem1[himem1_index[himem1_page]+address];
}

static uint8_t read_himem2(uint16_t address)
{
  if (himem2_page < EXRAM_START) {
    return (volatile uint8_t)himem2[himem2_index[himem2_page]+address];
  }
  return (volatile uint8_t)himem2[himem2_index[himem2_page]+address];
}

void* io_read_top_handler[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
uint8_t(*ioreadtop)(uint16_t);

static uint8_t read_himem2_top(uint16_t address)
{
  // Top 255 bytes is always pinned to bank0
  //

  uint8_t handler = (address>>4) & 0xf;
  if ( io_read_top_handler[handler]) {
    ioreadtop=io_read_top_handler[handler];
    return (*ioreadtop)(address);
  }

  return (volatile uint8_t)himem2[himem2_index[0]+address];
}

static const void* mem_read_handler[16] = {  &read_lomem_zeropage,
                                      &read_lomem_noio,
                                      &read_lomem_noio,
                                      &read_lomem_noio,
                                      &read_lomem_noio,
                                      &read_lomem_noio,
                                      &read_lomem_noio,
                                      &read_lomem_noio,
                                      &read_lomem_noio,
                                      &read_lomem_noio,
                                      &read_himem1,
                                      &read_himem1,
                                      &read_himem2,
                                      &read_himem2,
                                      &read_himem2,
                                      &read_himem2_top,
                                    };


static uint8_t(*readmem)(uint16_t);
inline uint8_t mem_read(uint16_t address)
{
  readmem=mem_read_handler[address >> 12];
  return (*readmem)(address);
}

//
// Base addresses of virtual memory
//

static void *mem_lomem_zeropage(uint16_t address)
{
  return &lowmem[address];
}

static void *mem_lomem_noio(uint16_t address)
{
  return &lowmem[address];
}
static void *mem_himem1(uint16_t address)
{
  return &himem1[himem1_index[himem1_page]+address];
}

static void *mem_himem2(uint16_t address)
{
  return &himem2[himem2_index[himem2_page]+address];
}

static const void *mem_base_handler[16] = { &mem_lomem_zeropage,
                                      &mem_lomem_noio,
                                      &mem_lomem_noio,
                                      &mem_lomem_noio,
                                      &mem_lomem_noio,
                                      &mem_lomem_noio,
                                      &mem_lomem_noio,
                                      &mem_lomem_noio,
                                      &mem_lomem_noio,
                                      &mem_lomem_noio,
                                      &mem_himem1,
                                      &mem_himem1,
                                      &mem_himem2,
                                      &mem_himem2,
                                      &mem_himem2,
                                      &mem_himem2,
                                    };

static void *(*basemem)(uint16_t);
inline void *mem_base(uint16_t address)
{
  basemem=mem_base_handler[address >> 12];
  return (*basemem)(address);
}
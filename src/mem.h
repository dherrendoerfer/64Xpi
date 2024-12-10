#include <sys/types.h>

#ifndef _mem_h
#define _mem_h

#define HIMEM1_START     0xA000
#define HIMEM2_START     0xC000
#define IO_START         0x9F00

#define LOMEM_SIZE       0xA000
#define HIMEM1_SIZE      0x200000
#define HIMEM2_SIZE      0xC00000

#define HIMEM1_START_IN_MEM (LOMEM_SIZE)
#define HIMEM2_START_IN_MEM (LOMEM_SIZE + HIMEM1_SIZE) 

#define EXRAM_START      0x00
#define VERA_START       0xF8

extern uint8_t *mem;
extern uint8_t *lowmem;
extern uint8_t *himem1;
extern uint8_t *himem2;

extern uint8_t himem1_page;
extern uint8_t himem2_page;

extern int32_t himem1_index[];
extern int32_t himem2_index[];

//IO Mem address&data
extern volatile uint16_t mem_io;
extern volatile uint8_t mem_rw;

//pluggable io handlers 0xFFxx
extern void* io_write_top_handler[];
extern void* io_read_top_handler[];

uint8_t mem_init();
void mem_write(uint16_t address, uint8_t data);
uint8_t mem_read(uint16_t address);
void *mem_base(uint16_t address);

#endif /*_mem*/

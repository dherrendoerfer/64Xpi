/* badXpi
 *
 * GPLv3
 * 
 */
 
#include <stdint.h>


#ifndef _CPU_H_
#define _CPU_H_

// I/O access
extern volatile uint32_t *gpio;

extern volatile uint32_t clockticks65C02;
extern volatile uint32_t _clockticks65C02;
extern volatile uint8_t _irq65C02;
extern volatile uint8_t cpu_init;
extern uint32_t bus_addr;
extern uint32_t bus_rw;

extern volatile void * page_handler[];

extern void reset65C02();
extern int step65C02();
extern void exec65C02(uint32_t tickcount);
extern void irq65C02(int state);

void regpage_r(uint8_t page,void* handler);
void regpage_w(uint8_t page, void* handler);

#endif
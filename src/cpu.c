/* badXpi
 *
 * GPLv3
 * 
 */

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#include "64xpi.h"
#include "cpu.h"
#include "mem.h"

int  mem_fd;
void *gpio_map;

// MMIO base (detected)
uint32_t mmio_peri_base;

// I/O access
volatile uint32_t *gpio;

// GPIO helpers
volatile uint32_t *gpio_i;
volatile uint32_t *gpio_o_set;
volatile uint32_t *gpio_o_clear;

// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0

#define GET_GPIO(g) (*(gpio+13)&(1<<g)) // 0 if LOW, (1<<g) if HIGH

#define GPIO_PULL *(gpio+37) // Pull up/pull down
#define GPIO_PULLCLK0 *(gpio+38) // Pull up/pull down clock

// delay helpers
#define ndelay20 asm volatile ("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop");
#define ndelay10 asm volatile ("nop\nnop\nnop\nnop\nnop");

// Defined elsewehere (hopefully)
extern uint8_t read65C02(uint16_t address);
extern void write65C02(uint16_t address, uint8_t value);
extern void update65C02();

// Tick counter
volatile uint32_t  clockticks65C02; //increases on transition to high
volatile uint32_t _clockticks65C02; //increases on transition to low

// Physical irq line
volatile uint8_t irq;
volatile uint8_t _irq; // irq 1 clock ago

volatile uint8_t cpu_init = 0;

// Defined for speed
#define _65C02_gpio_data_r 0x0
#define _65C02_gpio_data_w 0x249249


void detectPi()
{
  uint32_t reg;
  char *board;

  int num;
  int id_fd = 0;
  char id_str[256];
  bzero((void*)id_str,256);

  if ((id_fd=open("/sys/firmware/devicetree/base/model",O_RDONLY)) < 0) {
    printf("can't open /sys/firmware/devicetree/base/model\n");
    exit(1);
  }

  if ((num=read(id_fd, id_str, 256)) < 0) {
    printf("can't read /sys/firmware/devicetree/base/model\n");
    exit(1);
  }

  close(id_fd);

  if (strncmp(id_str, "Raspberry Pi Zero 2", 19) == 0) { 
      mmio_peri_base = 0x3F000000;
  } else if (strncmp(id_str, "Raspberry Pi 4 Model B", 22) == 0) { 
      mmio_peri_base = 0xFE000000; 
  } else if (strncmp(id_str, "Raspberry Pi 3 Model B Plus", 27) == 0) { 
      mmio_peri_base = 0x3F000000; 
  } else { 
      //default
      mmio_peri_base = 0x3F000000;
      printf("Pi model not known please update cpu.c with the output of\n  cat /sys/firmware/devicetree/base/model");
      printf(". Defaulting to Pi3");
  } 

    //printf("id:%s\n", id_str );
}

//
// Set up a memory regions to access GPIO
//
void setup_gpio()
{
   /* open /dev/mem */
   if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
      printf("can't open /dev/mem \n");
      exit(-1);
   }

   /* mmap GPIO */
   gpio_map = mmap(
      NULL, 
      4096,
      PROT_READ|PROT_WRITE,
      MAP_SHARED,
      mem_fd,
      GPIO_BASE
   );
   /* close fd */
   close(mem_fd); 

   if (gpio_map == MAP_FAILED) {
      printf("mmap error %d\n", (int)gpio_map);//errno also set!
      exit(-1);
   }

  /* store map pointer */
  gpio = gpio_map;

  gpio_i        = (volatile void *)gpio+0x34;
  gpio_o_set    = (volatile void *)gpio+0x1C;
  gpio_o_clear  = (volatile void *)gpio+0x28;
}

void init65C02()
{
  int g;
#ifdef DEBUG
  printf("Init65C05\n");
#endif

  detectPi();
  // Set up gpio pointer for direct register access
  setup_gpio();

  // Set GPIO pins 0-7 to output
  for (g=0; g<=7; g++) {
    INP_GPIO(g); // must use INP_GPIO before we can use OUT_GPIO
    OUT_GPIO(g);
  }

  // Set GPIO pins 8-23 to input
  for (g=8; g<=23; g++) {
    INP_GPIO(g);
  }

  // Set GPIO pins 24 to input (!RW)
  INP_GPIO(24);

  // Set GPIO pins 25 to output (CLOCK)
  INP_GPIO(25);
  OUT_GPIO(25);

  // Set GPIO pins 26 to output (!RESET)
  INP_GPIO(26);
  OUT_GPIO(26);
  GPIO_SET = 1<<26; //release !RESET

  // Set GPIO pins 27 to output (!IRQ)
  INP_GPIO(27);
  OUT_GPIO(27);
  GPIO_SET = 1<<27; //release !IRQ
  
  cpu_init = 1; 
}

static uint32_t bus = 0;

uint32_t bus_data = 0;
uint32_t bus_rw = 0;
uint32_t bus_addr = 0;
uint32_t data = 0;
uint32_t page = 0;

static uint32_t tmp;
static uint32_t vtmp;

int step65C02()
{  
  //TODO: The delays need to be optimized (scoped)

  // Clock cycle start (clock is low)
  *gpio = _65C02_gpio_data_r;

  tmp = *gpio_i;
  
  // 2nd part of clock cycle (clock goes high)
  _clockticks65C02++;
  *gpio_o_set = 1<<25;
  #ifdef ASM_FLUSH
    asm volatile ("dsb ishst" : : : "memory");
  #else
    vtmp=*gpio_o_set; //read back to flush to memory
  #endif

  // decode addr and !RW from the bus
  bus_rw = tmp & 1<<24;
  bus_addr = (tmp >> 8) & 0xFFFF;

  // Set DATA to INP or OUT based on RW
  if (bus_rw) {
    //set DATA to OUT
    *gpio = _65C02_gpio_data_w;

    data=mem_read(bus_addr);

    //write to 65C02
    *gpio_o_clear=(uint32_t) 0xff;
    *gpio_o_set=data;
    #ifdef ASM_FLUSH
      asm volatile ("dsb ishst" : : : "memory");
      ndelay20;
    #else
      vtmp=*gpio_o_set; //read back to flush to memory
    #endif

  }
  else {
    ndelay10;
    //read from 65C02
    data = *gpio_i & 0xFF;
    mem_write(bus_addr, data);
  }

  // Finish the clock cycle
  // Clock cycle start (clock goes low)
  clockticks65C02++;
  *gpio_o_clear = 1<<25;
  #ifdef ASM_FLUSH
    asm volatile ("dsb ishst" : : : "memory");
    ndelay10;
  #else
    vtmp=*gpio_o_clear; //read back to flush to memory
  #endif

  return 0;
}

void exec65C02(uint32_t tickcount)
{
  uint32_t g;
  for (g=0; g<tickcount; g++)
    step65C02();  
}

//
// one_clock: do one clock cycle (ignore everything)
//
void one_clock()
{
  *gpio_o_clear = 1<<25;
  ndelay20;
  ndelay20;
  *gpio_o_set = 1<<25;
  ndelay20;
  ndelay20;
}
void reset65C02()
{
  if (!cpu_init)
    init65C02();

#ifdef DEBUG
  printf("Reset65C02\n");
#endif

  // Perform 6502 reset
  *gpio_o_clear = 1<<26; //set !RESET
  one_clock();  
  one_clock();  
  one_clock();  
  *gpio_o_set = 1<<26; //clear !RESET
  clockticks65C02 = 0;
  _clockticks65C02 = 0;
}

//
// irq65C02: manipulate the physical irq line
//
void irq65C02(int state)
{
#ifdef DEBUG
  printf("IRQ65C05\n");
#endif

  if (state) {
    // Perform 6502 irq
    *gpio_o_clear = 1<<27;  
  }
  else {
    *gpio_o_set = 1<<27;
  }
}





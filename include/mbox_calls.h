enum {
	PI_MBOX=0x0200,
	PI_MBOX_INIT,
  	PI_MBOX_LAST_OPCODE,
};

#define ADDR_MBOX0 *(unsigned int *)0xFFF0
#define ADDR_MBOX1 *(unsigned int *)0xFFF2
#define ADDR_MBOX2 *(unsigned int *)0xFFF4
#define ADDR_MBOX3 *(unsigned int *)0xFFF6

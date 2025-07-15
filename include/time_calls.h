enum {
	PI_TIME=0x0A00,
	PI_TIME_INIT,
  	PI_TIME_LAST_OPCODE,
};

#define PI_TIME_NS *(long *)0xFF20
#define PI_TIME_MS *(unsigned long *)0xFF28
#define PI_TIME_S  *(long *)0xFF24

#define PI_TIME_DEADLINE *(unsigned int *)0xFF2C
#define PI_TIME_DEADLINE_PASSED *(unsigned char *)0xFF2C

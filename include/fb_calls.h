enum {
	PI_FB=0x0900,
	PI_FB_INIT,
    PI_FB_POINT,
    PI_FB_POINTRGB,
	PI_FB_LINE,
	PI_FB_CIRCLE,
	PI_FB_WAITFORVSYNC,
	PI_FB_CLS,
	PI_FB_BLOCK,
	PI_FB_MALLOC_SBUF,
    
	PI_FB_LAST_OPCODE,
};

enum {
	PI_FB_ASYNC=0x1900,

	PI_FB_ASYNC_LAST_OPCODE,
};

#define PI_FB_SIZE_X *(unsigned int*)0xFF00
#define PI_FB_SIZE_Y *(unsigned int*)0xFF02

#define PI_FB_CMD_PAN *(unsigned char*)0xFF0B

#define PI_FB_COLOR_BACKGROUND *(unsigned int*)0xFF0F
#define PI_FB_COLOR_BRUSH *(unsigned int*)0xFF0C

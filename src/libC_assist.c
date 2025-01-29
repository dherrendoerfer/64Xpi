/* badXpi
 *
 * GPLv3
 * 
 */

#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "mem.h"
#include "fb.h"
#include "time.h"
#include "mbox.h"
#include "mmu.h"
#include "../include/libC_calls.h"
#include "../include/fb_calls.h"
#include "../include/time_calls.h"
#include "../include/mbox_calls.h"
#include "../include/mmu_calls.h"

int cb_puts(uint16_t addr)
{
    uint16_t text=*(uint16_t*)mem_base(addr+2);
    printf("%s",(char*)mem_base(text));

    return 0;
}

int cb_open(uint16_t addr)
{
    int fd;
    uint16_t sys = *(uint16_t*)mem_base(addr+2);
    uint16_t pathname=*(uint16_t*)mem_base(sys);
    uint16_t flags=*(uint16_t*)mem_base(sys+2);
    //printf("open: pathname=%s, flags=0x%02x\n",
    //            (char*)mem_base(pathname),flags);

    return open((char*)mem_base(pathname), flags);
}

int cb_close(uint16_t addr)
{
    uint16_t sys = *(uint16_t*)mem_base(addr+2);
    uint16_t fd=*(uint16_t*)mem_base(sys);
    //printf("close: fd=%i\n",fd);

    return close(fd);
}

int cb_write(uint16_t addr)
{
    uint16_t sys = *(uint16_t*)mem_base(addr+2);
    uint16_t fd=*(uint16_t*)mem_base(sys);
    uint16_t buff=*(uint16_t*)mem_base(sys+2);
    uint16_t size=*(uint16_t*)mem_base(sys+4);
    //printf("write: fd=%i, buffer ,size=%i\n", fd, size);

    return write(fd,(char*)mem_base(buff), size);
}

int cb_read(uint16_t addr)
{
    uint16_t sys = *(uint16_t*)mem_base(addr+2);
    uint16_t fd=*(uint16_t*)mem_base(sys);
    uint16_t buff=*(uint16_t*)mem_base(sys+2);
    uint16_t size=*(uint16_t*)mem_base(sys+4);
    //printf("read: fd=%i, buffer ,size=%i\n", fd, size);

    return read(fd,(char*)mem_base(buff), size);
}

int cb_chdir(uint16_t addr)
{
    uint16_t sys = *(uint16_t*)mem_base(addr+2);
    uint16_t dirname_ptr=*(uint16_t*)mem_base(sys);
    //printf("chdir: dirname=%s\n", mem_base(dirname_ptr));

    return chdir((char*)mem_base(dirname_ptr));
}

int cb_getcwd(uint16_t addr)
{
    char* ret;
    uint16_t sys = *(uint16_t*)mem_base(addr+2);
    uint16_t dirname_ptr=*(uint16_t*)mem_base(sys);
    uint16_t size=*(uint16_t*)mem_base(sys+2);
    //printf("getcwd: dirname, size=%i\n", size);

    ret=getcwd((char*)mem_base(dirname_ptr), size);

    if (ret)
      return 0;

    return 255;
}

int cb_system(uint16_t addr)
{
    uint16_t sys = *(uint16_t*)mem_base(addr+2);
    uint16_t command_ptr=*(uint16_t*)mem_base(sys);
    //printf("system: command=%s\n", mem_base(command_ptr));

    return system((char*)mem_base(command_ptr));
}

int cb_lseek(uint16_t addr)
{
    uint16_t sys = *(uint16_t*)mem_base(addr+2);
    uint16_t fd=*(uint16_t*)mem_base(sys);
    uint16_t offset=*(uint16_t*)mem_base(sys+2);
    uint16_t whence=*(uint16_t*)mem_base(sys+4);
    //printf("lseek: fd=%i, offset=%i, whence=%i\n", fd, offset, whence);

    return lseek(fd, offset, whence);
}

int cb_unlink(uint16_t addr)
{
    uint16_t sys = *(uint16_t*)mem_base(addr+2);
    uint16_t filename_ptr=*(uint16_t*)mem_base(sys);
    //printf("unlink: filename=%s\n", mem_base(filename_ptr));

    return unlink((char*)mem_base(filename_ptr));
}

int cb_sysexit(uint16_t addr)
{
    uint16_t sys = *(uint16_t*)mem_base(addr+2);
    uint16_t rc=*(uint16_t*)mem_base(sys);
    //printf("sysexit: rc=%i\n", rc);

    exit(rc);
    return 0; // make the compiler happy
}

uint8_t sync_libC_assist()
{    
    uint16_t addr=*(uint16_t*)mem_base(0xFFF8);
    uint16_t data=*(uint16_t*)mem_base(addr);

    //printf("\nCALLBACK %04X\n",data);

    switch(data & 0xFF00)
    {
      case PI_FB:
        return sync_fb_assist();
      case PI_TIME:
        return sync_time_assist();
      case PI_MMU:
        return sync_mmu_assist();
      case PI_MBOX:
        return sync_mbox_assist();
    }
 
    switch (data)
    {
    case PI_LIBC_PUTS:
        return cb_puts(addr);
    case PI_LIBC_OPEN:
        return cb_open(addr);
    case PI_LIBC_CLOSE:
        return cb_close(addr);
    case PI_LIBC_WRITE:
        return cb_write(addr);
    case PI_LIBC_READ:
        return cb_read(addr);
    case PI_LIBC_CHDIR:
        return cb_chdir(addr);
    case PI_LIBC_GETCWD:
        return cb_getcwd(addr);
    case PI_LIBC_SYSTEM:
        return cb_system(addr);
    case PI_LIBC_LSEEK:
        return cb_lseek(addr);
    case PI_LIBC_UNLINK:
        return cb_unlink(addr);


    case PI_LIBC_SYSEXIT:
        return cb_sysexit(addr);

    default:
        return 255;
    }

    return 0;
}

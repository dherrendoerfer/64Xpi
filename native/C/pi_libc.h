/*
*  Callbacks
*
*  
*/

#include <stdio.h>
#include "../../include/libC_calls.h"

unsigned char call_buff[40];

struct callback{
    unsigned int id;
    unsigned int data_ptr;
} cbk;

int send_for_pi(unsigned int _callid, void *data)
{
    unsigned int *cb = (unsigned int*)0xFFF8;
    unsigned int ret;

    cbk.id=_callid;
    cbk.data_ptr=(unsigned int)data;
    *cb = (unsigned int)&cbk;

    ret = *(unsigned char*)0xFFF8;
    return ret;
}

#define CHAR_CLS "\033c"
#define CHAR_HOME "\033[h"
#define CHAR_CURSOR_HIDE "\033[?25l"
#define CHAR_CURSOR_SHOW "\033[?25h"
#define CHAR_HPOS(n)  "\033["#n"G"
#define CHAR_VPOS(n)  "\033["#n"d"
#define CHAR_SREGION(t,b) "\033["#t";"#b"r"


// .... need more 

int puts(const char *data)
{
    return send_for_pi(PI_LIBC_PUTS, data);
}

// cheap helper
#define putchar(pcha) { char pfbuf[]={(char)pcha,0}; puts(pfbuf); }

typedef struct {
	unsigned int    pathname_ptr;
	unsigned int    flags;
} syscall_open_t;

/* Flag values for the open() call */
#define O_RDONLY        0x01
#define O_WRONLY        0x02
#define O_RDWR          0x03
#define O_CREAT         0x10
#define O_TRUNC         0x20
#define O_APPEND        0x40
#define O_EXCL          0x80

#define STDIN           0
#define STDOUT          1
#define STDERR          2

#define stdin           0
#define stdout          1
#define stderr          2


// cheap helper
#define printf(...) { char pfbuf[128]; sprintf(pfbuf,__VA_ARGS__);puts(pfbuf); }

int open(const char *pathname, unsigned int flags)
{
    syscall_open_t *sys;
    sys=(void*)call_buff;

    sys->pathname_ptr = (unsigned int)pathname;
    sys->flags =        (unsigned int)flags;

    return send_for_pi(PI_LIBC_OPEN, sys);
}

typedef struct {
	unsigned int    fd;
} syscall_close_t;

int close(int fd)
{
    syscall_close_t *sys;
    sys=(void*)call_buff;

    sys->fd =        (unsigned int)fd;

    return send_for_pi(PI_LIBC_CLOSE, sys);
}

typedef struct {
    unsigned int    fd;
	unsigned int    buf_ptr;
	unsigned int    count;
} syscall_write_t;

int write(int fd, const void *buf, size_t count)
{
    syscall_write_t *sys;
    sys=(void*)call_buff;

    sys->fd =        (unsigned int)fd;
    sys->buf_ptr =   (unsigned int)buf;
    sys->count =     (unsigned int)count;

    return send_for_pi(PI_LIBC_WRITE, sys);
}

typedef struct {
    unsigned int    fd;
	unsigned int    buf_ptr;
	unsigned int    count;
} syscall_read_t;

int read(int fd, const void *buf, size_t count)
{
    syscall_read_t *sys;
    sys=(void*)call_buff;
    
    sys->fd =        (unsigned int)fd;
    sys->buf_ptr =   (unsigned int)buf;
    sys->count =     (unsigned int)count;

    return send_for_pi(PI_LIBC_READ, sys);
}

// helper
#undef getchar
int getchar()
{
    char gcbuf[] = {0,0};

    if (read(stdin, gcbuf, 1) < 0)
      return 255;
    return gcbuf[0];
}

typedef struct {
 	unsigned int    dirname_ptr;
 } syscall_chdir_t;

int chdir(const char* dirname)
{
    syscall_chdir_t *sys;
    sys=(void*)call_buff;
    
    sys->dirname_ptr =   (unsigned int)dirname;

    return send_for_pi(PI_LIBC_CHDIR, sys);
}

typedef struct {
 	unsigned int    dirname_ptr;
 	unsigned int    size;
 } syscall_getcwd_t;

int getcwd(const char* dirname, unsigned int size)
{
    syscall_getcwd_t *sys;
    sys=(void*)call_buff;
    
    sys->dirname_ptr =  (unsigned int)dirname;
    sys->size =         (unsigned int)size;

    return send_for_pi(PI_LIBC_GETCWD, sys);
}

typedef struct {
 	unsigned int    command_ptr;
 } syscall_system_t;

int system(const char* command)
{
    syscall_system_t *sys;
    sys=(void*)call_buff;
    
    sys->command_ptr =  (unsigned int)command;

    return send_for_pi(PI_LIBC_SYSTEM, sys);
}

typedef struct {
 	unsigned int    fd;
 	unsigned int    offset;
    unsigned int    whence;
 } syscall_lseek_t;

#undef SEEK_SET
#undef SEEK_CUR
#undef SEEK_END
#define SEEK_SET       0       /* Seek from beginning of file.  */
#define SEEK_CUR       1       /* Seek from current position.  */
#define SEEK_END       2       /* Seek from end of file.  */
#define SEEK_DATA      3       /* Seek to next data.  */
#define SEEK_HOLE      4       /* Seek to next hole.  */

int lseek(unsigned int fd, unsigned int offset, unsigned int whence)
{
    syscall_lseek_t *sys;
    sys=(void*)call_buff;
    
    sys->fd =           (unsigned int)fd;
    sys->offset =       (unsigned int)offset;
    sys->whence =       (unsigned int)whence;

    return send_for_pi(PI_LIBC_LSEEK, sys);
}

typedef struct {
 	unsigned int    filename_ptr;
 } syscall_unlink_t;

int unlink(const char* filename)
{
    syscall_unlink_t *sys;
    sys=(void*)call_buff;
    
    sys->filename_ptr =   (unsigned int)filename;

    return send_for_pi(PI_LIBC_UNLINK, sys);
}

typedef struct {
    unsigned int    rc;
} syscall_sysexit_t;

int sysexit(int rc)
{
    syscall_sysexit_t *sys;
    sys=(void*)call_buff;
    
    sys->rc =        (unsigned int)rc;

    return send_for_pi(PI_LIBC_SYSEXIT, sys);
}


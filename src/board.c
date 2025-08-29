/* badXpi
 *
 * GPLv3
 * 
 */
 
 #include <stdint.h>

void _start()
{
    uint32_t reg;
    uint32_t *mmio_base;
    char *board;

    /* read the system register */
#if __AARCH64__
    asm volatile ("mrs %x0, midr_el1" : "=r" (reg));
#else
    asm volatile ("mrc p15,0,%0,c0,c0,0" : "=r" (reg));
#endif

    /* get the PartNum, detect board and MMIO base address */
    switch ((reg >> 4) & 0xFFF) {
        case 0xB76: board = "Rpi1"; mmio_base = 0x20000000; break;
        case 0xC07: board = "Rpi2"; mmio_base = 0x3F000000; break;
        case 0xD03: board = "Rpi3"; mmio_base = 0x3F000000; break;
        case 0xD08: board = "Rpi4"; mmio_base = 0xFE000000; break;
        default:    board = "????"; mmio_base = 0x20000000; break;
    }
}
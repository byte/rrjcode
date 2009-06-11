#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <byteswap.h>
#include <linux/fb.h>
#include <errno.h>
#include "radeon.h"

/*
include drivers/video/radeonfb.h if you're using 2.4

Configure X to have dual head, then run this to make things go away. For Radeon Mobility 7500, iBook G3 2.2 release.

06th Feb 2004 
*/

void * mmio;

#define RADEON_RBASE	0xb0000000
//#define RADEON_RBASE	0x90000000

static inline unsigned char regr8(void *base, const unsigned long offset)
{
        unsigned char val; 
        __asm__ __volatile__(
                        "lbzx %0,%1,%2\n\t"
                        "eieio"
                        : "=r" (val)
                        : "b" (base), "r" (offset),
                        "m" (*((volatile unsigned char *)base+offset)));
        return(val);
}


static inline void regw8(void *base, const unsigned long offset,
			const unsigned char val)
{
	__asm__ __volatile__ ("stbx %1,%2,%3\n\t"
				"eieio"
                        : "=m" (*((volatile unsigned char *)base+offset))
                        : "r" (val), "b" (base), "r" (offset));
}


static inline unsigned int regr(void *base, const unsigned long offset)
{
	unsigned int val;

	__asm__ __volatile__ ("lwbrx %0,%1,%2\n\t"
				"eieio"
				: "=r" (val)
				: "b" (base), "r"(offset),
				"m" (*((volatile unsigned char *)base+offset)));
	return val;
}

static inline void regw(void *base, const unsigned long offset,
				const unsigned int val)
{
	__asm__ __volatile__ ("stwbrx %1,%2,%3\n\t"
				"eieio"
				: "=m" (*((volatile unsigned char *)base+offset))
				: "r"(val), "b"(base), "r"(offset));
}

static inline unsigned int pllr(void *base, const unsigned long index)
{
	unsigned int val;

	regw8(base, 0x8, index & 0x3f);
	return regr(base, 0xc);
}

#define INREG(r)	regr(mmio, (r))
#define OUTREG(r, v)	regw(mmio, (r), (v))
#define INREG8(r)	regr8(mmio, (r))
#define OUTREG8(r, v)	regw8(mmio, (r), (v))

#define OUTPLL(addr,val)	\
	do {	\
		OUTREG8(CLOCK_CNTL_INDEX, (addr & 0x0000003f) | 0x00000080); \
		OUTREG(CLOCK_CNTL_DATA, val); \
	} while(0)

static inline unsigned long _INPLL(unsigned long addr)
{
	OUTREG8(CLOCK_CNTL_INDEX, addr & 0x0000003f);
	return (INREG(CLOCK_CNTL_DATA));
}

#define INPLL(addr)		_INPLL(addr)


int main() {
	int fd, fbfd, i;
	unsigned int cci;
	struct fb_fix_screeninfo fix;
	
	fbfd = open("/dev/fb0", O_RDWR);
	if (fbfd == -1) {
		perror("cannot open framebuffer\n");
		return ENODEV;
	}
	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &fix)) {
		perror("cannot get sceen info\n");
		return ENODEV;
	}
	if (fix.accel != FB_ACCEL_ATI_RADEON) {
		fprintf(stderr, "not a radeon !\n");
		return ENODEV;
	}
	fd = open("/dev/mem", O_RDWR);
	if (fd == -1) {
		perror("cannot open /dev/mem\n");
		return ENODEV;
	}
	mmio = (void *)mmap(0, 0x80000, PROT_READ|PROT_WRITE,
			MAP_SHARED, fd, fix.mmio_start);
	/* Force switch to PLL0 */
	printf("Frobbing radeon PLL...\n");
	cci = INREG(8);
	OUTREG(8, cci & ~0x00000300);
}

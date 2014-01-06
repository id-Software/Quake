/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include <sys/segments.h>
#include <go32.h>
#include <unistd.h>
#include <sys/nearptr.h>
#include <dos.h>
#include <string.h>
#include <dpmi.h>
// #include <osfcn.h>
#include <bios.h>

#include "dosisms.h"

_go32_dpmi_registers hmm;

// globals
regs_t regs;
void (*dos_error_func)(char *msg, ...);

static unsigned conventional_memory = -1;

__dpmi_regs callback_regs;

void map_in_conventional_memory(void)
{
	if (conventional_memory == -1)
	{
		if (__djgpp_nearptr_enable())
		{
			conventional_memory = __djgpp_conventional_base;
		}
	}
}

unsigned int ptr2real(void *ptr)
{
	map_in_conventional_memory();
	return (int)ptr - conventional_memory;
}

void *real2ptr(unsigned int real)
{
	map_in_conventional_memory();
	return (void *) (real + conventional_memory);
}

void *far2ptr(unsigned int farptr)
{
	return real2ptr(((farptr & ~0xffff) >>12) + (farptr&0xffff));
}

unsigned int ptr2far(void *ptr)
{
	return ((ptr2real(ptr)&~0xf) << 12) + (ptr2real(ptr) & 0xf);
}

int dos_inportb(int port)
{
	return inportb(port);
}

int dos_inportw(int port)
{
	return inportw(port);
}

void dos_outportb(int port, int val)
{
	outportb(port, val);
}

void dos_outportw(int port, int val)
{
	outportw(port, val);
}

void dos_irqenable(void)
{
	enable();
}

void dos_irqdisable(void)
{
	disable();
}

//
// Returns 0 on success
//

int	dos_int86(int vec)
{
    int rc;
    regs.x.ss = regs.x.sp = 0;
    rc = _go32_dpmi_simulate_int(vec, (_go32_dpmi_registers *) &regs);
    return rc || (regs.x.flags & 1);
}

int	dos_int386(int vec, regs_t *inregs, regs_t *outregs)
{
	int rc;
	memcpy(outregs, inregs, sizeof(regs_t));
	outregs->x.ss = outregs->x.sp = 0;
	rc = _go32_dpmi_simulate_int(vec, (_go32_dpmi_registers *) outregs);
	return rc || (outregs->x.flags & 1);
}

//
// Because of a quirk in dj's alloc-dos-memory wrapper, you need to keep
// the seginfo structure around for when you free the mem.
//

static _go32_dpmi_seginfo seginfo[10];

void *dos_getmemory(int size)
{

	int rc;
	_go32_dpmi_seginfo info;
	static int firsttime=1;
	int i;

	if (firsttime)
	{
		memset(seginfo, 0, sizeof(seginfo));
		firsttime = 0;
	}

	info.size = (size+15) / 16;
	rc = _go32_dpmi_allocate_dos_memory(&info);
	if (rc)
		return 0;

	for (i=0;i<10;i++)
		if (!seginfo[i].rm_segment) break;
	seginfo[i] = info;
	return real2ptr((int) info.rm_segment << 4);

}

void dos_freememory(void *ptr)
{

	int i;
	int segment;

	segment = ptr2real(ptr) >> 4;
	for (i=0 ; i<10 ; i++)
		if (seginfo[i].rm_segment == segment)
		{
			_go32_dpmi_free_dos_memory(&seginfo[i]);
			seginfo[i].rm_segment = 0;
			break;
		}

}

static struct handlerhistory_s
{
	int intr;
	_go32_dpmi_seginfo pm_oldvec;
} handlerhistory[4];

static int handlercount=0;

void	dos_registerintr(int intr, void (*handler)(void))
{
	_go32_dpmi_seginfo info;
	struct handlerhistory_s *oldstuff;

	oldstuff = &handlerhistory[handlercount];

// remember old handler
	_go32_dpmi_get_protected_mode_interrupt_vector(intr, &oldstuff->pm_oldvec);
	oldstuff->intr = intr;

	info.pm_offset = (int) handler;
	_go32_dpmi_allocate_iret_wrapper(&info);

// set new protected mode handler
	_go32_dpmi_set_protected_mode_interrupt_vector(intr, &info);

	handlercount++;

}

void	dos_restoreintr(int intr)
{

	int i;
	struct handlerhistory_s *oldstuff;

// find and reinstall previous interrupt
	for (i=0 ; i<handlercount ; i++)
	{
		oldstuff = &handlerhistory[i];
		if (oldstuff->intr == intr)
		{
			_go32_dpmi_set_protected_mode_interrupt_vector(intr,
				&oldstuff->pm_oldvec);
			oldstuff->intr = -1;
			break;
		}
	}

}

void	dos_usleep(int usecs)
{
	usleep(usecs);
}

int dos_getheapsize(void)
{
	return _go32_dpmi_remaining_physical_memory();
}

int dos_lockmem(void *addr, int size)
{
	__dpmi_meminfo info;
	info.address = (long) addr + __djgpp_base_address;
	info.size = size;
	if (__dpmi_lock_linear_region(&info))
		return __dpmi_error;
	else
		return 0;
}

int dos_unlockmem(void *addr, int size)
{
	__dpmi_meminfo info;
	info.address = (long) addr + __djgpp_base_address;
	info.size = size;
	if (__dpmi_unlock_linear_region(&info))
		return __dpmi_error;
	else
		return 0;
}


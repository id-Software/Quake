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

#include "quakedef.h"

extern int desired_speed;
extern int desired_bits;

qboolean SNDDMA_Init(void)
{
	int size;

	size = 16384 + sizeof(dma_t);
	shm = malloc (size);
	memset((void*)shm, 0, size);

	shm->buffer = (char*)shm + sizeof(dma_t);
	shm->channels = 2;
	shm->speed = desired_speed;
	shm->samplebits = desired_bits;
	shm->samples = 16384 / (desired_bits / 8);
	shm->submission_chunk = 1;

	return true;
}

// return the current sample position (in mono samples read)
// inside the recirculating dma buffer
int SNDDMA_GetDMAPos(void)
{
	shm->samplepos = (int)(realtime*shm->speed*shm->channels) & (shm->samples-1);
	
	return shm->samplepos;
}

void SNDDMA_Shutdown(void)
{
}

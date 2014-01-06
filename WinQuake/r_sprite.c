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
// r_sprite.c

#include "quakedef.h"
#include "r_local.h"

static int				clip_current;
static vec5_t			clip_verts[2][MAXWORKINGVERTS];
static int				sprite_width, sprite_height;

spritedesc_t			r_spritedesc;
	

/*
================
R_RotateSprite
================
*/
void R_RotateSprite (float beamlength)
{
	vec3_t	vec;
	
	if (beamlength == 0.0)
		return;

	VectorScale (r_spritedesc.vpn, -beamlength, vec);
	VectorAdd (r_entorigin, vec, r_entorigin);
	VectorSubtract (modelorg, vec, modelorg);
}


/*
=============
R_ClipSpriteFace

Clips the winding at clip_verts[clip_current] and changes clip_current
Throws out the back side
==============
*/
int R_ClipSpriteFace (int nump, clipplane_t *pclipplane)
{
	int		i, outcount;
	float	dists[MAXWORKINGVERTS+1];
	float	frac, clipdist, *pclipnormal;
	float	*in, *instep, *outstep, *vert2;

	clipdist = pclipplane->dist;
	pclipnormal = pclipplane->normal;
	
// calc dists
	if (clip_current)
	{
		in = clip_verts[1][0];
		outstep = clip_verts[0][0];
		clip_current = 0;
	}
	else
	{
		in = clip_verts[0][0];
		outstep = clip_verts[1][0];
		clip_current = 1;
	}
	
	instep = in;
	for (i=0 ; i<nump ; i++, instep += sizeof (vec5_t) / sizeof (float))
	{
		dists[i] = DotProduct (instep, pclipnormal) - clipdist;
	}
	
// handle wraparound case
	dists[nump] = dists[0];
	Q_memcpy (instep, in, sizeof (vec5_t));


// clip the winding
	instep = in;
	outcount = 0;

	for (i=0 ; i<nump ; i++, instep += sizeof (vec5_t) / sizeof (float))
	{
		if (dists[i] >= 0)
		{
			Q_memcpy (outstep, instep, sizeof (vec5_t));
			outstep += sizeof (vec5_t) / sizeof (float);
			outcount++;
		}

		if (dists[i] == 0 || dists[i+1] == 0)
			continue;

		if ( (dists[i] > 0) == (dists[i+1] > 0) )
			continue;
			
	// split it into a new vertex
		frac = dists[i] / (dists[i] - dists[i+1]);
			
		vert2 = instep + sizeof (vec5_t) / sizeof (float);
		
		outstep[0] = instep[0] + frac*(vert2[0] - instep[0]);
		outstep[1] = instep[1] + frac*(vert2[1] - instep[1]);
		outstep[2] = instep[2] + frac*(vert2[2] - instep[2]);
		outstep[3] = instep[3] + frac*(vert2[3] - instep[3]);
		outstep[4] = instep[4] + frac*(vert2[4] - instep[4]);

		outstep += sizeof (vec5_t) / sizeof (float);
		outcount++;
	}	
	
	return outcount;
}


/*
================
R_SetupAndDrawSprite
================
*/
void R_SetupAndDrawSprite ()
{
	int			i, nump;
	float		dot, scale, *pv;
	vec5_t		*pverts;
	vec3_t		left, up, right, down, transformed, local;
	emitpoint_t	outverts[MAXWORKINGVERTS+1], *pout;

	dot = DotProduct (r_spritedesc.vpn, modelorg);

// backface cull
	if (dot >= 0)
		return;

// build the sprite poster in worldspace
	VectorScale (r_spritedesc.vright, r_spritedesc.pspriteframe->right, right);
	VectorScale (r_spritedesc.vup, r_spritedesc.pspriteframe->up, up);
	VectorScale (r_spritedesc.vright, r_spritedesc.pspriteframe->left, left);
	VectorScale (r_spritedesc.vup, r_spritedesc.pspriteframe->down, down);

	pverts = clip_verts[0];

	pverts[0][0] = r_entorigin[0] + up[0] + left[0];
	pverts[0][1] = r_entorigin[1] + up[1] + left[1];
	pverts[0][2] = r_entorigin[2] + up[2] + left[2];
	pverts[0][3] = 0;
	pverts[0][4] = 0;

	pverts[1][0] = r_entorigin[0] + up[0] + right[0];
	pverts[1][1] = r_entorigin[1] + up[1] + right[1];
	pverts[1][2] = r_entorigin[2] + up[2] + right[2];
	pverts[1][3] = sprite_width;
	pverts[1][4] = 0;

	pverts[2][0] = r_entorigin[0] + down[0] + right[0];
	pverts[2][1] = r_entorigin[1] + down[1] + right[1];
	pverts[2][2] = r_entorigin[2] + down[2] + right[2];
	pverts[2][3] = sprite_width;
	pverts[2][4] = sprite_height;

	pverts[3][0] = r_entorigin[0] + down[0] + left[0];
	pverts[3][1] = r_entorigin[1] + down[1] + left[1];
	pverts[3][2] = r_entorigin[2] + down[2] + left[2];
	pverts[3][3] = 0;
	pverts[3][4] = sprite_height;

// clip to the frustum in worldspace
	nump = 4;
	clip_current = 0;

	for (i=0 ; i<4 ; i++)
	{
		nump = R_ClipSpriteFace (nump, &view_clipplanes[i]);
		if (nump < 3)
			return;
		if (nump >= MAXWORKINGVERTS)
			Sys_Error("R_SetupAndDrawSprite: too many points");
	}

// transform vertices into viewspace and project
	pv = &clip_verts[clip_current][0][0];
	r_spritedesc.nearzi = -999999;

	for (i=0 ; i<nump ; i++)
	{
		VectorSubtract (pv, r_origin, local);
		TransformVector (local, transformed);

		if (transformed[2] < NEAR_CLIP)
			transformed[2] = NEAR_CLIP;

		pout = &outverts[i];
		pout->zi = 1.0 / transformed[2];
		if (pout->zi > r_spritedesc.nearzi)
			r_spritedesc.nearzi = pout->zi;

		pout->s = pv[3];
		pout->t = pv[4];
		
		scale = xscale * pout->zi;
		pout->u = (xcenter + scale * transformed[0]);

		scale = yscale * pout->zi;
		pout->v = (ycenter - scale * transformed[1]);

		pv += sizeof (vec5_t) / sizeof (*pv);
	}

// draw it
	r_spritedesc.nump = nump;
	r_spritedesc.pverts = outverts;
	D_DrawSprite ();
}


/*
================
R_GetSpriteframe
================
*/
mspriteframe_t *R_GetSpriteframe (msprite_t *psprite)
{
	mspritegroup_t	*pspritegroup;
	mspriteframe_t	*pspriteframe;
	int				i, numframes, frame;
	float			*pintervals, fullinterval, targettime, time;

	frame = currententity->frame;

	if ((frame >= psprite->numframes) || (frame < 0))
	{
		Con_Printf ("R_DrawSprite: no such frame %d\n", frame);
		frame = 0;
	}

	if (psprite->frames[frame].type == SPR_SINGLE)
	{
		pspriteframe = psprite->frames[frame].frameptr;
	}
	else
	{
		pspritegroup = (mspritegroup_t *)psprite->frames[frame].frameptr;
		pintervals = pspritegroup->intervals;
		numframes = pspritegroup->numframes;
		fullinterval = pintervals[numframes-1];

		time = cl.time + currententity->syncbase;

	// when loading in Mod_LoadSpriteGroup, we guaranteed all interval values
	// are positive, so we don't have to worry about division by 0
		targettime = time - ((int)(time / fullinterval)) * fullinterval;

		for (i=0 ; i<(numframes-1) ; i++)
		{
			if (pintervals[i] > targettime)
				break;
		}

		pspriteframe = pspritegroup->frames[i];
	}

	return pspriteframe;
}


/*
================
R_DrawSprite
================
*/
void R_DrawSprite (void)
{
	int				i;
	msprite_t		*psprite;
	vec3_t			tvec;
	float			dot, angle, sr, cr;

	psprite = currententity->model->cache.data;

	r_spritedesc.pspriteframe = R_GetSpriteframe (psprite);

	sprite_width = r_spritedesc.pspriteframe->width;
	sprite_height = r_spritedesc.pspriteframe->height;

// TODO: make this caller-selectable
	if (psprite->type == SPR_FACING_UPRIGHT)
	{
	// generate the sprite's axes, with vup straight up in worldspace, and
	// r_spritedesc.vright perpendicular to modelorg.
	// This will not work if the view direction is very close to straight up or
	// down, because the cross product will be between two nearly parallel
	// vectors and starts to approach an undefined state, so we don't draw if
	// the two vectors are less than 1 degree apart
		tvec[0] = -modelorg[0];
		tvec[1] = -modelorg[1];
		tvec[2] = -modelorg[2];
		VectorNormalize (tvec);
		dot = tvec[2];	// same as DotProduct (tvec, r_spritedesc.vup) because
						//  r_spritedesc.vup is 0, 0, 1
		if ((dot > 0.999848) || (dot < -0.999848))	// cos(1 degree) = 0.999848
			return;
		r_spritedesc.vup[0] = 0;
		r_spritedesc.vup[1] = 0;
		r_spritedesc.vup[2] = 1;
		r_spritedesc.vright[0] = tvec[1];
								// CrossProduct(r_spritedesc.vup, -modelorg,
		r_spritedesc.vright[1] = -tvec[0];
								//              r_spritedesc.vright)
		r_spritedesc.vright[2] = 0;
		VectorNormalize (r_spritedesc.vright);
		r_spritedesc.vpn[0] = -r_spritedesc.vright[1];
		r_spritedesc.vpn[1] = r_spritedesc.vright[0];
		r_spritedesc.vpn[2] = 0;
					// CrossProduct (r_spritedesc.vright, r_spritedesc.vup,
					//  r_spritedesc.vpn)
	}
	else if (psprite->type == SPR_VP_PARALLEL)
	{
	// generate the sprite's axes, completely parallel to the viewplane. There
	// are no problem situations, because the sprite is always in the same
	// position relative to the viewer
		for (i=0 ; i<3 ; i++)
		{
			r_spritedesc.vup[i] = vup[i];
			r_spritedesc.vright[i] = vright[i];
			r_spritedesc.vpn[i] = vpn[i];
		}
	}
	else if (psprite->type == SPR_VP_PARALLEL_UPRIGHT)
	{
	// generate the sprite's axes, with vup straight up in worldspace, and
	// r_spritedesc.vright parallel to the viewplane.
	// This will not work if the view direction is very close to straight up or
	// down, because the cross product will be between two nearly parallel
	// vectors and starts to approach an undefined state, so we don't draw if
	// the two vectors are less than 1 degree apart
		dot = vpn[2];	// same as DotProduct (vpn, r_spritedesc.vup) because
						//  r_spritedesc.vup is 0, 0, 1
		if ((dot > 0.999848) || (dot < -0.999848))	// cos(1 degree) = 0.999848
			return;
		r_spritedesc.vup[0] = 0;
		r_spritedesc.vup[1] = 0;
		r_spritedesc.vup[2] = 1;
		r_spritedesc.vright[0] = vpn[1];
										// CrossProduct (r_spritedesc.vup, vpn,
		r_spritedesc.vright[1] = -vpn[0];	//  r_spritedesc.vright)
		r_spritedesc.vright[2] = 0;
		VectorNormalize (r_spritedesc.vright);
		r_spritedesc.vpn[0] = -r_spritedesc.vright[1];
		r_spritedesc.vpn[1] = r_spritedesc.vright[0];
		r_spritedesc.vpn[2] = 0;
					// CrossProduct (r_spritedesc.vright, r_spritedesc.vup,
					//  r_spritedesc.vpn)
	}
	else if (psprite->type == SPR_ORIENTED)
	{
	// generate the sprite's axes, according to the sprite's world orientation
		AngleVectors (currententity->angles, r_spritedesc.vpn,
					  r_spritedesc.vright, r_spritedesc.vup);
	}
	else if (psprite->type == SPR_VP_PARALLEL_ORIENTED)
	{
	// generate the sprite's axes, parallel to the viewplane, but rotated in
	// that plane around the center according to the sprite entity's roll
	// angle. So vpn stays the same, but vright and vup rotate
		angle = currententity->angles[ROLL] * (M_PI*2 / 360);
		sr = sin(angle);
		cr = cos(angle);

		for (i=0 ; i<3 ; i++)
		{
			r_spritedesc.vpn[i] = vpn[i];
			r_spritedesc.vright[i] = vright[i] * cr + vup[i] * sr;
			r_spritedesc.vup[i] = vright[i] * -sr + vup[i] * cr;
		}
	}
	else
	{
		Sys_Error ("R_DrawSprite: Bad sprite type %d", psprite->type);
	}

	R_RotateSprite (psprite->beamlength);

	R_SetupAndDrawSprite ();
}


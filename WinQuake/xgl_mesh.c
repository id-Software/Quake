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
// xgl_mesh.c: bounded, memory-only alias command generation

#include "quakedef.h"
#include "xgl_local.h"
#include <string.h>

/*
=================================================================

ALIAS MODEL DISPLAY LIST GENERATION

=================================================================
*/

static aliashdr_t *paliashdr;

static qboolean used[MAXALIASTRIS];

// the command list holds counts and s/t values that are valid for
// every frame
static int commands[MAXALIASTRIS * 7 + 1];
static int numcommands;

// all frames will have their vertexes rearranged and expanded
// so they are in the order expected by the command list
static int vertexorder[MAXALIASTRIS * 3];
static int numorder;



static int stripverts[MAXALIASTRIS + 2];
static int striptris[MAXALIASTRIS];
static int stripcount;

/*
================
StripLength
================
*/
static int StripLength (int starttri, int startv)
{
	int			m1, m2;
	int			j;
	mtriangle_t	*last, *check;
	int			k;

	used[starttri] = 2;

	last = &triangles[starttri];

	stripverts[0] = last->vertindex[(startv)%3];
	stripverts[1] = last->vertindex[(startv+1)%3];
	stripverts[2] = last->vertindex[(startv+2)%3];

	striptris[0] = starttri;
	stripcount = 1;

	m1 = last->vertindex[(startv+2)%3];
	m2 = last->vertindex[(startv+1)%3];

	// look for a matching triangle
nexttri:
	for (j=starttri+1, check=&triangles[starttri+1] ; j<pheader->numtris ; j++, check++)
	{
		if (check->facesfront != last->facesfront)
			continue;
		for (k=0 ; k<3 ; k++)
		{
			if (check->vertindex[k] != m1)
				continue;
			if (check->vertindex[ (k+1)%3 ] != m2)
				continue;

			// this is the next part of the fan

			// if we can't use this triangle, this tristrip is done
			if (used[j])
				goto done;

			// the new edge
			if (stripcount & 1)
				m2 = check->vertindex[ (k+2)%3 ];
			else
				m1 = check->vertindex[ (k+2)%3 ];

			stripverts[stripcount+2] = check->vertindex[ (k+2)%3 ];
			striptris[stripcount] = j;
			stripcount++;

			used[j] = 2;
			goto nexttri;
		}
	}
done:

	// clear the temp used flags
	for (j=starttri+1 ; j<pheader->numtris ; j++)
		if (used[j] == 2)
			used[j] = 0;

	return stripcount;
}

/*
===========
FanLength
===========
*/
static int FanLength (int starttri, int startv)
{
	int		m1, m2;
	int		j;
	mtriangle_t	*last, *check;
	int		k;

	used[starttri] = 2;

	last = &triangles[starttri];

	stripverts[0] = last->vertindex[(startv)%3];
	stripverts[1] = last->vertindex[(startv+1)%3];
	stripverts[2] = last->vertindex[(startv+2)%3];

	striptris[0] = starttri;
	stripcount = 1;

	m1 = last->vertindex[(startv+0)%3];
	m2 = last->vertindex[(startv+2)%3];


	// look for a matching triangle
nexttri:
	for (j=starttri+1, check=&triangles[starttri+1] ; j<pheader->numtris ; j++, check++)
	{
		if (check->facesfront != last->facesfront)
			continue;
		for (k=0 ; k<3 ; k++)
		{
			if (check->vertindex[k] != m1)
				continue;
			if (check->vertindex[ (k+1)%3 ] != m2)
				continue;

			// this is the next part of the fan

			// if we can't use this triangle, this tristrip is done
			if (used[j])
				goto done;

			// the new edge
			m2 = check->vertindex[ (k+2)%3 ];

			stripverts[stripcount+2] = m2;
			striptris[stripcount] = j;
			stripcount++;

			used[j] = 2;
			goto nexttri;
		}
	}
done:

	// clear the temp used flags
	for (j=starttri+1 ; j<pheader->numtris ; j++)
		if (used[j] == 2)
			used[j] = 0;

	return stripcount;
}


/*
================
BuildTris

Generate a list of trifans or strips
for the model, which holds for all frames
================
*/
static void BuildTris (void)
{
    int i, j, k, startv;
    float s, t;
    int len, bestlen, besttype;
    int bestverts[MAXALIASTRIS + 2];
    int besttris[MAXALIASTRIS];
    int type;

	//
	// build tristrips
	//
	numorder = 0;
	numcommands = 0;
	memset (used, 0, sizeof(used));
	for (i=0 ; i<pheader->numtris ; i++)
	{
		// pick an unused triangle and start the trifan
		if (used[i])
			continue;

		bestlen = 0;
		for (type = 0 ; type < 2 ; type++)
//	type = 1;
		{
			for (startv =0 ; startv < 3 ; startv++)
			{
				if (type == 1)
					len = StripLength (i, startv);
				else
					len = FanLength (i, startv);
				if (len > bestlen)
				{
					besttype = type;
					bestlen = len;
					for (j=0 ; j<bestlen+2 ; j++)
						bestverts[j] = stripverts[j];
					for (j=0 ; j<bestlen ; j++)
						besttris[j] = striptris[j];
				}
			}
		}

		// mark the tris on the best strip as used
		for (j=0 ; j<bestlen ; j++)
			used[besttris[j]] = 1;

		if (numcommands + 1 + (bestlen+2)*2 >= (int)(sizeof(commands)/sizeof(commands[0])) ||
			numorder + bestlen + 2 > (int)(sizeof(vertexorder)/sizeof(vertexorder[0])))
			Sys_Error ("XGL alias command capacity exceeded");
		if (besttype == 1)
			commands[numcommands++] = (bestlen+2);
		else
			commands[numcommands++] = -(bestlen+2);

		for (j=0 ; j<bestlen+2 ; j++)
		{
			// emit a vertex into the reorder buffer
			k = bestverts[j];
			vertexorder[numorder++] = k;

			// emit s/t coords into the commands stream
			s = stverts[k].s;
			t = stverts[k].t;
			if (!triangles[besttris[0]].facesfront && stverts[k].onseam)
				s += pheader->skinwidth / 2;	// on back side
			s = (s + 0.5) / pheader->skinwidth;
			t = (t + 0.5) / pheader->skinheight;

			/* Copy bits without violating C strict-aliasing on SPARC or hosts. */
			memcpy (&commands[numcommands++], &s, sizeof(s));
			memcpy (&commands[numcommands++], &t, sizeof(t));
		}
	}

	commands[numcommands++] = 0;		// end of list marker

	Con_DPrintf ("%3i tri %3i vert %3i cmd\n", pheader->numtris, numorder, numcommands);

}


/* Preserve the GL model layout but never read host-endian glquake mesh cache
 * files. Bounded strips/fans are generated from the validated MDL topology. */
void XGL_MakeAliasModelDisplayLists (model_t *m, aliashdr_t *hdr)
{
    int i, j;
    int *cmds;
    trivertx_t *verts;
    if (sizeof(int) != sizeof(float) || hdr != pheader ||
        hdr->numtris < 1 || hdr->numtris > MAXALIASTRIS ||
        hdr->numverts < 1 || hdr->numverts > MAXALIASVERTS ||
        hdr->numposes < 1 || hdr->numposes > MAXALIASFRAMES)
        Sys_Error ("%s: invalid XGL alias command input", m->name);
    for (i = 0; i < hdr->numtris; ++i)
        for (j = 0; j < 3; ++j)
            if (triangles[i].vertindex[j] < 0 || triangles[i].vertindex[j] >= hdr->numverts)
                Sys_Error ("%s: invalid XGL alias vertex index", m->name);
    for (i = 0; i < hdr->numposes; ++i)
        if (!poseverts[i]) Sys_Error ("%s: missing XGL alias pose", m->name);
    paliashdr = hdr;
    BuildTris ();
    paliashdr->poseverts = numorder;
    cmds = Hunk_Alloc (numcommands * sizeof(*cmds));
    paliashdr->commands = (byte *)cmds - (byte *)paliashdr;
    memcpy (cmds, commands, numcommands * sizeof(*cmds));
    verts = Hunk_Alloc (paliashdr->numposes * numorder * sizeof(*verts));
    paliashdr->posedata = (byte *)verts - (byte *)paliashdr;
    for (i = 0; i < paliashdr->numposes; ++i)
        for (j = 0; j < numorder; ++j)
            *verts++ = poseverts[i][vertexorder[j]];
}

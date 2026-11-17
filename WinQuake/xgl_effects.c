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

/* Native geometry replacements for particle sprites and palette overlays. */
#include "quakedef.h"

void XGL_DrawParticle(const particle_t *particle)
{
    xgl_native_vertex_t *vertex;
    float scale, distance, rgb[3];
    int i, color, count;
    count = 3;
    vertex = XGL_ReserveTriangles(&count, 0, 1.0f, 1);
    if (!vertex) return;
    distance = (particle->org[0] - r_origin[0]) * vpn[0] +
               (particle->org[1] - r_origin[1]) * vpn[1] +
               (particle->org[2] - r_origin[2]) * vpn[2];
    scale = distance < 20 ? 1.0f : 1.0f + distance * .004f;
    color = (int)particle->color & 255;
    XGL_PreparePaletteColor(color, rgb);
    for (i = 0; i < 3; ++i) {
        VectorCopy(particle->org, vertex[i].position);
        VectorCopy(vpn, vertex[i].normal);
        VectorCopy(rgb, vertex[i].color.rgb);
    }
    for (i = 0; i < 3; ++i) {
        vertex[1].position[i] += vup[i] * 1.5f * scale;
        vertex[2].position[i] += vright[i] * 1.5f * scale;
    }
}

void XGL_PolyBlend(void)
{
    /* The engine still computes damage, bonus, powerup and contents blends.
     * Native transparency applies the result once before HUD/menu drawing. */
    if (v_blend[3] > 0)
        XGL_UIFillRGB((float)r_refdef.vrect.x, (float)r_refdef.vrect.y,
                      (float)r_refdef.vrect.width, (float)r_refdef.vrect.height,
                      v_blend, v_blend[3]);
}

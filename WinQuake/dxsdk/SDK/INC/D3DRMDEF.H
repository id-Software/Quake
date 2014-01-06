/*==========================================================================;
 *
 *  Copyright (C) 1995-1996 Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       d3drm.h
 *  Content:    Direct3DRM include file
 *
 ***************************************************************************/

#ifndef __D3DRMDEFS_H__
#define __D3DRMDEFS_H__

#include <stddef.h>
#include "d3dtypes.h"

#ifdef WIN32
#define D3DRMAPI  __stdcall
#else
#define D3DRMAPI
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#ifndef TRUE
#define FALSE 0
#define TRUE 1
#endif

typedef struct _D3DRMVECTOR4D
{   D3DVALUE x, y, z, w;
} D3DRMVECTOR4D, *LPD3DRMVECTOR4D;

typedef D3DVALUE D3DRMMATRIX4D[4][4];

typedef struct _D3DRMQUATERNION
{   D3DVALUE s;
    D3DVECTOR v;
} D3DRMQUATERNION, *LPD3DRMQUATERNION;

typedef struct _D3DRMBOX
{   D3DVECTOR min, max;
} D3DRMBOX, *LPD3DRMBOX;

typedef void (*D3DRMWRAPCALLBACK)
    (LPD3DVECTOR, int* u, int* v, LPD3DVECTOR a, LPD3DVECTOR b, LPVOID);

typedef enum _D3DRMLIGHTTYPE
{   D3DRMLIGHT_AMBIENT,
    D3DRMLIGHT_POINT,
    D3DRMLIGHT_SPOT,
    D3DRMLIGHT_DIRECTIONAL,
    D3DRMLIGHT_PARALLELPOINT
} D3DRMLIGHTTYPE, *LPD3DRMLIGHTTYPE;

typedef enum _D3DRMSHADEMODE {
    D3DRMSHADE_FLAT     = 0,
    D3DRMSHADE_GOURAUD  = 1,
    D3DRMSHADE_PHONG    = 2,

    D3DRMSHADE_MASK     = 7,
    D3DRMSHADE_MAX      = 8
} D3DRMSHADEMODE, *LPD3DRMSHADEMODE;

typedef enum _D3DRMLIGHTMODE {
    D3DRMLIGHT_OFF      = 0 * D3DRMSHADE_MAX,
    D3DRMLIGHT_ON       = 1 * D3DRMSHADE_MAX,

    D3DRMLIGHT_MASK     = 7 * D3DRMSHADE_MAX,
    D3DRMLIGHT_MAX      = 8 * D3DRMSHADE_MAX
} D3DRMLIGHTMODE, *LPD3DRMLIGHTMODE;

typedef enum _D3DRMFILLMODE {
    D3DRMFILL_POINTS    = 0 * D3DRMLIGHT_MAX,
    D3DRMFILL_WIREFRAME = 1 * D3DRMLIGHT_MAX,
    D3DRMFILL_SOLID     = 2 * D3DRMLIGHT_MAX,

    D3DRMFILL_MASK      = 7 * D3DRMLIGHT_MAX,
    D3DRMFILL_MAX       = 8 * D3DRMLIGHT_MAX
} D3DRMFILLMODE, *LPD3DRMFILLMODE;

typedef DWORD D3DRMRENDERQUALITY, *LPD3DRMRENDERQUALITY;

#define D3DRMRENDER_WIREFRAME   (D3DRMSHADE_FLAT+D3DRMLIGHT_OFF+D3DRMFILL_WIREFRAME)
#define D3DRMRENDER_UNLITFLAT   (D3DRMSHADE_FLAT+D3DRMLIGHT_OFF+D3DRMFILL_SOLID)
#define D3DRMRENDER_FLAT        (D3DRMSHADE_FLAT+D3DRMLIGHT_ON+D3DRMFILL_SOLID)
#define D3DRMRENDER_GOURAUD     (D3DRMSHADE_GOURAUD+D3DRMLIGHT_ON+D3DRMFILL_SOLID)
#define D3DRMRENDER_PHONG       (D3DRMSHADE_PHONG+D3DRMLIGHT_ON+D3DRMFILL_SOLID)

typedef enum _D3DRMTEXTUREQUALITY
{   D3DRMTEXTURE_NEAREST,               /* choose nearest texel */
    D3DRMTEXTURE_LINEAR,                /* interpolate 4 texels */
    D3DRMTEXTURE_MIPNEAREST,            /* nearest texel in nearest mipmap  */
    D3DRMTEXTURE_MIPLINEAR,             /* interpolate 2 texels from 2 mipmaps */
    D3DRMTEXTURE_LINEARMIPNEAREST,      /* interpolate 4 texels in nearest mipmap */
    D3DRMTEXTURE_LINEARMIPLINEAR        /* interpolate 8 texels from 2 mipmaps */
} D3DRMTEXTUREQUALITY, *LPD3DRMTEXTUREQUALITY;

typedef enum _D3DRMCOMBINETYPE
{   D3DRMCOMBINE_REPLACE,
    D3DRMCOMBINE_BEFORE,
    D3DRMCOMBINE_AFTER
} D3DRMCOMBINETYPE, *LPD3DRMCOMBINETYPE;

typedef D3DCOLORMODEL D3DRMCOLORMODEL, *LPD3DRMCOLORMODEL;

typedef enum _D3DRMPALETTEFLAGS
{   D3DRMPALETTE_FREE,                  /* renderer may use this entry freely */
    D3DRMPALETTE_READONLY,              /* fixed but may be used by renderer */
    D3DRMPALETTE_RESERVED               /* may not be used by renderer */
} D3DRMPALETTEFLAGS, *LPD3DRMPALETTEFLAGS;

typedef struct _D3DRMPALETTEENTRY
{   unsigned char red;          /* 0 .. 255 */
    unsigned char green;        /* 0 .. 255 */
    unsigned char blue;         /* 0 .. 255 */
    unsigned char flags;        /* one of D3DRMPALETTEFLAGS */
} D3DRMPALETTEENTRY, *LPD3DRMPALETTEENTRY;

typedef struct _D3DRMIMAGE
{   int width, height;          /* width and height in pixels */
    int aspectx, aspecty;       /* aspect ratio for non-square pixels */
    int depth;                  /* bits per pixel */
    int rgb;                    /* if false, pixels are indices into a
                                   palette otherwise, pixels encode
                                   RGB values. */
    int bytes_per_line;         /* number of bytes of memory for a
                                   scanline. This must be a multiple
                                   of 4. */
    void* buffer1;              /* memory to render into (first buffer). */
    void* buffer2;              /* second rendering buffer for double
                                   buffering, set to NULL for single
                                   buffering. */
    unsigned long red_mask;
    unsigned long green_mask;
    unsigned long blue_mask;
    unsigned long alpha_mask;   /* if rgb is true, these are masks for
                                   the red, green and blue parts of a
                                   pixel.  Otherwise, these are masks
                                   for the significant bits of the
                                   red, green and blue elements in the
                                   palette.  For instance, most SVGA
                                   displays use 64 intensities of red,
                                   green and blue, so the masks should
                                   all be set to 0xfc. */
    int palette_size;           /* number of entries in palette */
    D3DRMPALETTEENTRY* palette; /* description of the palette (only if
                                   rgb is false).  Must be (1<<depth)
                                   elements. */
} D3DRMIMAGE, *LPD3DRMIMAGE;

typedef enum _D3DRMWRAPTYPE
{   D3DRMWRAP_FLAT,
    D3DRMWRAP_CYLINDER,
    D3DRMWRAP_SPHERE,
    D3DRMWRAP_CHROME
} D3DRMWRAPTYPE, *LPD3DRMWRAPTYPE;

#define D3DRMWIREFRAME_CULL             1 /* cull backfaces */
#define D3DRMWIREFRAME_HIDDENLINE       2 /* lines are obscured by closer objects */

typedef enum _D3DRMPROJECTIONTYPE
{   D3DRMPROJECT_PERSPECTIVE,
    D3DRMPROJECT_ORTHOGRAPHIC
} D3DRMPROJECTIONTYPE, *LPD3DRMPROJECTIONTYPE;

typedef enum _D3DRMXOFFORMAT
{   D3DRMXOF_BINARY,
    D3DRMXOF_COMPRESSED,
    D3DRMXOF_TEXT
} D3DRMXOFFORMAT, *LPD3DRMXOFFORMAT;

typedef DWORD D3DRMSAVEOPTIONS;
#define D3DRMXOFSAVE_NORMALS 1
#define D3DRMXOFSAVE_TEXTURECOORDINATES 2
#define D3DRMXOFSAVE_MATERIALS 4
#define D3DRMXOFSAVE_TEXTURENAMES 8
#define D3DRMXOFSAVE_ALL 15

typedef enum _D3DRMCOLORSOURCE
{   D3DRMCOLOR_FROMFACE,
    D3DRMCOLOR_FROMVERTEX
} D3DRMCOLORSOURCE, *LPD3DRMCOLORSOURCE;

typedef enum _D3DRMFRAMECONSTRAINT
{   D3DRMCONSTRAIN_Z,           /* use only X and Y rotations */
    D3DRMCONSTRAIN_Y,           /* use only X and Z rotations */
    D3DRMCONSTRAIN_X            /* use only Y and Z rotations */
} D3DRMFRAMECONSTRAINT, *LPD3DRMFRAMECONSTRAINT;

typedef enum _D3DRMMATERIALMODE
{   D3DRMMATERIAL_FROMMESH,
    D3DRMMATERIAL_FROMPARENT,
    D3DRMMATERIAL_FROMFRAME
} D3DRMMATERIALMODE, *LPD3DRMMATERIALMODE;

typedef enum _D3DRMFOGMODE
{   D3DRMFOG_LINEAR,            /* linear between start and end */
    D3DRMFOG_EXPONENTIAL,       /* density * exp(-distance) */
    D3DRMFOG_EXPONENTIALSQUARED /* density * exp(-distance*distance) */
} D3DRMFOGMODE, *LPD3DRMFOGMODE;

typedef enum _D3DRMZBUFFERMODE {
    D3DRMZBUFFER_FROMPARENT,    /* default */
    D3DRMZBUFFER_ENABLE,        /* enable zbuffering */
    D3DRMZBUFFER_DISABLE        /* disable zbuffering */
} D3DRMZBUFFERMODE, *LPD3DRMZBUFFERMODE;

typedef enum _D3DRMSORTMODE {
    D3DRMSORT_FROMPARENT,       /* default */
    D3DRMSORT_NONE,             /* don't sort child frames */
    D3DRMSORT_FRONTTOBACK,      /* sort child frames front-to-back */
    D3DRMSORT_BACKTOFRONT       /* sort child frames back-to-front */
} D3DRMSORTMODE, *LPD3DRMSORTMODE;

typedef DWORD D3DRMANIMATIONOPTIONS;
#define D3DRMANIMATION_OPEN 0x01L
#define D3DRMANIMATION_CLOSED 0x02L
#define D3DRMANIMATION_LINEARPOSITION 0x04L
#define D3DRMANIMATION_SPLINEPOSITION 0x08L
#define D3DRMANIMATION_SCALEANDROTATION 0x00000010L
#define D3DRMANIMATION_POSITION 0x00000020L

typedef DWORD D3DRMLOADOPTIONS;

#define D3DRMLOAD_FROMFILE  0x00L
#define D3DRMLOAD_FROMRESOURCE 0x01L
#define D3DRMLOAD_FROMMEMORY 0x02L
#define D3DRMLOAD_FROMSTREAM 0x04L

#define D3DRMLOAD_BYNAME 0x10L
#define D3DRMLOAD_BYPOSITION 0x20L
#define D3DRMLOAD_BYGUID 0x40L
#define D3DRMLOAD_FIRST 0x80L

#define D3DRMLOAD_INSTANCEBYREFERENCE 0x100L
#define D3DRMLOAD_INSTANCEBYCOPYING 0x200L

typedef struct _D3DRMLOADRESOURCE {
  HMODULE hModule;
  LPCTSTR lpName;
  LPCTSTR lpType;
} D3DRMLOADRESOURCE, *LPD3DRMLOADRESOURCE;

typedef struct _D3DRMLOADMEMORY {
  LPVOID lpMemory;
  DWORD dSize;
} D3DRMLOADMEMORY, *LPD3DRMLOADMEMORY;

typedef enum _D3DRMUSERVISUALREASON {
    D3DRMUSERVISUAL_CANSEE,
    D3DRMUSERVISUAL_RENDER
} D3DRMUSERVISUALREASON, *LPD3DRMUSERVISUALREASON;


typedef DWORD D3DRMMAPPING, D3DRMMAPPINGFLAG, *LPD3DRMMAPPING;
static const D3DRMMAPPINGFLAG D3DRMMAP_WRAPU = 1;
static const D3DRMMAPPINGFLAG D3DRMMAP_WRAPV = 2;
static const D3DRMMAPPINGFLAG D3DRMMAP_PERSPCORRECT = 4;

typedef struct _D3DRMVERTEX
{   D3DVECTOR       position;
    D3DVECTOR       normal;
    D3DVALUE        tu, tv;
    D3DCOLOR        color;
} D3DRMVERTEX, *LPD3DRMVERTEX;

typedef LONG D3DRMGROUPINDEX; /* group indexes begin a 0 */
static const D3DRMGROUPINDEX D3DRMGROUP_ALLGROUPS = -1;

/*
 * Create a color from three components in the range 0-1 inclusive.
 */
extern D3DCOLOR D3DRMAPI        D3DRMCreateColorRGB(D3DVALUE red,
                                          D3DVALUE green,
                                          D3DVALUE blue);

/*
 * Create a color from four components in the range 0-1 inclusive.
 */
extern D3DCOLOR D3DRMAPI        D3DRMCreateColorRGBA(D3DVALUE red,
                                                 D3DVALUE green,
                                                 D3DVALUE blue,
                                                 D3DVALUE alpha);

/*
 * Get the red component of a color.
 */
extern D3DVALUE                 D3DRMAPI D3DRMColorGetRed(D3DCOLOR);

/*
 * Get the green component of a color.
 */
extern D3DVALUE                 D3DRMAPI D3DRMColorGetGreen(D3DCOLOR);

/*
 * Get the blue component of a color.
 */
extern D3DVALUE                 D3DRMAPI D3DRMColorGetBlue(D3DCOLOR);

/*
 * Get the alpha component of a color.
 */
extern D3DVALUE                 D3DRMAPI D3DRMColorGetAlpha(D3DCOLOR);

/*
 * Add two vectors.  Returns its first argument.
 */
extern LPD3DVECTOR      D3DRMAPI D3DRMVectorAdd(LPD3DVECTOR d,
                                          LPD3DVECTOR s1,
                                          LPD3DVECTOR s2);

/*
 * Subtract two vectors.  Returns its first argument.
 */
extern LPD3DVECTOR      D3DRMAPI D3DRMVectorSubtract(LPD3DVECTOR d,
                                               LPD3DVECTOR s1,
                                               LPD3DVECTOR s2);
/*
 * Reflect a ray about a given normal.  Returns its first argument.
 */
extern LPD3DVECTOR      D3DRMAPI D3DRMVectorReflect(LPD3DVECTOR d,
                                              LPD3DVECTOR ray,
                                              LPD3DVECTOR norm);

/*
 * Calculate the vector cross product.  Returns its first argument.
 */
extern LPD3DVECTOR      D3DRMAPI D3DRMVectorCrossProduct(LPD3DVECTOR d,
                                                   LPD3DVECTOR s1,
                                                   LPD3DVECTOR s2);
/*
 * Return the vector dot product.
 */
extern D3DVALUE                 D3DRMAPI D3DRMVectorDotProduct(LPD3DVECTOR s1,
                                                 LPD3DVECTOR s2);

/*
 * Scale a vector so that its modulus is 1.  Returns its argument or
 * NULL if there was an error (e.g. a zero vector was passed).
 */
extern LPD3DVECTOR      D3DRMAPI D3DRMVectorNormalize(LPD3DVECTOR);
#define D3DRMVectorNormalise D3DRMVectorNormalize

/*
 * Return the length of a vector (e.g. sqrt(x*x + y*y + z*z)).
 */
extern D3DVALUE                 D3DRMAPI D3DRMVectorModulus(LPD3DVECTOR v);

/*
 * Set the rotation part of a matrix to be a rotation of theta radians
 * around the given axis.
 */

extern LPD3DVECTOR      D3DRMAPI D3DRMVectorRotate(LPD3DVECTOR r, LPD3DVECTOR v, LPD3DVECTOR axis, D3DVALUE theta);

/*
 * Scale a vector uniformly in all three axes
 */
extern LPD3DVECTOR      D3DRMAPI D3DRMVectorScale(LPD3DVECTOR d, LPD3DVECTOR s, D3DVALUE factor);

/*
 * Return a random unit vector
 */
extern LPD3DVECTOR      D3DRMAPI D3DRMVectorRandom(LPD3DVECTOR d);

/*
 * Returns a unit quaternion that represents a rotation of theta radians
 * around the given axis.
 */

extern LPD3DRMQUATERNION D3DRMAPI D3DRMQuaternionFromRotation(LPD3DRMQUATERNION quat,
                                                              LPD3DVECTOR v,
                                                              D3DVALUE theta);

/*
 * Calculate the product of two quaternions
 */
extern LPD3DRMQUATERNION D3DRMAPI D3DRMQuaternionMultiply(LPD3DRMQUATERNION q,
                                                          LPD3DRMQUATERNION a,
                                                          LPD3DRMQUATERNION b);

/*
 * Interpolate between two quaternions
 */
extern LPD3DRMQUATERNION D3DRMAPI D3DRMQuaternionSlerp(LPD3DRMQUATERNION q,
                                                       LPD3DRMQUATERNION a,
                                                       LPD3DRMQUATERNION b,
                                                       D3DVALUE alpha);

/*
 * Calculate the matrix for the rotation that a unit quaternion represents
 */
extern void             D3DRMAPI D3DRMMatrixFromQuaternion(D3DRMMATRIX4D dmMat, LPD3DRMQUATERNION lpDqQuat);


#if defined(__cplusplus)
};
#endif

#endif

/*==========================================================================;
 *
 *  Copyright (C) 1995-1996 Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       d3dtypes.h
 *  Content:    Direct3D types include file
 *
 ***************************************************************************/

#ifndef _D3DTYPES_H_
#define _D3DTYPES_H_

#ifndef WIN32
#include "subwtype.h"
#else
#include <windows.h>
#endif

#include <ddraw.h>

#pragma pack(4)

#if defined(__cplusplus)
extern "C"
{
#endif

/* D3DVALUE is the fundamental Direct3D fractional data type */

#define D3DVALP(val, prec) ((float)(val))
#define D3DVAL(val) ((float)(val))
typedef float D3DVALUE, *LPD3DVALUE;
#define D3DDivide(a, b)    (float)((double) (a) / (double) (b))
#define D3DMultiply(a, b)    ((a) * (b))

typedef LONG D3DFIXED;

#ifndef RGB_MAKE
/*
 * Format of CI colors is
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |    alpha      |         color index           |   fraction    |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
#define CI_GETALPHA(ci)    ((ci) >> 24)
#define CI_GETINDEX(ci)    (((ci) >> 8) & 0xffff)
#define CI_GETFRACTION(ci) ((ci) & 0xff)
#define CI_ROUNDINDEX(ci)  CI_GETINDEX((ci) + 0x80)
#define CI_MASKALPHA(ci)   ((ci) & 0xffffff)
#define CI_MAKE(a, i, f)    (((a) << 24) | ((i) << 8) | (f))

/*
 * Format of RGBA colors is
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |    alpha      |      red      |     green     |     blue      |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
#define RGBA_GETALPHA(rgb)      ((rgb) >> 24)
#define RGBA_GETRED(rgb)        (((rgb) >> 16) & 0xff)
#define RGBA_GETGREEN(rgb)      (((rgb) >> 8) & 0xff)
#define RGBA_GETBLUE(rgb)       ((rgb) & 0xff)
#define RGBA_MAKE(r, g, b, a)   ((D3DCOLOR) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b)))

/* D3DRGB and D3DRGBA may be used as initialisers for D3DCOLORs
 * The float values must be in the range 0..1
 */
#define D3DRGB(r, g, b) \
    (0xff000000L | ( ((long)((r) * 255)) << 16) | (((long)((g) * 255)) << 8) | (long)((b) * 255))
#define D3DRGBA(r, g, b, a) \
    (   (((long)((a) * 255)) << 24) | (((long)((r) * 255)) << 16) \
    |   (((long)((g) * 255)) << 8) | (long)((b) * 255) \
    )

/*
 * Format of RGB colors is
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |    ignored    |      red      |     green     |     blue      |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
#define RGB_GETRED(rgb)         (((rgb) >> 16) & 0xff)
#define RGB_GETGREEN(rgb)       (((rgb) >> 8) & 0xff)
#define RGB_GETBLUE(rgb)        ((rgb) & 0xff)
#define RGBA_SETALPHA(rgba, x) (((x) << 24) | ((rgba) & 0x00ffffff))
#define RGB_MAKE(r, g, b)       ((D3DCOLOR) (((r) << 16) | ((g) << 8) | (b)))
#define RGBA_TORGB(rgba)       ((D3DCOLOR) ((rgba) & 0xffffff))
#define RGB_TORGBA(rgb)        ((D3DCOLOR) ((rgb) | 0xff000000))

#endif

/*
 * Flags for Enumerate functions
 */

/*
 * Stop the enumeration
 */
#define D3DENUMRET_CANCEL                        DDENUMRET_CANCEL

/*
 * Continue the enumeration
 */
#define D3DENUMRET_OK                            DDENUMRET_OK

typedef HRESULT (WINAPI* LPD3DVALIDATECALLBACK)(LPVOID lpUserArg, DWORD dwOffset);
typedef HRESULT (WINAPI* LPD3DENUMTEXTUREFORMATSCALLBACK)(LPDDSURFACEDESC lpDdsd, LPVOID lpContext);

typedef DWORD D3DCOLOR, D3DCOLOR, *LPD3DCOLOR;

typedef DWORD D3DMATERIALHANDLE, *LPD3DMATERIALHANDLE;
typedef DWORD D3DTEXTUREHANDLE, *LPD3DTEXTUREHANDLE;
typedef DWORD D3DMATRIXHANDLE, *LPD3DMATRIXHANDLE;

typedef struct _D3DCOLORVALUE {
    union {
        D3DVALUE r;
        D3DVALUE dvR;
    };
    union {
        D3DVALUE g;
        D3DVALUE dvG;
    };
    union {
        D3DVALUE b;
        D3DVALUE dvB;
    };
    union {
        D3DVALUE a;
        D3DVALUE dvA;
    };
} D3DCOLORVALUE;

typedef struct _D3DRECT {
    union {
        LONG x1;
        LONG lX1;
    };
    union {
        LONG y1;
        LONG lY1;
    };
    union {
        LONG x2;
        LONG lX2;
    };
    union {
        LONG y2;
        LONG lY2;
    };
} D3DRECT, *LPD3DRECT;

typedef struct _D3DVECTOR {
    union {
        D3DVALUE x;
        D3DVALUE dvX;
    };
    union {
        D3DVALUE y;
        D3DVALUE dvY;
    };
    union {
        D3DVALUE z;
        D3DVALUE dvZ;
    };
} D3DVECTOR, *LPD3DVECTOR;


/*
 * Vertex data types supported in an ExecuteBuffer.
 */

/*
 * Homogeneous vertices
 */

typedef struct _D3DHVERTEX {
    DWORD           dwFlags;        /* Homogeneous clipping flags */
    union {
        D3DVALUE    hx;
        D3DVALUE    dvHX;
    };
    union {
        D3DVALUE    hy;
        D3DVALUE    dvHY;
    };
    union {
        D3DVALUE    hz;
        D3DVALUE    dvHZ;
    };
} D3DHVERTEX, *LPD3DHVERTEX;

/*
 * Transformed/lit vertices
 */
typedef struct _D3DTLVERTEX {
    union {
        D3DVALUE    sx;             /* Screen coordinates */
        D3DVALUE    dvSX;
    };
    union {
        D3DVALUE    sy;
        D3DVALUE    dvSY;
    };
    union {
        D3DVALUE    sz;
        D3DVALUE    dvSZ;
    };
    union {
        D3DVALUE    rhw;            /* Reciprocal of homogeneous w */
        D3DVALUE    dvRHW;
    };
    union {
        D3DCOLOR    color;          /* Vertex color */
        D3DCOLOR    dcColor;
    };
    union {
        D3DCOLOR    specular;       /* Specular component of vertex */
        D3DCOLOR    dcSpecular;
    };
    union {
        D3DVALUE    tu;             /* Texture coordinates */
        D3DVALUE    dvTU;
    };
    union {
        D3DVALUE    tv;
        D3DVALUE    dvTV;
    };
} D3DTLVERTEX, *LPD3DTLVERTEX;

/*
 * Untransformed/lit vertices
 */
typedef struct _D3DLVERTEX {
    union {
        D3DVALUE     x;             /* Homogeneous coordinates */
        D3DVALUE     dvX;
    };
    union {
        D3DVALUE     y;
        D3DVALUE     dvY;
    };
    union {
        D3DVALUE     z;
        D3DVALUE     dvZ;
    };
    DWORD            dwReserved;
    union {
        D3DCOLOR     color;         /* Vertex color */
        D3DCOLOR     dcColor;
    };
    union {
        D3DCOLOR     specular;      /* Specular component of vertex */
        D3DCOLOR     dcSpecular;
    };
    union {
        D3DVALUE     tu;            /* Texture coordinates */
        D3DVALUE     dvTU;
    };
    union {
        D3DVALUE     tv;
        D3DVALUE     dvTV;
    };
} D3DLVERTEX, *LPD3DLVERTEX;

/*
 * Untransformed/unlit vertices
 */

typedef struct _D3DVERTEX {
    union {
        D3DVALUE     x;             /* Homogeneous coordinates */
        D3DVALUE     dvX;
    };
    union {
        D3DVALUE     y;
        D3DVALUE     dvY;
    };
    union {
        D3DVALUE     z;
        D3DVALUE     dvZ;
    };
    union {
        D3DVALUE     nx;            /* Normal */
        D3DVALUE     dvNX;
    };
    union {
        D3DVALUE     ny;
        D3DVALUE     dvNY;
    };
    union {
        D3DVALUE     nz;
        D3DVALUE     dvNZ;
    };
    union {
        D3DVALUE     tu;            /* Texture coordinates */
        D3DVALUE     dvTU;
    };
    union {
        D3DVALUE     tv;
        D3DVALUE     dvTV;
    };
} D3DVERTEX, *LPD3DVERTEX;

/*
 * Matrix, viewport, and tranformation structures and definitions.
 */

typedef struct _D3DMATRIX {
    D3DVALUE        _11, _12, _13, _14;
    D3DVALUE        _21, _22, _23, _24;
    D3DVALUE        _31, _32, _33, _34;
    D3DVALUE        _41, _42, _43, _44;
} D3DMATRIX, *LPD3DMATRIX;

typedef struct _D3DVIEWPORT {
    DWORD       dwSize;
    DWORD       dwX;
    DWORD       dwY;            /* Top left */
    DWORD       dwWidth;
    DWORD       dwHeight;       /* Dimensions */
    D3DVALUE    dvScaleX;       /* Scale homogeneous to screen */
    D3DVALUE    dvScaleY;       /* Scale homogeneous to screen */
    D3DVALUE    dvMaxX;         /* Min/max homogeneous x coord */
    D3DVALUE    dvMaxY;         /* Min/max homogeneous y coord */
    D3DVALUE    dvMinZ;
    D3DVALUE    dvMaxZ;         /* Min/max homogeneous z coord */
} D3DVIEWPORT, *LPD3DVIEWPORT;

/*
 * Values for clip fields.
 */
#define D3DCLIP_LEFT                            0x00000001L
#define D3DCLIP_RIGHT                           0x00000002L
#define D3DCLIP_TOP                             0x00000004L
#define D3DCLIP_BOTTOM                          0x00000008L
#define D3DCLIP_FRONT                           0x00000010L
#define D3DCLIP_BACK                            0x00000020L
#define D3DCLIP_GEN0                            0x00000040L
#define D3DCLIP_GEN1                            0x00000080L
#define D3DCLIP_GEN2                            0x00000100L
#define D3DCLIP_GEN3                            0x00000200L
#define D3DCLIP_GEN4                            0x00000400L
#define D3DCLIP_GEN5                            0x00000800L

/*
 * Values for d3d status.
 */
#define D3DSTATUS_CLIPUNIONLEFT                 D3DCLIP_LEFT
#define D3DSTATUS_CLIPUNIONRIGHT                D3DCLIP_RIGHT
#define D3DSTATUS_CLIPUNIONTOP                  D3DCLIP_TOP
#define D3DSTATUS_CLIPUNIONBOTTOM               D3DCLIP_BOTTOM
#define D3DSTATUS_CLIPUNIONFRONT                D3DCLIP_FRONT
#define D3DSTATUS_CLIPUNIONBACK                 D3DCLIP_BACK
#define D3DSTATUS_CLIPUNIONGEN0                 D3DCLIP_GEN0
#define D3DSTATUS_CLIPUNIONGEN1                 D3DCLIP_GEN1
#define D3DSTATUS_CLIPUNIONGEN2                 D3DCLIP_GEN2
#define D3DSTATUS_CLIPUNIONGEN3                 D3DCLIP_GEN3
#define D3DSTATUS_CLIPUNIONGEN4                 D3DCLIP_GEN4
#define D3DSTATUS_CLIPUNIONGEN5                 D3DCLIP_GEN5

#define D3DSTATUS_CLIPINTERSECTIONLEFT          0x00001000L
#define D3DSTATUS_CLIPINTERSECTIONRIGHT         0x00002000L
#define D3DSTATUS_CLIPINTERSECTIONTOP           0x00004000L
#define D3DSTATUS_CLIPINTERSECTIONBOTTOM        0x00008000L
#define D3DSTATUS_CLIPINTERSECTIONFRONT         0x00010000L
#define D3DSTATUS_CLIPINTERSECTIONBACK          0x00020000L
#define D3DSTATUS_CLIPINTERSECTIONGEN0          0x00040000L
#define D3DSTATUS_CLIPINTERSECTIONGEN1          0x00080000L
#define D3DSTATUS_CLIPINTERSECTIONGEN2          0x00100000L
#define D3DSTATUS_CLIPINTERSECTIONGEN3          0x00200000L
#define D3DSTATUS_CLIPINTERSECTIONGEN4          0x00400000L
#define D3DSTATUS_CLIPINTERSECTIONGEN5          0x00800000L
#define D3DSTATUS_ZNOTVISIBLE                   0x01000000L

#define D3DSTATUS_CLIPUNIONALL  (               \
            D3DSTATUS_CLIPUNIONLEFT     |       \
            D3DSTATUS_CLIPUNIONRIGHT    |       \
            D3DSTATUS_CLIPUNIONTOP      |       \
            D3DSTATUS_CLIPUNIONBOTTOM   |       \
            D3DSTATUS_CLIPUNIONFRONT    |       \
            D3DSTATUS_CLIPUNIONBACK     |       \
            D3DSTATUS_CLIPUNIONGEN0     |       \
            D3DSTATUS_CLIPUNIONGEN1     |       \
            D3DSTATUS_CLIPUNIONGEN2     |       \
            D3DSTATUS_CLIPUNIONGEN3     |       \
            D3DSTATUS_CLIPUNIONGEN4     |       \
            D3DSTATUS_CLIPUNIONGEN5             \
            )

#define D3DSTATUS_CLIPINTERSECTIONALL   (               \
            D3DSTATUS_CLIPINTERSECTIONLEFT      |       \
            D3DSTATUS_CLIPINTERSECTIONRIGHT     |       \
            D3DSTATUS_CLIPINTERSECTIONTOP       |       \
            D3DSTATUS_CLIPINTERSECTIONBOTTOM    |       \
            D3DSTATUS_CLIPINTERSECTIONFRONT     |       \
            D3DSTATUS_CLIPINTERSECTIONBACK      |       \
            D3DSTATUS_CLIPINTERSECTIONGEN0      |       \
            D3DSTATUS_CLIPINTERSECTIONGEN1      |       \
            D3DSTATUS_CLIPINTERSECTIONGEN2      |       \
            D3DSTATUS_CLIPINTERSECTIONGEN3      |       \
            D3DSTATUS_CLIPINTERSECTIONGEN4      |       \
            D3DSTATUS_CLIPINTERSECTIONGEN5              \
            )

#define D3DSTATUS_DEFAULT       (                       \
            D3DSTATUS_CLIPINTERSECTIONALL       |       \
            D3DSTATUS_ZNOTVISIBLE)


/*
 * Options for direct transform calls
 */
#define D3DTRANSFORM_CLIPPED       0x00000001l
#define D3DTRANSFORM_UNCLIPPED     0x00000002l

typedef struct _D3DTRANSFORMDATA {
    DWORD           dwSize;
    LPVOID          lpIn;           /* Input vertices */
    DWORD           dwInSize;       /* Stride of input vertices */
    LPVOID          lpOut;          /* Output vertices */
    DWORD           dwOutSize;      /* Stride of output vertices */
    LPD3DHVERTEX    lpHOut;         /* Output homogeneous vertices */
    DWORD           dwClip;         /* Clipping hint */
    DWORD           dwClipIntersection;
    DWORD           dwClipUnion;    /* Union of all clip flags */
    D3DRECT         drExtent;       /* Extent of transformed vertices */
} D3DTRANSFORMDATA, *LPD3DTRANSFORMDATA;

/*
 * Structure defining position and direction properties for lighting.
 */
typedef struct _D3DLIGHTINGELEMENT {
    D3DVECTOR dvPosition;           /* Lightable point in model space */
    D3DVECTOR dvNormal;             /* Normalised unit vector */
} D3DLIGHTINGELEMENT, *LPD3DLIGHTINGELEMENT;

/*
 * Structure defining material properties for lighting.
 */
typedef struct _D3DMATERIAL {
    DWORD               dwSize;
    union {
        D3DCOLORVALUE   diffuse;        /* Diffuse color RGBA */
        D3DCOLORVALUE   dcvDiffuse;
    };
    union {
        D3DCOLORVALUE   ambient;        /* Ambient color RGB */
        D3DCOLORVALUE   dcvAmbient;
    };
    union {
        D3DCOLORVALUE   specular;       /* Specular 'shininess' */
        D3DCOLORVALUE   dcvSpecular;
    };
    union {
        D3DCOLORVALUE   emissive;       /* Emissive color RGB */
        D3DCOLORVALUE   dcvEmissive;
    };
    union {
        D3DVALUE        power;          /* Sharpness if specular highlight */
        D3DVALUE        dvPower;
    };
    D3DTEXTUREHANDLE    hTexture;       /* Handle to texture map */
    DWORD               dwRampSize;
} D3DMATERIAL, *LPD3DMATERIAL;

typedef enum _D3DLIGHTTYPE {
    D3DLIGHT_POINT          = 1,
    D3DLIGHT_SPOT           = 2,
    D3DLIGHT_DIRECTIONAL    = 3,
    D3DLIGHT_PARALLELPOINT  = 4,
    D3DLIGHT_GLSPOT         = 5,
} D3DLIGHTTYPE;

/*
 * Structure defining a light source and its properties.
 */
typedef struct _D3DLIGHT {
    DWORD           dwSize;
    D3DLIGHTTYPE    dltType;            /* Type of light source */
    D3DCOLORVALUE   dcvColor;           /* Color of light */
    D3DVECTOR       dvPosition;         /* Position in world space */
    D3DVECTOR       dvDirection;        /* Direction in world space */
    D3DVALUE        dvRange;            /* Cutoff range */
    D3DVALUE        dvFalloff;          /* Falloff */
    D3DVALUE        dvAttenuation0;     /* Constant attenuation */
    D3DVALUE        dvAttenuation1;     /* Linear attenuation */
    D3DVALUE        dvAttenuation2;     /* Quadratic attenuation */
    D3DVALUE        dvTheta;            /* Inner angle of spotlight cone */
    D3DVALUE        dvPhi;              /* Outer angle of spotlight cone */
} D3DLIGHT, *LPD3DLIGHT;

typedef struct _D3DLIGHTDATA {
    DWORD                dwSize;
    LPD3DLIGHTINGELEMENT lpIn;          /* Input positions and normals */
    DWORD                dwInSize;      /* Stride of input elements */
    LPD3DTLVERTEX        lpOut;         /* Output colors */
    DWORD                dwOutSize;     /* Stride of output colors */
} D3DLIGHTDATA, *LPD3DLIGHTDATA;

typedef enum _D3DCOLORMODEL {
    D3DCOLOR_MONO           = 1,
    D3DCOLOR_RGB            = 2,
} D3DCOLORMODEL;

/*
 * Options for clearing
 */
#define D3DCLEAR_TARGET            0x00000001l /* Clear target surface */
#define D3DCLEAR_ZBUFFER           0x00000002l /* Clear target z buffer */

/*
 * Execute buffers are allocated via Direct3D.  These buffers may then
 * be filled by the application with instructions to execute along with
 * vertex data.
 */

/*
 * Supported op codes for execute instructions.
 */
typedef enum _D3DOPCODE {
    D3DOP_POINT                 = 1,
    D3DOP_LINE                  = 2,
    D3DOP_TRIANGLE              = 3,
    D3DOP_MATRIXLOAD            = 4,
    D3DOP_MATRIXMULTIPLY        = 5,
    D3DOP_STATETRANSFORM        = 6,
    D3DOP_STATELIGHT            = 7,
    D3DOP_STATERENDER           = 8,
    D3DOP_PROCESSVERTICES       = 9,
    D3DOP_TEXTURELOAD           = 10,
    D3DOP_EXIT                  = 11,
    D3DOP_BRANCHFORWARD         = 12,
    D3DOP_SPAN                  = 13,
    D3DOP_SETSTATUS             = 14,
} D3DOPCODE;

typedef struct _D3DINSTRUCTION {
    BYTE bOpcode;   /* Instruction opcode */
    BYTE bSize;     /* Size of each instruction data unit */
    WORD wCount;    /* Count of instruction data units to follow */
} D3DINSTRUCTION, *LPD3DINSTRUCTION;

/*
 * Structure for texture loads
 */
typedef struct _D3DTEXTURELOAD {
    D3DTEXTUREHANDLE hDestTexture;
    D3DTEXTUREHANDLE hSrcTexture;
} D3DTEXTURELOAD, *LPD3DTEXTURELOAD;

/*
 * Structure for picking
 */
typedef struct _D3DPICKRECORD {
    BYTE     bOpcode;
    BYTE     bPad;
    DWORD    dwOffset;
    D3DVALUE dvZ;
} D3DPICKRECORD, *LPD3DPICKRECORD;

/*
 * The following defines the rendering states which can be set in the
 * execute buffer.
 */

typedef enum _D3DSHADEMODE {
    D3DSHADE_FLAT              = 1,
    D3DSHADE_GOURAUD           = 2,
    D3DSHADE_PHONG             = 3,
} D3DSHADEMODE;

typedef enum _D3DFILLMODE {
    D3DFILL_POINT              = 1,
    D3DFILL_WIREFRAME          = 2,
    D3DFILL_SOLID              = 3,
} D3DFILLMODE;

typedef struct _D3DLINEPATTERN {
    WORD        wRepeatFactor;
    WORD        wLinePattern;
} D3DLINEPATTERN;

typedef enum _D3DTEXTUREFILTER {
    D3DFILTER_NEAREST          = 1,
    D3DFILTER_LINEAR           = 2,
    D3DFILTER_MIPNEAREST       = 3,
    D3DFILTER_MIPLINEAR        = 4,
    D3DFILTER_LINEARMIPNEAREST = 5,
    D3DFILTER_LINEARMIPLINEAR  = 6,
} D3DTEXTUREFILTER;

typedef enum _D3DBLEND {
    D3DBLEND_ZERO              = 1,
    D3DBLEND_ONE               = 2,
    D3DBLEND_SRCCOLOR          = 3,
    D3DBLEND_INVSRCCOLOR       = 4,
    D3DBLEND_SRCALPHA          = 5,
    D3DBLEND_INVSRCALPHA       = 6,
    D3DBLEND_DESTALPHA         = 7,
    D3DBLEND_INVDESTALPHA      = 8,
    D3DBLEND_DESTCOLOR         = 9,
    D3DBLEND_INVDESTCOLOR      = 10,
    D3DBLEND_SRCALPHASAT       = 11,
    D3DBLEND_BOTHSRCALPHA      = 12,
    D3DBLEND_BOTHINVSRCALPHA   = 13,
} D3DBLEND;

typedef enum _D3DTEXTUREBLEND {
    D3DTBLEND_DECAL            = 1,
    D3DTBLEND_MODULATE         = 2,
    D3DTBLEND_DECALALPHA       = 3,
    D3DTBLEND_MODULATEALPHA    = 4,
    D3DTBLEND_DECALMASK        = 5,
    D3DTBLEND_MODULATEMASK     = 6,
    D3DTBLEND_COPY             = 7,
} D3DTEXTUREBLEND;

typedef enum _D3DTEXTUREADDRESS {
    D3DTADDRESS_WRAP           = 1,
    D3DTADDRESS_MIRROR         = 2,
    D3DTADDRESS_CLAMP          = 3,
} D3DTEXTUREADDRESS;

typedef enum _D3DCULL {
    D3DCULL_NONE               = 1,
    D3DCULL_CW                 = 2,
    D3DCULL_CCW                = 3,
} D3DCULL;

typedef enum _D3DCMPFUNC {
    D3DCMP_NEVER               = 1,
    D3DCMP_LESS                = 2,
    D3DCMP_EQUAL               = 3,
    D3DCMP_LESSEQUAL           = 4,
    D3DCMP_GREATER             = 5,
    D3DCMP_NOTEQUAL            = 6,
    D3DCMP_GREATEREQUAL        = 7,
    D3DCMP_ALWAYS              = 8,
} D3DCMPFUNC;

typedef enum _D3DFOGMODE {
    D3DFOG_NONE                = 0,
    D3DFOG_EXP                 = 1,
    D3DFOG_EXP2                = 2,
    D3DFOG_LINEAR              = 3
} D3DFOGMODE;

/*
 * Amount to add to a state to generate the override for that state.
 */
#define D3DSTATE_OVERRIDE_BIAS          256

/*
 * A state which sets the override flag for the specified state type.
 */
#define D3DSTATE_OVERRIDE(type) ((DWORD) (type) + D3DSTATE_OVERRIDE_BIAS)

typedef enum _D3DTRANSFORMSTATETYPE {
    D3DTRANSFORMSTATE_WORLD           = 1,
    D3DTRANSFORMSTATE_VIEW            = 2,
    D3DTRANSFORMSTATE_PROJECTION      = 3,
} D3DTRANSFORMSTATETYPE;

typedef enum _D3DLIGHTSTATETYPE {
    D3DLIGHTSTATE_MATERIAL            = 1,
    D3DLIGHTSTATE_AMBIENT             = 2,
    D3DLIGHTSTATE_COLORMODEL          = 3,
    D3DLIGHTSTATE_FOGMODE             = 4,
    D3DLIGHTSTATE_FOGSTART            = 5,
    D3DLIGHTSTATE_FOGEND              = 6,
    D3DLIGHTSTATE_FOGDENSITY          = 7,
} D3DLIGHTSTATETYPE;

typedef enum _D3DRENDERSTATETYPE {
    D3DRENDERSTATE_TEXTUREHANDLE      = 1,    /* Texture handle */
    D3DRENDERSTATE_ANTIALIAS          = 2,    /* Antialiasing prim edges */
    D3DRENDERSTATE_TEXTUREADDRESS     = 3,    /* D3DTEXTUREADDRESS      */
    D3DRENDERSTATE_TEXTUREPERSPECTIVE = 4,    /* TRUE for perspective correction */
    D3DRENDERSTATE_WRAPU              = 5,    /* TRUE for wrapping in u */
    D3DRENDERSTATE_WRAPV              = 6,    /* TRUE for wrapping in v */
    D3DRENDERSTATE_ZENABLE            = 7,    /* TRUE to enable z test */
    D3DRENDERSTATE_FILLMODE           = 8,    /* D3DFILL_MODE            */
    D3DRENDERSTATE_SHADEMODE          = 9,    /* D3DSHADEMODE */
    D3DRENDERSTATE_LINEPATTERN        = 10,   /* D3DLINEPATTERN */
    D3DRENDERSTATE_MONOENABLE         = 11,   /* TRUE to enable mono rasterization */
    D3DRENDERSTATE_ROP2               = 12,   /* ROP2 */
    D3DRENDERSTATE_PLANEMASK          = 13,   /* DWORD physical plane mask */
    D3DRENDERSTATE_ZWRITEENABLE       = 14,   /* TRUE to enable z writes */
    D3DRENDERSTATE_ALPHATESTENABLE    = 15,   /* TRUE to enable alpha tests */
    D3DRENDERSTATE_LASTPIXEL          = 16,   /* TRUE for last-pixel on lines */
    D3DRENDERSTATE_TEXTUREMAG         = 17,   /* D3DTEXTUREFILTER */
    D3DRENDERSTATE_TEXTUREMIN         = 18,   /* D3DTEXTUREFILTER */
    D3DRENDERSTATE_SRCBLEND           = 19,   /* D3DBLEND */
    D3DRENDERSTATE_DESTBLEND          = 20,   /* D3DBLEND */
    D3DRENDERSTATE_TEXTUREMAPBLEND    = 21,   /* D3DTEXTUREBLEND */
    D3DRENDERSTATE_CULLMODE           = 22,   /* D3DCULL */
    D3DRENDERSTATE_ZFUNC              = 23,   /* D3DCMPFUNC */
    D3DRENDERSTATE_ALPHAREF           = 24,   /* D3DFIXED */
    D3DRENDERSTATE_ALPHAFUNC          = 25,   /* D3DCMPFUNC */
    D3DRENDERSTATE_DITHERENABLE       = 26,   /* TRUE to enable dithering */
    D3DRENDERSTATE_BLENDENABLE        = 27,   /* TRUE to enable alpha blending */
    D3DRENDERSTATE_FOGENABLE          = 28,   /* TRUE to enable fog */
    D3DRENDERSTATE_SPECULARENABLE     = 29,   /* TRUE to enable specular */
    D3DRENDERSTATE_ZVISIBLE           = 30,   /* TRUE to enable z checking */
    D3DRENDERSTATE_SUBPIXEL           = 31,   /* TRUE to enable subpixel correction */
    D3DRENDERSTATE_SUBPIXELX          = 32,   /* TRUE to enable correction in X only */
    D3DRENDERSTATE_STIPPLEDALPHA      = 33,   /* TRUE to enable stippled alpha */
    D3DRENDERSTATE_FOGCOLOR           = 34,   /* D3DCOLOR */
    D3DRENDERSTATE_FOGTABLEMODE       = 35,   /* D3DFOGMODE */
    D3DRENDERSTATE_FOGTABLESTART      = 36,   /* Fog table start        */
    D3DRENDERSTATE_FOGTABLEEND        = 37,   /* Fog table end          */
    D3DRENDERSTATE_FOGTABLEDENSITY    = 38,   /* Fog table density      */
    D3DRENDERSTATE_STIPPLEENABLE      = 39,   /* TRUE to enable stippling */
    D3DRENDERSTATE_STIPPLEPATTERN00   = 64,   /* Stipple pattern 01...  */      
    D3DRENDERSTATE_STIPPLEPATTERN01   = 65,
    D3DRENDERSTATE_STIPPLEPATTERN02   = 66,
    D3DRENDERSTATE_STIPPLEPATTERN03   = 67,
    D3DRENDERSTATE_STIPPLEPATTERN04   = 68,
    D3DRENDERSTATE_STIPPLEPATTERN05   = 69,
    D3DRENDERSTATE_STIPPLEPATTERN06   = 70,
    D3DRENDERSTATE_STIPPLEPATTERN07   = 71,
    D3DRENDERSTATE_STIPPLEPATTERN08   = 72,
    D3DRENDERSTATE_STIPPLEPATTERN09   = 73,
    D3DRENDERSTATE_STIPPLEPATTERN10   = 74,
    D3DRENDERSTATE_STIPPLEPATTERN11   = 75,
    D3DRENDERSTATE_STIPPLEPATTERN12   = 76,
    D3DRENDERSTATE_STIPPLEPATTERN13   = 77,
    D3DRENDERSTATE_STIPPLEPATTERN14   = 78,
    D3DRENDERSTATE_STIPPLEPATTERN15   = 79,
    D3DRENDERSTATE_STIPPLEPATTERN16   = 80,
    D3DRENDERSTATE_STIPPLEPATTERN17   = 81,
    D3DRENDERSTATE_STIPPLEPATTERN18   = 82,
    D3DRENDERSTATE_STIPPLEPATTERN19   = 83,
    D3DRENDERSTATE_STIPPLEPATTERN20   = 84,
    D3DRENDERSTATE_STIPPLEPATTERN21   = 85,
    D3DRENDERSTATE_STIPPLEPATTERN22   = 86,
    D3DRENDERSTATE_STIPPLEPATTERN23   = 87,
    D3DRENDERSTATE_STIPPLEPATTERN24   = 88,
    D3DRENDERSTATE_STIPPLEPATTERN25   = 89,
    D3DRENDERSTATE_STIPPLEPATTERN26   = 90,
    D3DRENDERSTATE_STIPPLEPATTERN27   = 91,
    D3DRENDERSTATE_STIPPLEPATTERN28   = 92,
    D3DRENDERSTATE_STIPPLEPATTERN29   = 93,
    D3DRENDERSTATE_STIPPLEPATTERN30   = 94,
    D3DRENDERSTATE_STIPPLEPATTERN31   = 95,
} D3DRENDERSTATETYPE;

#define D3DRENDERSTATE_STIPPLEPATTERN(y) (D3DRENDERSTATE_STIPPLEPATTERN00 + (y))

typedef struct _D3DSTATE {
    union {
        D3DTRANSFORMSTATETYPE   dtstTransformStateType;
        D3DLIGHTSTATETYPE       dlstLightStateType;
        D3DRENDERSTATETYPE      drstRenderStateType;
    };
    union {
        DWORD                   dwArg[1];
        D3DVALUE                dvArg[1];
    };
} D3DSTATE, *LPD3DSTATE;

/*
 * Operation used to load matrices
 * hDstMat = hSrcMat
 */
typedef struct _D3DMATRIXLOAD {
    D3DMATRIXHANDLE hDestMatrix;   /* Destination matrix */
    D3DMATRIXHANDLE hSrcMatrix;   /* Source matrix */
} D3DMATRIXLOAD, *LPD3DMATRIXLOAD;

/*
 * Operation used to multiply matrices
 * hDstMat = hSrcMat1 * hSrcMat2
 */
typedef struct _D3DMATRIXMULTIPLY {
    D3DMATRIXHANDLE hDestMatrix;   /* Destination matrix */
    D3DMATRIXHANDLE hSrcMatrix1;  /* First source matrix */
    D3DMATRIXHANDLE hSrcMatrix2;  /* Second source matrix */
} D3DMATRIXMULTIPLY, *LPD3DMATRIXMULTIPLY;

/*
 * Operation used to transform and light vertices.
 */
typedef struct _D3DPROCESSVERTICES {
    DWORD        dwFlags;    /* Do we transform or light or just copy? */
    WORD         wStart;     /* Index to first vertex in source */
    WORD         wDest;      /* Index to first vertex in local buffer */
    DWORD        dwCount;    /* Number of vertices to be processed */
    DWORD        dwReserved; /* Must be zero */
} D3DPROCESSVERTICES, *LPD3DPROCESSVERTICES;

#define D3DPROCESSVERTICES_TRANSFORMLIGHT       0x00000000L
#define D3DPROCESSVERTICES_TRANSFORM            0x00000001L
#define D3DPROCESSVERTICES_COPY                 0x00000002L
#define D3DPROCESSVERTICES_OPMASK               0x00000007L

#define D3DPROCESSVERTICES_UPDATEEXTENTS        0x00000008L
#define D3DPROCESSVERTICES_NOCOLOR              0x00000010L


/*
 * Triangle flags
 */
 
/*
 * Tri strip and fan flags.
 * START loads all three vertices
 * EVEN and ODD load just v3 with even or odd culling
 * START_FLAT contains a count from 0 to 29 that allows the
 * whole strip or fan to be culled in one hit.
 * e.g. for a quad len = 1
 */
#define D3DTRIFLAG_START                        0x00000000L
#define D3DTRIFLAG_STARTFLAT(len) (len)         /* 0 < len < 30 */
#define D3DTRIFLAG_ODD                          0x0000001eL
#define D3DTRIFLAG_EVEN                         0x0000001fL

/*
 * Triangle edge flags
 * enable edges for wireframe or antialiasing
 */
#define D3DTRIFLAG_EDGEENABLE1                  0x00000100L /* v0-v1 edge */
#define D3DTRIFLAG_EDGEENABLE2                  0x00000200L /* v1-v2 edge */
#define D3DTRIFLAG_EDGEENABLE3                  0x00000400L /* v2-v0 edge */
#define D3DTRIFLAG_EDGEENABLETRIANGLE \
        (D3DTRIFLAG_EDGEENABLE1 | D3DTRIFLAG_EDGEENABLE2 | D3DTRIFLAG_EDGEENABLE3)
        
/*
 * Primitive structures and related defines.  Vertex offsets are to types
 * D3DVERTEX, D3DLVERTEX, or D3DTLVERTEX.
 */

/*
 * Triangle list primitive structure
 */
typedef struct _D3DTRIANGLE {
    union {
        WORD    v1;            /* Vertex indices */
        WORD    wV1;
    };
    union {
        WORD    v2;
        WORD    wV2;
    };
    union {
        WORD    v3;
        WORD    wV3;
    };
    WORD        wFlags;       /* Edge (and other) flags */
} D3DTRIANGLE, *LPD3DTRIANGLE;

/*
 * Line strip structure.
 * The instruction count - 1 defines the number of line segments.
 */
typedef struct _D3DLINE {
    union {
        WORD    v1;            /* Vertex indices */
        WORD    wV1;
    };
    union {
        WORD    v2;
        WORD    wV2;
    };
} D3DLINE, *LPD3DLINE;

/*
 * Span structure
 * Spans join a list of points with the same y value.
 * If the y value changes, a new span is started.
 */
typedef struct _D3DSPAN {
    WORD        wCount; /* Number of spans */
    WORD        wFirst; /* Index to first vertex */
} D3DSPAN, *LPD3DSPAN;

/*
 * Point structure
 */
typedef struct _D3DPOINT {
    WORD        wCount;         /* number of points         */
    WORD        wFirst;         /* index to first vertex    */
} D3DPOINT, *LPD3DPOINT;


/*
 * Forward branch structure.
 * Mask is logically anded with the driver status mask
 * if the result equals 'value', the branch is taken.
 */
typedef struct _D3DBRANCH {
    DWORD       dwMask;         /* Bitmask against D3D status */
    DWORD       dwValue;
    BOOL        bNegate;        /* TRUE to negate comparison */
    DWORD       dwOffset;       /* How far to branch forward (0 for exit)*/
} D3DBRANCH, *LPD3DBRANCH;

/*
 * Status used for set status instruction.
 * The D3D status is initialised on device creation
 * and is modified by all execute calls.
 */
typedef struct _D3DSTATUS {
    DWORD       dwFlags;        /* Do we set extents or status */
    DWORD       dwStatus;       /* D3D status */
    D3DRECT     drExtent;
} D3DSTATUS, *LPD3DSTATUS;

#define D3DSETSTATUS_STATUS             0x00000001L
#define D3DSETSTATUS_EXTENTS            0x00000002L
#define D3DSETSTATUS_ALL        (D3DSETSTATUS_STATUS | D3DSETSTATUS_EXTENTS)

/*
 * Statistics structure
 */
typedef struct _D3DSTATS {
    DWORD        dwSize;
    DWORD        dwTrianglesDrawn;
    DWORD        dwLinesDrawn;
    DWORD        dwPointsDrawn;
    DWORD        dwSpansDrawn;
    DWORD        dwVerticesProcessed;
} D3DSTATS, *LPD3DSTATS;

/*
 * Execute options.
 * When calling using D3DEXECUTE_UNCLIPPED all the primitives 
 * inside the buffer must be contained within the viewport.
 */
#define D3DEXECUTE_CLIPPED       0x00000001l
#define D3DEXECUTE_UNCLIPPED     0x00000002l

typedef struct _D3DEXECUTEDATA {
    DWORD       dwSize;
    DWORD       dwVertexOffset;
    DWORD       dwVertexCount;
    DWORD       dwInstructionOffset;
    DWORD       dwInstructionLength;
    DWORD       dwHVertexOffset;
    D3DSTATUS   dsStatus;       /* Status after execute */
} D3DEXECUTEDATA, *LPD3DEXECUTEDATA;

/*
 * Palette flags.
 * This are or'ed with the peFlags in the PALETTEENTRYs passed to DirectDraw.
 */
#define D3DPAL_FREE     0x00    /* Renderer may use this entry freely */
#define D3DPAL_READONLY 0x40    /* Renderer may not set this entry */
#define D3DPAL_RESERVED 0x80    /* Renderer may not use this entry */

#if defined(__cplusplus)
};
#endif

#pragma pack()

#endif /* _D3DTYPES_H_ */

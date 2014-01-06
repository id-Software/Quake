/*==========================================================================;
 *
 *  Copyright (C) 1995-1996 Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       d3drm.h
 *  Content:    Direct3DRM include file
 *
 ***************************************************************************/

#ifndef __D3DRM_H__
#define __D3DRM_H__

#include "d3drmobj.h"

#ifdef __cplusplus
extern "C" {
struct IDirect3DRM;
#endif


typedef void (*D3DRMDEVICEPALETTECALLBACK)
    (LPDIRECT3DRMDEVICE lpDirect3DRMDev, LPVOID lpArg, DWORD dwIndex, LONG red, LONG green, LONG blue);

DEFINE_GUID(IID_IDirect3DRM,    0x2bc49361, 0x8327, 0x11cf, 0xac, 0x4a, 0x0, 0x0, 0xc0, 0x38, 0x25, 0xa1);
WIN_TYPES(IDirect3DRM, DIRECT3DRM);


/* Create a Direct3DRM API */
STDAPI Direct3DRMCreate(LPDIRECT3DRM FAR *lplpDirect3DRM);

#undef INTERFACE
#define INTERFACE IDirect3DRM

DECLARE_INTERFACE_(IDirect3DRM, IUnknown)
{
    IUNKNOWN_METHODS(PURE);

    STDMETHOD(CreateObject)
        (THIS_ REFCLSID rclsid, LPUNKNOWN pUnkOuter, REFIID riid, LPVOID FAR* ppv) PURE;
    STDMETHOD(CreateFrame)      (THIS_ LPDIRECT3DRMFRAME, LPDIRECT3DRMFRAME *) PURE;
    STDMETHOD(CreateMesh)       (THIS_ LPDIRECT3DRMMESH *) PURE;
    STDMETHOD(CreateMeshBuilder)(THIS_ LPDIRECT3DRMMESHBUILDER *) PURE;
    STDMETHOD(CreateFace)       (THIS_ LPDIRECT3DRMFACE *) PURE;
    STDMETHOD(CreateAnimation)  (THIS_ LPDIRECT3DRMANIMATION *) PURE;
    STDMETHOD(CreateAnimationSet)(THIS_ LPDIRECT3DRMANIMATIONSET *) PURE;
    STDMETHOD(CreateTexture)    (THIS_ LPD3DRMIMAGE, LPDIRECT3DRMTEXTURE *) PURE;
    STDMETHOD(CreateLight)      (THIS_ D3DRMLIGHTTYPE, D3DCOLOR, LPDIRECT3DRMLIGHT *) PURE;
    STDMETHOD(CreateLightRGB)
        (THIS_ D3DRMLIGHTTYPE, D3DVALUE, D3DVALUE, D3DVALUE, LPDIRECT3DRMLIGHT *) PURE;
    STDMETHOD(CreateMaterial)   (THIS_ D3DVALUE, LPDIRECT3DRMMATERIAL *) PURE;
    STDMETHOD(CreateDevice)     (THIS_ DWORD, DWORD, LPDIRECT3DRMDEVICE *) PURE;

    /* Create a Windows Device using DirectDraw surfaces */
    STDMETHOD(CreateDeviceFromSurface)
    (   THIS_ LPGUID lpGUID, LPDIRECTDRAW lpDD,
        LPDIRECTDRAWSURFACE lpDDSBack, LPDIRECT3DRMDEVICE *
    ) PURE;

    /* Create a Windows Device using D3D objects */
    STDMETHOD(CreateDeviceFromD3D)
    (   THIS_ LPDIRECT3D lpD3D, LPDIRECT3DDEVICE lpD3DDev,
        LPDIRECT3DRMDEVICE *
    ) PURE;

    STDMETHOD(CreateDeviceFromClipper)
    (   THIS_ LPDIRECTDRAWCLIPPER lpDDClipper, LPGUID lpGUID,
        int width, int height, LPDIRECT3DRMDEVICE *) PURE;

    STDMETHOD(CreateTextureFromSurface)(THIS_ LPDIRECTDRAWSURFACE lpDDS, LPDIRECT3DRMTEXTURE *) PURE;
   
    STDMETHOD(CreateShadow)
    (   THIS_ LPDIRECT3DRMVISUAL, LPDIRECT3DRMLIGHT,
        D3DVALUE px, D3DVALUE py, D3DVALUE pz,
        D3DVALUE nx, D3DVALUE ny, D3DVALUE nz,
        LPDIRECT3DRMVISUAL *
    ) PURE;
    STDMETHOD(CreateViewport)
    (   THIS_ LPDIRECT3DRMDEVICE, LPDIRECT3DRMFRAME, DWORD, DWORD,
        DWORD, DWORD, LPDIRECT3DRMVIEWPORT *
    ) PURE;
    STDMETHOD(CreateWrap)
    (   THIS_ D3DRMWRAPTYPE, LPDIRECT3DRMFRAME,
        D3DVALUE ox, D3DVALUE oy, D3DVALUE oz,
        D3DVALUE dx, D3DVALUE dy, D3DVALUE dz,
        D3DVALUE ux, D3DVALUE uy, D3DVALUE uz,
        D3DVALUE ou, D3DVALUE ov,
        D3DVALUE su, D3DVALUE sv,
        LPDIRECT3DRMWRAP *
    ) PURE;
    STDMETHOD(CreateUserVisual) (THIS_ D3DRMUSERVISUALCALLBACK, LPVOID lPArg, LPDIRECT3DRMUSERVISUAL *) PURE;
    STDMETHOD(LoadTexture)      (THIS_ const char *, LPDIRECT3DRMTEXTURE *) PURE;
    STDMETHOD(LoadTextureFromResource)  (THIS_ HRSRC rs, LPDIRECT3DRMTEXTURE *) PURE;
   
    STDMETHOD(SetSearchPath)    (THIS_ LPCSTR) PURE;
    STDMETHOD(AddSearchPath)    (THIS_ LPCSTR) PURE;
    STDMETHOD(GetSearchPath)    (THIS_ DWORD *size_return, LPSTR path_return);
    //STDMETHOD(GetSearchPath)  (THIS_ int *return_count, char ***return_path) PURE;
    STDMETHOD(SetDefaultTextureColors)(THIS_ DWORD) PURE;
    STDMETHOD(SetDefaultTextureShades)(THIS_ DWORD) PURE;
   
    STDMETHOD(GetDevices)       (THIS_ LPDIRECT3DRMDEVICEARRAY *) PURE;
    STDMETHOD(GetNamedObject)   (THIS_ const char *, LPDIRECT3DRMOBJECT *) PURE;
   
    STDMETHOD(EnumerateObjects) (THIS_ D3DRMOBJECTCALLBACK, LPVOID) PURE;
   
    STDMETHOD(Load)             
    (   THIS_ LPVOID, LPVOID, LPIID *, DWORD, D3DRMLOADOPTIONS,
        D3DRMLOADCALLBACK, LPVOID, D3DRMLOADTEXTURECALLBACK, LPVOID,
        LPDIRECT3DRMFRAME
    ) PURE;
    STDMETHOD(Tick)             (THIS_ D3DVALUE) PURE;
};

#define D3DRM_OK                        DD_OK
#define D3DRMERR_BADOBJECT              MAKE_DDHRESULT(781)
#define D3DRMERR_BADTYPE                MAKE_DDHRESULT(782)
#define D3DRMERR_BADALLOC               MAKE_DDHRESULT(783)
#define D3DRMERR_FACEUSED               MAKE_DDHRESULT(784)
#define D3DRMERR_NOTFOUND               MAKE_DDHRESULT(785)
#define D3DRMERR_NOTDONEYET             MAKE_DDHRESULT(786)
#define D3DRMERR_FILENOTFOUND           MAKE_DDHRESULT(787)
#define D3DRMERR_BADFILE                MAKE_DDHRESULT(788)
#define D3DRMERR_BADDEVICE              MAKE_DDHRESULT(789)
#define D3DRMERR_BADVALUE               MAKE_DDHRESULT(790)
#define D3DRMERR_BADMAJORVERSION        MAKE_DDHRESULT(791)
#define D3DRMERR_BADMINORVERSION        MAKE_DDHRESULT(792)
#define D3DRMERR_UNABLETOEXECUTE        MAKE_DDHRESULT(793)

#ifdef __cplusplus
};
#endif

#endif /* _D3DRMAPI_H_ */


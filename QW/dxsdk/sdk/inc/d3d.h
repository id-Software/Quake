/*==========================================================================;
 *
 *  Copyright (C) 1995-1996 Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       d3d.h
 *  Content:    Direct3D include file
 *
 ***************************************************************************/

#ifndef _D3D_H_
#define _D3D_H_

#include <stdlib.h>

#ifdef _WIN32
#define COM_NO_WINDOWS_H
#include <objbase.h>
#else
#include "d3dcom.h"
#endif

#ifdef _WIN32
#define D3DAPI WINAPI
#else
#define D3DAPI
#endif

/*
 * Interface IID's
 */
#if defined( _WIN32 ) && !defined( _NO_COM)
DEFINE_GUID( IID_IDirect3D,             0x3BBA0080,0x2421,0x11CF,0xA3,0x1A,0x00,0xAA,0x00,0xB9,0x33,0x56 );
DEFINE_GUID( IID_IDirect3DTexture,      0x2CDCD9E0,0x25A0,0x11CF,0xA3,0x1A,0x00,0xAA,0x00,0xB9,0x33,0x56 );
DEFINE_GUID( IID_IDirect3DLight,        0x4417C142,0x33AD,0x11CF,0x81,0x6F,0x00,0x00,0xC0,0x20,0x15,0x6E );
DEFINE_GUID( IID_IDirect3DMaterial,     0x4417C144,0x33AD,0x11CF,0x81,0x6F,0x00,0x00,0xC0,0x20,0x15,0x6E );
DEFINE_GUID( IID_IDirect3DExecuteBuffer,0x4417C145,0x33AD,0x11CF,0x81,0x6F,0x00,0x00,0xC0,0x20,0x15,0x6E );
DEFINE_GUID( IID_IDirect3DViewport,     0x4417C146,0x33AD,0x11CF,0x81,0x6F,0x00,0x00,0xC0,0x20,0x15,0x6E );
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Data structures
 */
#ifdef __cplusplus

/* 'struct' not 'class' per the way DECLARE_INTERFACE_ is defined */
struct IDirect3D;
struct IDirect3DDevice;
struct IDirect3DExecuteBuffer;
struct IDirect3DLight;
struct IDirect3DMaterial;
struct IDirect3DTexture;
struct IDirect3DViewport;
typedef struct IDirect3D                *LPDIRECT3D;
typedef struct IDirect3DDevice          *LPDIRECT3DDEVICE;
typedef struct IDirect3DExecuteBuffer   *LPDIRECT3DEXECUTEBUFFER;
typedef struct IDirect3DLight           *LPDIRECT3DLIGHT;
typedef struct IDirect3DMaterial        *LPDIRECT3DMATERIAL;
typedef struct IDirect3DTexture         *LPDIRECT3DTEXTURE;
typedef struct IDirect3DViewport        *LPDIRECT3DVIEWPORT;

#else

typedef struct IDirect3D                *LPDIRECT3D;
typedef struct IDirect3DDevice          *LPDIRECT3DDEVICE;
typedef struct IDirect3DExecuteBuffer   *LPDIRECT3DEXECUTEBUFFER;
typedef struct IDirect3DLight           *LPDIRECT3DLIGHT;
typedef struct IDirect3DMaterial        *LPDIRECT3DMATERIAL;
typedef struct IDirect3DTexture         *LPDIRECT3DTEXTURE;
typedef struct IDirect3DViewport        *LPDIRECT3DVIEWPORT;

#endif

#include "d3dtypes.h"
#include "d3dcaps.h"

/*
 * IDirect3D
 */
#undef INTERFACE
#define INTERFACE IDirect3D
DECLARE_INTERFACE_(IDirect3D, IUnknown)
{
    /*** IUnknown methods ***/
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;
    /*** IDirect3D methods ***/
    STDMETHOD(Initialize) (THIS_ REFIID) PURE;
    STDMETHOD(EnumDevices)(THIS_ LPD3DENUMDEVICESCALLBACK, LPVOID) PURE;
    STDMETHOD(CreateLight) (THIS_ LPDIRECT3DLIGHT*, IUnknown*) PURE;
    STDMETHOD(CreateMaterial) (THIS_ LPDIRECT3DMATERIAL*, IUnknown*) PURE;
    STDMETHOD(CreateViewport) (THIS_ LPDIRECT3DVIEWPORT*, IUnknown*) PURE;
    STDMETHOD(FindDevice)(THIS_ LPD3DFINDDEVICESEARCH, LPD3DFINDDEVICERESULT) PURE;
};

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirect3D_QueryInterface(p, a, b)         (p)->lpVtbl->QueryInterface(p, a, b)
#define IDirect3D_AddRef(p)                       (p)->lpVtbl->AddRef(p)
#define IDirect3D_Release(p)                      (p)->lpVtbl->Release(p)
#define IDirect3D_Initialize(p, a)                (p)->lpVtbl->Initialize(p, a)
#define IDirect3D_EnumDevices(p, a, b)            (p)->lpVtbl->EnumDevices(p, a, b)
#define IDirect3D_CreateLight(p, a, b)            (p)->lpVtbl->CreateLight(p, a, b)
#define IDirect3D_CreateMaterial(p, a, b)         (p)->lpVtbl->CreateMaterial(p, a, b)
#define IDirect3D_CreateViewport(p, a, b)         (p)->lpVtbl->CreateViewport(p, a, b)
#define IDirect3D_FindDevice(p, a, b)             (p)->lpVtbl->FindDevice(p, a, b)
#endif

/*
 * IDirect3DDevice
 */
#undef INTERFACE
#define INTERFACE IDirect3DDevice
DECLARE_INTERFACE_(IDirect3DDevice, IUnknown)
{
    /*** IUnknown methods ***/
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;
    /*** IDirect3DDevice methods ***/
    STDMETHOD(Initialize) (THIS_ LPDIRECT3D, LPGUID, LPD3DDEVICEDESC) PURE;
    STDMETHOD(GetCaps) (THIS_ LPD3DDEVICEDESC, LPD3DDEVICEDESC) PURE;
    STDMETHOD(SwapTextureHandles) (THIS_ LPDIRECT3DTEXTURE, LPDIRECT3DTEXTURE) PURE;
    STDMETHOD(CreateExecuteBuffer) (THIS_ LPD3DEXECUTEBUFFERDESC, LPDIRECT3DEXECUTEBUFFER*, IUnknown*) PURE;
    STDMETHOD(GetStats) (THIS_ LPD3DSTATS) PURE;
    STDMETHOD(Execute) (THIS_ LPDIRECT3DEXECUTEBUFFER, LPDIRECT3DVIEWPORT, DWORD) PURE;
    STDMETHOD(AddViewport) (THIS_ LPDIRECT3DVIEWPORT) PURE;
    STDMETHOD(DeleteViewport) (THIS_ LPDIRECT3DVIEWPORT) PURE;
    STDMETHOD(NextViewport) (THIS_ LPDIRECT3DVIEWPORT, LPDIRECT3DVIEWPORT*, DWORD) PURE;
    STDMETHOD(Pick) (THIS_ LPDIRECT3DEXECUTEBUFFER, LPDIRECT3DVIEWPORT, DWORD, LPD3DRECT) PURE;
    STDMETHOD(GetPickRecords)(THIS_ LPDWORD, LPD3DPICKRECORD) PURE;
    STDMETHOD(EnumTextureFormats) (THIS_ LPD3DENUMTEXTUREFORMATSCALLBACK, LPVOID) PURE;
    STDMETHOD(CreateMatrix) (THIS_ LPD3DMATRIXHANDLE) PURE;
    STDMETHOD(SetMatrix) (THIS_ D3DMATRIXHANDLE, LPD3DMATRIX) PURE;
    STDMETHOD(GetMatrix) (THIS_ D3DMATRIXHANDLE, LPD3DMATRIX) PURE;
    STDMETHOD(DeleteMatrix) (THIS_ D3DMATRIXHANDLE) PURE;
    STDMETHOD_(HRESULT, BeginScene) (THIS) PURE;
    STDMETHOD_(HRESULT, EndScene) (THIS) PURE;
    STDMETHOD(GetDirect3D) (THIS_ LPDIRECT3D*) PURE;
};

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirect3DDevice_QueryInterface(p, a, b)         (p)->lpVtbl->QueryInterface(p, a, b)
#define IDirect3DDevice_AddRef(p)                       (p)->lpVtbl->AddRef(p)
#define IDirect3DDevice_Release(p)                      (p)->lpVtbl->Release(p)
#define IDirect3DDevice_Initialize(p, a, b, c)          (p)->lpVtbl->Initialize(p, a, b, c)
#define IDirect3DDevice_GetCaps(p, a, b)                (p)->lpVtbl->GetCaps(p, a, b)
#define IDirect3DDevice_SwapTextureHandles(p, a, b)     (p)->lpVtbl->SwapTextureHandles(p, a, b)
#define IDirect3DDevice_CreateExecuteBuffer(p, a, b, c) (p)->lpVtbl->ExecuteBuffer(p, a, b, c)
#define IDirect3DDevice_GetStats(p, a)                  (p)->lpVtbl->CreateViewport(p, a)
#define IDirect3DDevice_Execute(p, a, b, c)             (p)->lpVtbl->Execute(p, a, b, c)
#define IDirect3DDevice_AddViewport(p, a)               (p)->lpVtbl->AddViewport(p, a)
#define IDirect3DDevice_DeleteViewport(p, a)            (p)->lpVtbl->DeleteViewport(p, a)
#define IDirect3DDevice_NextViewport(p, a, b)           (p)->lpVtbl->NextViewport(p, a, b)
#define IDirect3DDevice_Pick(p, a, b, c, d)             (p)->lpVtbl->Pick(p, a, b, c, d)
#define IDirect3DDevice_GetPickRecords(p, a, b)         (p)->lpVtbl->GetPickRecords(p, a, b)
#define IDirect3DDevice_EnumTextureFormats(p, a, b)     (p)->lpVtbl->EnumTextureFormats(p, a, b)
#define IDirect3DDevice_CreateMatrix(p, a)              (p)->lpVtbl->CreateMatrix(p, a)
#define IDirect3DDevice_SetMatrix(p, a, b)              (p)->lpVtbl->SetMatrix(p, a, b)
#define IDirect3DDevice_GetMatrix(p, a, b)              (p)->lpVtbl->GetMatrix(p, a, b)
#define IDirect3DDevice_DeleteMatrix(p, a)              (p)->lpVtbl->DeleteMatrix(p, a)
#define IDirect3DDevice_BeginScene(p)                   (p)->lpVtbl->BeginScene(p)
#define IDirect3DDevice_EndScene(p)                     (p)->lpVtbl->EndScene(p)
#define IDirect3DDevice_GetDirect3D(p, a)               (p)->lpVtbl->GetDirect3D(p, a)
#endif

/*
 * IDirect3DExecuteBuffer
 */
#undef INTERFACE
#define INTERFACE IDirect3DExecuteBuffer
DECLARE_INTERFACE_(IDirect3DExecuteBuffer, IUnknown)
{
    /*** IUnknown methods ***/
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;
    /*** IDirect3DExecuteBuffer methods ***/
    STDMETHOD(Initialize) (THIS_ LPDIRECT3DDEVICE, LPD3DEXECUTEBUFFERDESC) PURE;
    STDMETHOD(Lock) (THIS_ LPD3DEXECUTEBUFFERDESC) PURE;
    STDMETHOD_(HRESULT, Unlock) (THIS) PURE;
    STDMETHOD(SetExecuteData) (THIS_ LPD3DEXECUTEDATA) PURE;
    STDMETHOD(GetExecuteData) (THIS_ LPD3DEXECUTEDATA) PURE;
    STDMETHOD(Validate) (THIS_ LPDWORD, LPD3DVALIDATECALLBACK, LPVOID, DWORD) PURE;
    STDMETHOD(Optimize) (THIS_ DWORD) PURE;
};

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirect3DExecuteBuffer_QueryInterface(p, a, b) (p)->lpVtbl->QueryInterface(p, a, b)
#define IDirect3DExecuteBuffer_AddRef(p)               (p)->lpVtbl->AddRef(p)
#define IDirect3DExecuteBuffer_Release(p)              (p)->lpVtbl->Release(p)
#define IDirect3DExecuteBuffer_Initialize(p, a, b)     (p)->lpVtbl->Initialize(p, a, b)
#define IDirect3DExecuteBuffer_Lock(p, a)              (p)->lpVtbl->Lock(p, a)
#define IDirect3DExecuteBuffer_Unlock(p, a)            (p)->lpVtbl->Unlock(p)
#define IDirect3DExecuteBuffer_SetExecuteData(p, a)    (p)->lpVtbl->SetExecuteData(p, a)
#define IDirect3DExecuteBuffer_Validate(p, a, b, c, d) (p)->lpVtbl->Validata(p, a, b, c, d)
#endif

/*
 * Flags for execute buffer calls
 */
#define D3DNEXT_NEXT    0x00000001l
#define D3DNEXT_HEAD    0x00000002l
#define D3DNEXT_TAIL    0x00000004l

/*
 * IDirect3DLight
 */
#undef INTERFACE
#define INTERFACE IDirect3DLight
DECLARE_INTERFACE_(IDirect3DLight, IUnknown)
{
    /*** IUnknown methods ***/
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;
    /*** IDirect3DLight methods ***/
    STDMETHOD(Initialize) (THIS_ LPDIRECT3D) PURE;
    STDMETHOD(SetLight) (THIS_ LPD3DLIGHT) PURE;
    STDMETHOD(GetLight) (THIS_ LPD3DLIGHT) PURE;
};

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirect3DLight_QueryInterface(p, a, b) (p)->lpVtbl->QueryInterface(p, a, b)
#define IDirect3DLight_AddRef(p)               (p)->lpVtbl->AddRef(p)
#define IDirect3DLight_Release(p)              (p)->lpVtbl->Release(p)
#define IDirect3DLight_Initialize(p, a)        (p)->lpVtbl->Initialize(p, a)
#define IDirect3DLight_SetLight(p, a)          (p)->lpVtbl->SetLight(p, a)
#define IDirect3DLight_GetLight(p, a)          (p)->lpVtbl->GetLight(p, a)
#endif

/*
 * IDirect3DMaterial
 */
#undef INTERFACE
#define INTERFACE IDirect3DMaterial
DECLARE_INTERFACE_(IDirect3DMaterial, IUnknown)
{
    /*** IUnknown methods ***/
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;
    /*** IDirect3DMaterial methods ***/
    STDMETHOD(Initialize) (THIS_ LPDIRECT3D) PURE;
    STDMETHOD(SetMaterial) (THIS_ LPD3DMATERIAL) PURE;
    STDMETHOD(GetMaterial) (THIS_ LPD3DMATERIAL) PURE;
    STDMETHOD(GetHandle) (THIS_ LPDIRECT3DDEVICE, LPD3DMATERIALHANDLE) PURE;
    STDMETHOD_(HRESULT, Reserve) (THIS) PURE;
    STDMETHOD_(HRESULT, Unreserve) (THIS) PURE;
};

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirect3DMaterial_QueryInterface(p, a, b) (p)->lpVtbl->QueryInterface(p, a, b)
#define IDirect3DMaterial_AddRef(p)               (p)->lpVtbl->AddRef(p)
#define IDirect3DMaterial_Release(p)              (p)->lpVtbl->Release(p)
#define IDirect3DMaterial_Initialize(p, a)        (p)->lpVtbl->Initialize(p, a)
#define IDirect3DMaterial_SetMaterial(p, a)       (p)->lpVtbl->SetLight(p, a)
#define IDirect3DMaterial_GetMaterial(p, a)       (p)->lpVtbl->GetLight(p, a)
#define IDirect3DMaterial_GetHandle(p, a, b)      (p)->lpVtbl->GetHandle(p, a, b)
#define IDirect3DMaterial_Reserve(p)              (p)->lpVtbl->Reserve(p)
#define IDirect3DMaterial_Unreserve(p)            (p)->lpVtbl->Unreserve(p)
#endif

/*
 * IDirect3DTexture
 */
#undef INTERFACE
#define INTERFACE IDirect3DTexture
DECLARE_INTERFACE_(IDirect3DTexture, IUnknown)
{
    /*** IUnknown methods ***/
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;
    /*** IDirect3DTexture methods ***/
    STDMETHOD(Initialize) (THIS_ LPDIRECT3DDEVICE, LPDIRECTDRAWSURFACE) PURE;
    STDMETHOD(GetHandle) (THIS_ LPDIRECT3DDEVICE, LPD3DTEXTUREHANDLE) PURE;
    STDMETHOD(PaletteChanged) (THIS_ DWORD, DWORD) PURE;
    STDMETHOD(Load) (THIS_ LPDIRECT3DTEXTURE) PURE;
    STDMETHOD_(HRESULT, Unload) (THIS) PURE;
};

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirect3DTexture_QueryInterface(p, a, b) (p)->lpVtbl->QueryInterface(p, a, b)
#define IDirect3DTexture_AddRef(p)               (p)->lpVtbl->AddRef(p)
#define IDirect3DTexture_Release(p)              (p)->lpVtbl->Release(p)
#define IDirect3DTexture_Initialize(p, a, b)     (p)->lpVtbl->Initialize(p, a, b)
#define IDirect3DTexture_GetHandle(p, a, b)      (p)->lpVtbl->GetHandle(p, a, b)
#define IDirect3DTexture_PaletteChanged(p, a, b) (p)->lpVtbl->PaletteChanged(p, a, b)
#define IDirect3DTexture_Load(p, a)              (p)->lpVtbl->Load(p, a)
#define IDirect3DTexture_Unload(p)               (p)->lpVtbl->Unload(p)
#endif

/*
 * IDirect3DViewport
 */
#undef INTERFACE
#define INTERFACE IDirect3DViewport
DECLARE_INTERFACE_(IDirect3DViewport, IUnknown)
{
    /*** IUnknown methods ***/
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;
    /*** IDirect3DViewport methods ***/
    STDMETHOD(Initialize) (THIS_ LPDIRECT3D) PURE;
    STDMETHOD(GetViewport) (THIS_ LPD3DVIEWPORT) PURE;
    STDMETHOD(SetViewport) (THIS_ LPD3DVIEWPORT) PURE;
    STDMETHOD(TransformVertices) (THIS_ DWORD, LPD3DTRANSFORMDATA, DWORD, LPDWORD) PURE;
    STDMETHOD(LightElements) (THIS_ DWORD, LPD3DLIGHTDATA) PURE;
    STDMETHOD(SetBackground) (THIS_ D3DMATERIALHANDLE) PURE;
    STDMETHOD(GetBackground) (THIS_ LPD3DMATERIALHANDLE, LPBOOL) PURE;
    STDMETHOD(SetBackgroundDepth) (THIS_ LPDIRECTDRAWSURFACE) PURE;
    STDMETHOD(GetBackgroundDepth) (THIS_ LPDIRECTDRAWSURFACE*, LPBOOL) PURE;
    STDMETHOD(Clear) (THIS_ DWORD, LPD3DRECT, DWORD) PURE;
    STDMETHOD(AddLight) (THIS_ LPDIRECT3DLIGHT) PURE;
    STDMETHOD(DeleteLight) (THIS_ LPDIRECT3DLIGHT) PURE;
    STDMETHOD(NextLight) (THIS_ LPDIRECT3DLIGHT, LPDIRECT3DLIGHT*, DWORD) PURE;
};

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirect3DViewport_QueryInterface(p, a, b)          (p)->lpVtbl->QueryInterface(p, a, b)
#define IDirect3DViewport_AddRef(p)                        (p)->lpVtbl->AddRef(p)
#define IDirect3DViewport_Release(p)                       (p)->lpVtbl->Release(p)
#define IDirect3DViewport_Initialize(p, a)                 (p)->lpVtbl->Initialize(p, a)
#define IDirect3DViewport_GetViewport(p, a)                (p)->lpVtbl->GetViewport(p, a)
#define IDirect3DViewport_SetViewport(p, a)                (p)->lpVtbl->SetViewport(p, a)
#define IDirect3DViewport_TransformVertices(p, a, b, c, d) (p)->lpVtbl->TransformVertices(p, a, b, c, d)
#define IDirect3DViewport_LightElements(p, a, b)           (p)->lpVtbl->LightElements(p, a, b)
#define IDirect3DViewport_SetBackground(p, a)              (p)->lpVtbl->SetBackground(p, a)
#define IDirect3DViewport_GetBackground(p, a, b)           (p)->lpVtbl->GetBackground(p, a, b)
#define IDirect3DViewport_Clear(p, a, b, c)                (p)->lpVtbl->Clear(p, a, b, c)
#define IDirect3DViewport_AddLight(p, a)                   (p)->lpVtbl->AddLight(p, a)
#define IDirect3DViewport_DeleteLight(p, a)                (p)->lpVtbl->DeleteLight(p, a)
#define IDirect3DViewport_NextLight(p, a, b, c)            (p)->lpVtbl->NextLight(p, a, b, c)
#endif

/*
 * Direct3D Errors
 * DirectDraw error codes are used when errors not specified here.
 */
#define D3D_OK                          DD_OK
#define D3DERR_BADMAJORVERSION          MAKE_DDHRESULT(700)
#define D3DERR_BADMINORVERSION          MAKE_DDHRESULT(701)

#define D3DERR_EXECUTE_CREATE_FAILED    MAKE_DDHRESULT(710)
#define D3DERR_EXECUTE_DESTROY_FAILED   MAKE_DDHRESULT(711)
#define D3DERR_EXECUTE_LOCK_FAILED      MAKE_DDHRESULT(712)
#define D3DERR_EXECUTE_UNLOCK_FAILED    MAKE_DDHRESULT(713)
#define D3DERR_EXECUTE_LOCKED           MAKE_DDHRESULT(714)
#define D3DERR_EXECUTE_NOT_LOCKED       MAKE_DDHRESULT(715)

#define D3DERR_EXECUTE_FAILED           MAKE_DDHRESULT(716)
#define D3DERR_EXECUTE_CLIPPED_FAILED   MAKE_DDHRESULT(717)

#define D3DERR_TEXTURE_NO_SUPPORT       MAKE_DDHRESULT(720)
#define D3DERR_TEXTURE_CREATE_FAILED    MAKE_DDHRESULT(721)
#define D3DERR_TEXTURE_DESTROY_FAILED   MAKE_DDHRESULT(722)
#define D3DERR_TEXTURE_LOCK_FAILED      MAKE_DDHRESULT(723)
#define D3DERR_TEXTURE_UNLOCK_FAILED    MAKE_DDHRESULT(724)
#define D3DERR_TEXTURE_LOAD_FAILED      MAKE_DDHRESULT(725)
#define D3DERR_TEXTURE_SWAP_FAILED      MAKE_DDHRESULT(726)
#define D3DERR_TEXTURE_LOCKED           MAKE_DDHRESULT(727)
#define D3DERR_TEXTURE_NOT_LOCKED       MAKE_DDHRESULT(728)
#define D3DERR_TEXTURE_GETSURF_FAILED   MAKE_DDHRESULT(729)

#define D3DERR_MATRIX_CREATE_FAILED     MAKE_DDHRESULT(730)
#define D3DERR_MATRIX_DESTROY_FAILED    MAKE_DDHRESULT(731)
#define D3DERR_MATRIX_SETDATA_FAILED    MAKE_DDHRESULT(732)
#define D3DERR_MATRIX_GETDATA_FAILED    MAKE_DDHRESULT(733)
#define D3DERR_SETVIEWPORTDATA_FAILED   MAKE_DDHRESULT(734)

#define D3DERR_MATERIAL_CREATE_FAILED   MAKE_DDHRESULT(740)
#define D3DERR_MATERIAL_DESTROY_FAILED  MAKE_DDHRESULT(741)
#define D3DERR_MATERIAL_SETDATA_FAILED  MAKE_DDHRESULT(742)
#define D3DERR_MATERIAL_GETDATA_FAILED  MAKE_DDHRESULT(743)

#define D3DERR_LIGHT_SET_FAILED         MAKE_DDHRESULT(750)

#define D3DERR_SCENE_IN_SCENE           MAKE_DDHRESULT(760)
#define D3DERR_SCENE_NOT_IN_SCENE       MAKE_DDHRESULT(761)
#define D3DERR_SCENE_BEGIN_FAILED       MAKE_DDHRESULT(762)
#define D3DERR_SCENE_END_FAILED         MAKE_DDHRESULT(763)

#ifdef __cplusplus
};
#endif

#endif /* _D3D_H_ */

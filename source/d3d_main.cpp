//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	$Id$
//**
//**	Copyright (C) 1999-2001 JÆnis Legzdi·ý
//**
//**	This program is free software; you can redistribute it and/or
//**  modify it under the terms of the GNU General Public License
//**  as published by the Free Software Foundation; either version 2
//**  of the License, or (at your option) any later version.
//**
//**	This program is distributed in the hope that it will be useful,
//**  but WITHOUT ANY WARRANTY; without even the implied warranty of
//**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//**  GNU General Public License for more details.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "d3d_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static TDirect3DDrawer		Direct3DDrawer;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	TDirect3DDrawer::TDirect3DDrawer
//
//==========================================================================

TDirect3DDrawer::TDirect3DDrawer(void) :
	device("d3d_device", "0", CVAR_ARCHIVE),
	clear("d3d_clear", "0", CVAR_ARCHIVE),
	tex_linear("d3d_tex_linear", "1", CVAR_ARCHIVE),
	dither("d3d_dither", "0", CVAR_ARCHIVE),
	IdentityMatrix(	1, 0, 0, 0,
					0, 1, 0, 0,
					0, 0, 1, 0,
					0, 0, 0, 1)
{
	_Direct3DDrawer = this;

	//	DirectDraw interfaces
	DDraw = NULL;
	PrimarySurface = NULL;
	RenderSurface = NULL;
	ZBuffer = NULL;

	//	Direct3D interfaces
	Direct3D = NULL;
	RenderDevice = NULL;
}

//==========================================================================
//
//	TDirect3DDrawer::Init
//
// 	Determine the hardware configuration
//
//==========================================================================

void TDirect3DDrawer::Init(void)
{
	HRESULT			result;

	// Create DirectDraw object
	result = CoCreateInstance(CLSID_DirectDraw7, NULL,
		CLSCTX_ALL, IID_IDirectDraw7, (void**)&DDraw);
	if (result != DD_OK)
		Sys_Error("I_InitGraphics: Failed to create DirecDraw7");

	// Initialize
	result = DDraw->Initialize(NULL);
	if (result != DD_OK)
		Sys_Error("I_InitGraphics: Failed to initialize DirectDraw");

	// Set cooperative level
	result = DDraw->SetCooperativeLevel(hwnd,
		DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_FPUPRESERVE);
	if (result != DD_OK)
		Sys_Error("I_InitGraphics: Failed to set cooperative level.");

	// Create Direct3D object
	result = DDraw->QueryInterface(IID_IDirect3D7, (void**)&Direct3D);
	if (FAILED(result))
		Sys_Error("Failed to create Direct3D object");
}

//==========================================================================
//
//	TDirect3DDrawer::InitData
//
//==========================================================================

void TDirect3DDrawer::InitData(void)
{
}

//==========================================================================
//
//	EnumDevicesCallback
//
//==========================================================================

static HRESULT CALLBACK EnumDevicesCallback(
	LPSTR lpDeviceDesc,
	LPSTR lpDeviceName,
	LPD3DDEVICEDESC7 lpD3DDeviceDesc,
	LPVOID lpContext)
{
	cond << "Device " << lpDeviceName << endl;
	cond << "Description: " << lpDeviceDesc << endl;
	cond << lpD3DDeviceDesc;
	cond << "-------------------------------------\n";

	return D3DENUMRET_OK;
}

//==========================================================================
//
//	EnumZBufferCallback
//
//==========================================================================

static HRESULT CALLBACK EnumZBufferCallback(LPDDPIXELFORMAT pf, void* dst)
{
	cond << "Z buffer format\n"
		<< pf
		<< "-------------------------------------\n";

	// Choose best available z-buffer bit depth
	if (pf->dwFlags == DDPF_ZBUFFER &&
		pf->dwZBufferBitDepth > ((LPDDPIXELFORMAT)dst)->dwZBufferBitDepth)
	{
		memcpy(dst, pf, sizeof(DDPIXELFORMAT));
	}
 
	// Return with D3DENUMRET_OK to continue the search.
	return D3DENUMRET_OK;
}

//==========================================================================
//
//	EnumPixelFormatsCallback
//
//==========================================================================

static HRESULT CALLBACK EnumPixelFormatsCallback(LPDDPIXELFORMAT pf, void* dst)
{
	cond << "Pixel format\n"
		<< pf
		<< "-------------------------------------\n";

	if ((pf->dwFlags == (DDPF_RGB|DDPF_ALPHAPIXELS)) &&
		(pf->dwRGBBitCount == 16))
	{
		memcpy(dst, pf, sizeof(DDPIXELFORMAT));
 
		// Return with D3DENUMRET_CANCEL to end the search.
		return D3DENUMRET_CANCEL;
	}

	// Return with D3DENUMRET_OK to continue the search.
	return D3DENUMRET_OK;
}

//==========================================================================
//
//	EnumPixelFormats32Callback
//
//==========================================================================

static HRESULT CALLBACK EnumPixelFormats32Callback(LPDDPIXELFORMAT pf, void* dst)
{
	if ((pf->dwFlags == (DDPF_RGB|DDPF_ALPHAPIXELS)) &&
		(pf->dwRGBBitCount == 32))
	{
		memcpy(dst, pf, sizeof(DDPIXELFORMAT));

		// Return with D3DENUMRET_CANCEL to end the search.
		return D3DENUMRET_CANCEL;
	}

	// Return with D3DENUMRET_OK to continue the search.
	return D3DENUMRET_OK;
}

//==========================================================================
//
//	GetBits
//
//==========================================================================

inline int GetBits(dword mask)
{
	int answer = 0;
	while (mask)
	{
		if (mask & 1)
		{
			answer++;
		}
		mask >>= 1;
	}
	return answer;
}

//==========================================================================
//
//	GetShift
//
//==========================================================================

inline int GetShift(dword mask)
{
	if (!mask)
		return 0;
	int answer = 0;
	while (!(mask & 1))
	{
		answer++;
		mask >>= 1;
	}
	return answer;
}

//==========================================================================
//
// 	TDirect3DDrawer::SetResolution
//
// 	Set up the video mode
//
//==========================================================================

bool TDirect3DDrawer::SetResolution(int Width, int Height, int BPP)
{
	if (!Width || !Height)
	{
		//	Set defaults
		Width = 640;
		Height = 480;
		BPP = 16;
	}

	if (BPP == 15) BPP = 16;

	if (BPP < 16)
	{
		//	True-color only
		return false;
	}

	con << "-------------------------------------\n";

	DDSURFACEDESC2	ddsd;

	//	Shut down current mode
	ReleaseTextures();
	SAFE_RELEASE(RenderDevice)
	SAFE_RELEASE(ZBuffer)
	SAFE_RELEASE(RenderSurface)
	SAFE_RELEASE(PrimarySurface)

	//	Set video mode
	if (DDraw->SetDisplayMode(Width, Height, BPP, 0, 0) != DD_OK)
		return false;

	//	Create primary surface
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	if (DDraw->CreateSurface(&ddsd, &PrimarySurface, NULL) != DD_OK)
		Sys_Error("I_SetResolution: Failed to create primary surface");

	Direct3D->EnumDevices(EnumDevicesCallback, NULL);

	GUID DeviceGUID;
	switch (device)
	{
	 default:
		device = 0;
	 case 0:
		DeviceGUID = IID_IDirect3DHALDevice;
		SurfaceMemFlag = DDSCAPS_VIDEOMEMORY;
		break;

	 case 1:
		DeviceGUID = IID_IDirect3DRGBDevice;
		SurfaceMemFlag = DDSCAPS_SYSTEMMEMORY;
		break;

	 case 2:
		DeviceGUID = IID_IDirect3DRefDevice;
		SurfaceMemFlag = DDSCAPS_SYSTEMMEMORY;
		break;
	}

	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE | SurfaceMemFlag;
    ddsd.dwWidth  = Width;
    ddsd.dwHeight = Height;
 
    //	Create the back buffer.
	if (FAILED(DDraw->CreateSurface(&ddsd, &RenderSurface, NULL)))
		return false;

    //	Create the z-buffer
    DDPIXELFORMAT ddpfZBuffer;
	memset(&ddpfZBuffer, 0, sizeof(ddpfZBuffer));
	Direct3D->EnumZBufferFormats(DeviceGUID,
		EnumZBufferCallback, (VOID*)&ddpfZBuffer);

	//	If the enumerated format is good (it should be), the dwSize member
	// will be properly initialized. Check this just in case.
	if (sizeof(DDPIXELFORMAT) != ddpfZBuffer.dwSize)
		return false;
 
	// Get z-buffer dimensions from the render target
	// Setup the surface desc for the z-buffer.
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | SurfaceMemFlag;
	ddsd.dwWidth = Width;
	ddsd.dwHeight = Height;
	memcpy(&ddsd.ddpfPixelFormat, &ddpfZBuffer, sizeof(DDPIXELFORMAT));

	// Create the depth-buffer.
	if (FAILED(DDraw->CreateSurface(&ddsd, &ZBuffer, NULL)))
        return false;

	// Attach the z-buffer to the back buffer.
	if (FAILED(RenderSurface->AddAttachedSurface(ZBuffer)))
		return false;

	//	Create rendering device
	if (FAILED(Direct3D->CreateDevice(DeviceGUID,
		RenderSurface, &RenderDevice)))
	{
		return false;
    }

	D3DDEVICEDESC7	DeviceDesc;
	RenderDevice->GetCaps(&DeviceDesc);
	Cvar_Set("r_sort_sprites", int((DeviceDesc.dwDevCaps & D3DDEVCAPS_SORTINCREASINGZ) != 0));
	square_textures = (DeviceDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY) != 0;
	// needed?
	//minTexSize = MAX(DeviceDesc.dwMinTextureWidth, DeviceDesc.dwMinTextureHeight);
	maxTexSize = MAX(DeviceDesc.dwMaxTextureWidth, DeviceDesc.dwMaxTextureHeight);
	if (square_textures)
	{
		//	Limit texture size when square textures are requred
		maxTexSize = 256;
	}
	maxMultiTex = DeviceDesc.wMaxSimultaneousTextures;
	if (device == 1)
	{
		//	In software actually can be only one texture
		maxMultiTex = 1;
	}

	memset(&PixelFormat, 0, sizeof(PixelFormat));
	RenderDevice->EnumTextureFormats(EnumPixelFormatsCallback, &PixelFormat);
	if (PixelFormat.dwSize != sizeof(DDPIXELFORMAT))
		return false;
	abits = GetBits(PixelFormat.dwRGBAlphaBitMask);
	ashift = GetShift(PixelFormat.dwRGBAlphaBitMask);
	rbits = GetBits(PixelFormat.dwRBitMask);
	rshift = GetShift(PixelFormat.dwRBitMask);
	gbits = GetBits(PixelFormat.dwGBitMask);
	gshift = GetShift(PixelFormat.dwGBitMask);
	bbits = GetBits(PixelFormat.dwBBitMask);
	bshift = GetShift(PixelFormat.dwBBitMask);

	memset(&PixelFormat32, 0, sizeof(PixelFormat32));
	RenderDevice->EnumTextureFormats(EnumPixelFormats32Callback, &PixelFormat32);
	ashift32 = GetShift(PixelFormat32.dwRGBAlphaBitMask);
	rshift32 = GetShift(PixelFormat32.dwRBitMask);
	gshift32 = GetShift(PixelFormat32.dwGBitMask);
	bshift32 = GetShift(PixelFormat32.dwBBitMask);

	//	Set screen params
	ScreenWidth = Width;
	ScreenHeight = Height;
	ScreenBPP = BPP;

//	Sys_HighFPPrecision();

	return true;
}

//==========================================================================
//
//	TDirect3DDrawer::InitResolution
//
//==========================================================================

void TDirect3DDrawer::InitResolution(void)
{
	RenderDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &IdentityMatrix);

	RenderDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);
 	
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATER);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, 170);

	RenderDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	RenderDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);

	RenderDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	RenderDevice->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_FLAT);
	RenderDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);

	RenderDevice->SetRenderState(D3DRENDERSTATE_FOGVERTEXMODE, D3DFOG_LINEAR);
	RenderDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR, 0x007f7f7f);
	RenderDevice->SetRenderState(D3DRENDERSTATE_FOGDENSITY, PassFloat(0.5));
	RenderDevice->SetRenderState(D3DRENDERSTATE_FOGSTART, PassFloat(1.0));
	RenderDevice->SetRenderState(D3DRENDERSTATE_FOGEND, PassFloat(2048.0));
}

//==========================================================================
//
//	TDirect3DDrawer::NewMap
//
//==========================================================================

void TDirect3DDrawer::NewMap(void)
{
	FlushCaches(false);
}

//==========================================================================
//
//	TDirect3DDrawer::StartUpdate
//
//==========================================================================

void TDirect3DDrawer::StartUpdate(void)
{
	// Check for lost surface
	if (RenderSurface->IsLost() != DD_OK)
	{
		RenderSurface->Restore();
	}

	//	Clear surface
	if (clear)
	{
		RenderDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.0, 0);
	}

	//	Setup texture filtering
	if (tex_linear == 4)
	{
		magfilter = D3DTFG_ANISOTROPIC;
		minfilter = D3DTFN_ANISOTROPIC;
		mipfilter = D3DTFP_LINEAR;
	}
	else if (tex_linear == 3)
	{
		magfilter = D3DTFG_LINEAR;
		minfilter = D3DTFN_LINEAR;
		mipfilter = D3DTFP_LINEAR;
	}
	else if (tex_linear == 2)
	{
		magfilter = D3DTFG_LINEAR;
		minfilter = D3DTFN_LINEAR;
		mipfilter = D3DTFP_POINT;
	}
	else if (tex_linear)
	{
		magfilter = D3DTFG_LINEAR;
		minfilter = D3DTFN_LINEAR;
		mipfilter = D3DTFP_NONE;
	}
	else
	{
		magfilter = D3DTFG_POINT;
		minfilter = D3DTFN_POINT;
		mipfilter = D3DTFP_NONE;
	}

	RenderDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, magfilter);
	RenderDevice->SetTextureStageState(0, D3DTSS_MINFILTER, minfilter);
	RenderDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, mipfilter);

	//	Dithering
	if (dither)
	{
		RenderDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE);
	}
	else
	{
		RenderDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE);
	}

	// Begin the scene.
	RenderDevice->BeginScene();

	Setup2D();
}

//==========================================================================
//
//	TDirect3DDrawer::Setup2D
//
//==========================================================================

void TDirect3DDrawer::Setup2D(void)
{
	//	Setup viewport
	D3DVIEWPORT7 view2D;
	memset(&view2D, 0, sizeof(D3DVIEWPORT7));
	view2D.dwX = 0;
	view2D.dwY = 0;
	view2D.dwWidth  = ScreenWidth;
	view2D.dwHeight = ScreenHeight;
	view2D.dvMinZ = 0.0f;
	view2D.dvMaxZ = 1.0f;
    RenderDevice->SetViewport(&view2D);

	//	Setup projection
	D3DMATRIX proj2D = IdentityMatrix;
	proj2D(0, 0) = 2.0 / (float)ScreenWidth;
	proj2D(1, 1) = -2.0 / (float)ScreenHeight;
	proj2D(3, 0) = -1.0;
	proj2D(3, 1) = 1.0;
	RenderDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &proj2D);

	RenderDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, &IdentityMatrix);

	RenderDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);
	RenderDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
	RenderDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
}

//==========================================================================
//
//	TDirect3DDrawer::SetupView
//
//==========================================================================

void TDirect3DDrawer::SetupView(const refdef_t *rd)
{
	if (rd->drawworld && rd->width != ScreenWidth)
	{
		R_DrawViewBorder();
	}

	//	Setup viewport
	memset(&viewData, 0, sizeof(D3DVIEWPORT7));
	viewData.dwX = rd->x;
	viewData.dwY = rd->y;
	viewData.dwWidth  = rd->width;
	viewData.dwHeight = rd->height;
	viewData.dvMinZ = 0;
	viewData.dvMaxZ = 1;
    RenderDevice->SetViewport(&viewData);

	//	Setup projection
	memset(&matProj, 0, sizeof(D3DMATRIX));
	matProj(0, 0) = 1 / rd->fovx;
	matProj(1, 1) = 1 / rd->fovy;
	float zFar = 8192.0;
	float zNear = 1.0;
	float Q = zFar / (zFar - zNear);
    matProj(2, 2) = Q;
    matProj(3, 2) = -Q * zNear;
	matProj(2, 3) = 1;
	RenderDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &matProj);

	// The view matrix defines the position and orientation of the camera.
	D3DMATRIX matView;
	matView(0, 0) = viewright.x;
	matView(1, 0) = viewright.y;
	matView(2, 0) = viewright.z;
	matView(3, 0) = -DotProduct(vieworg, viewright);
	matView(0, 1) = viewup.x;
	matView(1, 1) = viewup.y;
	matView(2, 1) = viewup.z;
	matView(3, 1) = -DotProduct(vieworg, viewup);
	matView(0, 2) = viewforward.x;
	matView(1, 2) = viewforward.y;
	matView(2, 2) = viewforward.z;
	matView(3, 2) = -DotProduct(vieworg, viewforward);
	matView(0, 3) = 0;
	matView(1, 3) = 0;
	matView(2, 3) = 0;
	matView(3, 3) = 1;
	RenderDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, &matView);

	RenderDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);

	RenderDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
	RenderDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, r_use_fog ? TRUE : FALSE);

	memset(light_chain, 0, sizeof(light_chain));

	RenderDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0, 1.0, 0);
}

//==========================================================================
//
//	TDirect3DDrawer::EndView
//
//==========================================================================

void TDirect3DDrawer::EndView(void)
{
	Setup2D();

	cl.cshifts[7] = cl.prev_cshifts[7];
	for (int i = 0; i < NUM_CSHIFTS; i++)
	{
		if (!cl.cshifts[i])
		{
			continue;
		}

		D3DLVERTEX	dv[4];

		RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);

		dv[0] = D3DLVERTEX(D3DVECTOR(0, 0, 0), cl.cshifts[i], 0, 0, 0);
		dv[1] = D3DLVERTEX(D3DVECTOR(ScreenWidth, 0, 0), cl.cshifts[i], 0, 0, 0);
		dv[2] = D3DLVERTEX(D3DVECTOR(ScreenWidth, ScreenHeight, 0), cl.cshifts[i], 0, 0, 0);
		dv[3] = D3DLVERTEX(D3DVECTOR(0, ScreenHeight, 0), cl.cshifts[i], 0, 0, 0);

		RenderDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, D3DFVF_LVERTEX, dv, 4, 0);

		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
		RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	}
}

//==========================================================================
//
//	TDirect3DDrawer::Update
//
// 	Blit to the screen / Flip surfaces
//
//==========================================================================

void TDirect3DDrawer::Update(void)
{
	// End the scene.
	RenderDevice->EndScene();

	// Check for lost surface
	if (PrimarySurface->IsLost() != DD_OK)
	{
		PrimarySurface->Restore();
	}

	PrimarySurface->Blt(NULL, RenderSurface, NULL, DDBLT_WAIT, NULL);
}

//==========================================================================
//
// 	TDirect3DDrawer::Shutdown
//
//	Close the graphics
//
//==========================================================================

void TDirect3DDrawer::Shutdown(void)
{
	ReleaseTextures();
	SAFE_RELEASE(RenderDevice)
	SAFE_RELEASE(Direct3D)

	SAFE_RELEASE(ZBuffer)
	SAFE_RELEASE(RenderSurface)
	SAFE_RELEASE(PrimarySurface)
	SAFE_RELEASE(DDraw)
}

//==========================================================================
//
//	TDirect3DDrawer::ReadScreen
//
//==========================================================================

void *TDirect3DDrawer::ReadScreen(int *bpp, bool *bot2top)
{
	DDSURFACEDESC2	ddsd;

	//	Allocate buffer
	void *dst = Z_Malloc(ScreenWidth * ScreenHeight * sizeof(rgb_t), PU_VIDEO, 0);
	if (!dst)
	{
		return NULL;
	}

	//	Lock surface
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_LPSURFACE | DDSD_PITCH | DDSD_PIXELFORMAT;
	if (RenderSurface->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL) != DD_OK)
		Sys_Error("V_Update: Failed to lock screen");

	//	Decode pixel format
	int scr_rbits = GetBits(ddsd.ddpfPixelFormat.dwRBitMask);
	int scr_rshift = GetShift(ddsd.ddpfPixelFormat.dwRBitMask);
	int scr_gbits = GetBits(ddsd.ddpfPixelFormat.dwGBitMask);
	int scr_gshift = GetShift(ddsd.ddpfPixelFormat.dwGBitMask);
	int scr_bbits = GetBits(ddsd.ddpfPixelFormat.dwBBitMask);
	int scr_bshift = GetShift(ddsd.ddpfPixelFormat.dwBBitMask);

	rgb_t *pdst = (rgb_t*)dst;
	for (int j = 0; j < ScreenHeight; j++)
	{
		byte *psrc = (byte*)ddsd.lpSurface + j * ddsd.lPitch;
		for (int i = 0; i < ScreenWidth; i++)
		{
			pdst->r = ((*(dword*)psrc >> scr_rshift) << (8 - scr_rbits)) & 0xff;
			pdst->g = ((*(dword*)psrc >> scr_gshift) << (8 - scr_gbits)) & 0xff;
			pdst->b = ((*(dword*)psrc >> scr_bshift) << (8 - scr_bbits)) & 0xff;
			psrc += PixelBytes;
			pdst++;
		}
	}

	RenderSurface->Unlock(NULL);
	*bpp = 24;
	*bot2top = false;
	return dst;
}

//==========================================================================
//
//	TDirect3DDrawer::SetPalette
//
//==========================================================================

void TDirect3DDrawer::SetPalette(int pnum)
{
	rgb_t *pal = (rgb_t*)W_CacheLumpName("playpal", PU_CACHE);

	pal += 256 * pnum;
	int cmax = MAX(MAX(pal[0].r, pal[0].g), pal[0].b);
	if (!cmax)
	{
		cl.cshifts[7] = 0;
	}
	else
	{
		cl.cshifts[7] = (cmax << 24) | ((255 * pal[0].r / cmax) << 16) |
			((255 * pal[0].g / cmax) << 8) | (255 * pal[0].b / cmax);
	}
	cl.prev_cshifts[7] = cl.cshifts[7];
}

//**************************************************************************
//
//	$Log$
//	Revision 1.9  2001/08/29 17:47:55  dj_jl
//	Added texture filtering variables
//
//	Revision 1.8  2001/08/24 17:03:57  dj_jl
//	Added mipmapping, removed bumpmap test code
//	
//	Revision 1.7  2001/08/15 17:15:55  dj_jl
//	Drawer API changes, removed wipes
//	
//	Revision 1.6  2001/08/07 16:46:23  dj_jl
//	Added player models, skins and weapon
//	
//	Revision 1.5  2001/08/04 17:29:54  dj_jl
//	Fixed fog, beautification
//	
//	Revision 1.4  2001/08/01 17:40:09  dj_jl
//	Fixed check for sprite sorting, beautification
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************

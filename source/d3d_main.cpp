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

extern byte					gammatable[5][256];

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

ostream &operator << (ostream &str, const LPD3DDEVICEDESC7 dd);
ostream &operator << (ostream &str, const LPDDPIXELFORMAT pf);

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
	amask = PixelFormat.dwRGBAlphaBitMask;
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
		RenderDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0, 0);
	else
		RenderDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0, 1.0, 0);

	//	Setup texture filtering
	if (tex_linear)
	{
		RenderDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
		RenderDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_LINEAR);
	}
	else
	{
		RenderDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
		RenderDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_POINT);
	}

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

void TDirect3DDrawer::SetupView(int x, int y, int width, int height, float fovx, float fovy)
{
	//	Setup viewport
	memset(&viewData, 0, sizeof(D3DVIEWPORT7));
	viewData.dwX = x;
	viewData.dwY = y;
	viewData.dwWidth  = width;
	viewData.dwHeight = height;
	viewData.dvMinZ = 0;
	viewData.dvMaxZ = 1;

	//	Setup projection
	memset(&matProj, 0, sizeof(D3DMATRIX));
	matProj(0, 0) = 1 / fovx;
	matProj(1, 1) = 1 / fovy;
#if 0
	matProj(2, 2) = 1;
	matProj(3, 2) = -0.1;
#else
	float zFar = 8192.0;
	float zNear = 1.0;
	float Q;
    Q = zFar / (zFar - zNear);
    matProj(2, 2) = Q;
    matProj(3, 2) = -Q * zNear;
#endif
	matProj(2, 3) = 1;
}

//==========================================================================
//
//	TDirect3DDrawer::SetupFrame
//
//==========================================================================

void TDirect3DDrawer::SetupFrame(void)
{
	if (viewwidth != ScreenWidth)
	{
		R_DrawViewBorder();
	}

    RenderDevice->SetViewport(&viewData);
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
//	TDirect3DDrawer::InitWipe
//
//==========================================================================

bool TDirect3DDrawer::InitWipe(void)
{
	return false;
}

//==========================================================================
//
//	TDirect3DDrawer::DoWipe
//
//==========================================================================

void TDirect3DDrawer::DoWipe(int)
{
}

//==========================================================================
//
//	TDirect3DDrawer::SetPalette
//
//==========================================================================

void TDirect3DDrawer::SetPalette(int pnum)
{
	rgb_t *pal = (rgb_t*)W_CacheLumpName("playpal", PU_CACHE);
	for (int i = 0; i < 256; i++)
	{
		pal8_to16[i] = MakeCol16(pal[i].r, pal[i].g, pal[i].b);
	}

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

//==========================================================================
//
//
//
//==========================================================================

ostream &operator << (ostream &str, const D3DPRIMCAPS pc)
{
	str << "Misc caps:";
	if (pc.dwMiscCaps & D3DPMISCCAPS_MASKPLANES)
	{
		str << " MaskPlanes";
	}
	if (pc.dwMiscCaps & D3DPMISCCAPS_MASKZ)
	{
		str << " MaskZ";
	}
	if (pc.dwMiscCaps & D3DPMISCCAPS_LINEPATTERNREP)
	{
		str << " LinePatternRep";
	}
	if (pc.dwMiscCaps & D3DPMISCCAPS_CONFORMANT)
	{
		str << " Conformant";
	}
	if (pc.dwMiscCaps & D3DPMISCCAPS_CULLNONE)
	{
		str << " CullNone";
	}
	if (pc.dwMiscCaps & D3DPMISCCAPS_CULLCW)
	{
		str << " CullClockwise";
	}
	if (pc.dwMiscCaps & D3DPMISCCAPS_CULLCCW)
	{
		str << " CullCounterClockwise";
	}
	str << endl;

	str << "Rasterization caps:";
	if (pc.dwRasterCaps & D3DPRASTERCAPS_DITHER)
	{
		str << " Dither";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_ROP2)
	{
		str << " ROp2";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_XOR)
	{
		str << " XOR";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_PAT)
	{
		str << " Pat";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_ZTEST)
	{
		str << " ZTest";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_SUBPIXEL)
	{
		str << " SubPixel";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_SUBPIXELX)
	{
		str << " SubPixelX";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_FOGVERTEX)
	{
		str << " FogVertex";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_FOGTABLE)
	{
		str << " FogTable";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_STIPPLE)
	{
		str << " Stipple";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTDEPENDENT)
	{
		str << " AntialiasSortDependent";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT)
	{
		str << " AntialiasSortIndependent";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASEDGES)
	{
		str << " AntialiasEdges";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_MIPMAPLODBIAS)
	{
		str << " MipMapLodBias";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_ZBIAS)
	{
		str << " ZBias";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_ZBUFFERLESSHSR)
	{
		str << " ZBufferLessHSR";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_FOGRANGE)
	{
		str << " FogRange";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_ANISOTROPY)
	{
		str << " Anisotropy";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_WBUFFER)
	{
		str << " WBuffer";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_TRANSLUCENTSORTINDEPENDENT)
	{
		str << " TranslucentSortIndependent";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_WFOG)
	{
		str << " WFog";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_ZFOG)
	{
		str << " ZFog";
	}
	str << endl;

	str << "Supported depth tests:";
	if (pc.dwZCmpCaps & D3DPCMPCAPS_ALWAYS)
	{
		str << " always";
	}
	if (pc.dwZCmpCaps & D3DPCMPCAPS_EQUAL)
	{
		str << " equal";
	}
	if (pc.dwZCmpCaps & D3DPCMPCAPS_GREATER)
	{
		str << " greater";
	}
	if (pc.dwZCmpCaps & D3DPCMPCAPS_GREATEREQUAL)
	{
		str << " greater_equal";
	}
	if (pc.dwZCmpCaps & D3DPCMPCAPS_LESS)
	{
		str << " less";
	}
	if (pc.dwZCmpCaps & D3DPCMPCAPS_LESSEQUAL)
	{
		str << " less_equal";
	}
	if (pc.dwZCmpCaps & D3DPCMPCAPS_NEVER)
	{
		str << " never";
	}
	if (pc.dwZCmpCaps & D3DPCMPCAPS_NOTEQUAL)
	{
		str << " not_equal";
	}
	str << endl;

	str << "Src blend caps:";
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_ZERO)
	{
		str << " Zero";
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_ONE)
	{
		str << " One";
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_SRCCOLOR)
	{
		str << " SrcColor";
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_INVSRCCOLOR)
	{
		str << " InvSrcColor";
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_SRCALPHA)
	{
		str << " SrcAlpha";
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_INVSRCALPHA)
	{
		str << " InvSrcAlpha";
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_DESTALPHA)
	{
		str << " DestAlpha";
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_INVDESTALPHA)
	{
		str << " InvDestAlpha";
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_DESTCOLOR)
	{
		str << " DestColor";
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_INVDESTCOLOR)
	{
		str << " InvDestColor";
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_SRCALPHASAT)
	{
		str << " SrcAlphaSat";
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_BOTHSRCALPHA)
	{
		str << " BothSrcAlpha";
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_BOTHINVSRCALPHA)
	{
		str << " BothInvSrcAlpha";
	}
	str << endl;

	str << "Dest blend caps:";
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_ZERO)
	{
		str << " Zero";
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_ONE)
	{
		str << " One";
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_SRCCOLOR)
	{
		str << " SrcColor";
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_INVSRCCOLOR)
	{
		str << " InvSrcColor";
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_SRCALPHA)
	{
		str << " SrcAlpha";
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_INVSRCALPHA)
	{
		str << " InvSrcAlpha";
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_DESTALPHA)
	{
		str << " DestAlpha";
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_INVDESTALPHA)
	{
		str << " InvDestAlpha";
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_DESTCOLOR)
	{
		str << " DestColor";
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_INVDESTCOLOR)
	{
		str << " InvDestColor";
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_SRCALPHASAT)
	{
		str << " SrcAlphaSat";
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_BOTHSRCALPHA)
	{
		str << " BothSrcAlpha";
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_BOTHINVSRCALPHA)
	{
		str << " BothInvSrcAlpha";
	}
	str << endl;

	str << "Supported alpaha tests:";
	if (pc.dwAlphaCmpCaps & D3DPCMPCAPS_ALWAYS)
	{
		str << " always";
	}
	if (pc.dwAlphaCmpCaps & D3DPCMPCAPS_EQUAL)
	{
		str << " equal";
	}
	if (pc.dwAlphaCmpCaps & D3DPCMPCAPS_GREATER)
	{
		str << " greater";
	}
	if (pc.dwAlphaCmpCaps & D3DPCMPCAPS_GREATEREQUAL)
	{
		str << " greater_equal";
	}
	if (pc.dwAlphaCmpCaps & D3DPCMPCAPS_LESS)
	{
		str << " less";
	}
	if (pc.dwAlphaCmpCaps & D3DPCMPCAPS_LESSEQUAL)
	{
		str << " less_equal";
	}
	if (pc.dwAlphaCmpCaps & D3DPCMPCAPS_NEVER)
	{
		str << " never";
	}
	if (pc.dwAlphaCmpCaps & D3DPCMPCAPS_NOTEQUAL)
	{
		str << " not_equal";
	}
	str << endl;

	str << "Shade caps:";
	if (pc.dwShadeCaps & D3DPSHADECAPS_COLORFLATMONO)
	{
		str << " ColorFlatMono";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_COLORFLATRGB)
	{
		str << " ColorFlatRGB";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_COLORGOURAUDMONO)
	{
		str << " ColorGouraudMono";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_COLORGOURAUDRGB)
	{
		str << " ColorGouraudRGB";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_COLORPHONGMONO)
	{
		str << " ColorPhongMono";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_COLORPHONGRGB)
	{
		str << " ColorPhongRGB";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_SPECULARFLATMONO)
	{
		str << " SpecularFlatMono";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_SPECULARFLATRGB)
	{
		str << " SpecularFlatRGB";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_SPECULARGOURAUDMONO)
	{
		str << " SpecularGouraudMono";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_SPECULARGOURAUDRGB)
	{
		str << " SpecularGouraudRGB";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_SPECULARPHONGMONO)
	{
		str << " SpecularPhongMono";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_SPECULARPHONGRGB)
	{
		str << " SpecularPhongRGB";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_ALPHAFLATBLEND)
	{
		str << " AlphaFlatBlend";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_ALPHAFLATSTIPPLED)
	{
		str << " AlphaFlatStippled";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_ALPHAGOURAUDBLEND)
	{
		str << " AlphaGouraudBlend";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_ALPHAGOURAUDSTIPPLED)
	{
		str << " AlphaGouraudStippled";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_ALPHAPHONGBLEND)
	{
		str << " AlphaPhongBlend";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_ALPHAPHONGSTIPPLED)
	{
		str << " AlphaPhongStippled";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_FOGFLAT)
	{
		str << " FogFlat";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_FOGGOURAUD)
	{
		str << " FogGouraud";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_FOGPHONG)
	{
		str << " FogPhong";
	}
	str << endl;

	str << "Texture caps:\n";
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_PERSPECTIVE)
	{
		str << "- Perspective-correct texturing is supported\n";
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_POW2)
	{
		str << "- Power-of-2 texture dimensions are required\n";
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_ALPHA)
	{
		str << "- Alpha in texture pixels is supported\n";
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_TRANSPARENCY)
	{
		str << "- Color-keyed textures are supported\n";
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_BORDER)
	{
		str << "- obsolete, see D3DPTADDRESSCAPS_BORDER\n";
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY)
	{
		str << "- Only square textures are supported\n";
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE)
	{
		str << "- Texture indices are not scaled by the texture size prior to interpolation.\n";
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_ALPHAPALETTE)
	{
		str << "- Device can draw alpha from texture palettes\n";
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL)
	{
		str << "- Device can use non-POW2 textures\n";
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_PROJECTED)
	{
		str << "- Device can divide transformed texture coordinates by the COUNTth texture coordinate (can do D3DTTFF_PROJECTED)\n";
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_CUBEMAP)
	{
		str << "- Device can do cubemap textures\n";
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_COLORKEYBLEND)
	{
		str << "- colorkeyblend\n";
	}

	str << "Texture filter caps:";
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_NEAREST)
	{
		str << " Nearest";
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEAR)
	{
		str << " Linear";
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MIPNEAREST)
	{
		str << " MipNearest";
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MIPLINEAR)
	{
		str << " MipLinear";
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEARMIPNEAREST)
	{
		str << " LinearMipNearest";
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEARMIPLINEAR)
	{
		str << " LinearMipLinear";
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MINFPOINT)
	{
		str << " MinFPoint";
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR)
	{
		str << " MinFLinear";
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC)
	{
		str << " MinFAnisotropic";
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MIPFPOINT)
	{
		str << " MipFPoint";
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR)
	{
		str << " MipFLinear";
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MAGFPOINT)
	{
		str << " MagFPoint";
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR)
	{
		str << " MagFLinear";
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC)
	{
		str << " MagFAnisotropic";
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MAGFAFLATCUBIC)
	{
		str << " MagFAFlatCubic";
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MAGFGAUSSIANCUBIC)
	{
		str << " MagFGaussianCubic";
	}
	str << endl;

	str << "Texture blend caps:";
	if (pc.dwTextureBlendCaps & D3DPTBLENDCAPS_DECAL)
	{
		str << " Decal";
	}
	if (pc.dwTextureBlendCaps & D3DPTBLENDCAPS_MODULATE)
	{
		str << " Modulate";
	}
	if (pc.dwTextureBlendCaps & D3DPTBLENDCAPS_DECALALPHA)
	{
		str << " DecalAlpha";
	}
	if (pc.dwTextureBlendCaps & D3DPTBLENDCAPS_MODULATEALPHA)
	{
		str << " ModulateAlpha";
	}
	if (pc.dwTextureBlendCaps & D3DPTBLENDCAPS_DECALMASK)
	{
		str << " DecalMask";
	}
	if (pc.dwTextureBlendCaps & D3DPTBLENDCAPS_MODULATEMASK)
	{
		str << " ModulateMask";
	}
	if (pc.dwTextureBlendCaps & D3DPTBLENDCAPS_COPY)
	{
		str << " Copy";
	}
	if (pc.dwTextureBlendCaps & D3DPTBLENDCAPS_ADD)
	{
		str << " Add";
	}
	str << endl;

	str << "Texture address caps:";
	if (pc.dwTextureAddressCaps & D3DPTADDRESSCAPS_WRAP)
	{
		str << " Wrap";
	}
	if (pc.dwTextureAddressCaps & D3DPTADDRESSCAPS_MIRROR)
	{
		str << " Mirror";
	}
	if (pc.dwTextureAddressCaps & D3DPTADDRESSCAPS_CLAMP)
	{
		str << " Clamp";
	}
	if (pc.dwTextureAddressCaps & D3DPTADDRESSCAPS_BORDER)
	{
		str << " Border";
	}
	if (pc.dwTextureAddressCaps & D3DPTADDRESSCAPS_INDEPENDENTUV)
	{
		str << " IndependentUV";
	}
	str << endl;

    str << "Stipple width: " << pc.dwStippleWidth << endl;
    str << "Stipple height: " << pc.dwStippleHeight << endl;
	return str;
}

//==========================================================================
//
//
//
//==========================================================================

ostream &operator << (ostream &str, const LPD3DDEVICEDESC7 dd)
{
	str << "Device caps:\n";
	if (dd->dwDevCaps & D3DDEVCAPS_FLOATTLVERTEX)
	{
		str << "- accepts floating point for post-transform vertex data.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_SORTINCREASINGZ)
	{
		str << "- needs data sorted for increasing depth.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_SORTDECREASINGZ)
	{
		str << "- needs data sorted for decreasing depth.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_SORTEXACT)
	{
		str << "- needs data sorted exactly.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_EXECUTESYSTEMMEMORY)
	{
		str << "- can use execute buffers from system memory.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_EXECUTEVIDEOMEMORY)
	{
		str << "- can use execute buffer from video memory.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_TLVERTEXSYSTEMMEMORY)
	{
		str << "- can use buffers from system memory for transformed and lit vertices.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_TLVERTEXVIDEOMEMORY)
	{
		str << "- can use buffers from video memory for transformed and lit vertices.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_TEXTURESYSTEMMEMORY)
	{
		str << "- can retrieve textures from system memory.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_TEXTUREVIDEOMEMORY)
	{
		str << "- can retrieve textures from device memory.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_DRAWPRIMTLVERTEX)
	{
		str << "- exports a DrawPrimitive-aware HAL.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_CANRENDERAFTERFLIP)
	{
		str << "- can queue rendering commands after a page flip.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_TEXTURENONLOCALVIDMEM)
	{
		str << "- can retrieve textures from non-local video (AGP) memory.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_DRAWPRIMITIVES2)
	{
		str << "- can support DrawPrimitives2.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_SEPARATETEXTUREMEMORIES)
	{
		str << "- uses discrete texture memory pools for each stage.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_DRAWPRIMITIVES2EX)
	{
		str << "- can support Extended DrawPrimitives2 i.e. DX7 compliant driver.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
	{
		str << "- supports transformation and lighting in hardware.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_CANBLTSYSTONONLOCAL)
	{
		str << "- supports blits from system-memory textures to non-local video-memory textures.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_HWRASTERIZATION)
	{
		str << "- has HW acceleration for rasterization.\n";
	}
	str << endl;

	str << "-- Line caps --\n" << dd->dpcLineCaps;
	str << endl;

	str << "-- Triangle caps --\n" << dd->dpcTriCaps;
 	str << endl;

	str << "Rendering bit depth: ";
 	if (dd->dwDeviceRenderBitDepth & DDBD_8) str << "8 ";
 	if (dd->dwDeviceRenderBitDepth & DDBD_16) str << "16 ";
 	if (dd->dwDeviceRenderBitDepth & DDBD_24) str << "24 ";
 	if (dd->dwDeviceRenderBitDepth & DDBD_32) str << "32 ";
 	str << endl;
	str << "Z-Buffer bit depth: ";
 	if (dd->dwDeviceZBufferBitDepth & DDBD_8) str << "8 ";
 	if (dd->dwDeviceZBufferBitDepth & DDBD_16) str << "16 ";
 	if (dd->dwDeviceZBufferBitDepth & DDBD_24) str << "24 ";
 	if (dd->dwDeviceZBufferBitDepth & DDBD_32) str << "32 ";
 	str << endl;
	str << endl;

	str << "Min texture width: " << dd->dwMinTextureWidth << endl;
	str << "Min texture height: " << dd->dwMinTextureHeight << endl;
	str << "Max texture width: " << dd->dwMaxTextureWidth << endl;
	str << "Max texture height: " << dd->dwMaxTextureHeight << endl;
	str << endl;

	str << "Max texture repeat: " << dd->dwMaxTextureRepeat << endl;
	str << "Max texture aspect ratio: " << dd->dwMaxTextureAspectRatio << endl;
	str << "Max anisotropy: " << dd->dwMaxAnisotropy << endl;
	str << endl;

	str << "Guard band left: " << dd->dvGuardBandLeft << endl;
	str << "Guard band top: " << dd->dvGuardBandTop << endl;
	str << "Guard band right: " << dd->dvGuardBandRight << endl;
	str << "Guard band bottom: " << dd->dvGuardBandBottom << endl;
	str << endl;

	str << "Extents adjust: " << dd->dvExtentsAdjust << endl;
	str << endl;

	str << "Stencil caps: ";
	if (dd->dwStencilCaps & D3DSTENCILCAPS_DECR)
	{
		str << "DECR ";
	}
	if (dd->dwStencilCaps & D3DSTENCILCAPS_DECRSAT)
	{
		str << "DECRSAT ";
	}
	if (dd->dwStencilCaps & D3DSTENCILCAPS_INCR)
	{
		str << "INCR ";
	}
	if (dd->dwStencilCaps & D3DSTENCILCAPS_INCRSAT)
	{
		str << "INCRSAT ";
	}
	if (dd->dwStencilCaps & D3DSTENCILCAPS_INVERT)
	{
		str << "INVERT ";
	}
	if (dd->dwStencilCaps & D3DSTENCILCAPS_KEEP)
	{
		str << "KEEP ";
	}
	if (dd->dwStencilCaps & D3DSTENCILCAPS_REPLACE)
	{
		str << "REPLACE ";
	}
	if (dd->dwStencilCaps & D3DSTENCILCAPS_ZERO)
	{
		str << "ZERO ";
	}
	str << endl;

	str << "Flexible vertex format capabilities:\n";
	if (dd->dwFVFCaps & D3DFVFCAPS_DONOTSTRIPELEMENTS)
	{
		str << "- prefers that vertex elements not be stripped.\n";
	}
	str << "- total " << (dd->dwFVFCaps & D3DFVFCAPS_TEXCOORDCOUNTMASK) << " texture coordinate sets\n";

	str << "Supported texture operations: ";
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_ADD)
	{
		str << "ADD ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_ADDSIGNED)
	{
		str << "ADDSIGNED ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_ADDSIGNED2X)
	{
		str << "ADDSIGNED2X ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_ADDSMOOTH)
	{
		str << "ADDSMOOTH ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_BLENDCURRENTALPHA)
	{
		str << "BLENDCURRENTALPHA ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_BLENDDIFFUSEALPHA)
	{
		str << "BLENDDIFFUSEALPHA ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_BLENDFACTORALPHA)
	{
		str << "BLENDFACTORALPHA ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_BLENDTEXTUREALPHA)
	{
		str << "BLENDTEXTUREALPHA ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_BLENDTEXTUREALPHAPM)
	{
		str << "BLENDTEXTUREALPHAPM ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_BUMPENVMAP)
	{
		str << "BUMPENVMAP ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_BUMPENVMAPLUMINANCE)
	{
		str << "BUMPENVMAPLUMINANCE ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_DISABLE)
	{
		str << "DISABLE ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_DOTPRODUCT3)
	{
		str << "DOTPRODUCT3 ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_MODULATE)
	{
		str << "MODULATE ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_MODULATE2X)
	{
		str << "MODULATE2X ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_MODULATE4X)
	{
		str << "MODULATE4X ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR)
	{
		str << "MODULATEALPHA_ADDCOLOR ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA)
	{
		str << "MODULATEALPHA_ADDCOLOR ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_MODULATEINVALPHA_ADDCOLOR)
	{
		str << "MODULATEINVALPHA_ADDCOLOR ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_MODULATEINVCOLOR_ADDALPHA)
	{
		str << "MODULATEINVCOLOR_ADDALPHA ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_PREMODULATE)
	{
		str << "PREMODULATE ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_SELECTARG1)
	{
		str << "SELECTARG1 ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_SELECTARG2)
	{
		str << "SELECTARG2 ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_SUBTRACT)
	{
		str << "SUBTRACT ";
	}
	str << endl;
	str << "Max texture blend stages: " << dd->wMaxTextureBlendStages << endl;
	str << "Max simultaneous textures: " << dd->wMaxSimultaneousTextures << endl;
	str << "Max active lights: " << dd->dwMaxActiveLights << endl;
	str << "Max vertex W: " << dd->dvMaxVertexW << endl;
	str << "Max user clip planes: " << dd->wMaxUserClipPlanes << endl;
	str << "Max vertex blend matrices: " << dd->wMaxVertexBlendMatrices << endl;

/*
    GUID        deviceGUID;

    DWORD       dwVertexProcessingCaps;
*/
	return str;
}

//==========================================================================
//
//
//
//==========================================================================

ostream &operator << (ostream &str, const LPDDPIXELFORMAT pf)
{
	if (pf->dwFlags & DDPF_ALPHAPIXELS)
		str << "- has alpha channel." << endl;
	if (pf->dwFlags & DDPF_ALPHA)
		str << "- Alpha-only surface." << endl;
	if (pf->dwFlags & DDPF_FOURCC)
		str << "- contains a FOURCC code describing a non-RGB pixel format." << endl;
	if (pf->dwFlags & DDPF_PALETTEINDEXED1)
		str << "- 1-bit color indexed." << endl;
	if (pf->dwFlags & DDPF_PALETTEINDEXED2)
		str << "- 2-bit color indexed." << endl;
	if (pf->dwFlags & DDPF_PALETTEINDEXED4)
		str << "- 4-bit color indexed." << endl;
	if (pf->dwFlags & DDPF_PALETTEINDEXED8)
		str << "- 8-bit color indexed." << endl;
	if (pf->dwFlags & DDPF_PALETTEINDEXEDTO8)
		str << "- indexed to an 8-bit palette." << endl;
	if (pf->dwFlags & DDPF_RGB)
		str << "- RGB surface." << endl;
	if (pf->dwFlags & DDPF_COMPRESSED)
		str << "- will accept pixel data in the specified format and compress it during the write operation." << endl;
	if (pf->dwFlags & DDPF_RGBTOYUV)
		str << "- translates RGB data YUV data." << endl;
	if (pf->dwFlags & DDPF_YUV)
		str << "- YUV surface." << endl;
	if (pf->dwFlags & DDPF_ZBUFFER)
		str << "- z-buffer surface." << endl;
	if (pf->dwFlags & DDPF_ZPIXELS)
		str << "- contains z information in the pixels." << endl;
	if (pf->dwFlags & DDPF_STENCILBUFFER)
		str << "- contains stencil information along with Z." << endl;
	if (pf->dwFlags & DDPF_ALPHAPREMULT)
		str << "- uses the premultiplied alpha format." << endl;
	if (pf->dwFlags & DDPF_LUMINANCE)
		str << "- luminance-only or luminance-alpha surface." << endl;
	if (pf->dwFlags & DDPF_BUMPLUMINANCE)
		str << "- luminance off bumpmap surface." << endl;
	if (pf->dwFlags & DDPF_BUMPDUDV)
		str << "- has Bump-map." << endl;
	str << "- Bit depth: " << pf->dwRGBBitCount << endl;
	str << "- Mask for red bits: " << hex << pf->dwRBitMask << dec << endl;
	str << "- Mask for green bits: " << hex << pf->dwGBitMask << dec << endl;
	str << "- Mask for blue bits: " << hex << pf->dwBBitMask << dec << endl;
	str << "- Mask for alpha/Z channel: " << hex << pf->dwRGBAlphaBitMask << dec << endl;

	return str;
}

//**************************************************************************
//
//	$Log$
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

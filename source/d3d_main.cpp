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
//**	Copyright (C) 1999-2006 Jānis Legzdiņš
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

IMPLEMENT_DRAWER(VDirect3DDrawer, DRAWER_Direct3D, "Direct3D",
	"Direct3D rasteriser device", "-d3d");

VCvarI VDirect3DDrawer::device("d3d_device", "0", CVAR_Archive);
VCvarI VDirect3DDrawer::clear("d3d_clear", "0", CVAR_Archive);
VCvarI VDirect3DDrawer::tex_linear("d3d_tex_linear", "2", CVAR_Archive);
VCvarI VDirect3DDrawer::dither("d3d_dither", "0", CVAR_Archive);
VCvarI VDirect3DDrawer::blend_sprites("d3d_blend_sprites", "0", CVAR_Archive);
VCvarF VDirect3DDrawer::maxdist("d3d_maxdist", "8192.0", CVAR_Archive);
VCvarI VDirect3DDrawer::model_lighting("d3d_model_lighting", "0", CVAR_Archive);
VCvarI VDirect3DDrawer::specular_highlights("d3d_specular_highlights", "1", CVAR_Archive);

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VDirect3DDrawer::VDirect3DDrawer
//
//==========================================================================

VDirect3DDrawer::VDirect3DDrawer()
: Windowed(false)
, DLLHandle(0)
, Direct3D(0)
, RenderDevice(0)
, IdentityMatrix(1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 1, 0,
				0, 0, 0, 1)
, TexStage(0)
, lastgamma(0)
, CurrentFade(0)
, particle_texture(0)
, tscount(0)
{
	memset(light_surf, 0, sizeof(light_surf));
	memset(add_surf, 0, sizeof(add_surf));
}

//==========================================================================
//
//	VDirect3DDrawer::Init
//
// 	Determine the hardware configuration
//
//==========================================================================

void VDirect3DDrawer::Init()
{
	guard(VDirect3DDrawer::Init);
	typedef IDirect3D9* (WINAPI*fp_Direct3DCreate9)(UINT SDKVersion);

	fp_Direct3DCreate9 p_Direct3DCreate9;

	DLLHandle = LoadLibrary("d3d9.dll");
	if (!DLLHandle)
	{
		Sys_Error("Couldn't load d3d9.dll");
	}

	p_Direct3DCreate9 = (fp_Direct3DCreate9)GetProcAddress(DLLHandle,
		"Direct3DCreate9");
	if (!p_Direct3DCreate9)
	{
		Sys_Error("Symbol Direct3DCreate9 not found");
	}

	// Create Direct3D object
	Direct3D = p_Direct3DCreate9(D3D_SDK_VERSION);
	if (!Direct3D)
	{
		Sys_Error("Failed to create Direct3D object");
	}
	unguard;
}

//==========================================================================
//
// 	VDirect3DDrawer::SetResolution
//
// 	Set up the video mode
//
//==========================================================================

bool VDirect3DDrawer::SetResolution(int Width, int Height, int BPP,
	bool AWindowed)
{
	guard(VDirect3DDrawer::SetResolution);
	Windowed = AWindowed;
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
		//	True-colour only
		return false;
	}

	//	Shut down current mode
	ReleaseTextures();
	SAFE_RELEASE(RenderDevice)

	D3DPRESENT_PARAMETERS d3dpp;

	if (Windowed)
	{
		RECT WindowRect;
		WindowRect.left = 0;
		WindowRect.right = Width;
		WindowRect.top = 0;
		WindowRect.bottom = Height;
		AdjustWindowRectEx(&WindowRect, WS_OVERLAPPEDWINDOW, FALSE,
			WS_EX_APPWINDOW);
		SetWindowPos(hwnd, HWND_TOP, 0, 0, WindowRect.right - WindowRect.left,
			WindowRect.bottom - WindowRect.top, SWP_NOMOVE);
	}

	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.BackBufferWidth = Width;
	d3dpp.BackBufferHeight = Height;
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.BackBufferCount = 1;
	d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
	d3dpp.hDeviceWindow = hwnd;
	d3dpp.Windowed = Windowed;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

	if (Direct3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
//		D3DCREATE_FPU_PRESERVE | D3DCREATE_HARDWARE_VERTEXPROCESSING,
		D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE,
		&d3dpp, &RenderDevice) != D3D_OK)
	{
		return false;
	}
	HasStencil = true;

	D3DCAPS9 DeviceCaps;
	RenderDevice->GetDeviceCaps(&DeviceCaps);
//	VCvar::Set("r_sort_sprites", int((DeviceCaps.DevCaps & D3DDEVCAPS_SORTINCREASINGZ) != 0));
	square_textures = (DeviceCaps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY) != 0;
	maxTexSize = MAX(DeviceCaps.MaxTextureWidth, DeviceCaps.MaxTextureHeight);
	if (square_textures)
	{
		//	Limit texture size when square textures are requred
		maxTexSize = 256;
	}
	maxMultiTex = DeviceCaps.MaxSimultaneousTextures;
	if (device == 1)
	{
		//	In software actually can be only one texture
		maxMultiTex = 1;
	}
	RenderDevice->SetSamplerState(0, D3DSAMP_MAXANISOTROPY, DeviceCaps.MaxAnisotropy);

	//	Set screen params
	ScreenWidth = Width;
	ScreenHeight = Height;
	ScreenBPP = BPP;

	return true;
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::InitResolution
//
//==========================================================================

void VDirect3DDrawer::InitResolution()
{
	guard(VDirect3DDrawer::InitResolution);
	RenderDevice->SetStreamSource(0, NULL, 0, sizeof(MyD3DVertex));
	RenderDevice->SetFVF(MYD3D_VERTEX_FORMAT);
	RenderDevice->SetTransform(D3DTS_WORLD, &IdentityMatrix);

	RenderDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	
	RenderDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	RenderDevice->SetRenderState(D3DRS_ALPHAREF, 170);

	RenderDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	RenderDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	RenderDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	RenderDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
	RenderDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::StartUpdate
//
//==========================================================================

void VDirect3DDrawer::StartUpdate()
{
	guard(VDirect3DDrawer::StartUpdate);
	//	Clear surface
	if (clear)
	{
		RenderDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.0, 0);
	}

	//	Setup texture filtering
	if (tex_linear == 4)
	{
		magfilter = D3DTEXF_ANISOTROPIC;
		minfilter = D3DTEXF_ANISOTROPIC;
		mipfilter = D3DTEXF_LINEAR;
	}
	else if (tex_linear == 3)
	{
		magfilter = D3DTEXF_LINEAR;
		minfilter = D3DTEXF_LINEAR;
		mipfilter = D3DTEXF_LINEAR;
	}
	else if (tex_linear == 2)
	{
		magfilter = D3DTEXF_LINEAR;
		minfilter = D3DTEXF_LINEAR;
		mipfilter = D3DTEXF_POINT;
	}
	else if (tex_linear)
	{
		magfilter = D3DTEXF_LINEAR;
		minfilter = D3DTEXF_LINEAR;
		mipfilter = D3DTEXF_NONE;
	}
	else
	{
		magfilter = D3DTEXF_POINT;
		minfilter = D3DTEXF_POINT;
		mipfilter = D3DTEXF_NONE;
	}

	if (lastgamma != usegamma)
	{
		FlushTextures();
		lastgamma = usegamma;
	}

	RenderDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, magfilter);
	RenderDevice->SetSamplerState(0, D3DSAMP_MINFILTER, minfilter);
	RenderDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, mipfilter);

	//	Dithering
	if (dither)
	{
		RenderDevice->SetRenderState(D3DRS_DITHERENABLE, TRUE);
	}
	else
	{
		RenderDevice->SetRenderState(D3DRS_DITHERENABLE, FALSE);
	}

	// Begin the scene.
	RenderDevice->BeginScene();

	Setup2D();
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::Setup2D
//
//==========================================================================

void VDirect3DDrawer::Setup2D()
{
	guard(VDirect3DDrawer::Setup2D);
	//	Setup viewport
	D3DVIEWPORT9 view2D;
	memset(&view2D, 0, sizeof(D3DVIEWPORT9));
	view2D.X = 0;
	view2D.Y = 0;
	view2D.Width  = ScreenWidth;
	view2D.Height = ScreenHeight;
	view2D.MinZ = 0.0f;
	view2D.MaxZ = 1.0f;
	RenderDevice->SetViewport(&view2D);

	//	Setup projection
	MyD3DMatrix proj2D = IdentityMatrix;
	proj2D(0, 0) = 2.0 / (float)ScreenWidth;
	proj2D(1, 1) = -2.0 / (float)ScreenHeight;
	proj2D(3, 0) = -1.0;
	proj2D(3, 1) = 1.0;
	RenderDevice->SetTransform(D3DTS_PROJECTION, &proj2D);

	RenderDevice->SetTransform(D3DTS_VIEW, &IdentityMatrix);

	RenderDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	RenderDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	RenderDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	RenderDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);

	SetFade(0);
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::SetupView
//
//==========================================================================

void VDirect3DDrawer::SetupView(VRenderLevelDrawer* ARLev, const refdef_t *rd)
{
	guard(VDirect3DDrawer::SetupView);
	RendLev = ARLev;

	if (!rd->DrawCamera && rd->drawworld && rd->width != ScreenWidth)
	{
		R_DrawViewBorder();
	}

	//	Setup viewport
	memset(&viewData, 0, sizeof(D3DVIEWPORT9));
	viewData.X = rd->x;
	viewData.Y = rd->y;
	viewData.Width  = rd->width;
	viewData.Height = rd->height;
	viewData.MinZ = 0;
	viewData.MaxZ = 1;
	RenderDevice->SetViewport(&viewData);

	//	Setup projection
	memset(&matProj, 0, sizeof(D3DMATRIX));
	matProj(0, 0) = 1 / rd->fovx;
	matProj(1, 1) = 1 / rd->fovy;
	float mindist = 1.0;
	float Q = maxdist / (maxdist - mindist);
	matProj(2, 2) = Q;
	matProj(3, 2) = -Q * mindist;
	matProj(2, 3) = 1;
	RenderDevice->SetTransform(D3DTS_PROJECTION, &matProj);

	RenderDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	RenderDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	RenderDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	RenderDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

	cacheframecount++;

	RenderDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0, 1.0, 0);
	if (HasStencil)
	{
        RenderDevice->Clear(0, NULL, D3DCLEAR_STENCIL, 0, 1.0, 0);
	}
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::SetupViewOrg
//
//==========================================================================

void VDirect3DDrawer::SetupViewOrg()
{
	guard(VDirect3DDrawer::SetupViewOrg);
	// The view matrix defines the position and orientation of the camera.
	MyD3DMatrix matView;
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
	RenderDevice->SetTransform(D3DTS_VIEW, &matView);

	if (MirrorFlip)
	{
		RenderDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	}
	else
	{
		RenderDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	}

	if (MirrorClip)
	{
        RenderDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, 1);
		float eq[4] = { view_clipplanes[4].normal.x,
			view_clipplanes[4].normal.y, view_clipplanes[4].normal.z,
			-view_clipplanes[4].dist };
        RenderDevice->SetClipPlane(0, eq);
	}
	else
	{
        RenderDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, 0);
	}

	memset(light_chain, 0, sizeof(light_chain));
	memset(add_chain, 0, sizeof(add_chain));
	SimpleSurfsHead = NULL;
	SimpleSurfsTail = NULL;
	SkyPortalsHead = NULL;
	SkyPortalsTail = NULL;
	HorizonPortalsHead = NULL;
	HorizonPortalsTail = NULL;
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::EndView
//
//==========================================================================

void VDirect3DDrawer::EndView()
{
	guard(VDirect3DDrawer::EndView);
	Setup2D();

	if (cl && cl->CShift)
	{
		MyD3DVertex	dv[4];

		RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
		RenderDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		RenderDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

		dv[0] = MyD3DVertex(0, 0, cl->CShift, 0, 0);
		dv[1] = MyD3DVertex(ScreenWidth, 0, cl->CShift, 0, 0);
		dv[2] = MyD3DVertex(ScreenWidth, ScreenHeight, cl->CShift, 0, 0);
		dv[3] = MyD3DVertex(0, ScreenHeight, cl->CShift, 0, 0);

		RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, dv, sizeof(MyD3DVertex));

		RenderDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		RenderDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
		RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	}
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::Update
//
// 	Blit to the screen / Flip surfaces
//
//==========================================================================

void VDirect3DDrawer::Update()
{
	guard(VDirect3DDrawer::Update);
	// End the scene.
	RenderDevice->EndScene();

	RenderDevice->Present(NULL, NULL, NULL, NULL);
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::BeginDirectUpdate
//
//==========================================================================

void VDirect3DDrawer::BeginDirectUpdate()
{
	guard(VDirect3DDrawer::BeginDirectUpdate);
	// Begin the scene.
	RenderDevice->BeginScene();
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::EndDirectUpdate
//
//==========================================================================

void VDirect3DDrawer::EndDirectUpdate()
{
	Update();
}

//==========================================================================
//
// 	VDirect3DDrawer::Shutdown
//
//	Close the graphics
//
//==========================================================================

void VDirect3DDrawer::Shutdown()
{
	guard(VDirect3DDrawer::Shutdown);
	ReleaseTextures();
	SAFE_RELEASE(RenderDevice)
	SAFE_RELEASE(Direct3D)
	if (DLLHandle)
	{
		FreeLibrary(DLLHandle);
		DLLHandle = NULL;
	}
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::ReadScreen
//
//==========================================================================

void *VDirect3DDrawer::ReadScreen(int *bpp, bool *bot2top)
{
	guard(VDirect3DDrawer::ReadScreen);
	//	Allocate buffer
	void* dst = Z_Malloc(ScreenWidth * ScreenHeight * sizeof(rgb_t));

	LPDIRECT3DSURFACE9 surf;
	RenderDevice->GetRenderTarget(0, &surf);

	D3DSURFACE_DESC desc;
	surf->GetDesc(&desc);


	//	Decode pixel format
	int scr_rbits;
	int scr_rshift;
	int scr_gbits;
	int scr_gshift;
	int scr_bbits;
	int scr_bshift;
	int scr_pixbytes;
	if (desc.Format == D3DFMT_X1R5G5B5)
	{
		scr_rbits = 5;
		scr_rshift = 10;
		scr_gbits = 5;
		scr_gshift = 5;
		scr_bbits = 5;
		scr_bshift = 0;
		scr_pixbytes = 2;
	}
	else if (desc.Format == D3DFMT_R5G6B5)
	{
		scr_rbits = 5;
		scr_rshift = 11;
		scr_gbits = 6;
		scr_gshift = 5;
		scr_bbits = 5;
		scr_bshift = 0;
		scr_pixbytes = 2;
	}
	else if (desc.Format == D3DFMT_X8R8G8B8 || desc.Format == D3DFMT_A8R8G8B8)
	{
		scr_rbits = 8;
		scr_rshift = 16;
		scr_gbits = 8;
		scr_gshift = 8;
		scr_bbits = 8;
		scr_bshift = 0;
		scr_pixbytes = 4;
	}
	else
	{
		GCon->Log(NAME_Init, "Invalid pixel format");
		Z_Free(dst);
		return NULL;
	}

	D3DLOCKED_RECT lrect;
	if (FAILED(surf->LockRect(&lrect, NULL, D3DLOCK_READONLY)))
	{
		Sys_Error("ReadScreen: Failed to lock screen");
	}

	rgb_t *pdst = (rgb_t*)dst;
	for (int j = 0; j < ScreenHeight; j++)
	{
		byte *psrc = (byte*)lrect.pBits + j * lrect.Pitch;
		for (int i = 0; i < ScreenWidth; i++)
		{
			pdst->r = byte((*(vuint32*)psrc >> scr_rshift) << (8 - scr_rbits));
			pdst->g = byte((*(vuint32*)psrc >> scr_gshift) << (8 - scr_gbits));
			pdst->b = byte((*(vuint32*)psrc >> scr_bshift) << (8 - scr_bbits));
			psrc += scr_pixbytes;
			pdst++;
		}
	}

	surf->UnlockRect();
	surf->Release();

	*bpp = 24;
	*bot2top = false;
	return dst;
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::ReadBackScreen
//
//==========================================================================

void VDirect3DDrawer::ReadBackScreen(int Width, int Height, rgba_t* Dest)
{
	guard(VDirect3DDrawer::ReadBackScreen);
	LPDIRECT3DSURFACE9 surf;
	RenderDevice->GetRenderTarget(0, &surf);

	D3DSURFACE_DESC desc;
	surf->GetDesc(&desc);

	//	Decode pixel format
	int scr_rbits;
	int scr_rshift;
	int scr_gbits;
	int scr_gshift;
	int scr_bbits;
	int scr_bshift;
	int scr_pixbytes;
	if (desc.Format == D3DFMT_X1R5G5B5)
	{
		scr_rbits = 5;
		scr_rshift = 10;
		scr_gbits = 5;
		scr_gshift = 5;
		scr_bbits = 5;
		scr_bshift = 0;
		scr_pixbytes = 2;
	}
	else if (desc.Format == D3DFMT_R5G6B5)
	{
		scr_rbits = 5;
		scr_rshift = 11;
		scr_gbits = 6;
		scr_gshift = 5;
		scr_bbits = 5;
		scr_bshift = 0;
		scr_pixbytes = 2;
	}
	else if (desc.Format == D3DFMT_X8R8G8B8 || desc.Format == D3DFMT_A8R8G8B8)
	{
		scr_rbits = 8;
		scr_rshift = 16;
		scr_gbits = 8;
		scr_gshift = 8;
		scr_bbits = 8;
		scr_bshift = 0;
		scr_pixbytes = 4;
	}
	else
	{
		GCon->Log(NAME_Init, "Invalid pixel format");
		return;
	}

	D3DLOCKED_RECT lrect;
	if (FAILED(surf->LockRect(&lrect, NULL, D3DLOCK_READONLY)))
	{
		Sys_Error("ReadScreen: Failed to lock screen");
	}

	rgba_t *pdst = Dest;
	for (int j = 0; j < Height; j++)
	{
		byte *psrc = (byte*)lrect.pBits + j * lrect.Pitch;
		for (int i = 0; i < Width; i++)
		{
			pdst->r = byte((*(vuint32*)psrc >> scr_rshift) << (8 - scr_rbits));
			pdst->g = byte((*(vuint32*)psrc >> scr_gshift) << (8 - scr_gbits));
			pdst->b = byte((*(vuint32*)psrc >> scr_bshift) << (8 - scr_bbits));
			pdst->a = 255;
			psrc += scr_pixbytes;
			pdst++;
		}
	}

	surf->UnlockRect();
	surf->Release();
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::SetFade
//
//==========================================================================

void VDirect3DDrawer::SetFade(vuint32 NewFade)
{
	guard(VDirect3DDrawer::SetFade);
	if (CurrentFade == NewFade)
	{
		return;
	}
	if (NewFade)
	{
		static const D3DFOGMODE fog_mode[4] = {
			D3DFOG_LINEAR, D3DFOG_LINEAR, D3DFOG_EXP, D3DFOG_EXP2 };

		RenderDevice->SetRenderState(D3DRS_FOGVERTEXMODE, fog_mode[r_fog & 3]);
		RenderDevice->SetRenderState(D3DRS_FOGCOLOR, NewFade);
		RenderDevice->SetRenderState(D3DRS_FOGDENSITY, PassFloat(r_fog_density));
		RenderDevice->SetRenderState(D3DRS_FOGSTART, PassFloat(r_fog_start));
		RenderDevice->SetRenderState(D3DRS_FOGEND, PassFloat(r_fog_end));
		RenderDevice->SetRenderState(D3DRS_FOGENABLE, TRUE);
	}
	else
	{
		RenderDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
	}
	CurrentFade = NewFade;
	unguard;
}

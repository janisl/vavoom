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

#include "gamedefs.h"
#include "cl_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

typedef struct
{
    char			manufacturer;
    char			version;
    char			encoding;
    char			bits_per_pixel;

    unsigned short	xmin;
    unsigned short	ymin;
    unsigned short	xmax;
    unsigned short	ymax;
    
    unsigned short	hres;
    unsigned short	vres;

    unsigned char	palette[48];
    
    char			reserved;
    char			color_planes;
    unsigned short	bytes_per_line;
    unsigned short	palette_type;
    
    char			filler[58];
    unsigned char	data;		// unbounded
} pcx_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void CalcFadetable16(byte *pal);
void V_DarkenScreen(int darkening);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern int				screenblocks;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int		ScreenWidth = 0;
int		ScreenHeight = 0;
int		ScreenBPP = 0;

int		PixelBytes;
float	PixelAspect;

float	fScaleX;
float	fScaleY;
float	fScaleXI;
float	fScaleYI;

int		usegamma = 0;

boolean				graphics_started = false;

// Table of RGB values in current gamma corection level
byte 	gammatable[5][256] =
{
    {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
     17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,
     33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,
     49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,
     65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,
     81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,
     97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,
     113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,
     128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
     144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
     160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
     176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
     192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
     208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
     224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
     240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255},

    {2,4,5,7,8,10,11,12,14,15,16,18,19,20,21,23,24,25,26,27,29,30,31,
     32,33,34,36,37,38,39,40,41,42,44,45,46,47,48,49,50,51,52,54,55,
     56,57,58,59,60,61,62,63,64,65,66,67,69,70,71,72,73,74,75,76,77,
     78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,
     99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,
	 115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,129,
     130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,
     146,147,148,148,149,150,151,152,153,154,155,156,157,158,159,160,
     161,162,163,163,164,165,166,167,168,169,170,171,172,173,174,175,
     175,176,177,178,179,180,181,182,183,184,185,186,186,187,188,189,
     190,191,192,193,194,195,196,196,197,198,199,200,201,202,203,204,
     205,205,206,207,208,209,210,211,212,213,214,214,215,216,217,218,
     219,220,221,222,222,223,224,225,226,227,228,229,230,230,231,232,
     233,234,235,236,237,237,238,239,240,241,242,243,244,245,245,246,
     247,248,249,250,251,252,252,253,254,255},

    {4,7,9,11,13,15,17,19,21,22,24,26,27,29,30,32,33,35,36,38,39,40,42,
     43,45,46,47,48,50,51,52,54,55,56,57,59,60,61,62,63,65,66,67,68,69,
     70,72,73,74,75,76,77,78,79,80,82,83,84,85,86,87,88,89,90,91,92,93,
     94,95,96,97,98,100,101,102,103,104,105,106,107,108,109,110,111,112,
     113,114,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,
     129,130,131,132,133,133,134,135,136,137,138,139,140,141,142,143,144,
     144,145,146,147,148,149,150,151,152,153,153,154,155,156,157,158,159,
	 160,160,161,162,163,164,165,166,166,167,168,169,170,171,172,172,173,
     174,175,176,177,178,178,179,180,181,182,183,183,184,185,186,187,188,
     188,189,190,191,192,193,193,194,195,196,197,197,198,199,200,201,201,
     202,203,204,205,206,206,207,208,209,210,210,211,212,213,213,214,215,
     216,217,217,218,219,220,221,221,222,223,224,224,225,226,227,228,228,
     229,230,231,231,232,233,234,235,235,236,237,238,238,239,240,241,241,
     242,243,244,244,245,246,247,247,248,249,250,251,251,252,253,254,254,
     255},

    {8,12,16,19,22,24,27,29,31,34,36,38,40,41,43,45,47,49,50,52,53,55,
     57,58,60,61,63,64,65,67,68,70,71,72,74,75,76,77,79,80,81,82,84,85,
     86,87,88,90,91,92,93,94,95,96,98,99,100,101,102,103,104,105,106,107,
     108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,
     125,126,127,128,129,130,131,132,133,134,135,135,136,137,138,139,140,
     141,142,143,143,144,145,146,147,148,149,150,150,151,152,153,154,155,
     155,156,157,158,159,160,160,161,162,163,164,165,165,166,167,168,169,
     169,170,171,172,173,173,174,175,176,176,177,178,179,180,180,181,182,
     183,183,184,185,186,186,187,188,189,189,190,191,192,192,193,194,195,
	 195,196,197,197,198,199,200,200,201,202,202,203,204,205,205,206,207,
     207,208,209,210,210,211,212,212,213,214,214,215,216,216,217,218,219,
     219,220,221,221,222,223,223,224,225,225,226,227,227,228,229,229,230,
     231,231,232,233,233,234,235,235,236,237,237,238,238,239,240,240,241,
     242,242,243,244,244,245,246,246,247,247,248,249,249,250,251,251,252,
     253,253,254,254,255},

    {16,23,28,32,36,39,42,45,48,50,53,55,57,60,62,64,66,68,69,71,73,75,76,
     78,80,81,83,84,86,87,89,90,92,93,94,96,97,98,100,101,102,103,105,106,
     107,108,109,110,112,113,114,115,116,117,118,119,120,121,122,123,124,
     125,126,128,128,129,130,131,132,133,134,135,136,137,138,139,140,141,
     142,143,143,144,145,146,147,148,149,150,150,151,152,153,154,155,155,
     156,157,158,159,159,160,161,162,163,163,164,165,166,166,167,168,169,
     169,170,171,172,172,173,174,175,175,176,177,177,178,179,180,180,181,
     182,182,183,184,184,185,186,187,187,188,189,189,190,191,191,192,193,
     193,194,195,195,196,196,197,198,198,199,200,200,201,202,202,203,203,
     204,205,205,206,207,207,208,208,209,210,210,211,211,212,213,213,214,
     214,215,216,216,217,217,218,219,219,220,220,221,221,222,223,223,224,
	 224,225,225,226,227,227,228,228,229,229,230,230,231,232,232,233,233,
     234,234,235,235,236,236,237,237,238,239,239,240,240,241,241,242,242,
     243,243,244,244,245,245,246,246,247,247,248,248,249,249,250,250,251,
     251,252,252,253,254,254,255,255}
};

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static boolean	setresolutionneeded = false;
static int		setwidth;
static int		setheight;
static int		setbpp;

static TCvarI	menu_darkening("menu_darkening", "16", CVAR_ARCHIVE);
static TCvarI	draw_pause("draw_pause", "1");

static TCvarI	screen_width("screen_width", "0", CVAR_ARCHIVE);
static TCvarI	screen_height("screen_height", "0", CVAR_ARCHIVE);
static TCvarI	screen_bpp("screen_bpp", "0", CVAR_ARCHIVE);
static TCvarI	brightness("brightness", "0", CVAR_ARCHIVE);

static TCvarI	draw_fps("draw_fps", "0", CVAR_ARCHIVE);
static double	fps_start = 0.0;
static int		fps_frames = 0;
static int		show_fps = 0;

static int		scr_cur_pal;

// CODE --------------------------------------------------------------------

//**************************************************************************
//
//	Screenshots
//
//**************************************************************************

//==========================================================================
//
//  WritePCXfile
//
//==========================================================================

void WritePCXfile(char* filename, void* data, int width, int height, int bpp,
	byte* palette, bool bot2top)
{
    int		i;
	int		j;
    int		length;
    pcx_t*	pcx;
    byte*	pack;
	
    pcx = (pcx_t*)Z_Malloc(width * height * (bpp == 8 ? 2 : 6) + 1000, PU_STATIC, NULL);

    pcx->manufacturer = 0x0a;	// PCX id
    pcx->version = 5;			// 256 color
    pcx->encoding = 1;			// uncompressed
    pcx->bits_per_pixel = 8;	// 256 color
    pcx->xmin = 0;
    pcx->ymin = 0;
    pcx->xmax = LittleShort(width - 1);
    pcx->ymax = LittleShort(height - 1);
    pcx->hres = LittleShort(width);
    pcx->vres = LittleShort(height);
    memset(pcx->palette, 0, sizeof(pcx->palette));
    pcx->color_planes = bpp == 8 ? 1 : 3;
    pcx->bytes_per_line = LittleShort(width);
    pcx->palette_type = LittleShort(1);	// not a grey scale
    memset(pcx->filler, 0, sizeof(pcx->filler));

    // pack the image
    pack = &pcx->data;

	if (bpp == 8)
	{
		for (j = 0; j < height; j++)
		{
			byte *src = (byte*)data + j * width;
			for (i = 0; i < width; i++)
			{
				if ((src[i] & 0xc0) == 0xc0)
				{
					*pack++ = 0xc1;
				}
				*pack++ = src[i];
			}
		}

		// write the palette
		*pack++ = 0x0c;	// palette ID byte
		for (i = 0; i < 768; i++)
		{
			*pack++ = *palette++;
		}
	}
	else if	(bpp == 24)
	{
		for (j = 0; j < height; j++)
		{
			rgb_t *src = (rgb_t*)data + (bot2top ? height - j - 1 : j) * width;
			for (i = 0; i < width; i++)
			{
				if ((src[i].r & 0xc0) == 0xc0)
				{
					*pack++ = 0xc1;
				}
				*pack++ = src[i].r;
			}
			for (i = 0; i < width; i++)
			{
				if ((src[i].g & 0xc0) == 0xc0)
				{
					*pack++ = 0xc1;
				}
				*pack++ = src[i].g;
			}
			for (i = 0; i < width; i++)
			{
				if ((src[i].b & 0xc0) == 0xc0)
				{
					*pack++ = 0xc1;
				}
				*pack++ = src[i].b;
			}
		}
	}
    
    // write output file
    length = pack - (byte *)pcx;
    M_WriteFile(filename, pcx, length);

    Z_Free(pcx);
}

//==========================================================================
//
//  ScreenShot_f
//
//==========================================================================

COMMAND(ScreenShot)
{
	int		i;
	int		bpp;
	bool	bot2top;
	void	*data;
	char	filename[128];
    
	//	Find a file name to save it to
	for (i = 0; i <= 9999; i++)
	{
#ifdef DEVELOPER
		sprintf(filename, "basev/shot%04d.pcx", i);
#else
		sprintf(filename, "%s/shot%04d.pcx", fl_gamedir, i);
#endif
		if (!Sys_FileExists(filename))
			break;	//	File doesn't exist
	}
	if (i == 10000)
	{
		con << "Couldn't create a PCX";
		return;
	}
    
    // save the pcx file
	data = Drawer->ReadScreen(&bpp, &bot2top);
	if (data)
	{
	    WritePCXfile(filename, data, ScreenWidth, ScreenHeight, bpp,
			(byte*)W_CacheLumpName("PLAYPAL", PU_CACHE), bot2top);
		Z_Free(data);
	}
}

//**************************************************************************
//
//	Misc drawing stuff
//
//**************************************************************************

//==========================================================================
//
//	DrawFPS
//
//==========================================================================

static void DrawFPS(void)
{
	double		time;

	if (draw_fps)
	{
		time = Sys_Time();
		fps_frames++;

		if (time - fps_start > 1.0)
		{
			show_fps = (int)(fps_frames / (time - fps_start));
			fps_start = time;
			fps_frames = 0;
		}

		T_SetFont(font_small);
		T_SetAlign(hright, vtop);
		T_DrawString(319, 0, va("%d fps", show_fps));
	}
}

//**************************************************************************
//
//	Resolution change
//
//**************************************************************************

//==========================================================================
//
//	ChangeResolution
//
//==========================================================================

static void ChangeResolution(int width, int height, int bpp)
{
	if (width > MAXSCREENWIDTH)
		width = MAXSCREENWIDTH;
	if (height > MAXSCREENHEIGHT)
		height = MAXSCREENHEIGHT;
	if (bpp != 8 && bpp != 15 && bpp != 16 && bpp != 24 && bpp != 32)
	{
		con << "Invalid bpp, using 8\n";
		bpp = 8;
	}

	// Changing resolution
    if (!Drawer->SetResolution(width, height, bpp))
   	{
        con << "Failed to set resolution " << width << "x" << height << "x" << bpp << endl;
   	   	if (ScreenWidth)
       	{
           	if (!Drawer->SetResolution(ScreenWidth, ScreenHeight, ScreenBPP))
           		Sys_Error("ChangeResolution: failed to restore resolution");
            else
   	            con << "Restoring previous resolution\n";
       	}
        else
   		{
   			if (!Drawer->SetResolution(0, 0, 0))
   				Sys_Error("ChangeResolution: Failed to set default resolution");
			else
           		con << "Setting default resolution\n";
	  	}
   	}
	con << ScreenWidth << "x" << ScreenHeight << "x" << ScreenBPP << ".\n";

	screen_width = ScreenWidth;
	screen_height = ScreenHeight;
	screen_bpp = ScreenBPP;

	PixelBytes = (ScreenBPP + 7) / 8;
	PixelAspect = ((float)ScreenHeight * 4.0) / ((float)ScreenWidth * 3.0);

    fScaleX = (float)ScreenWidth / 320.0;
    fScaleY = (float)ScreenHeight / 200.0;
    fScaleXI = 320.0 / (float)ScreenWidth;
    fScaleYI = 200.0 / (float)ScreenHeight;
}

//==========================================================================
//
//  CheckResolutionChange
//
//==========================================================================

static void CheckResolutionChange(void)
{
	bool		must_set_pal = false;
	bool		res_changed = false;

	if (brightness != usegamma)
	{
		usegamma = brightness;
		if (usegamma < 0)
		{
			usegamma = 0;
			brightness = usegamma;
		}
		if (usegamma > 4)
		{
			usegamma = 4;
			brightness = usegamma;
		}
		must_set_pal = true;
	}
	if (setresolutionneeded)
	{
		ChangeResolution(setwidth, setheight, setbpp);
		setresolutionneeded = false;
		res_changed = true;
		must_set_pal = true;
	}
	else if (!screen_width || screen_width != ScreenWidth ||
		screen_height != ScreenHeight || screen_bpp != ScreenBPP)
	{
		ChangeResolution(screen_width, screen_height, screen_bpp);
		res_changed = true;
		must_set_pal = true;
	}

	if (must_set_pal)
	{
		Drawer->SetPalette(scr_cur_pal);
	}

	if (res_changed)
	{
		Drawer->InitResolution();
		//	Recalculate view size and other data
		R_SetViewSize(screenblocks);
	}
	graphics_started = true;
}

//==========================================================================
//
//  SetResolution_f
//
//==========================================================================

COMMAND(SetResolution)
{
   	if (Argc() == 3)
	{
     	setwidth = superatoi(Argv(1));
		setheight = superatoi(Argv(2));
		setbpp = ScreenBPP;
		setresolutionneeded = true;
	}
   	else if (Argc() == 4)
	{
     	setwidth = superatoi(Argv(1));
		setheight = superatoi(Argv(2));
		setbpp = superatoi(Argv(3));
		setresolutionneeded = true;
	}
    else
    {
		con << "SetResolution <width> <height> [<bpp>]:change resolution\n";
	}
}

//**************************************************************************
//
//	General (pubic) stuff
//
//**************************************************************************

//==========================================================================
//
//  SCR_Init
//
//==========================================================================

void SCR_Init(void)
{
}

//==========================================================================
//
//  SCR_Update
//
//==========================================================================

void SCR_Update(void)
{
	if (cls.state == ca_connected && cls.signon != SIGNONS)
	{
		return;
	}


	CheckResolutionChange();

	Drawer->StartUpdate();

	if (scr_cur_pal != cl.palette)
	{
		Drawer->SetPalette(cl.palette);
		scr_cur_pal = cl.palette;
	}

	// do buffered drawing
	if (cls.state == ca_disconnected)
	{
		clpr.SetGlobal("frametime", PassFloat(host_frametime));
		clpr.Exec("PageDrawer");
	}
	else
	{
		switch (cl.intermission)
		{
		  case 0:
			if (automapactive)
			{
				AM_Drawer();
			}
			else
			{
				R_RenderPlayerView();
			}
			CT_Drawer();
			SB_Drawer();
			break;

		  case 1:
			IM_Drawer();
			break;

		  case 2:
			F_Drawer();
			break;
		}
	}

	if (MN_Active())
		V_DarkenScreen(menu_darkening);

	// Console drawing
	C_Drawer();

	// Menu drawing
	MN_Drawer();

	MB_Drawer();

	DrawFPS();

	Drawer->Update();              // page flip or blit buffer
}

//==========================================================================
//
// Draw_TeleportIcon
//
//==========================================================================

void Draw_TeleportIcon(void)
{
	if (W_CheckNumForName("teleicon") >= 0)
	{
		Drawer->BeginDirectUpdate();
		R_DrawPic(100, 68, R_RegisterPic("teleicon", PIC_PATCH));
		Drawer->EndDirectUpdate();
	}
}

//==========================================================================
//
// Draw_SaveIcon
//
//==========================================================================

void Draw_SaveIcon(void)
{
	if (W_CheckNumForName("saveicon") >= 0)
	{
		Drawer->BeginDirectUpdate();
		R_DrawPic(100, 68, R_RegisterPic("saveicon", PIC_PATCH));
		Drawer->EndDirectUpdate();
	}
}

//==========================================================================
//
// Draw_LoadIcon
//
//==========================================================================

void Draw_LoadIcon(void)
{
	if (W_CheckNumForName("loadicon") >= 0)
	{
		Drawer->BeginDirectUpdate();
		R_DrawPic(100, 68, R_RegisterPic("loadicon", PIC_PATCH));
		Drawer->EndDirectUpdate();
	}
}

//**************************************************************************
//
//	$Log$
//	Revision 1.9  2001/09/12 17:35:13  dj_jl
//	Direct update for Hexen icons
//
//	Revision 1.8  2001/08/30 17:46:21  dj_jl
//	Removed game dependency
//	
//	Revision 1.7  2001/08/17 17:43:40  dj_jl
//	LINUX fixes
//	
//	Revision 1.6  2001/08/15 17:15:55  dj_jl
//	Drawer API changes, removed wipes
//	
//	Revision 1.5  2001/08/07 16:47:44  dj_jl
//	Fixed screenshots
//	
//	Revision 1.4  2001/08/04 17:25:14  dj_jl
//	Moved title / demo loop to progs
//	Removed shareware / ExtendedWAD from engine
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************

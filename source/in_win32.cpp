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

#define DIRECTINPUT_VERSION		0x0500
#include "winlocal.h"
#include <dinput.h>
#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VDirectInputDevice : public VInputDevice
{
public:
	VDirectInputDevice();
	~VDirectInputDevice();

	void ReadInput();
	void SetActiveWindow(HWND window);

private:
	LPDIRECTINPUT			DInput;

	LPDIRECTINPUTDEVICE		lpKeyboard;

	LPDIRECTINPUTDEVICE		lpMouse;
	bool					mousepresent;
	int						old_mouse_x;
	int						old_mouse_y;
	int						old_mouse_z;

	LPDIRECTINPUTDEVICE2	lpJoystick;
	bool					joystick_started;

	static const vuint8			scan2key[256];
	static DIOBJECTDATAFORMAT	rgodf_Keyboard[];
	static DIDATAFORMAT			df_Keyboard;
	static DIOBJECTDATAFORMAT	rgodf_Mouse[];
	static DIDATAFORMAT			df_Mouse;
	static DIOBJECTDATAFORMAT	rgodf_Joy[];
	static DIDATAFORMAT			df_Joystick;

	void StartupKeyboard();
	void ReadKeyboard();
	void ShutdownKeyboard();

	void StartupMouse();
	void ReadMouse();
	void ShutdownMouse();

	void StartupJoystick();
	static int FAR PASCAL JoystickEnumCallback(LPCDIDEVICEINSTANCE, LPVOID);
	int JoystickEnumCallback(LPCDIDEVICEINSTANCE);
	void ReadJoystick();
	void ShutdownJoystick();
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// Key code translation table
const vuint8 VDirectInputDevice::scan2key[256] =
{
	0, K_ESCAPE, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', K_BACKSPACE, K_TAB,
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', K_ENTER, K_LCTRL, 'a', 's',
	'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', K_LSHIFT, '\\', 'z', 'x', 'c', 'v',
	'b', 'n', 'm', ',', '.', '/', K_RSHIFT, K_PADMULTIPLE, K_LALT, ' ', K_CAPSLOCK, K_F1, K_F2, K_F3, K_F4, K_F5,
	K_F6, K_F7, K_F8, K_F9, K_F10, K_NUMLOCK, K_SCROLLLOCK, K_PAD7, K_PAD8, K_PAD9, K_PADMINUS, K_PAD4, K_PAD5, K_PAD6, K_PADPLUS, K_PAD1,
	K_PAD2, K_PAD3, K_PAD0, K_PADDOT, 84, 85, 86, K_F11, K_F12, 89, 90, 91, 92, 93, 94, 95,
	96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
	112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
	128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
	144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, K_PADENTER, K_RCTRL, 158, 159,
	160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
	176, 177, 178, 179, 180, K_PADDIVIDE, 182, K_PRINTSCRN, K_RALT, 185, 186, 187, 188, 189, 190, 191,
	192, 193, 194, 195, 196, 197, 198, K_HOME, K_UPARROW, K_PAGEUP, 202, K_LEFTARROW, 204, K_RIGHTARROW, 206, K_END, 
	K_DOWNARROW, K_PAGEDOWN, K_INSERT, K_DELETE, 212, 213, 214, 215, 216, 217, 218, K_LWIN, K_RWIN, K_MENU, 222, 223,
	224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
	240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
};

DIOBJECTDATAFORMAT VDirectInputDevice::rgodf_Keyboard[] =
{
	{ &GUID_Key,   0, 0x8000000c, 0},
	{ &GUID_Key,   1, 0x8000010c, 0},
	{ &GUID_Key,   2, 0x8000020c, 0},
	{ &GUID_Key,   3, 0x8000030c, 0},
	{ &GUID_Key,   4, 0x8000040c, 0},
	{ &GUID_Key,   5, 0x8000050c, 0},
	{ &GUID_Key,   6, 0x8000060c, 0},
	{ &GUID_Key,   7, 0x8000070c, 0},
	{ &GUID_Key,   8, 0x8000080c, 0},
	{ &GUID_Key,   9, 0x8000090c, 0},
	{ &GUID_Key,  10, 0x80000a0c, 0},
	{ &GUID_Key,  11, 0x80000b0c, 0},
	{ &GUID_Key,  12, 0x80000c0c, 0},
	{ &GUID_Key,  13, 0x80000d0c, 0},
	{ &GUID_Key,  14, 0x80000e0c, 0},
	{ &GUID_Key,  15, 0x80000f0c, 0},
	{ &GUID_Key,  16, 0x8000100c, 0},
	{ &GUID_Key,  17, 0x8000110c, 0},
	{ &GUID_Key,  18, 0x8000120c, 0},
	{ &GUID_Key,  19, 0x8000130c, 0},
	{ &GUID_Key,  20, 0x8000140c, 0},
	{ &GUID_Key,  21, 0x8000150c, 0},
	{ &GUID_Key,  22, 0x8000160c, 0},
	{ &GUID_Key,  23, 0x8000170c, 0},
	{ &GUID_Key,  24, 0x8000180c, 0},
	{ &GUID_Key,  25, 0x8000190c, 0},
	{ &GUID_Key,  26, 0x80001a0c, 0},
	{ &GUID_Key,  27, 0x80001b0c, 0},
	{ &GUID_Key,  28, 0x80001c0c, 0},
	{ &GUID_Key,  29, 0x80001d0c, 0},
	{ &GUID_Key,  30, 0x80001e0c, 0},
	{ &GUID_Key,  31, 0x80001f0c, 0},
	{ &GUID_Key,  32, 0x8000200c, 0},
	{ &GUID_Key,  33, 0x8000210c, 0},
	{ &GUID_Key,  34, 0x8000220c, 0},
	{ &GUID_Key,  35, 0x8000230c, 0},
	{ &GUID_Key,  36, 0x8000240c, 0},
	{ &GUID_Key,  37, 0x8000250c, 0},
	{ &GUID_Key,  38, 0x8000260c, 0},
	{ &GUID_Key,  39, 0x8000270c, 0},
	{ &GUID_Key,  40, 0x8000280c, 0},
	{ &GUID_Key,  41, 0x8000290c, 0},
	{ &GUID_Key,  42, 0x80002a0c, 0},
	{ &GUID_Key,  43, 0x80002b0c, 0},
	{ &GUID_Key,  44, 0x80002c0c, 0},
	{ &GUID_Key,  45, 0x80002d0c, 0},
	{ &GUID_Key,  46, 0x80002e0c, 0},
	{ &GUID_Key,  47, 0x80002f0c, 0},
	{ &GUID_Key,  48, 0x8000300c, 0},
	{ &GUID_Key,  49, 0x8000310c, 0},
	{ &GUID_Key,  50, 0x8000320c, 0},
	{ &GUID_Key,  51, 0x8000330c, 0},
	{ &GUID_Key,  52, 0x8000340c, 0},
	{ &GUID_Key,  53, 0x8000350c, 0},
	{ &GUID_Key,  54, 0x8000360c, 0},
	{ &GUID_Key,  55, 0x8000370c, 0},
	{ &GUID_Key,  56, 0x8000380c, 0},
	{ &GUID_Key,  57, 0x8000390c, 0},
	{ &GUID_Key,  58, 0x80003a0c, 0},
	{ &GUID_Key,  59, 0x80003b0c, 0},
	{ &GUID_Key,  60, 0x80003c0c, 0},
	{ &GUID_Key,  61, 0x80003d0c, 0},
	{ &GUID_Key,  62, 0x80003e0c, 0},
	{ &GUID_Key,  63, 0x80003f0c, 0},
	{ &GUID_Key,  64, 0x8000400c, 0},
	{ &GUID_Key,  65, 0x8000410c, 0},
	{ &GUID_Key,  66, 0x8000420c, 0},
	{ &GUID_Key,  67, 0x8000430c, 0},
	{ &GUID_Key,  68, 0x8000440c, 0},
	{ &GUID_Key,  69, 0x8000450c, 0},
	{ &GUID_Key,  70, 0x8000460c, 0},
	{ &GUID_Key,  71, 0x8000470c, 0},
	{ &GUID_Key,  72, 0x8000480c, 0},
	{ &GUID_Key,  73, 0x8000490c, 0},
	{ &GUID_Key,  74, 0x80004a0c, 0},
	{ &GUID_Key,  75, 0x80004b0c, 0},
	{ &GUID_Key,  76, 0x80004c0c, 0},
	{ &GUID_Key,  77, 0x80004d0c, 0},
	{ &GUID_Key,  78, 0x80004e0c, 0},
	{ &GUID_Key,  79, 0x80004f0c, 0},
	{ &GUID_Key,  80, 0x8000500c, 0},
	{ &GUID_Key,  81, 0x8000510c, 0},
	{ &GUID_Key,  82, 0x8000520c, 0},
	{ &GUID_Key,  83, 0x8000530c, 0},
	{ &GUID_Key,  84, 0x8000540c, 0},
	{ &GUID_Key,  85, 0x8000550c, 0},
	{ &GUID_Key,  86, 0x8000560c, 0},
	{ &GUID_Key,  87, 0x8000570c, 0},
	{ &GUID_Key,  88, 0x8000580c, 0},
	{ &GUID_Key,  89, 0x8000590c, 0},
	{ &GUID_Key,  90, 0x80005a0c, 0},
	{ &GUID_Key,  91, 0x80005b0c, 0},
	{ &GUID_Key,  92, 0x80005c0c, 0},
	{ &GUID_Key,  93, 0x80005d0c, 0},
	{ &GUID_Key,  94, 0x80005e0c, 0},
	{ &GUID_Key,  95, 0x80005f0c, 0},
	{ &GUID_Key,  96, 0x8000600c, 0},
	{ &GUID_Key,  97, 0x8000610c, 0},
	{ &GUID_Key,  98, 0x8000620c, 0},
	{ &GUID_Key,  99, 0x8000630c, 0},
	{ &GUID_Key, 100, 0x8000640c, 0},
	{ &GUID_Key, 101, 0x8000650c, 0},
	{ &GUID_Key, 102, 0x8000660c, 0},
	{ &GUID_Key, 103, 0x8000670c, 0},
	{ &GUID_Key, 104, 0x8000680c, 0},
	{ &GUID_Key, 105, 0x8000690c, 0},
	{ &GUID_Key, 106, 0x80006a0c, 0},
	{ &GUID_Key, 107, 0x80006b0c, 0},
	{ &GUID_Key, 108, 0x80006c0c, 0},
	{ &GUID_Key, 109, 0x80006d0c, 0},
	{ &GUID_Key, 110, 0x80006e0c, 0},
	{ &GUID_Key, 111, 0x80006f0c, 0},
	{ &GUID_Key, 112, 0x8000700c, 0},
	{ &GUID_Key, 113, 0x8000710c, 0},
	{ &GUID_Key, 114, 0x8000720c, 0},
	{ &GUID_Key, 115, 0x8000730c, 0},
	{ &GUID_Key, 116, 0x8000740c, 0},
	{ &GUID_Key, 117, 0x8000750c, 0},
	{ &GUID_Key, 118, 0x8000760c, 0},
	{ &GUID_Key, 119, 0x8000770c, 0},
	{ &GUID_Key, 120, 0x8000780c, 0},
	{ &GUID_Key, 121, 0x8000790c, 0},
	{ &GUID_Key, 122, 0x80007a0c, 0},
	{ &GUID_Key, 123, 0x80007b0c, 0},
	{ &GUID_Key, 124, 0x80007c0c, 0},
	{ &GUID_Key, 125, 0x80007d0c, 0},
	{ &GUID_Key, 126, 0x80007e0c, 0},
	{ &GUID_Key, 127, 0x80007f0c, 0},
	{ &GUID_Key, 128, 0x8000800c, 0},
	{ &GUID_Key, 129, 0x8000810c, 0},
	{ &GUID_Key, 130, 0x8000820c, 0},
	{ &GUID_Key, 131, 0x8000830c, 0},
	{ &GUID_Key, 132, 0x8000840c, 0},
	{ &GUID_Key, 133, 0x8000850c, 0},
	{ &GUID_Key, 134, 0x8000860c, 0},
	{ &GUID_Key, 135, 0x8000870c, 0},
	{ &GUID_Key, 136, 0x8000880c, 0},
	{ &GUID_Key, 137, 0x8000890c, 0},
	{ &GUID_Key, 138, 0x80008a0c, 0},
	{ &GUID_Key, 139, 0x80008b0c, 0},
	{ &GUID_Key, 140, 0x80008c0c, 0},
	{ &GUID_Key, 141, 0x80008d0c, 0},
	{ &GUID_Key, 142, 0x80008e0c, 0},
	{ &GUID_Key, 143, 0x80008f0c, 0},
	{ &GUID_Key, 144, 0x8000900c, 0},
	{ &GUID_Key, 145, 0x8000910c, 0},
	{ &GUID_Key, 146, 0x8000920c, 0},
	{ &GUID_Key, 147, 0x8000930c, 0},
	{ &GUID_Key, 148, 0x8000940c, 0},
	{ &GUID_Key, 149, 0x8000950c, 0},
	{ &GUID_Key, 150, 0x8000960c, 0},
	{ &GUID_Key, 151, 0x8000970c, 0},
	{ &GUID_Key, 152, 0x8000980c, 0},
	{ &GUID_Key, 153, 0x8000990c, 0},
	{ &GUID_Key, 154, 0x80009a0c, 0},
	{ &GUID_Key, 155, 0x80009b0c, 0},
	{ &GUID_Key, 156, 0x80009c0c, 0},
	{ &GUID_Key, 157, 0x80009d0c, 0},
	{ &GUID_Key, 158, 0x80009e0c, 0},
	{ &GUID_Key, 159, 0x80009f0c, 0},
	{ &GUID_Key, 160, 0x8000a00c, 0},
	{ &GUID_Key, 161, 0x8000a10c, 0},
	{ &GUID_Key, 162, 0x8000a20c, 0},
	{ &GUID_Key, 163, 0x8000a30c, 0},
	{ &GUID_Key, 164, 0x8000a40c, 0},
	{ &GUID_Key, 165, 0x8000a50c, 0},
	{ &GUID_Key, 166, 0x8000a60c, 0},
	{ &GUID_Key, 167, 0x8000a70c, 0},
	{ &GUID_Key, 168, 0x8000a80c, 0},
	{ &GUID_Key, 169, 0x8000a90c, 0},
	{ &GUID_Key, 170, 0x8000aa0c, 0},
	{ &GUID_Key, 171, 0x8000ab0c, 0},
	{ &GUID_Key, 172, 0x8000ac0c, 0},
	{ &GUID_Key, 173, 0x8000ad0c, 0},
	{ &GUID_Key, 174, 0x8000ae0c, 0},
	{ &GUID_Key, 175, 0x8000af0c, 0},
	{ &GUID_Key, 176, 0x8000b00c, 0},
	{ &GUID_Key, 177, 0x8000b10c, 0},
	{ &GUID_Key, 178, 0x8000b20c, 0},
	{ &GUID_Key, 179, 0x8000b30c, 0},
	{ &GUID_Key, 180, 0x8000b40c, 0},
	{ &GUID_Key, 181, 0x8000b50c, 0},
	{ &GUID_Key, 182, 0x8000b60c, 0},
	{ &GUID_Key, 183, 0x8000b70c, 0},
	{ &GUID_Key, 184, 0x8000b80c, 0},
	{ &GUID_Key, 185, 0x8000b90c, 0},
	{ &GUID_Key, 186, 0x8000ba0c, 0},
	{ &GUID_Key, 187, 0x8000bb0c, 0},
	{ &GUID_Key, 188, 0x8000bc0c, 0},
	{ &GUID_Key, 189, 0x8000bd0c, 0},
	{ &GUID_Key, 190, 0x8000be0c, 0},
	{ &GUID_Key, 191, 0x8000bf0c, 0},
	{ &GUID_Key, 192, 0x8000c00c, 0},
	{ &GUID_Key, 193, 0x8000c10c, 0},
	{ &GUID_Key, 194, 0x8000c20c, 0},
	{ &GUID_Key, 195, 0x8000c30c, 0},
	{ &GUID_Key, 196, 0x8000c40c, 0},
	{ &GUID_Key, 197, 0x8000c50c, 0},
	{ &GUID_Key, 198, 0x8000c60c, 0},
	{ &GUID_Key, 199, 0x8000c70c, 0},
	{ &GUID_Key, 200, 0x8000c80c, 0},
	{ &GUID_Key, 201, 0x8000c90c, 0},
	{ &GUID_Key, 202, 0x8000ca0c, 0},
	{ &GUID_Key, 203, 0x8000cb0c, 0},
	{ &GUID_Key, 204, 0x8000cc0c, 0},
	{ &GUID_Key, 205, 0x8000cd0c, 0},
	{ &GUID_Key, 206, 0x8000ce0c, 0},
	{ &GUID_Key, 207, 0x8000cf0c, 0},
	{ &GUID_Key, 208, 0x8000d00c, 0},
	{ &GUID_Key, 209, 0x8000d10c, 0},
	{ &GUID_Key, 210, 0x8000d20c, 0},
	{ &GUID_Key, 211, 0x8000d30c, 0},
	{ &GUID_Key, 212, 0x8000d40c, 0},
	{ &GUID_Key, 213, 0x8000d50c, 0},
	{ &GUID_Key, 214, 0x8000d60c, 0},
	{ &GUID_Key, 215, 0x8000d70c, 0},
	{ &GUID_Key, 216, 0x8000d80c, 0},
	{ &GUID_Key, 217, 0x8000d90c, 0},
	{ &GUID_Key, 218, 0x8000da0c, 0},
	{ &GUID_Key, 219, 0x8000db0c, 0},
	{ &GUID_Key, 220, 0x8000dc0c, 0},
	{ &GUID_Key, 221, 0x8000dd0c, 0},
	{ &GUID_Key, 222, 0x8000de0c, 0},
	{ &GUID_Key, 223, 0x8000df0c, 0},
	{ &GUID_Key, 224, 0x8000e00c, 0},
	{ &GUID_Key, 225, 0x8000e10c, 0},
	{ &GUID_Key, 226, 0x8000e20c, 0},
	{ &GUID_Key, 227, 0x8000e30c, 0},
	{ &GUID_Key, 228, 0x8000e40c, 0},
	{ &GUID_Key, 229, 0x8000e50c, 0},
	{ &GUID_Key, 230, 0x8000e60c, 0},
	{ &GUID_Key, 231, 0x8000e70c, 0},
	{ &GUID_Key, 232, 0x8000e80c, 0},
	{ &GUID_Key, 233, 0x8000e90c, 0},
	{ &GUID_Key, 234, 0x8000ea0c, 0},
	{ &GUID_Key, 235, 0x8000eb0c, 0},
	{ &GUID_Key, 236, 0x8000ec0c, 0},
	{ &GUID_Key, 237, 0x8000ed0c, 0},
	{ &GUID_Key, 238, 0x8000ee0c, 0},
	{ &GUID_Key, 239, 0x8000ef0c, 0},
	{ &GUID_Key, 240, 0x8000f00c, 0},
	{ &GUID_Key, 241, 0x8000f10c, 0},
	{ &GUID_Key, 242, 0x8000f20c, 0},
	{ &GUID_Key, 243, 0x8000f30c, 0},
	{ &GUID_Key, 244, 0x8000f40c, 0},
	{ &GUID_Key, 245, 0x8000f50c, 0},
	{ &GUID_Key, 246, 0x8000f60c, 0},
	{ &GUID_Key, 247, 0x8000f70c, 0},
	{ &GUID_Key, 248, 0x8000f80c, 0},
	{ &GUID_Key, 249, 0x8000f90c, 0},
	{ &GUID_Key, 250, 0x8000fa0c, 0},
	{ &GUID_Key, 251, 0x8000fb0c, 0},
	{ &GUID_Key, 252, 0x8000fc0c, 0},
	{ &GUID_Key, 253, 0x8000fd0c, 0},
	{ &GUID_Key, 254, 0x8000fe0c, 0},
	{ &GUID_Key, 255, 0x8000ff0c, 0}
};

DIDATAFORMAT	VDirectInputDevice::df_Keyboard =
{
	sizeof(DIDATAFORMAT),				// this structure
	sizeof(DIOBJECTDATAFORMAT),			// size of object data format
	DIDF_RELAXIS,						// absolute axis coordinates
	256,								// device data size
	sizeof(VDirectInputDevice::rgodf_Keyboard) /
		sizeof(VDirectInputDevice::rgodf_Keyboard[0]),	// number of objects
	VDirectInputDevice::rgodf_Keyboard,	// and here they are
};

typedef struct
{
	LONG  lX;					// X axis goes here
	LONG  lY;					// Y axis goes here
	LONG  lZ;					// Z axis goes here
	BYTE  bButtonA;				// One button goes here
	BYTE  bButtonB;				// Another button goes here
	BYTE  bButtonC;				// Another button goes here
	BYTE  bButtonD;				// Another button goes here
} MOUSE_DATA;

DIOBJECTDATAFORMAT VDirectInputDevice::rgodf_Mouse[] =
{
	{ &GUID_XAxis,	FIELD_OFFSET(MOUSE_DATA, lX),		DIDFT_AXIS | DIDFT_ANYINSTANCE, 0},
	{ &GUID_YAxis,	FIELD_OFFSET(MOUSE_DATA, lY),		DIDFT_AXIS | DIDFT_ANYINSTANCE, 0},
	{ &GUID_ZAxis,	FIELD_OFFSET(MOUSE_DATA, lZ),		0x80000000 | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0},
	{ 0,			FIELD_OFFSET(MOUSE_DATA, bButtonA),	DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0},
	{ 0,			FIELD_OFFSET(MOUSE_DATA, bButtonB),	DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0},
	{ 0,			FIELD_OFFSET(MOUSE_DATA, bButtonC),	0x80000000 | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0},
	{ 0,			FIELD_OFFSET(MOUSE_DATA, bButtonD),	0x80000000 | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0}
};

DIDATAFORMAT	VDirectInputDevice::df_Mouse =
{
	sizeof(DIDATAFORMAT),				// this structure
	sizeof(DIOBJECTDATAFORMAT),			// size of object data format
	DIDF_RELAXIS,						// absolute axis coordinates
	sizeof(MOUSE_DATA),					// device data size
	sizeof(VDirectInputDevice::rgodf_Mouse) /
		sizeof(VDirectInputDevice::rgodf_Mouse[0]),	// number of objects
	VDirectInputDevice::rgodf_Mouse,	// and here they are
};

DIOBJECTDATAFORMAT VDirectInputDevice::rgodf_Joy[] =
{
	{ &GUID_XAxis,   0, 0x80ffff03, 256},
	{ &GUID_YAxis,   4, 0x80ffff03, 256},
	{ &GUID_ZAxis,   8, 0x80ffff03, 256},
	{ &GUID_RxAxis, 12, 0x80ffff03, 256},
	{ &GUID_RyAxis, 16, 0x80ffff03, 256},
	{ &GUID_RzAxis, 20, 0x80ffff03, 256},
	{ &GUID_Slider, 24, 0x80ffff03, 256},
	{ &GUID_Slider, 28, 0x80ffff03, 256},
	{ &GUID_POV,	32, 0x80ffff10, 0},
	{ &GUID_POV,	36, 0x80ffff10, 0},
	{ &GUID_POV,	40, 0x80ffff10, 0},
	{ &GUID_POV,	44, 0x80ffff10, 0},
	{ 0,			48, 0x80ffff0c, 0},
	{ 0,			49, 0x80ffff0c, 0},
	{ 0,			50, 0x80ffff0c, 0},
	{ 0,			51, 0x80ffff0c, 0},
	{ 0,			52, 0x80ffff0c, 0},
	{ 0,			53, 0x80ffff0c, 0},
	{ 0,			54, 0x80ffff0c, 0},
	{ 0,			55, 0x80ffff0c, 0},
	{ 0,			56, 0x80ffff0c, 0},
	{ 0,			57, 0x80ffff0c, 0},
	{ 0,			58, 0x80ffff0c, 0},
	{ 0,			59, 0x80ffff0c, 0},
	{ 0,			60, 0x80ffff0c, 0},
	{ 0,			61, 0x80ffff0c, 0},
	{ 0,			62, 0x80ffff0c, 0},
	{ 0,			63, 0x80ffff0c, 0},
	{ 0,			64, 0x80ffff0c, 0},
	{ 0,			65, 0x80ffff0c, 0},
	{ 0,			66, 0x80ffff0c, 0},
	{ 0,			67, 0x80ffff0c, 0},
	{ 0,			68, 0x80ffff0c, 0},
	{ 0,			69, 0x80ffff0c, 0},
	{ 0,			70, 0x80ffff0c, 0},
	{ 0,			71, 0x80ffff0c, 0},
	{ 0,			72, 0x80ffff0c, 0},
	{ 0,			73, 0x80ffff0c, 0},
	{ 0,			74, 0x80ffff0c, 0},
	{ 0,			75, 0x80ffff0c, 0},
	{ 0,			76, 0x80ffff0c, 0},
	{ 0,			77, 0x80ffff0c, 0},
	{ 0,			78, 0x80ffff0c, 0},
	{ 0,			79, 0x80ffff0c, 0},
};

DIDATAFORMAT	VDirectInputDevice::df_Joystick =
{
	sizeof(DIDATAFORMAT),			// this structure
	sizeof(DIOBJECTDATAFORMAT),		// size of object data format
	DIDF_ABSAXIS,					// absolute axis coordinates
	80,								// device data size
	sizeof(VDirectInputDevice::rgodf_Joy) /
		sizeof(VDirectInputDevice::rgodf_Joy[0]),	// number of objects
	VDirectInputDevice::rgodf_Joy,	// and here they are
};

static VCvarI				m_filter("m_filter", "1", CVAR_Archive);

static VDirectInputDevice*	CurrentDevice;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VDirectInputDevice::VDirectInputDevice
//
//==========================================================================

VDirectInputDevice::VDirectInputDevice()
: DInput(NULL)
, lpKeyboard(NULL)
, lpMouse(NULL)
, mousepresent(false)
, old_mouse_x(0)
, old_mouse_y(0)
, old_mouse_z(0)
, lpJoystick(NULL)
, joystick_started(false)
{
	guard(VDirectInputDevice::VDirectInputDevice);
	HRESULT		result;

	result = CoCreateInstance(CLSID_DirectInput, NULL,
		CLSCTX_INPROC_SERVER, IID_IDirectInput, (void**)&DInput);
	if (result != DI_OK)
		Sys_Error("Failed to create DirectInput object");

	result = DInput->Initialize(hInst, DIRECTINPUT_VERSION);
	if (result != DI_OK)
		Sys_Error("Failed to initialize DirectInput object");

	StartupKeyboard();
	StartupMouse();
	StartupJoystick();

	CurrentDevice = this;
	unguard;
}

//==========================================================================
//
//	VDirectInputDevice::~VDirectInputDevice
//
//==========================================================================

VDirectInputDevice::~VDirectInputDevice()
{
	guard(VDirectInputDevice::~VDirectInputDevice);
	ShutdownJoystick();
	ShutdownMouse();
	ShutdownKeyboard();

	if (DInput)
	{
		DInput->Release();
		DInput = NULL; 
	}
	unguard;
}

//==========================================================================
//
//  VDirectInputDevice::ReadInput
//
//	Reads input from the input devices.
//
//==========================================================================

void VDirectInputDevice::ReadInput()
{
	guard(VDirectInputDevice::ReadInput);
	ReadKeyboard();
	ReadMouse();
	ReadJoystick();
	unguard;
}

//==========================================================================
//
//	VDirectInputDevice::SetActiveWindow
//
//==========================================================================

void VDirectInputDevice::SetActiveWindow(HWND window)
{
	guard(VDirectInputDevice::SetActiveWindow);
	if (lpKeyboard)
	{
		lpKeyboard->Unacquire();
		lpKeyboard->SetCooperativeLevel(window, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
		lpKeyboard->Acquire();
	}

	if (mousepresent)
	{
		lpMouse->Unacquire();
		lpMouse->SetCooperativeLevel(window, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
		lpMouse->Acquire();
	}

	if (joystick_started)
	{
		lpJoystick->Unacquire();
		lpJoystick->SetCooperativeLevel(window, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
		lpJoystick->Acquire();
	}
	unguard;
}

//**************************************************************************
//**
//**	KEYBOARD
//**
//**************************************************************************

//==========================================================================
//
//	VDirectInputDevice::StartupKeyboard
//
//	Installs the keyboard handler.
//
//==========================================================================

void VDirectInputDevice::StartupKeyboard()
{
	guard(VDirectInputDevice::StartupKeyboard);
	HRESULT		Result;

	//	Create keyboard device
	Result = DInput->CreateDevice(GUID_SysKeyboard, &lpKeyboard, NULL);
	if (Result != DI_OK)
		Sys_Error("Failed to initialize keyboard");

	//	Set data format
	Result = lpKeyboard->SetDataFormat(&df_Keyboard);
	if (Result != DI_OK)
		Sys_Error("Failed to set keyboard data format");

	//	Set cooperative level
	Result = lpKeyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (Result != DI_OK)
		Sys_Error("Failed to set keyboard cooperative level");

	//	Set buffer size
	DIPROPDWORD		dipdw;
	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = 32;
	Result = lpKeyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);
	if (Result != DI_OK)
		Sys_Error("Failed to set keyboard buffer size");

	//	Acquire device
	lpKeyboard->Acquire();
	unguard;
}

//==========================================================================
//
//  VDirectInputDevice::ReadKeyboard
//
//==========================================================================

void VDirectInputDevice::ReadKeyboard()
{
	guard(VDirectInputDevice::ReadKeyboard);

	// Attempt to get the device data.
	DIDEVICEOBJECTDATA DevData[32];
	DWORD NumItems = 32;
	HRESULT Result = lpKeyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA),
		DevData, &NumItems, 0);

	// Check the return code.
	switch (Result)
	{
	case DI_OK:
		break;

	case DI_BUFFEROVERFLOW:
		GCon->Log(NAME_Dev, "Keyboard buffer overflowed.");
		break;

	case DIERR_INPUTLOST:
	case DIERR_NOTACQUIRED:
		lpKeyboard->Acquire();
		return;

	case E_ACCESSDENIED:
		return;

	default:
		Sys_Error("Failed to read keyboard state");
	}

	// Process the data.
	for (size_t i = 0; i < NumItems; i++)
	{
		GInput->KeyEvent(scan2key[DevData[i].dwOfs], DevData[i].dwData & 0x80);
	}
	unguard;
}

//==========================================================================
//
//  VDirectInputDevice::ShutdownKeyboard
//
// 	Removes the keyboard handler.
//
//==========================================================================

void VDirectInputDevice::ShutdownKeyboard()
{
	guard(VDirectInputDevice::ShutdownKeyboard);
	if (lpKeyboard)
	{
		lpKeyboard->Unacquire();
		lpKeyboard->Release();
		lpKeyboard = NULL;
	}
	unguard;
}

//**************************************************************************
//**
//**	MOUSE
//**
//**************************************************************************

//==========================================================================
//
// 	VDirectInputDevice::StartupMouse
//
//	Initializes mouse
//
//==========================================================================

void VDirectInputDevice::StartupMouse()
{
	guard(VDirectInputDevice::StartupMouse);
	HRESULT		Result;

	if (GArgs.CheckParm("-nomouse"))
		return;

	Result = DInput->CreateDevice(GUID_SysMouse, &lpMouse, NULL);
	if (Result != DI_OK)
		return;

	Result = lpMouse->SetDataFormat(&df_Mouse);
	if (Result != DI_OK)
		return;

	Result = lpMouse->SetCooperativeLevel(hwnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
	if (Result != DI_OK)
		return;

	lpMouse->Acquire();

	mousepresent = true;
	unguard;
}

//==========================================================================
//
//  VDirectInputDevice::ReadMouse
//
// 	Reads mouse.
//
//==========================================================================

void VDirectInputDevice::ReadMouse()
{
	guard(VDirectInputDevice::ReadMouse);

	// Static mouse last button state.
	static byte lastbuttons[4] = {0, 0, 0, 0};

	// If we don't have a mouse then skip.
	if (!mousepresent)
	{
		return;
	}

	// Read the mouse state.
	DIMOUSESTATE MouseState;
	HRESULT Result = lpMouse->GetDeviceState(sizeof(DIMOUSESTATE), &MouseState);

	// Check the return code.
	switch (Result)
	{
	case DI_OK:
		break;

	case DI_BUFFEROVERFLOW:
		GCon->Log(NAME_Dev, "Mouse buffer overflowed.");
		break;

	case DIERR_INPUTLOST:
	case DIERR_NOTACQUIRED:
		lpMouse->Acquire();
		return;

	case E_ACCESSDENIED:
		return;

	default:
		Sys_Error("Failed to read mouse state");
	}

	// Update the mouse x and y positions.
	int xmickeys = MouseState.lX;
	int ymickeys = MouseState.lY;
	int mouse_x;
	int mouse_y;
	if (m_filter == 2)
	{
		mouse_x = (xmickeys + old_mouse_x) / 2;
		mouse_y = (ymickeys + old_mouse_y) / 2;
		old_mouse_x = mouse_x;
		old_mouse_y = mouse_y;
	}
	else if (m_filter == 1)
	{
		mouse_x = (xmickeys + old_mouse_x) / 2;
		mouse_y = (ymickeys + old_mouse_y) / 2;
		old_mouse_x = xmickeys;
		old_mouse_y = ymickeys;
	}
	else
	{
		mouse_x = xmickeys;
		mouse_y = ymickeys;
		old_mouse_x = mouse_x;
		old_mouse_y = mouse_y;
	}

	// Handle mouse move.
	if (mouse_x || mouse_y)
	{
		// Build and post mouse event.
		event_t event;
		event.type  = ev_mouse;
		event.data1 = 0;
		event.data2 = mouse_x;
		event.data3 = -mouse_y;
		GInput->PostEvent(&event);
	}

	// Handle mouse buttons.
	for (int i = 0; i < 3; i++)
	{
		if ((MouseState.rgbButtons[i] ^ lastbuttons[i]) & 0x80)
		{
			GInput->KeyEvent(K_MOUSE1 + i, MouseState.rgbButtons[i] & 0x80);
		}
		lastbuttons[i] = MouseState.rgbButtons[i];
	}

	//	Handle mouse wheel.
	if (MouseState.lZ > 0 || old_mouse_z > 0)
	{
		GInput->KeyEvent(K_MWHEELUP, MouseState.lZ > 0);
	}
	if (MouseState.lZ < 0 || old_mouse_z < 0)
	{
		GInput->KeyEvent(K_MWHEELDOWN, MouseState.lZ < 0);
	}
	old_mouse_z = MouseState.lZ;
	unguard;
}

//==========================================================================
//
//	VDirectInputDevice::ShutdownMouse
//
//==========================================================================

void VDirectInputDevice::ShutdownMouse()
{
	guard(VDirectInputDevice::ShutdownMouse);
	if (lpMouse)
	{
		lpMouse->Unacquire();
		lpMouse->Release();
		lpMouse = NULL;
	}
	mousepresent = false;
	unguard;
}

//**************************************************************************
//**
//**	JOYSTICK
//**
//**************************************************************************

//==========================================================================
//
//	VDirectInputDevice::StartupJoystick
//
// 	Initializes joystick
//
//==========================================================================

void VDirectInputDevice::StartupJoystick()
{
	guard(VDirectInputDevice::StartupJoystick);
  	if (GArgs.CheckParm("-nojoy"))
		return;

	DInput->EnumDevices(DIDEVTYPE_JOYSTICK, JoystickEnumCallback, this,
		DIEDFL_ATTACHEDONLY);
	unguard;
}

//==========================================================================
//
//	JoystickEnumCallback
//
//==========================================================================

int FAR PASCAL VDirectInputDevice::JoystickEnumCallback(
		LPCDIDEVICEINSTANCE pdinst, LPVOID UsrPtr)
{
	return ((VDirectInputDevice*)UsrPtr)->JoystickEnumCallback(pdinst);
}

//==========================================================================
//
//	JoystickEnumCallback
//
//==========================================================================

int VDirectInputDevice::JoystickEnumCallback(LPCDIDEVICEINSTANCE pdinst)
{
	HRESULT					Result;
	LPDIRECTINPUTDEVICE		lpJoystick1;
	DIPROPRANGE				diprg; 
	DIPROPDWORD				dipdw;

	Result = DInput->CreateDevice(pdinst->guidInstance, &lpJoystick1, NULL);
	if (Result != DI_OK)
		return DIENUM_CONTINUE;

	Result = lpJoystick1->QueryInterface(IID_IDirectInputDevice2,(LPVOID *)&lpJoystick);
	IDirectInputDevice_Release(lpJoystick1);
	if (Result != DI_OK)
		return DIENUM_CONTINUE;

	Result = lpJoystick->SetDataFormat(&df_Joystick);
	if (Result != DI_OK)
		return DIENUM_CONTINUE;

	Result = lpJoystick->SetCooperativeLevel(hwnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
	if (Result != DI_OK)
		return DIENUM_CONTINUE;

	diprg.diph.dwSize		= sizeof(diprg);
	diprg.diph.dwHeaderSize	= sizeof(diprg.diph);
	diprg.diph.dwObj		= DIJOFS_X;
	diprg.diph.dwHow		= DIPH_BYOFFSET;
	diprg.lMin				= -1000;
	diprg.lMax				= +1000;

	Result = lpJoystick->SetProperty(DIPROP_RANGE, &diprg.diph);
	if (Result != DI_OK)
		return DIENUM_CONTINUE;

	diprg.diph.dwObj		= DIJOFS_Y;

	Result = lpJoystick->SetProperty(DIPROP_RANGE, &diprg.diph);
	if (Result != DI_OK)
		return DIENUM_CONTINUE;

	dipdw.diph.dwSize		= sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize	= sizeof(dipdw.diph);
	dipdw.diph.dwHow		= DIPH_BYOFFSET;
	dipdw.dwData			= 1500;
	dipdw.diph.dwObj		= DIJOFS_X;

	Result = lpJoystick->SetProperty(DIPROP_DEADZONE, &dipdw.diph);
	if (Result != DI_OK)
		return DIENUM_CONTINUE;

	dipdw.diph.dwObj		= DIJOFS_Y;

	Result = lpJoystick->SetProperty(DIPROP_DEADZONE, &dipdw.diph);
	if (Result != DI_OK)
		return DIENUM_CONTINUE;

	lpJoystick->Acquire();

	joystick_started = true;
	return DIENUM_STOP;
}

//==========================================================================
//
//  VDirectInputDevice::ReadJoystick
//
//==========================================================================

void VDirectInputDevice::ReadJoystick()
{
	guard(VDirectInputDevice::StartupJoystick);

	// Static previous joystick state.
	static int oldx = 0;
	static int oldy = 0;
	static byte oldb[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	// If we don't have a jpystick then skip.
	if (!joystick_started)
	{
		return;
	}

	// Poll joystick.
	lpJoystick->Poll();

	// Try to read joystick state.
	DIJOYSTATE JoyState;
	HRESULT Result = lpJoystick->GetDeviceState(sizeof(DIJOYSTATE), &JoyState);

	// Check the result code.
	switch (Result)
	{
	case DI_OK:
		break;

	case DI_BUFFEROVERFLOW:
		GCon->Log(NAME_Dev, "Joystick buffer overflowed.");
		break;

	case DIERR_INPUTLOST:
	case DIERR_NOTACQUIRED:
		lpJoystick->Acquire();
		return;

	case E_ACCESSDENIED:
		return;

	default:
		Sys_Error("Failed to read joystick state");
	}

	// Handle joystick move.
	if ((oldx != JoyState.lX) || (oldy != JoyState.lY))
	{
		event_t event;
		event.type = ev_joystick;
		event.data1 = 0;
		event.data2 = JoyState.lX;
		event.data3 = JoyState.lY;
		GInput->PostEvent(&event);
	}
	oldx = JoyState.lX;
	oldy = JoyState.lY;

	// Handle joystick buttons.
	for (int i = 0; i < 16; i++)
	{
		if ((JoyState.rgbButtons[i] ^ oldb[i]) & 0x80)
		{
			GInput->KeyEvent(K_JOY1 + i, JoyState.rgbButtons[i] & 0x80);
		}
		oldb[i] = JoyState.rgbButtons[i];
	}
	unguard;
}

//==========================================================================
//
//	VDirectInputDevice::ShutdownJoystick
//
//==========================================================================

void VDirectInputDevice::ShutdownJoystick()
{
	guard(VDirectInputDevice::ShutdownJoystick);
	if (lpJoystick)
	{
		lpJoystick->Unacquire();
		lpJoystick->Release();
		lpJoystick = NULL;
	}
	joystick_started = false;
	unguard;
}

//**************************************************************************
//**
//** 	INPUT
//**
//**************************************************************************

//==========================================================================
//
//  VInputDevice::CreateDevice
//
//==========================================================================

VInputDevice* VInputDevice::CreateDevice()
{
	return new VDirectInputDevice();
}

//==========================================================================
//
//	IN_SetActiveWindow
//
//==========================================================================

void IN_SetActiveWindow(HWND window)
{
	CurrentDevice->SetActiveWindow(window);
}

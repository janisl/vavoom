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
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TLauncherForm *LauncherForm;
//---------------------------------------------------------------------------
__fastcall TLauncherForm::TLauncherForm(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TLauncherForm::FormCreate(TObject *Sender)
{
	Reg = new TRegistry;
	Reg->OpenKey("\\Software\\JL\\vlaunch", true);
	try
	{
		Game->ItemIndex = Reg->ReadInteger("Game");
		RendererBox->ItemIndex = Reg->ReadInteger("Renderer");
		CheckBoxNoSound->Checked = Reg->ReadInteger("NoSound");
		CheckBoxNoSfx->Checked = Reg->ReadInteger("NoSfx");
		CheckBoxNo3DSound->Checked = Reg->ReadInteger("No3DSound");
		CheckBoxNoMusic->Checked = Reg->ReadInteger("NoMusic");
		CheckBoxNoCDAudio->Checked = Reg->ReadInteger("NoCDAudio");
		CheckBoxNoLan->Checked = Reg->ReadInteger("NoLAN");
		CheckBoxNoUDP->Checked = Reg->ReadInteger("NoUDP");
		CheckBoxNoIPX->Checked = Reg->ReadInteger("NoIPX");
		EditIPAddress->Text = Reg->ReadString("IPAddress");
		EditPort->Text = Reg->ReadString("Port");
		CheckBoxNoMouse->Checked = Reg->ReadInteger("NoMouse");
		CheckBoxNoJoy->Checked = Reg->ReadInteger("NoJoy");
		CheckBoxDebug->Checked = Reg->ReadInteger("Debug");
		EditFiles->Text = Reg->ReadString("Files");
		EditProgs->Text = Reg->ReadString("Progs");
		EditMisc->Text = Reg->ReadString("Options");
	}
	catch (...)
	{
		if (Game->ItemIndex < 0)
			Game->ItemIndex = 0;
		if (RendererBox->ItemIndex < 0)
			RendererBox->ItemIndex = 0;
	}
}
//---------------------------------------------------------------------------
void __fastcall TLauncherForm::FormDestroy(TObject *Sender)
{
	Reg->WriteInteger("Game", Game->ItemIndex);
	Reg->WriteInteger("Renderer", RendererBox->ItemIndex);
	Reg->WriteInteger("NoSound", CheckBoxNoSound->Checked);
	Reg->WriteInteger("NoSfx", CheckBoxNoSfx->Checked);
	Reg->WriteInteger("No3DSound", CheckBoxNo3DSound->Checked);
	Reg->WriteInteger("NoMusic", CheckBoxNoMusic->Checked);
	Reg->WriteInteger("NoCDAudio", CheckBoxNoCDAudio->Checked);
	Reg->WriteInteger("NoLAN", CheckBoxNoLan->Checked);
	Reg->WriteInteger("NoUDP", CheckBoxNoUDP->Checked);
	Reg->WriteInteger("NoIPX", CheckBoxNoIPX->Checked);
	Reg->WriteString("IPAddress", EditIPAddress->Text);
	Reg->WriteString("Port", EditPort->Text);
	Reg->WriteInteger("NoMouse", CheckBoxNoMouse->Checked);
	Reg->WriteInteger("NoJoy", CheckBoxNoJoy->Checked);
	Reg->WriteInteger("Debug", CheckBoxDebug->Checked);
	Reg->WriteString("Files", EditFiles->Text);
	Reg->WriteString("Progs", EditProgs->Text);
	Reg->WriteString("Options", EditMisc->Text);
	delete Reg;
}
//---------------------------------------------------------------------------
void __fastcall TLauncherForm::RunButtonClick(TObject *Sender)
{
	STARTUPINFO			StartInfo;
	PROCESS_INFORMATION	ProcInfo;
	char				CmdLine[1024];
	int					len;

	//	Create command line
#define CAT(opt)	strcat(CmdLine, " "opt)
	strcpy(CmdLine, "Vavoom95");
	switch (Game->ItemIndex)
	{
	 case 1:
		CAT("-doom");
		break;
	 case 2:
		CAT("-doom2");
		break;
	 case 3:
		CAT("-heretic");
		break;
	 case 4:
		CAT("-hexen");
		break;
	 case 5:
		CAT("-strife");
		break;
	}
	switch (RendererBox->ItemIndex)
	{
	 case 1:
		CAT("-opengl");
		break;
	 case 2:
		CAT("-d3d");
		break;
	}
	// Sound
	if (CheckBoxNoSound->Checked)	CAT("-nosound");
	if (CheckBoxNoSfx->Checked)		CAT("-nosfx");
	if (CheckBoxNo3DSound->Checked)	CAT("-no3dsound");
	if (CheckBoxNoMusic->Checked)	CAT("-nomusic");
	if (CheckBoxNoCDAudio->Checked)	CAT("-nocdaudio");
	// Network
	if (CheckBoxNoLan->Checked)		CAT("-nolan");
	if (CheckBoxNoUDP->Checked)		CAT("-noudp");
	if (CheckBoxNoIPX->Checked)		CAT("-noipx");
	len = EditIPAddress->GetTextLen();
	if (len)
	{
		len++;
		char *buf = new char[len + 5];
		strcpy(buf, " -ip ");
		EditIPAddress->GetTextBuf(buf + 5, len);
		strcat(CmdLine, buf);
		delete buf;
	}
	len = EditPort->GetTextLen();
	if (len)
	{
		len++;
		char *buf = new char[len + 7];
		strcpy(buf, " -port ");
		EditPort->GetTextBuf(buf + 7, len);
		strcat(CmdLine, buf);
		delete buf;
	}
	// Input
	if (CheckBoxNoMouse->Checked)	CAT("-nomouse");
	if (CheckBoxNoJoy->Checked)		CAT("-nojoy");
	// Misc
	if (CheckBoxDebug->Checked)		CAT("-debug");
	len = EditFiles->GetTextLen();
	if (len)
	{
		len++;
		char *buf = new char[len + 7];
		strcpy(buf, " -file ");
		EditFiles->GetTextBuf(buf + 7, len);
		strcat(CmdLine, buf);
		delete buf;
	}
	len = EditProgs->GetTextLen();
	if (len)
	{
		len++;
		char *buf = new char[len + 8];
		strcpy(buf, " -progs ");
		EditProgs->GetTextBuf(buf + 8, len);
		strcat(CmdLine, buf);
		delete buf;
	}
	len = EditMisc->GetTextLen();
	if (len)
	{
		len++;
		char *buf = new char[len + 1];
		strcpy(buf, " ");
		EditMisc->GetTextBuf(buf + 1, len);
		strcat(CmdLine, buf);
		delete buf;
	}
#undef CAT

	//	Hide cursor
	ShowCursor(false);

	//	Run game
	memset(&StartInfo, 0, sizeof(StartInfo));
	memset(&ProcInfo, 0 , sizeof(ProcInfo));
	StartInfo.cb = sizeof(StartInfo);
	if (CreateProcess(
			NULL, // pointer to name of executable module
			CmdLine, // pointer to command line string
			NULL, // pointer to process security attributes
			NULL, // pointer to thread security attributes
			false, // handle inheritance flag
			0, // creation flags
			NULL, // pointer to new environment block
			NULL, // pointer to current directory name
			&StartInfo, // pointer to STARTUPINFO
			&ProcInfo // pointer to PROCESS_INFORMATION
		))
	{
		WaitForSingleObject(ProcInfo.hProcess, INFINITE);
	}

	//	Show cursor
	ShowCursor(true);
}
//---------------------------------------------------------------------------
void __fastcall TLauncherForm::ExitButtonClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
//**************************************************************************
//
//	$Log$
//	Revision 1.4  2001/10/18 17:42:53  dj_jl
//	Fixed show window command
//
//	Revision 1.3  2001/09/24 17:30:40  dj_jl
//	Beautification
//	
//	Revision 1.2  2001/07/27 14:27:56  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************

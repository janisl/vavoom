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
//**	Copyright (C) 1999-2002 JÆnis Legzdi·ý
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
#ifndef mainH
#define mainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Graphics.hpp>
#include <jpeg.hpp>
#include <registry.hpp>
//---------------------------------------------------------------------------
class TLauncherForm : public TForm
{
__published:	// IDE-managed Components
	TButton *RunButton;
	TComboBox *Game;
	TLabel *Label1;
	TButton *ExitButton;
	TLabel *Label2;
	TComboBox *RendererBox;
	TGroupBox *GroupBox1;
	TCheckBox *CheckBoxNoSound;
	TCheckBox *CheckBoxNoSfx;
	TCheckBox *CheckBoxNo3DSound;
	TCheckBox *CheckBoxNoMusic;
	TCheckBox *CheckBoxNoCDAudio;
	TGroupBox *GroupBox2;
	TCheckBox *CheckBoxNoMouse;
	TCheckBox *CheckBoxNoJoy;
	TGroupBox *GroupBox3;
	TCheckBox *CheckBoxNoLan;
	TCheckBox *CheckBoxNoUDP;
	TCheckBox *CheckBoxNoIPX;
	TEdit *EditIPAddress;
	TLabel *Label3;
	TLabel *Label4;
	TEdit *EditPort;
	TGroupBox *GroupBox4;
	TCheckBox *CheckBoxDebug;
	TImage *Image1;
	TLabel *Label5;
	TEdit *EditFiles;
	TLabel *Label6;
	TEdit *EditMisc;
	TLabel *Label7;
	TEdit *EditProgs;
	TCheckBox *CheckBoxUseOpenAL;
	TLabel *Label8;
	TEdit *EditGame;
	TCheckBox *CheckBoxDevGame;
	void __fastcall RunButtonClick(TObject *Sender);
	void __fastcall ExitButtonClick(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall FormDestroy(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TLauncherForm(TComponent* Owner);
	TRegistry	*Reg;
};
//---------------------------------------------------------------------------
extern PACKAGE TLauncherForm *LauncherForm;
//---------------------------------------------------------------------------
#endif
//**************************************************************************
//
//	$Log$
//	Revision 1.5  2002/08/05 17:24:54  dj_jl
//	Updated with new stuff.
//
//	Revision 1.4  2002/01/07 12:31:36  dj_jl
//	Changed copyright year
//	
//	Revision 1.3  2001/09/24 17:30:40  dj_jl
//	Beautification
//	
//	Revision 1.2  2001/07/27 14:27:56  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************

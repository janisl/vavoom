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

#if 0
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
		CheckBoxUseOpenAL->Checked = Reg->ReadInteger("UseOpenAL");
		EditGame->Text = Reg->ReadString("CustomGame");
		CheckBoxDevGame->Checked = Reg->ReadInteger("DevGame");
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
	Reg->WriteInteger("UseOpenAL", CheckBoxUseOpenAL->Checked);
	Reg->WriteInteger("NoLAN", CheckBoxNoLan->Checked);
	Reg->WriteInteger("NoUDP", CheckBoxNoUDP->Checked);
	Reg->WriteInteger("NoIPX", CheckBoxNoIPX->Checked);
	Reg->WriteString("IPAddress", EditIPAddress->Text);
	Reg->WriteString("Port", EditPort->Text);
	Reg->WriteInteger("NoMouse", CheckBoxNoMouse->Checked);
	Reg->WriteInteger("NoJoy", CheckBoxNoJoy->Checked);
	Reg->WriteInteger("Debug", CheckBoxDebug->Checked);
	Reg->WriteString("CustomGame", EditGame->Text);
	Reg->WriteInteger("DevGame", CheckBoxDevGame->Checked);
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
		CAT("-tnt");
		break;
	 case 4:
		CAT("-plutonia");
		break;
	 case 5:
		CAT("-heretic");
		break;
	 case 6:
		CAT("-hexen");
		break;
	 case 7:
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
	if (CheckBoxUseOpenAL->Checked)	CAT("-openal");
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
	len = EditGame->GetTextLen();
	if (len)
	{
		len++;
		if (CheckBoxDevGame->Checked)
		{
			char *buf = new char[len + 10];
			strcpy(buf, " -devgame ");
			EditGame->GetTextBuf(buf + 10, len);
			strcat(CmdLine, buf);
			delete buf;
		}
		else
		{
			char *buf = new char[len + 7];
			strcpy(buf, " -game ");
			EditGame->GetTextBuf(buf + 7, len);
			strcat(CmdLine, buf);
			delete buf;
		}
	}
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
#endif

/////////////////////////////////////////////////////////////////////////////
// Name:        minimal.cpp
// Purpose:     Minimal wxWidgets sample
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// RCS-ID:      $Id$
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------
 
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
 
#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

// ----------------------------------------------------------------------------
// resources
// ----------------------------------------------------------------------------

// the application icon (under Windows and OS/2 it is in resources and even
// though we could still include the XPM here it would be unused)
#if !defined(__WXMSW__) && !defined(__WXPM__)
    #include "vlaunch.xpm"
#endif

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

// Define a new application type, each program should derive a class from wxApp
class MyApp : public wxApp
{
public:
    // override base class virtuals
    // ----------------------------

    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    virtual bool OnInit();
};

// Define a new frame type: this is going to be our main frame
class MyFrame : public wxFrame
{
public:
    // ctor(s)
    MyFrame();

    // event handlers (these functions should _not_ be virtual)
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

private:
    // any class wishing to process wxWidgets events must use this macro
    DECLARE_EVENT_TABLE()
};

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// IDs for the controls and the menu commands
enum
{
    // menu items
    Minimal_Quit = wxID_EXIT,

    // it is important for the id corresponding to the "About" command to have
    // this standard value as otherwise it won't be handled properly under Mac
    // (where it is special and put into the "Apple" menu)
    Minimal_About = wxID_ABOUT
};

// ----------------------------------------------------------------------------
// event tables and other macros for wxWidgets
// ----------------------------------------------------------------------------

// the event tables connect the wxWidgets events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(Minimal_Quit,  MyFrame::OnQuit)
    EVT_MENU(Minimal_About, MyFrame::OnAbout)
END_EVENT_TABLE()

// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)
IMPLEMENT_APP(MyApp)

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

// 'Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit()
{
    // call the base class initialization method, currently it only parses a
    // few common command-line options but it could be do more in the future
    if ( !wxApp::OnInit() )
        return false;

    // create the main application window
    MyFrame *frame = new MyFrame();

    // and show it (the frames, unlike simple controls, are not shown when
    // created initially)
    frame->Show(true);

    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
    return true;
}

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// frame constructor
MyFrame::MyFrame()
: wxFrame(NULL, wxID_ANY, _T("Vavoom launcher"), wxDefaultPosition, wxSize(400, 300),
	wxDEFAULT_FRAME_STYLE & ~ (wxRESIZE_BORDER | wxRESIZE_BOX | wxMAXIMIZE_BOX))
{
    // set the frame icon
    SetIcon(wxICON(mainicon));

    CreateStatusBar();

    wxPanel* panel = new wxPanel(this);

	new wxCheckBox(panel, -1, _T("Test label"), wxPoint(20, 50));
}


// event handlers

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    // true is to force the frame to close
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox(wxString::Format(
                    _T("Welcome to %s!\n")
                    _T("\n")
                    _T("This is the minimal wxWidgets sample\n")
                    _T("running under %s."),
                    wxVERSION_STRING,
                    wxGetOsDescription().c_str()
                 ),
                 _T("About wxWidgets minimal sample"),
                 wxOK | wxICON_INFORMATION,
                 this);
}

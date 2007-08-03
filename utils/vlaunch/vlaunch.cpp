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
//**	Copyright (C) 2007 Jānis Legzdiņš
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

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include "wx/config.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

enum
{
	VLaunch_Run = wxID_HIGHEST,
	VLaunch_Exit = wxID_EXIT,
};

//
//	Main frame of the launcher.
//
class VMain : public wxFrame
{
public:
	wxComboBox*		Game;
	wxComboBox*		RendererBox;
	wxCheckBox*		CheckBoxNoSound;
	wxCheckBox*		CheckBoxNoSfx;
	wxCheckBox*		CheckBoxNo3DSound;
	wxCheckBox*		CheckBoxNoMusic;
	wxCheckBox*		CheckBoxNoCDAudio;
	wxCheckBox*		CheckBoxUseOpenAL;
	wxCheckBox*		CheckBoxNoLan;
	wxCheckBox*		CheckBoxNoUDP;
	wxCheckBox*		CheckBoxNoIPX;
	wxTextCtrl*		EditIPAddress;
	wxTextCtrl*		EditPort;
	wxCheckBox*		CheckBoxNoMouse;
	wxCheckBox*		CheckBoxNoJoy;
	wxCheckBox*		CheckBoxDebug;
	wxTextCtrl*		EditGame;
	wxCheckBox*		CheckBoxDevGame;
	wxTextCtrl*		EditFiles;
	wxTextCtrl*		EditProgs;
	wxTextCtrl*		EditMisc;

    VMain();
	~VMain();

    void OnRun(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);

private:
    DECLARE_EVENT_TABLE()
};

//
//	Launcher application class.
//
class VLaunchApp : public wxApp
{
public:
    virtual bool OnInit();
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

BEGIN_EVENT_TABLE(VMain, wxFrame)
    EVT_BUTTON(VLaunch_Run,  VMain::OnRun)
    EVT_BUTTON(VLaunch_Exit,  VMain::OnExit)
END_EVENT_TABLE()

IMPLEMENT_APP(VLaunchApp)

#include "vavoom.xpm"

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VMain::VMain
//
//==========================================================================

VMain::VMain()
: wxFrame(NULL, wxID_ANY, wxT("Vavoom launcher"), wxDefaultPosition, wxDefaultSize,
	wxDEFAULT_FRAME_STYLE & ~ (wxRESIZE_BORDER | wxRESIZE_BOX | wxMAXIMIZE_BOX))
{
	SetClientSize(447, 314);
	wxPanel* panel = new wxPanel(this);

	new wxStaticBitmap(panel, -1, wxBitmap(vavoom_xpm), wxPoint(0, 0), wxSize(447, 105));

	new wxStaticText(panel, -1, wxT("Game:"), wxPoint(24, 240), wxSize(31, 13));
	wxString GameChoices[8];
	GameChoices[0] = wxT("(Autodetect)");
	GameChoices[1] = wxT("Doom");
	GameChoices[2] = wxT("Doom 2: Hell On Earth");
	GameChoices[3] = wxT("Doom 2: TNT Evilution");
	GameChoices[4] = wxT("Doom 2: The Plutonia Experiment");
	GameChoices[5] = wxT("Heretic");
	GameChoices[6] = wxT("Hexen");
	GameChoices[7] = wxT("Strife");
	Game = new wxComboBox(panel, -1, GameChoices[0], wxPoint(56, 240), wxSize(97, 21), 8, GameChoices, wxCB_READONLY);

	new wxStaticText(panel, -1, wxT("Renderer:"), wxPoint(8, 264), wxSize(47, 13));
	wxString RendChoices[3];
	RendChoices[0] = wxT("Software");
	RendChoices[1] = wxT("OpenGL");
	RendChoices[2] = wxT("Direct3D");
	RendererBox = new wxComboBox(panel, -1, RendChoices[0], wxPoint(56, 264), wxSize(97, 21), 3, RendChoices, wxCB_READONLY);

	wxPanel* grp = new wxPanel(panel, 8, 112, 129, 121);
	new wxStaticBox(grp, -1, wxT("Sound"), wxPoint(0, 0), wxSize(129, 121));
	CheckBoxNoSound = new wxCheckBox(grp, -1, wxT("Disable all sound"), wxPoint(8, 16), wxSize(105, 17));
	CheckBoxNoSfx = new wxCheckBox(grp, -1, wxT("No sounds"), wxPoint(8, 32), wxSize(105, 17));
	CheckBoxNo3DSound = new wxCheckBox(grp, -1, wxT("No 3D sound"), wxPoint(8, 48), wxSize(105, 17));
	CheckBoxNoMusic = new wxCheckBox(grp, -1, wxT("No music"), wxPoint(8, 64), wxSize(105, 17));
	CheckBoxNoCDAudio = new wxCheckBox(grp, -1, wxT("No CD audio"), wxPoint(8, 80), wxSize(105, 17));
	CheckBoxUseOpenAL = new wxCheckBox(grp, -1, wxT("Use OpenAL"), wxPoint(8, 96), wxSize(97, 17));

	grp = new wxPanel(panel, 144, 112, 161, 105);
	new wxStaticBox(grp, -1, wxT("Network"), wxPoint(0, 0), wxSize(161, 105));
	CheckBoxNoLan = new wxCheckBox(grp, -1, wxT("Disable all LAN drivers"), wxPoint(8, 16), wxSize(129, 17));
	CheckBoxNoUDP = new wxCheckBox(grp, -1, wxT("Disable TCP/IP driver"), wxPoint(8, 32), wxSize(129, 17));
	CheckBoxNoIPX = new wxCheckBox(grp, -1, wxT("Disable IPX driver"), wxPoint(8, 48), wxSize(129, 17));
	new wxStaticText(grp, -1, wxT("IP address:"), wxPoint(8, 64), wxSize(53, 13));
	EditIPAddress = new wxTextCtrl(grp, -1, wxT(""), wxPoint(8, 80), wxSize(97, 21));
	new wxStaticText(grp, -1, wxT("Port:"), wxPoint(112, 64), wxSize(22, 13));
	EditPort = new wxTextCtrl(grp, -1, wxT(""), wxPoint(112, 80), wxSize(41, 21));

	grp = new wxPanel(panel, 312, 112, 129, 57);
	new wxStaticBox(grp, -1, wxT("Input"), wxPoint(0, 0), wxSize(129, 57));
	CheckBoxNoMouse = new wxCheckBox(grp, -1, wxT("Disable mouse"), wxPoint(8, 16), wxSize(97, 17));
	CheckBoxNoJoy = new wxCheckBox(grp, -1, wxT("Disable joystick"), wxPoint(8, 32), wxSize(97, 17));

	grp = new wxPanel(panel, 312, 176, 129, 41);
	new wxStaticBox(grp, -1, wxT("Misc"), wxPoint(0, 0), wxSize(129, 41));
	CheckBoxDebug = new wxCheckBox(grp, -1, wxT("Create debug file"), wxPoint(8, 16), wxSize(105, 17));

	new wxStaticText(panel, -1, wxT("Custom game:"), wxPoint(152, 224), wxSize(70, 13));
	EditGame = new wxTextCtrl(panel, -1, wxT(""), wxPoint(224, 224), wxSize(97, 21));
	CheckBoxDevGame = new wxCheckBox(panel, -1, wxT("Development mode"), wxPoint(328, 224), wxSize(113, 17));
	new wxStaticText(panel, -1, wxT("Files:"), wxPoint(160, 248), wxSize(24, 13));
	EditFiles = new wxTextCtrl(panel, -1, wxT(""), wxPoint(184, 248), wxSize(185, 21));
	new wxStaticText(panel, -1, wxT("Progs:"), wxPoint(8, 288), wxSize(30, 13));
	EditProgs = new wxTextCtrl(panel, -1, wxT(""), wxPoint(40, 288), wxSize(113, 21));
	new wxStaticText(panel, -1, wxT("Other options:"), wxPoint(160, 272), wxSize(66, 13));
	EditMisc = new wxTextCtrl(panel, -1, wxT(""), wxPoint(160, 288), wxSize(209, 21));

	new wxButton(panel, VLaunch_Run, wxT("Run"), wxPoint(376, 248), wxSize(65, 25));
	new wxButton(panel, VLaunch_Exit, wxT("Exit"), wxPoint(376, 280), wxSize(65, 25));

	//	Load saved settings.
	wxConfigBase* Conf = wxConfigBase::Get();
	Game->SetSelection(Conf->Read(wxT("Game"), 0l));
	RendererBox->SetSelection(Conf->Read(wxT("Renderer"), 0l));
	CheckBoxNoSound->SetValue(!!Conf->Read(wxT("NoSound"), 0l));
	CheckBoxNoSfx->SetValue(!!Conf->Read(wxT("NoSfx"), 0l));
	CheckBoxNo3DSound->SetValue(!!Conf->Read(wxT("No3DSound"), 0l));
	CheckBoxNoMusic->SetValue(!!Conf->Read(wxT("NoMusic"), 0l));
	CheckBoxNoCDAudio->SetValue(!!Conf->Read(wxT("NoCDAudio"), 0l));
	CheckBoxUseOpenAL->SetValue(!!Conf->Read(wxT("UseOpenAL"), 0l));
	CheckBoxNoLan->SetValue(!!Conf->Read(wxT("NoLAN"), 0l));
	CheckBoxNoUDP->SetValue(!!Conf->Read(wxT("NoUDP"), 0l));
	CheckBoxNoIPX->SetValue(!!Conf->Read(wxT("NoIPX"), 0l));
	EditIPAddress->SetValue(Conf->Read(wxT("IPAddress"), wxT("")));
	EditPort->SetValue(Conf->Read(wxT("Port"), wxT("")));
	CheckBoxNoMouse->SetValue(!!Conf->Read(wxT("NoMouse"), 0l));
	CheckBoxNoJoy->SetValue(!!Conf->Read(wxT("NoJoy"), 0l));
	CheckBoxDebug->SetValue(!!Conf->Read(wxT("Debug"), 0l));
	EditGame->SetValue(Conf->Read(wxT("CustomGame"), wxT("")));
	CheckBoxDevGame->SetValue(!!Conf->Read(wxT("DevGame"), 0l));
	EditFiles->SetValue(Conf->Read(wxT("Files"), wxT("")));
	EditProgs->SetValue(Conf->Read(wxT("Progs"), wxT("")));
	EditMisc->SetValue(Conf->Read(wxT("Options"), wxT("")));
}

//==========================================================================
//
//	VMain::~VMain
//
//==========================================================================

VMain::~VMain()
{
	//	Save settings.
	wxConfigBase* Conf = wxConfigBase::Get();
	Conf->Write(wxT("Game"), Game->GetSelection());
	Conf->Write(wxT("Renderer"), RendererBox->GetSelection());
	Conf->Write(wxT("NoSound"), CheckBoxNoSound->IsChecked());
	Conf->Write(wxT("NoSfx"), CheckBoxNoSfx->IsChecked());
	Conf->Write(wxT("No3DSound"), CheckBoxNo3DSound->IsChecked());
	Conf->Write(wxT("NoMusic"), CheckBoxNoMusic->IsChecked());
	Conf->Write(wxT("NoCDAudio"), CheckBoxNoCDAudio->IsChecked());
	Conf->Write(wxT("UseOpenAL"), CheckBoxUseOpenAL->IsChecked());
	Conf->Write(wxT("NoLAN"), CheckBoxNoLan->IsChecked());
	Conf->Write(wxT("NoUDP"), CheckBoxNoUDP->IsChecked());
	Conf->Write(wxT("NoIPX"), CheckBoxNoIPX->IsChecked());
	Conf->Write(wxT("IPAddress"), EditIPAddress->GetValue());
	Conf->Write(wxT("Port"), EditPort->GetValue());
	Conf->Write(wxT("NoMouse"), CheckBoxNoMouse->IsChecked());
	Conf->Write(wxT("NoJoy"), CheckBoxNoJoy->IsChecked());
	Conf->Write(wxT("Debug"), CheckBoxDebug->IsChecked());
	Conf->Write(wxT("CustomGame"), EditGame->GetValue());
	Conf->Write(wxT("DevGame"), CheckBoxDevGame->IsChecked());
	Conf->Write(wxT("Files"), EditFiles->GetValue());
	Conf->Write(wxT("Progs"), EditProgs->GetValue());
	Conf->Write(wxT("Options"), EditMisc->GetValue());
}

//==========================================================================
//
//	VMain::OnRun
//
//==========================================================================

void VMain::OnRun(wxCommandEvent&)
{
	//	Create command line
#ifdef _WIN32
	wxString CmdLine = wxT("Vavoom95");
#else
	wxString CmdLine = wxT("vavoom");
#endif

	switch (Game->GetSelection())
	{
	case 1:
		CmdLine += wxT(" -doom");
		break;
	case 2:
		CmdLine += wxT(" -doom2");
		break;
	case 3:
		CmdLine += wxT(" -tnt");
		break;
	case 4:
		CmdLine += wxT(" -plutonia");
		break;
	case 5:
		CmdLine += wxT(" -heretic");
		break;
	case 6:
		CmdLine += wxT(" -hexen");
		break;
	case 7:
		CmdLine += wxT(" -strife");
		break;
	}

	switch (RendererBox->GetSelection())
	{
	case 1:
		CmdLine += wxT(" -opengl");
		break;
	case 2:
		CmdLine += wxT(" -d3d");
		break;
	}

	// Sound
	if (CheckBoxNoSound->IsChecked())
		CmdLine += wxT(" -nosound");
	if (CheckBoxNoSfx->IsChecked())
		CmdLine += wxT(" -nosfx");
	if (CheckBoxNo3DSound->IsChecked())
		CmdLine += wxT(" -no3dsound");
	if (CheckBoxNoMusic->IsChecked())
		CmdLine += wxT(" -nomusic");
	if (CheckBoxNoCDAudio->IsChecked())
		CmdLine += wxT(" -nocdaudio");
	if (CheckBoxUseOpenAL->IsChecked())
		CmdLine += wxT(" -openal");

	// Network
	if (CheckBoxNoLan->IsChecked())
		CmdLine += wxT(" -nolan");
	if (CheckBoxNoUDP->IsChecked())
		CmdLine += wxT(" -noudp");
	if (CheckBoxNoIPX->IsChecked())
		CmdLine += wxT(" -noipx");
	if (EditIPAddress->GetValue().Length())
		CmdLine += wxT(" -ip ") + EditIPAddress->GetValue();
	if (EditPort->GetValue().Length())
		CmdLine += wxT(" -port ") + EditPort->GetValue();

	// Input
	if (CheckBoxNoMouse->IsChecked())
		CmdLine += wxT(" -nomouse");
	if (CheckBoxNoJoy->IsChecked())
		CmdLine += wxT(" -nojoy");

	// Misc
	if (CheckBoxDebug->IsChecked())
		CmdLine += wxT(" -debug");
	if (EditGame->GetValue().Length())
	{
		if (CheckBoxDevGame->IsChecked())
			CmdLine += wxT(" -devgame ");
		else
			CmdLine += wxT(" -game ");
		CmdLine += EditGame->GetValue();
	}
	if (EditFiles->GetValue().Length())
		CmdLine += wxT(" -file ") + EditFiles->GetValue();
	if (EditProgs->GetValue().Length())
		CmdLine += wxT(" -progs ") + EditProgs->GetValue();
	if (EditMisc->GetValue().Length())
		CmdLine += wxT(" ") + EditMisc->GetValue();

	//	Run game
	wxExecute(CmdLine, wxEXEC_SYNC);
}

//==========================================================================
//
//	VMain::OnExit
//
//==========================================================================

void VMain::OnExit(wxCommandEvent&)
{
	Close(true);
}

//==========================================================================
//
//	VLaunchApp::OnInit
//
//==========================================================================

bool VLaunchApp::OnInit()
{
	if (!wxApp::OnInit())
		return false;

	//	Needed for config
	SetVendorName(wxT("JL"));

	//	Create our main frame object and show it.
	VMain* frame = new VMain();
	frame->Show(true);
	return true;
}

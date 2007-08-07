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
#include "wx/notebook.h"

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
	wxPanel* panel = new wxPanel(this);
	wxBoxSizer* mainsizer = new wxBoxSizer(wxVERTICAL);

	mainsizer->Add(new wxStaticBitmap(panel, -1, wxBitmap(vavoom_xpm)));

	wxNotebook* nbook = new wxNotebook(panel, -1, wxPoint(0, 105), wxSize(447, 270));
	mainsizer->Add(nbook);

	wxPanel* page = new wxPanel(nbook);
	nbook->AddPage(page, wxT("Main"));
	wxFlexGridSizer* gsizer = new wxFlexGridSizer(2);

	gsizer->Add(new wxStaticText(page, -1, wxT("Game:")), 0, wxALL, 4);
	wxString GameChoices[8];
	GameChoices[0] = wxT("(Autodetect)");
	GameChoices[1] = wxT("Doom");
	GameChoices[2] = wxT("Doom 2: Hell On Earth");
	GameChoices[3] = wxT("Doom 2: TNT Evilution");
	GameChoices[4] = wxT("Doom 2: The Plutonia Experiment");
	GameChoices[5] = wxT("Heretic");
	GameChoices[6] = wxT("Hexen");
	GameChoices[7] = wxT("Strife");
	Game = new wxComboBox(page, -1, GameChoices[0], wxDefaultPosition, wxDefaultSize, 8, GameChoices, wxCB_READONLY);
	gsizer->Add(Game, 0, wxALL, 4);

	gsizer->Add(new wxStaticText(page, -1, wxT("Renderer:")), 0, wxALL, 4);
	wxString RendChoices[3];
	RendChoices[0] = wxT("Software");
	RendChoices[1] = wxT("OpenGL");
	RendChoices[2] = wxT("Direct3D");
	RendererBox = new wxComboBox(page, -1, RendChoices[0], wxDefaultPosition, wxDefaultSize, 3, RendChoices, wxCB_READONLY);
	gsizer->Add(RendererBox, 0, wxALL, 4);

	gsizer->Add(new wxStaticText(page, -1, wxT("Custom game:")), 0, wxALL, 4);
	EditGame = new wxTextCtrl(page, -1, wxT(""), wxDefaultPosition, wxSize(128, -1));
	gsizer->Add(EditGame, 0, wxALL, 4);
	CheckBoxDevGame = new wxCheckBox(page, -1, wxT("Development mode"));
	gsizer->AddSpacer(1);
	gsizer->Add(CheckBoxDevGame, 0, wxALL, 4);
	gsizer->Add(new wxStaticText(page, -1, wxT("Files:")), 0, wxALL, 4);
	EditFiles = new wxTextCtrl(page, -1, wxT(""), wxDefaultPosition, wxSize(200, -1));
	gsizer->Add(EditFiles, 0, wxALL, 4);
	CheckBoxDebug = new wxCheckBox(page, -1, wxT("Create debug file"));
	gsizer->AddSpacer(1);
	gsizer->Add(CheckBoxDebug, 0, wxALL, 4);
	gsizer->Add(new wxStaticText(page, -1, wxT("Other options:")), 0, wxALL, 4);
	EditMisc = new wxTextCtrl(page, -1, wxT(""), wxDefaultPosition, wxSize(209, -1));
	gsizer->Add(EditMisc, 0, wxALL, 4);
	page->SetSizer(gsizer);
	gsizer->Layout();

	//	Sound options
	page = new wxPanel(nbook);
	nbook->AddPage(page, wxT("Sound"));
	wxBoxSizer* bsizer = new wxBoxSizer(wxVERTICAL);
	CheckBoxNoSound = new wxCheckBox(page, -1, wxT("Disable all sound"));
	bsizer->Add(CheckBoxNoSound, 0, wxALL, 4);
	CheckBoxNoSfx = new wxCheckBox(page, -1, wxT("No sounds"));
	bsizer->Add(CheckBoxNoSfx, 0, wxALL, 4);
	CheckBoxNo3DSound = new wxCheckBox(page, -1, wxT("No 3D sound"));
	bsizer->Add(CheckBoxNo3DSound, 0, wxALL, 4);
	CheckBoxNoMusic = new wxCheckBox(page, -1, wxT("No music"));
	bsizer->Add(CheckBoxNoMusic, 0, wxALL, 4);
	CheckBoxNoCDAudio = new wxCheckBox(page, -1, wxT("No CD audio"));
	bsizer->Add(CheckBoxNoCDAudio, 0, wxALL, 4);
	CheckBoxUseOpenAL = new wxCheckBox(page, -1, wxT("Use OpenAL"));
	bsizer->Add(CheckBoxUseOpenAL, 0, wxALL, 4);
	page->SetSizer(bsizer);
	bsizer->Layout();

	//	Input options
	page = new wxPanel(nbook);
	nbook->AddPage(page, wxT("Input"));
	bsizer = new wxBoxSizer(wxVERTICAL);
	CheckBoxNoMouse = new wxCheckBox(page, -1, wxT("Disable mouse"));
	bsizer->Add(CheckBoxNoMouse, 0, wxALL, 4);
	CheckBoxNoJoy = new wxCheckBox(page, -1, wxT("Disable joystick"));
	bsizer->Add(CheckBoxNoJoy, 0, wxALL, 4);
	page->SetSizer(bsizer);
	bsizer->Layout();

	//	Network options
	page = new wxPanel(nbook);
	nbook->AddPage(page, wxT("Network"));
	gsizer = new wxFlexGridSizer(2);
	CheckBoxNoLan = new wxCheckBox(page, -1, wxT("Disable all LAN drivers"));
	gsizer->AddSpacer(1);
	gsizer->Add(CheckBoxNoLan, 0, wxALL, 4);
	CheckBoxNoUDP = new wxCheckBox(page, -1, wxT("Disable TCP/IP driver"));
	gsizer->AddSpacer(1);
	gsizer->Add(CheckBoxNoUDP, 0, wxALL, 4);
	CheckBoxNoIPX = new wxCheckBox(page, -1, wxT("Disable IPX driver"));
	gsizer->AddSpacer(1);
	gsizer->Add(CheckBoxNoIPX, 0, wxALL, 4);
	gsizer->Add(new wxStaticText(page, -1, wxT("IP address:")), 0, wxALL, 4);
	EditIPAddress = new wxTextCtrl(page, -1, wxT(""), wxDefaultPosition, wxSize(128, -1));
	gsizer->Add(EditIPAddress, 0, wxALL, 4);
	gsizer->Add(new wxStaticText(page, -1, wxT("Port:")), 0, wxALL, 4);
	EditPort = new wxTextCtrl(page, -1, wxT(""), wxDefaultPosition, wxSize(48, -1));
	gsizer->Add(EditPort, 0, wxALL, 4);
	page->SetSizer(gsizer);
	gsizer->Layout();

	bsizer = new wxBoxSizer(wxHORIZONTAL);
	bsizer->Add(new wxButton(panel, VLaunch_Run, wxT("Run")), 0, wxALL, 4);
	bsizer->Add(new wxButton(panel, VLaunch_Exit, wxT("Exit")), 0, wxALL, 4);
	mainsizer->Add(bsizer, 0, wxALIGN_RIGHT);
	panel->SetSizer(mainsizer);
	mainsizer->SetSizeHints(this);

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

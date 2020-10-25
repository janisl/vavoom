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
	wxComboBox*		Resolution;
	wxCheckBox*		CheckBoxUseCustomRes;
	wxTextCtrl*		CustomResolutionHeight;
	wxTextCtrl*		CustomResolutionWidth;
	wxComboBox*		Colour;
	wxTextCtrl*		Particles;
	wxCheckBox*		CheckBoxNoSound;
	wxCheckBox*		CheckBoxNoSfx;
	wxCheckBox*		CheckBox3DSound;
	wxCheckBox*		CheckBoxNoMusic;
	wxCheckBox*		CheckBoxNoCDAudio;
	wxCheckBox*		CheckBoxUseOpenAL;
	wxCheckBox*		CheckBoxUseTimidity;
	wxTextCtrl*		PatchFiles;
	wxTextCtrl*		MouseX;
	wxTextCtrl*		MouseY;
	wxCheckBox*		CheckBoxNoLan;
	wxCheckBox*		CheckBoxMaster;
	wxTextCtrl*		EditIPAddress;
	wxTextCtrl*		EditPort;
	wxTextCtrl*		EditMasterIPAddress;
	wxTextCtrl*		EditMasterPort;
	wxCheckBox*		CheckBoxNoMouse;
	wxCheckBox*		CheckBoxNoJoy;
	wxCheckBox*		CheckBoxDebug;
	wxCheckBox*		CheckBoxWindowedMode;
	wxTextCtrl*		EditIWadDir;
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
#include "vlaunch.xpm"

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VMain::VMain
//
//==========================================================================

VMain::VMain()
: wxFrame(NULL, wxID_ANY, wxT("Vavoom launcher"), wxDefaultPosition, wxDefaultSize,
	wxDEFAULT_FRAME_STYLE & ~ (wxRESIZE_BORDER | wxMAXIMIZE_BOX))
{
	SetIcon(wxICON(vlaunch));

	wxPanel* panel = new wxPanel(this);
	wxBoxSizer* mainsizer = new wxBoxSizer(wxVERTICAL);

	mainsizer->Add(new wxStaticBitmap(panel, -1, wxBitmap(vavoom_xpm)));

	wxNotebook* nbook = new wxNotebook(panel, -1, wxPoint(0, 105), wxSize(447, 270));
	mainsizer->Add(nbook);

	wxPanel* page = new wxPanel(nbook);
	nbook->AddPage(page, wxT("Main"));
	wxFlexGridSizer* gsizer = new wxFlexGridSizer(2);

	gsizer->Add(new wxStaticText(page, -1, wxT(" ")), 0, wxALL, 4);
	gsizer->Add(new wxStaticText(page, -1, wxT(" ")), 0, wxALL, 4);

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

	gsizer->Add(new wxStaticText(page, -1, wxT("Custom game:")), 0, wxALL, 4);
	EditGame = new wxTextCtrl(page, -1, wxT(""), wxDefaultPosition, wxSize(128, -1));
	gsizer->Add(EditGame, 0, wxALL, 4);
	gsizer->Add(new wxStaticText(page, -1, wxT("Main WAD directory:")), 0, wxALL, 4);
	EditIWadDir = new wxTextCtrl(page, -1, wxT(""), wxDefaultPosition, wxSize(209, -1));
	gsizer->Add(EditIWadDir, 0, wxALL, 4);
	CheckBoxDevGame = new wxCheckBox(page, -1, wxT("Development mode"));
	gsizer->AddSpacer(1);
	gsizer->Add(CheckBoxDevGame, 0, wxALL, 4);
	gsizer->Add(new wxStaticText(page, -1, wxT("Files:")), 0, wxALL, 4);
	EditFiles = new wxTextCtrl(page, -1, wxT(""), wxDefaultPosition, wxSize(209, -1));
	gsizer->Add(EditFiles, 0, wxALL, 4);
	CheckBoxDebug = new wxCheckBox(page, -1, wxT("Create debug file"));
	gsizer->AddSpacer(1);
	gsizer->Add(CheckBoxDebug, 0, wxALL, 4);
	gsizer->Add(new wxStaticText(page, -1, wxT("Other options:")), 0, wxALL, 4);
	EditMisc = new wxTextCtrl(page, -1, wxT(""), wxDefaultPosition, wxSize(209, -1));
	gsizer->Add(EditMisc, 0, wxALL, 4);
	page->SetSizer(gsizer);
	gsizer->Layout();

	//	Video options
	page = new wxPanel(nbook);
	nbook->AddPage(page, wxT("Video"));
	wxFlexGridSizer* vsizer = new wxFlexGridSizer(2);

	vsizer->Add(new wxStaticText(page, -1, wxT("Renderer:")), 0, wxALL, 4);
	wxString RendChoices[2];
	RendChoices[0] = wxT("OpenGL");
	RendChoices[1] = wxT("Direct3D");
	RendererBox = new wxComboBox(page, -1, RendChoices[1], wxDefaultPosition, wxDefaultSize,
#ifdef _WIN32
		2,
#else
		1,
#endif
		RendChoices, wxCB_READONLY);
	vsizer->Add(RendererBox, 0, wxALL, 4);
	vsizer->Add(new wxStaticText(page, -1, wxT("Resolution:")), 0, wxALL, 4);
	wxString ResolutionChoices[6];
	ResolutionChoices[0] = wxT("640x480");
	ResolutionChoices[1] = wxT("800x600");
	ResolutionChoices[2] = wxT("1024x768");
	ResolutionChoices[3] = wxT("1152x864");
	ResolutionChoices[4] = wxT("1280x1024");
	ResolutionChoices[5] = wxT("1600x1200");
	Resolution = new wxComboBox(page, -1, ResolutionChoices[0], wxDefaultPosition, wxDefaultSize, 6, ResolutionChoices, wxCB_READONLY);
	vsizer->Add(Resolution, 0, wxALL, 4);
	vsizer->AddSpacer(1);
	CheckBoxUseCustomRes = new wxCheckBox(page, -1, wxT("Use Custom Resolution"));
	vsizer->Add(CheckBoxUseCustomRes, 0, wxALL, 4);
	vsizer->Add(new wxStaticText(page, -1, wxT("Custom Resolution Height:")), 0, wxALL, 4);
	CustomResolutionHeight = new wxTextCtrl(page, -1, wxT(""), wxDefaultPosition,wxSize(48, -1));
	vsizer->Add(CustomResolutionHeight, 0, wxALL, 4);
	vsizer->Add(new wxStaticText(page, -1, wxT("Custom Resolution Width:")), 0, wxALL, 4);
	CustomResolutionWidth = new wxTextCtrl(page, -1, wxT(""), wxDefaultPosition,wxSize(48, -1));
	vsizer->Add(CustomResolutionWidth, 0, wxALL, 4);
	vsizer->Add(new wxStaticText(page, -1, wxT("Color Depth:")), 0, wxALL, 4);
	wxString ColourChoices[3];
	ColourChoices[0] = wxT("16 bits");
	ColourChoices[1] = wxT("24 bits");
	ColourChoices[2] = wxT("32 bits");
	Colour = new wxComboBox(page, -1, ColourChoices[0], wxDefaultPosition, wxDefaultSize, 3, ColourChoices, wxCB_READONLY);
	vsizer->Add(Colour, 0, wxALL, 4);
	vsizer->AddSpacer(1);
	CheckBoxWindowedMode = new wxCheckBox(page, -1, wxT("Run in Windowed Mode"));
	vsizer->Add(CheckBoxWindowedMode, 0, wxALL, 4);
	vsizer->Add(new wxStaticText(page, -1, wxT("Particles:")), 0, wxALL, 4);
	Particles = new wxTextCtrl(page, -1, wxT(""), wxDefaultPosition,wxSize(48, -1));
	vsizer->Add(Particles, 0, wxALL, 4);
	page->SetSizer(vsizer);
	vsizer->Layout();

	//	Sound options
	page = new wxPanel(nbook);
	nbook->AddPage(page, wxT("Sound"));
	wxFlexGridSizer* ssizer = new wxFlexGridSizer(2);

	ssizer->Add(new wxStaticText(page, -1, wxT(" ")), 0, wxALL, 4);
	ssizer->Add(new wxStaticText(page, -1, wxT(" ")), 0, wxALL, 4);

	ssizer->AddSpacer(1);
	CheckBoxNoSound = new wxCheckBox(page, -1, wxT("Disable all sound"));
	ssizer->Add(CheckBoxNoSound, 0, wxALL, 4);
	ssizer->AddSpacer(1);
	CheckBoxNoSfx = new wxCheckBox(page, -1, wxT("No sounds"));
	ssizer->Add(CheckBoxNoSfx, 0, wxALL, 4);
	ssizer->AddSpacer(1);
	CheckBoxNoMusic = new wxCheckBox(page, -1, wxT("No music"));
	ssizer->Add(CheckBoxNoMusic, 0, wxALL, 4);
	ssizer->AddSpacer(1);
	CheckBoxNoCDAudio = new wxCheckBox(page, -1, wxT("No CD audio"));
	ssizer->Add(CheckBoxNoCDAudio, 0, wxALL, 4);
	ssizer->AddSpacer(1);
	CheckBox3DSound = new wxCheckBox(page, -1, wxT("Use 3D sound"));
	ssizer->Add(CheckBox3DSound, 0, wxALL, 4);
	ssizer->AddSpacer(1);
	CheckBoxUseOpenAL = new wxCheckBox(page, -1, wxT("Use OpenAL"));
	ssizer->Add(CheckBoxUseOpenAL, 0, wxALL, 4);
	ssizer->AddSpacer(1);
	CheckBoxUseTimidity = new wxCheckBox(page, -1, wxT("Use Timidity"));
	ssizer->Add(CheckBoxUseTimidity, 0, wxALL, 4);
	ssizer->Add(new wxStaticText(page, -1, wxT("Timidity Patches Location:")), 0, wxALL, 4);
	PatchFiles = new wxTextCtrl(page, -1, wxT(""), wxDefaultPosition, wxSize(209, -1));
	ssizer->Add(PatchFiles, 0, wxALL, 4);
	page->SetSizer(ssizer);
	ssizer->Layout();

	//	Input options
	page = new wxPanel(nbook);
	nbook->AddPage(page, wxT("Input"));
	wxFlexGridSizer* isizer = new wxFlexGridSizer(2);

	isizer->Add(new wxStaticText(page, -1, wxT(" ")), 0, wxALL, 4);
	isizer->Add(new wxStaticText(page, -1, wxT(" ")), 0, wxALL, 4);

	isizer->AddSpacer(1);
	CheckBoxNoMouse = new wxCheckBox(page, -1, wxT("Disable mouse"));
	isizer->Add(CheckBoxNoMouse, 0, wxALL, 4);
	isizer->AddSpacer(1);
	CheckBoxNoJoy = new wxCheckBox(page, -1, wxT("Disable joystick"));
	isizer->Add(CheckBoxNoJoy, 0, wxALL, 4);
	isizer->AddSpacer(1);
	isizer->AddSpacer(1);
	isizer->Add(new wxStaticText(page, -1, wxT("Mouse X sensitivity:")), 0, wxALL, 4);
	MouseX = new wxTextCtrl(page, -1, wxT(""), wxDefaultPosition, wxSize(48, -1));
	isizer->Add(MouseX, 0, wxALL, 4);
	isizer->Add(new wxStaticText(page, -1, wxT("Mouse Y sensitivity:")), 0, wxALL, 4);
	MouseY = new wxTextCtrl(page, -1, wxT(""), wxDefaultPosition, wxSize(48, -1));
	isizer->Add(MouseY, 0, wxALL, 4);
	page->SetSizer(isizer);
	isizer->Layout();

	//	Network options
	page = new wxPanel(nbook);
	nbook->AddPage(page, wxT("Network"));
	gsizer = new wxFlexGridSizer(2);
	gsizer->AddSpacer(1);

	gsizer->Add(new wxStaticText(page, -1, wxT(" ")), 0, wxALL, 4);
	gsizer->Add(new wxStaticText(page, -1, wxT(" ")), 0, wxALL, 4);

	CheckBoxNoLan = new wxCheckBox(page, -1, wxT("Disable network driver"));
	gsizer->Add(CheckBoxNoLan, 0, wxALL, 4);
	gsizer->Add(new wxStaticText(page, -1, wxT("IP address:")), 0, wxALL, 4);
	EditIPAddress = new wxTextCtrl(page, -1, wxT(""), wxDefaultPosition, wxSize(128, -1));
	gsizer->Add(EditIPAddress, 0, wxALL, 4);
	gsizer->Add(new wxStaticText(page, -1, wxT("Port:")), 0, wxALL, 4);
	EditPort = new wxTextCtrl(page, -1, wxT(""), wxDefaultPosition, wxSize(48, -1));
	gsizer->Add(EditPort, 0, wxALL, 4);
	gsizer->AddSpacer(1);
	gsizer->AddSpacer(1);
	gsizer->AddSpacer(1);
	gsizer->AddSpacer(1);
	gsizer->AddSpacer(1);
	gsizer->AddSpacer(1);
	gsizer->AddSpacer(1);
	gsizer->AddSpacer(1);
	gsizer->AddSpacer(1);
	CheckBoxMaster = new wxCheckBox(page, -1, wxT("Enable Master Server"));
	gsizer->Add(CheckBoxMaster, 0, wxALL, 4);
	gsizer->Add(new wxStaticText(page, -1, wxT("Master Server IP address:")), 0, wxALL, 4);
	EditMasterIPAddress = new wxTextCtrl(page, -1, wxT(""), wxDefaultPosition, wxSize(128, -1));
	gsizer->Add(EditMasterIPAddress, 0, wxALL, 4);
	page->SetSizer(gsizer);
	gsizer->Layout();

	wxBoxSizer* bsizer = new wxBoxSizer(wxHORIZONTAL);
	bsizer->Add(new wxButton(panel, VLaunch_Run, wxT("Run")), 0, wxALL, 4);
	bsizer->Add(new wxButton(panel, VLaunch_Exit, wxT("Exit")), 0, wxALL, 4);
	mainsizer->Add(bsizer, 0, wxALIGN_RIGHT);
	panel->SetSizer(mainsizer);
	mainsizer->SetSizeHints(this);

	//	Load saved settings.
	wxConfigBase* Conf = wxConfigBase::Get();
	Game->SetSelection(Conf->Read(wxT("Game"), 0l));
	RendererBox->SetSelection(Conf->Read(wxT("Renderer"), 0l));
	Resolution->SetSelection(Conf->Read(wxT("Resolution"), 0l));
	CheckBoxUseCustomRes->SetValue(!!Conf->Read(wxT("UseCustomResolution"), 01));
	CustomResolutionHeight->SetValue(Conf->Read(wxT("CustomResolutionHeight"),  wxT("")));
	CustomResolutionWidth->SetValue(Conf->Read(wxT("CustomResolutionWidth"),  wxT("")));
	Colour->SetSelection(Conf->Read(wxT("Colour"), 0l));
	Particles->SetValue(Conf->Read(wxT("Particles"), wxT("")));
	CheckBoxNoSound->SetValue(!!Conf->Read(wxT("NoSound"), 0l));
	CheckBoxNoSfx->SetValue(!!Conf->Read(wxT("NoSfx"), 0l));
	CheckBox3DSound->SetValue(!!Conf->Read(wxT("3DSound"), 0l));
	CheckBoxNoMusic->SetValue(!!Conf->Read(wxT("NoMusic"), 0l));
	CheckBoxNoCDAudio->SetValue(!!Conf->Read(wxT("NoCDAudio"), 0l));
	CheckBoxUseOpenAL->SetValue(!!Conf->Read(wxT("UseOpenAL"), 0l));
	CheckBoxUseTimidity->SetValue(!!Conf->Read(wxT("UseTimidity"), 0l));
	CheckBoxNoLan->SetValue(!!Conf->Read(wxT("NoLAN"), 0l));
	EditIPAddress->SetValue(Conf->Read(wxT("IPAddress"), wxT("")));
	EditPort->SetValue(Conf->Read(wxT("Port"), wxT("")));
	CheckBoxMaster->SetValue(!!Conf->Read(wxT("UseMaster"), 0l));
	EditMasterIPAddress->SetValue(Conf->Read(wxT("MasterIPAddress"), wxT("")));
	CheckBoxNoMouse->SetValue(!!Conf->Read(wxT("NoMouse"), 0l));
	CheckBoxNoJoy->SetValue(!!Conf->Read(wxT("NoJoy"), 0l));
	CheckBoxDebug->SetValue(!!Conf->Read(wxT("Debug"), 0l));
	CheckBoxWindowedMode->SetValue(!!Conf->Read(wxT("WindowedMode"), 0l));
	EditIWadDir->SetValue(Conf->Read(wxT("IWadDir"), wxT("")));
	EditGame->SetValue(Conf->Read(wxT("CustomGame"), wxT("")));
	MouseX->SetValue(Conf->Read(wxT("MouseX"), wxT("")));
	MouseY->SetValue(Conf->Read(wxT("MouseY"), wxT("")));
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
	Conf->Write(wxT("Resolution"), Resolution->GetSelection());
	Conf->Write(wxT("UseCustomResolution"), CheckBoxUseCustomRes->GetValue());
	Conf->Write(wxT("CustomResolutionHeight"), CustomResolutionHeight->GetValue());
	Conf->Write(wxT("CustomResolutionWidth"), CustomResolutionWidth->GetValue());
	Conf->Write(wxT("Colour"), Colour->GetSelection());
	Conf->Write(wxT("WindowedMode"), CheckBoxWindowedMode->IsChecked());
	Conf->Write(wxT("Particles"), Particles->GetValue());
	Conf->Write(wxT("NoSound"), CheckBoxNoSound->IsChecked());
	Conf->Write(wxT("NoSfx"), CheckBoxNoSfx->IsChecked());
	Conf->Write(wxT("3DSound"), CheckBox3DSound->IsChecked());
	Conf->Write(wxT("NoMusic"), CheckBoxNoMusic->IsChecked());
	Conf->Write(wxT("NoCDAudio"), CheckBoxNoCDAudio->IsChecked());
	Conf->Write(wxT("UseOpenAL"), CheckBoxUseOpenAL->IsChecked());
	Conf->Write(wxT("UseTimidity"), CheckBoxUseTimidity->IsChecked());
	Conf->Write(wxT("NoLAN"), CheckBoxNoLan->IsChecked());
	Conf->Write(wxT("IPAddress"), EditIPAddress->GetValue());
	Conf->Write(wxT("Port"), EditPort->GetValue());
	Conf->Write(wxT("UseMaster"), CheckBoxNoLan->IsChecked());
	Conf->Write(wxT("MasterIPAddress"), EditIPAddress->GetValue());
	Conf->Write(wxT("NoMouse"), CheckBoxNoMouse->IsChecked());
	Conf->Write(wxT("NoJoy"), CheckBoxNoJoy->IsChecked());
	Conf->Write(wxT("Debug"), CheckBoxDebug->IsChecked());
	Conf->Write(wxT("IWadDir"), EditIWadDir->GetValue());
	Conf->Write(wxT("CustomGame"), EditGame->GetValue());
	Conf->Write(wxT("PatchFiles"), PatchFiles->GetValue());
	Conf->Write(wxT("MouseX"), MouseX->GetValue());
	Conf->Write(wxT("MouseY"), MouseY->GetValue());
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
	wxString CmdLine = wxT("vavoom");

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
	case 0:
		CmdLine += wxT(" -opengl");
		break;
	case 1:
		CmdLine += wxT(" -d3d");
		break;
	}

	// Particles
	if (Particles->GetValue().Length())
		CmdLine += wxT(" -particles ") + EditMisc->GetValue();

	// Sound
	if (CheckBoxNoSound->IsChecked())
		CmdLine += wxT(" -nosound");
	if (CheckBoxNoSfx->IsChecked())
		CmdLine += wxT(" -nosfx");
	if (CheckBoxNoMusic->IsChecked())
		CmdLine += wxT(" -nomusic");
	if (CheckBoxNoCDAudio->IsChecked())
		CmdLine += wxT(" -nocdaudio");
	if (CheckBox3DSound->IsChecked())
		CmdLine += wxT(" -3dsound");
	if (CheckBoxUseOpenAL->IsChecked())
		CmdLine += wxT(" -openal");

	// Network
	if (CheckBoxNoLan->IsChecked())
		CmdLine += wxT(" -nolan");
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
	if (EditIWadDir->GetValue().Length())
		CmdLine += wxT(" -iwaddir ") + EditIWadDir->GetValue();
	if (EditFiles->GetValue().Length())
		CmdLine += wxT(" -file ") + EditFiles->GetValue();

	// Timidity
	if (CheckBoxUseTimidity->IsChecked())
	{
		CmdLine += wxT(" +snd_mid_player 1");
	}
	else
	{
		CmdLine += wxT(" +snd_mid_player 0");
	}

	if (PatchFiles->GetValue().Length())
		CmdLine += wxT(" +snd_timidity_patches ") + PatchFiles->GetValue();

	// Mouse Sensitivity
	if (MouseX->GetValue().Length())
		CmdLine += wxT(" +mouse_x_sensitivity ") + MouseX->GetValue();

	if (MouseY->GetValue().Length())
		CmdLine += wxT(" +mouse_y_sensitivity ") + MouseY->GetValue();

	// Master Server
	if (CheckBoxMaster->IsChecked())
		CmdLine += wxT(" +use_master 1");
	if (EditMasterIPAddress->GetValue().Length())
		CmdLine += wxT(" +master_srv ") + EditMasterIPAddress->GetValue();

	// Set Resolution
	// Are we using custom resolutions?
	if (!CheckBoxUseCustomRes->IsChecked())
	{
		switch (Resolution->GetSelection())
		{
		case 0:
			CmdLine += wxT(" +setresolution 640 480");
			break;
		case 1:
			CmdLine += wxT(" +setresolution 800 600");
			break;
		case 2:
			CmdLine += wxT(" +setresolution 1024 768");
			break;
		case 3:
			CmdLine += wxT(" +setresolution 1152 864");
			break;
		case 4:
			CmdLine += wxT(" +setresolution 1280 1024");
			break;
		case 5:
			CmdLine += wxT(" +setresolution 1600 1200");
			break;
		}
	}
	else
	{
		CmdLine += wxT(" +setresolution ") + CustomResolutionHeight->GetValue() + wxT(" ") + CustomResolutionWidth->GetValue();
	}

	switch (Colour->GetSelection())
	{
	case 0:
		CmdLine += wxT(" 16");
		break;
	case 1:
		CmdLine += wxT(" 24");
		break;
	case 2:
		CmdLine += wxT(" 32");
		break;
	}

	// Windowed Mode
	if (CheckBoxWindowedMode->IsChecked())
	{
		CmdLine += wxT(" +screen_windowed 1");
	}
	else
	{
		CmdLine += wxT(" +screen_windowed 0");
	}

	// EditMisc must always be the last option we fill
	// to the command line, to avoid problems while
	// loading dehacked patches, for example...
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

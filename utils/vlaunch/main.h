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

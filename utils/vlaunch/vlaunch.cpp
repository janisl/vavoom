//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
USERES("vlaunch.res");
USEFORM("main.cpp", LauncherForm);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	try
	{
		Application->Initialize();
		Application->Title = "Vavoom launcher";
		Application->CreateForm(__classid(TLauncherForm), &LauncherForm);
		Application->Run();
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	return 0;
}
//---------------------------------------------------------------------------

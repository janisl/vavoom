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

enum EWinType
{
	WIN_Normal,
	WIN_Modal,
	WIN_Root
};

class VWindow : public VObject
{
	DECLARE_CLASS(VWindow, VObject, 0)

	friend class VRootWindow;

	// Quick class reference
	int WindowType;				// Contains window type

	// Booleans
	enum
	{
		WF_IsVisible		= 0x0001,	// True if the window is visible
		WF_IsSensitive		= 0x0002,	// True if the window can take input
		WF_IsSelectable		= 0x0004,	// True if the window can have keyboard focus
		WF_TickEnabled		= 0x0008,	// True if () event should be called
		WF_IsInitialised	= 0x0010,	// True if the window has been initialized

		// Destructor information
		WF_BeingDestroyed	= 0x0020,	// True if this window is going bye-bye
	};
	vuint32 WindowFlags;

	int X;
	int Y;
	int Width;
	int Height;

	// Clipping rectangle
	VClipRect ClipRect;			// Clipping rectangle; maintained at all times

private:
	VGC *WinGC;

	// Relatives
	VWindow *Parent;			// Parent window; NULL if this is root
	VWindow *FirstChild;		// "Lowest" child (first one drawn)
	VWindow *LastChild;			// "Highest" child (last one drawn)
	VWindow *PrevSibling;		// Next "lowest" sibling (previous one drawn)
	VWindow *NextSibling;		// Next "highest" sibling (next one drawn)

public:
	VWindow(void);
	virtual void Init(VWindow *InParent);
	virtual void CleanUp(void);
	void Destroy(void);

	// Ancestral routines
	VRootWindow *GetRootWindow(void);
	VModalWindow *GetModalWindow(void);
	VWindow *GetParent(void);

	// Child routines
	VWindow *GetBottomChild(bool bVisibleOnly = true);
	VWindow *GetTopChild(bool bVisibleOnly = true);

	// Sibling routines
	VWindow *GetLowerSibling(bool bVisibleOnly = true);
	VWindow *GetHigherSibling(bool bVisibleOnly = true);

	// Routines which change order of siblings
	void Raise(void);
	void Lower(void);

	// Visibility routines
	void SetVisibility(bool NewVisibility);
	void Show(void) { SetVisibility(true); }
	void Hide(void) { SetVisibility(false); }
	bool IsVisible(bool bRecurse = true)
	{
		if (bRecurse)
		{
			VWindow *pParent = this;
			while (pParent)
			{
				if (!(pParent->WindowFlags & WF_IsVisible))
					break;
				pParent = pParent->Parent;
			}
			return (pParent ? false : true);
		}
		else
			return !!(WindowFlags & WF_IsVisible);
	}

	// Sensitivity routines
	void SetSensitivity(bool NewSensitivity);
	void Enable(void) { SetSensitivity(true); }
	void Disable(void) { SetSensitivity(false); }
	bool IsSensitive(bool bRecurse = true)
	{
		if (bRecurse)
		{
			VWindow *pParent = this;
			while (pParent)
			{
				if (!(pParent->WindowFlags & WF_IsSensitive))
					break;
				pParent = pParent->Parent;
			}
			return (pParent ? false : true);
		}
		else
			return !!(WindowFlags & WF_IsSensitive);
	}

	// Selectability routines
	void SetSelectability(bool NewSelectability);
	bool IsSelectable(void) { return !!(WindowFlags & WF_IsSelectable); }
	//bool IsTraversable(bool bCheckModal = true);
	//bool IsFocusWindow(void);

	//	Reconfiguration routines.
	void SetPos(int NewX, int NewY);
	void SetSize(int NewWidth, int NewHeight);
	void SetConfiguration(int NewX, int NewY, int NewWidth, int HewHeight);
	void SetWidth(int NewWidth);
	void SetHeight(int NewHeight);

	// Slayer of innocent children
	void DestroyAllChildren(void) { KillAllChildren(); }

	void InitWindow(void)
	{
		clpr.Exec(GetVFunction("InitWindow"), (int)this);
	}
	void DestroyWindow(void)
	{
		clpr.Exec(GetVFunction("DestroyWindow"), (int)this);
	}
	void WindowReady(void)
	{
		clpr.Exec(GetVFunction("WindowReady"), (int)this);
	}

	virtual void ChildAdded(VWindow *Child)
	{
		clpr.Exec(GetVFunction("ChildAdded"), (int)this, (int)Child);
	}
	virtual void ChildRemoved(VWindow *Child)
	{
		clpr.Exec(GetVFunction("ChildRemoved"), (int)this, (int)Child);
	}
	virtual void DescendantAdded(VWindow *Descendant)
	{
		clpr.Exec(GetVFunction("DescendantAdded"), (int)this, (int)Descendant);
	}
	virtual void DescendantRemoved(VWindow *Descendant)
	{
		clpr.Exec(GetVFunction("DescendantRemoved"), (int)this, (int)Descendant);
	}

	virtual void ConfigurationChanged(void)
	{
		clpr.Exec(GetVFunction("ConfigurationChanged"), (int)this);
	}
	virtual void VisibilityChanged(bool NewVisibility)
	{
		clpr.Exec(GetVFunction("VisibilityChanged"), (int)this, NewVisibility);
	}
	virtual void SensitivityChanged(bool bNewSensitivity)
	{
		clpr.Exec(GetVFunction("SensitivityChanged"), (int)this, bNewSensitivity);
	}

	virtual void DrawWindow(VGC *gc)
	{
		clpr.Exec(GetVFunction("DrawWindow"), (int)this, (int)gc);
	}
	virtual void PostDrawWindow(VGC *gc)
	{
		clpr.Exec(GetVFunction("PostDrawWindow"), (int)this, (int)gc);
	}

	virtual void Tick(float) { }
	void eventTick(float DeltaTime)
	{
		clpr.Exec(GetVFunction("Tick"), (int)this, PassFloat(DeltaTime));
	}

	static VWindow *CreateNewWindow(VClass *NewClass, VWindow *ParentWindow);

protected:
	void KillAllChildren(void);

private:
	void AddChild(VWindow *);
	void RemoveChild(VWindow *);

	void DrawTree(void);

	void ClipTree(void);

	void TickTree(float DeltaTime);

public:
	DECLARE_FUNCTION(Destroy)
	DECLARE_FUNCTION(NewChild)
	DECLARE_FUNCTION(Raise)
	DECLARE_FUNCTION(Lower)
	DECLARE_FUNCTION(SetVisibility)
	DECLARE_FUNCTION(Show)
	DECLARE_FUNCTION(Hide)
	DECLARE_FUNCTION(IsVisible)
	DECLARE_FUNCTION(SetSensitivity)
	DECLARE_FUNCTION(Enable)
	DECLARE_FUNCTION(Disable)
	DECLARE_FUNCTION(IsSensitive)
	DECLARE_FUNCTION(SetSelectability)
	
	DECLARE_FUNCTION(GetRootWindow)
	DECLARE_FUNCTION(GetModalWindow)
	DECLARE_FUNCTION(GetParent)

	DECLARE_FUNCTION(SetPos)
	DECLARE_FUNCTION(SetSize)
	DECLARE_FUNCTION(SetConfiguration)
	DECLARE_FUNCTION(SetWidth)
	DECLARE_FUNCTION(SetHeight)

	DECLARE_FUNCTION(GetBottomChild)
	DECLARE_FUNCTION(GetTopChild)
	DECLARE_FUNCTION(GetLowerSibling)
	DECLARE_FUNCTION(GetHigherSibling)

	DECLARE_FUNCTION(DestroyAllChildren)
};

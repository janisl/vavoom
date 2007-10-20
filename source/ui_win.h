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

//==========================================================================
//
//	VClipRect - clipping rectangle class
//
//==========================================================================

struct VClipRect
{
	// Default constructor
	VClipRect()
	{ 
		OriginX    = 0;
		OriginY    = 0; 
		ClipX      = 0;
		ClipY      = 0; 
		ClipWidth  = 0;
		ClipHeight = 0;
	}

	// Other convenience constructors
	VClipRect(float NewClipX,     float NewClipY,
	          float NewClipWidth, float NewClipHeight)
	{
		OriginX    = NewClipX;
		OriginY    = NewClipY;

		ClipX      = 0;
		ClipY      = 0;
		ClipWidth  = NewClipWidth;
		ClipHeight = NewClipHeight;
	}

	VClipRect(float NewOriginX,   float NewOriginY,
	          float NewClipX,     float NewClipY,
	          float NewClipWidth, float NewClipHeight)
	{
		OriginX    = NewOriginX;
		OriginY    = NewOriginY;
		ClipX      = NewClipX;
		ClipY      = NewClipY;
		ClipWidth  = NewClipWidth;
		ClipHeight = NewClipHeight;
	}

	VClipRect(VClipRect &Rect1, VClipRect &Rect2)
	{
		OriginX    = Rect1.OriginX;
		OriginY    = Rect1.OriginY;
		ClipX      = Rect1.ClipX;
		ClipY      = Rect1.ClipY;
		ClipWidth  = Rect1.ClipWidth;
		ClipHeight = Rect1.ClipHeight;
		Intersect(Rect2);
	}

	// Public methods
	void SetOrigin(float NewOriginX, float NewOriginY)
	{
		ClipX   += (OriginX - NewOriginX);
		ClipY   += (OriginY - NewOriginY);
		OriginX =  NewOriginX;
		OriginY =  NewOriginY;
	}

	void MoveOrigin(float NewDeltaX, float NewDeltaY)
	{
		SetOrigin(OriginX + NewDeltaX, OriginY + NewDeltaY);
	}

	void Intersect(float NewClipX,     float NewClipY,
	               float NewClipWidth, float NewClipHeight)
	{
		VClipRect TempRect(NewClipX + OriginX, NewClipY + OriginY, 
			NewClipWidth, NewClipHeight);
		Intersect(TempRect);
	}

	void Intersect(VClipRect &NewRect)
	{
		float fromX1, fromY1;
		float fromX2, fromY2;
		float toX1,   toY1;
		float toX2,   toY2;

		// Convert everything to absolute coordinates
		fromX1 = ClipX         + OriginX;
		fromY1 = ClipY         + OriginY;
		fromX2 = NewRect.ClipX + NewRect.OriginX;
		fromY2 = NewRect.ClipY + NewRect.OriginY;
		toX1   = fromX1        + ClipWidth;
		toY1   = fromY1        + ClipHeight;
		toX2   = fromX2        + NewRect.ClipWidth;
		toY2   = fromY2        + NewRect.ClipHeight;

		// Clip
		if (fromX1 < fromX2)
			fromX1 = fromX2;
		if (fromY1 < fromY2)
			fromY1 = fromY2;
		if (toX1 > toX2)
			toX1 = toX2;
		if (toY1 > toY2)
			toY1 = toY2;

		// Reconvert to origin of this object
		ClipX      = fromX1 - OriginX;
		ClipY      = fromY1 - OriginY;
		ClipWidth  = toX1   - fromX1;
		ClipHeight = toY1   - fromY1;
	}

	bool HasArea(void)
	{
		return ((ClipWidth > 0) && (ClipHeight > 0));
	}

	// Members
	float OriginX;		// X origin of rectangle, in absolute coordinates
	float OriginY;		// Y origin of rectangle, in absolute coordinates

	float ClipX;		// Leftmost edge of rectangle relative to origin
	float ClipY;		// Topmost edge of rectangle relative to origin
	float ClipWidth;	// Width of rectangle
	float ClipHeight;	// Height of rectangle
};

enum EWinType
{
	WIN_Normal,
	WIN_Modal,
	WIN_Root
};

class VWindow : public VWidget
{
	DECLARE_CLASS(VWindow, VWidget, 0)

	friend class VRootWindow;

	// Quick class reference
	vuint8 WindowType;				// Contains window type

	// Booleans
	enum
	{
		WF_IsVisible		= 0x0001,	// True if the window is visible
		WF_IsSensitive		= 0x0002,	// True if the window can take input
		WF_IsSelectable		= 0x0004,	// True if the window can have keyboard focus
		WF_TickEnabled		= 0x0008,	// True if () event should be called
		WF_IsInitialised	= 0x0010,	// True if the window has been initialised

		// Destructor information
		WF_BeingDestroyed	= 0x0020,	// True if this window is going bye-bye
	};
	vuint32 WindowFlags;

	// Clipping rectangle
	VClipRect ClipRect;			// Clipping rectangle; maintained at all times

public:
	VWindow();
	virtual void Init(VWindow *InParent);
	virtual void CleanUp();
	void Destroy();

	// Ancestral routines
	VRootWindow *GetRootWindow();
	VModalWindow *GetModalWindow();
	VWindow *GetParent();

	// Child routines
	VWindow *GetBottomChild(bool bVisibleOnly = true);
	VWindow *GetTopChild(bool bVisibleOnly = true);

	// Sibling routines
	VWindow *GetLowerSibling(bool bVisibleOnly = true);
	VWindow *GetHigherSibling(bool bVisibleOnly = true);

	// Visibility routines
	void SetVisibility(bool NewVisibility);
	void Show() { SetVisibility(true); }
	void Hide() { SetVisibility(false); }
	bool IsVisible(bool bRecurse = true)
	{
		if (bRecurse)
		{
			VWindow *pParent = this;
			while (pParent)
			{
				if (!(pParent->WindowFlags & WF_IsVisible))
					break;
				pParent = static_cast<VWindow*>(pParent->ParentWidget);
			}
			return (pParent ? false : true);
		}
		else
			return !!(WindowFlags & WF_IsVisible);
	}

	// Sensitivity routines
	void SetSensitivity(bool NewSensitivity);
	void Enable() { SetSensitivity(true); }
	void Disable() { SetSensitivity(false); }
	bool IsSensitive(bool bRecurse = true)
	{
		if (bRecurse)
		{
			VWindow *pParent = this;
			while (pParent)
			{
				if (!(pParent->WindowFlags & WF_IsSensitive))
					break;
				pParent = static_cast<VWindow*>(pParent->ParentWidget);
			}
			return (pParent ? false : true);
		}
		else
			return !!(WindowFlags & WF_IsSensitive);
	}

	// Selectability routines
	void SetSelectability(bool NewSelectability);
	bool IsSelectable() { return !!(WindowFlags & WF_IsSelectable); }
	//bool IsTraversable(bool bCheckModal = true);
	//bool IsFocusWindow();

	//	Reconfiguration routines.
	void SetPos(int NewX, int NewY);
	void SetSize(int NewWidth, int NewHeight);
	void SetConfiguration(int NewX, int NewY, int NewWidth, int HewHeight);
	void SetWidth(int NewWidth);
	void SetHeight(int NewHeight);

	// Slayer of innocent children
	void DestroyAllChildren() { KillAllChildren(); }

	void InitWindow()
	{
		P_PASS_SELF;
		EV_RET_VOID(NAME_InitWindow);
	}
	void DestroyWindow()
	{
		P_PASS_SELF;
		EV_RET_VOID(NAME_DestroyWindow);
	}
	void WindowReady()
	{
		P_PASS_SELF;
		EV_RET_VOID(NAME_WindowReady);
	}

	virtual void ConfigurationChanged()
	{
		P_PASS_SELF;
		EV_RET_VOID(NAME_ConfigurationChanged);
	}
	virtual void VisibilityChanged(bool NewVisibility)
	{
		P_PASS_SELF;
		P_PASS_BOOL(NewVisibility);
		EV_RET_VOID(NAME_VisibilityChanged);
	}
	virtual void SensitivityChanged(bool bNewSensitivity)
	{
		P_PASS_SELF;
		P_PASS_BOOL(bNewSensitivity);
		EV_RET_VOID(NAME_SensitivityChanged);
	}

	virtual void DrawWindow()
	{
		P_PASS_SELF;
		EV_RET_VOID(NAME_DrawWindow);
	}
	virtual void PostDrawWindow()
	{
		P_PASS_SELF;
		EV_RET_VOID(NAME_PostDrawWindow);
	}

	virtual void Tick(float) { }
	void eventTick(float DeltaTime)
	{
		P_PASS_SELF;
		P_PASS_FLOAT(DeltaTime);
		EV_RET_VOID(NAME_Tick);
	}

	static VWindow *CreateNewWindow(VClass *NewClass, VWindow *ParentWindow);

protected:
	void KillAllChildren();

private:
	void DrawTree();

	void ClipTree();

	void TickTree(float DeltaTime);

public:
	DECLARE_FUNCTION(Destroy)
	DECLARE_FUNCTION(NewChild)
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

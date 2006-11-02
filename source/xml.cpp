//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	$Id: template.cpp 1583 2006-06-27 19:05:42Z dj_jl $
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

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VXmlNode::VXmlNode
//
//==========================================================================

VXmlNode::VXmlNode()
: Parent(NULL)
, FirstChild(NULL)
, LastChild(NULL)
, PrevSibling(NULL)
, NextSibling(NULL)
{
}

//==========================================================================
//
//	VXmlNode::~VXmlNode
//
//==========================================================================

VXmlNode::~VXmlNode()
{
	guard(VXmlNode::~VXmlNode);
	while (FirstChild)
	{
		VXmlNode* Temp = FirstChild;
		FirstChild = FirstChild->NextSibling;
		delete Temp;
	}
	unguard;
}

//==========================================================================
//
//	VXmlNode::FindChild
//
//==========================================================================

VXmlNode* VXmlNode::FindChild(const char* AName) const
{
	guard(VXmlNode::FindChild);
	for (VXmlNode* N = FirstChild; N; N = N->NextSibling)
		if (N->Name == AName)
			return N;
	return NULL;
	unguard;
}

//==========================================================================
//
//	VXmlNode::FindChild
//
//==========================================================================

VXmlNode* VXmlNode::FindChild(const VStr& AName) const
{
	guard(VXmlNode::FindChild);
	for (VXmlNode* N = FirstChild; N; N = N->NextSibling)
		if (N->Name == AName)
			return N;
	return NULL;
	unguard;
}

//==========================================================================
//
//	VXmlNode::GetChild
//
//==========================================================================

VXmlNode* VXmlNode::GetChild(const char* AName) const
{
	guard(VXmlNode::GetChild);
	VXmlNode* N = FindChild(AName);
	if (!N)
		Sys_Error("XML node %s not found", AName);
	return N;
	unguard;
}

//==========================================================================
//
//	VXmlNode::GetChild
//
//==========================================================================

VXmlNode* VXmlNode::GetChild(const VStr& AName) const
{
	guard(VXmlNode::GetChild);
	VXmlNode* N = FindChild(AName);
	if (!N)
		Sys_Error("XML node %s not found", *AName);
	return N;
	unguard;
}

//==========================================================================
//
//	VXmlNode::FindNext
//
//==========================================================================

VXmlNode* VXmlNode::FindNext(const char* AName) const
{
	guard(VXmlNode::FindNext);
	for (VXmlNode* N = NextSibling; N; N = N->NextSibling)
		if (N->Name == AName)
			return N;
	return NULL;
	unguard;
}

//==========================================================================
//
//	VXmlNode::FindNext
//
//==========================================================================

VXmlNode* VXmlNode::FindNext(const VStr& AName) const
{
	guard(VXmlNode::FindNext);
	for (VXmlNode* N = NextSibling; N; N = N->NextSibling)
		if (N->Name == AName)
			return N;
	return NULL;
	unguard;
}

//==========================================================================
//
//	VXmlNode::FindNext
//
//==========================================================================

VXmlNode* VXmlNode::FindNext() const
{
	guard(VXmlNode::FindNext);
	for (VXmlNode* N = NextSibling; N; N = N->NextSibling)
		if (N->Name == Name)
			return N;
	return NULL;
	unguard;
}

//==========================================================================
//
//	VXmlNode::HasAttribute
//
//==========================================================================

bool VXmlNode::HasAttribute(const char* AttrName) const
{
	guard(VXmlNode::HasAttribute);
	for (int i = 0; i < Attributes.Num(); i++)
		if (Attributes[i].Name == AttrName)
			return true;
	return false;
	unguard;
}

//==========================================================================
//
//	VXmlNode::FindChild
//
//==========================================================================

bool VXmlNode::HasAttribute(const VStr& AttrName) const
{
	guard(VXmlNode::HasAttribute);
	for (int i = 0; i < Attributes.Num(); i++)
		if (Attributes[i].Name == AttrName)
			return true;
	return false;
	unguard;
}

//==========================================================================
//
//	VXmlNode::GetAttribute
//
//==========================================================================

VStr VXmlNode::GetAttribute(const char* AttrName, bool Required) const
{
	guard(VXmlNode::GetAttribute);
	for (int i = 0; i < Attributes.Num(); i++)
		if (Attributes[i].Name == AttrName)
			return Attributes[i].Value;
	if (Required)
		Sys_Error("XML attribute %s not found", AttrName);
	return VStr();
	unguard;
}

//==========================================================================
//
//	VXmlNode::GetAttribute
//
//==========================================================================

VStr VXmlNode::GetAttribute(const VStr& AttrName, bool Required) const
{
	guard(VXmlNode::GetAttribute);
	for (int i = 0; i < Attributes.Num(); i++)
		if (Attributes[i].Name == AttrName)
			return Attributes[i].Value;
	if (Required)
		Sys_Error("XML attribute %s not found", *AttrName);
	return VStr();
	unguard;
}

//==========================================================================
//
//	VXmlDocument::Parse
//
//==========================================================================

void VXmlDocument::Parse(VStream& Strm, VStr AName)
{
	guard(VXmlDocument::Parse);
	Name = AName;

	Buf = new char[Strm.TotalSize() + 1];
	Strm.Seek(0);
	Strm.Serialise(Buf, Strm.TotalSize());
	Buf[Strm.TotalSize()] = 0;
	CurPos = 0;
	EndPos = Strm.TotalSize();

	//	Skip garbage some editors add in the begining of UTF-8 files.
	if ((vuint8)Buf[0] == 0xef && (vuint8)Buf[1] == 0xbb && (vuint8)Buf[2] == 0xbf)
	{
		CurPos += 3;
	}

	do
	{
		SkipWhitespace();
	} while (SkipComment());

	if (CurPos >= EndPos)
	{
		Error("Empty document");
	}

	if (!(Buf[CurPos] == '<' && Buf[CurPos + 1] == '?' && Buf[CurPos + 2] == 'x' &&
		Buf[CurPos + 3] == 'm' && Buf[CurPos + 4] == 'l' && Buf[CurPos + 5] > 0 && Buf[CurPos + 5] <= ' '))
	{
		Error("XML declaration expected");
	}
	CurPos += 5;
	SkipWhitespace();
	VStr AttrName;
	VStr AttrValue;
	if (!ParseAttribute(AttrName, AttrValue))
	{
		Error("XML version expected");
	}
	if (AttrName != "version")
	{
		Error("XML version expected");
	}
	if (AttrValue != "1.0" && AttrValue != "1.1")
	{
		Error("Bad XML version");
	}
	SkipWhitespace();
	while (ParseAttribute(AttrName, AttrValue))
	{
		if (AttrName == "encoding")
		{
			if (AttrValue.ToUpper() != "UTF-8")
			{
				Error("Only UTF-8 is supported");
			}
		}
		else if (AttrName == "standalone")
		{
			if (AttrValue.ToLower() != "yes")
			{
				Error("Only standalone is supported");
			}
		}
		else
		{
			Error("Unknown attribute");
		}
		SkipWhitespace();
	}
	if (Buf[CurPos] != '?' || Buf[CurPos + 1] != '>')
	{
		Error("Bad syntax");
	}
	CurPos += 2;

	do
	{
		SkipWhitespace();
	} while (SkipComment());

	if (Buf[CurPos] != '<')
	{
		Error("Root node expected");
	}
	ParseNode(&Root);

	do
	{
		SkipWhitespace();
	} while (SkipComment());

	if (CurPos != EndPos)
	{
		Error("Text after root node");
	}
	unguard;
}

//==========================================================================
//
//	VXmlDocument::SkipWhitespace
//
//==========================================================================

void VXmlDocument::SkipWhitespace()
{
	guard(VXmlDocument::SkipWhitespace);
	while (Buf[CurPos] > 0 && Buf[CurPos] <= ' ')
	{
		CurPos++;
	}
	unguard;
}

//==========================================================================
//
//	VXmlDocument::SkipComment
//
//==========================================================================

bool VXmlDocument::SkipComment()
{
	guard(VXmlDocument::SkipComment);
	if (Buf[CurPos] == '<' && Buf[CurPos + 1] == '!' &&
		Buf[CurPos + 2] == '-' && Buf[CurPos + 3] == '-')
	{
		//	Skip comment.
		CurPos += 4;
		while (CurPos < EndPos - 2 && (Buf[CurPos] != '-' ||
			Buf[CurPos + 1] != '-' || Buf[CurPos + 2] != '>'))
		{
			CurPos++;
		}
		if (CurPos >= EndPos - 2)
		{
			Error("Unterminated comment");
		}
		CurPos += 3;
		return true;
	}
	return false;
	unguard;
}

//==========================================================================
//
//	VXmlDocument::Error
//
//==========================================================================

void VXmlDocument::Error(const char* Msg)
{
	guard(VXmlDocument::Error);
	Sys_Error("%s: %s", *Name, Msg);
	unguard;
}

//==========================================================================
//
//	VXmlDocument::ParseName
//
//==========================================================================

VStr VXmlDocument::ParseName()
{
	guard(VXmlDocument::ParseName);
	VStr Ret;
	char c = Buf[CurPos];
	if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '-' || c == ':'))
	{
		return VStr();
	}

	do
	{
		Ret += c;
		CurPos++;
		c = Buf[CurPos];
	}
	while ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-' || c == ':' || c == '.');

	return Ret;
	unguard;
}

//==========================================================================
//
//	VXmlDocument::ParseAttrValue
//
//==========================================================================

VStr VXmlDocument::ParseAttrValue(char EndChar)
{
	guard(VXmlDocument::ParseAttrValue);
	VStr Ret;
	while (CurPos < EndPos && Buf[CurPos] != EndChar)
	{
		if (Buf[CurPos] == '\r' && Buf[CurPos + 1] == '\n')
			CurPos++;
		Ret += Buf[CurPos];
		CurPos++;
	}
	if (CurPos >= EndPos)
	{
		Error("Unterminated attribute value");
	}
	CurPos++;
	return HandleReferences(Ret);
	unguard;
}

//==========================================================================
//
//	VXmlDocument::ParseAttribute
//
//==========================================================================

bool VXmlDocument::ParseAttribute(VStr& AttrName, VStr& AttrValue)
{
	guard(VXmlDocument::ParseAttribute);
	AttrName = ParseName();
	if (!AttrName)
	{
		return false;
	}
	SkipWhitespace();
	if (Buf[CurPos] != '=')
	{
		Error("Attribute value expected");
	}
	CurPos++;
	SkipWhitespace();
	if (CurPos >= EndPos)
	{
		Error("unexpected end of document");
	}
	if (Buf[CurPos] == '\"' || Buf[CurPos] == '\'')
	{
		CurPos++;
		AttrValue = ParseAttrValue(Buf[CurPos - 1]);
	}
	else
	{
		Error("Unquoted attribute value");
	}
	return true;
	unguard;
}

//==========================================================================
//
//	VXmlDocument::ParseNode
//
//==========================================================================

void VXmlDocument::ParseNode(VXmlNode* Node)
{
	guard(VXmlDocument::ParseNode);
	if (Buf[CurPos] != '<')
	{
		Error("Bad tag start");
	}
	CurPos++;
	Node->Name = ParseName();
	if (!Node->Name)
	{
		Error("Bad or missing tag name");
	}

	SkipWhitespace();
	VStr AttrName;
	VStr AttrValue;
	while (ParseAttribute(AttrName, AttrValue))
	{
		VXmlAttribute& A = Node->Attributes.Alloc();
		A.Name = AttrName;
		A.Value = AttrValue;
		SkipWhitespace();
	}

	if (Buf[CurPos] == '/' && Buf[CurPos + 1] == '>')
	{
		CurPos += 2;
	}
	else if (Buf[CurPos] == '>')
	{
		CurPos++;
		while (CurPos < EndPos && (Buf[CurPos] != '<' || Buf[CurPos + 1] != '/'))
		{
			if (Buf[CurPos] == '<')
			{
				if (Buf[CurPos + 1] == '!' && Buf[CurPos + 2] == '-' && Buf[CurPos + 3] == '-')
				{
					SkipComment();
				}
				else if (Buf[CurPos + 1] == '!' && Buf[CurPos + 2] == '[' &&
					Buf[CurPos + 3] == 'C' && Buf[CurPos + 4] == 'D' &&
					Buf[CurPos + 5] == 'A' && Buf[CurPos + 6] == 'T' &&
					Buf[CurPos + 7] == 'A' && Buf[CurPos + 8] == '[')
				{
					CurPos += 9;
					VStr TextVal;
					while (CurPos < EndPos && (Buf[CurPos] != ']' ||
						Buf[CurPos + 1] != ']' || Buf[CurPos + 2] != '>'))
					{
						if (Buf[CurPos] == '\r' && Buf[CurPos + 1] == '\n')
							CurPos++;
						TextVal += Buf[CurPos];
						CurPos++;
					}
					if (CurPos >= EndPos)
					{
						Error("Unexpected end of file in CDATA");
					}
					Node->Value += TextVal;
					CurPos += 3;
				}
				else
				{
					VXmlNode* NewChild = new VXmlNode();
					NewChild->PrevSibling = Node->LastChild;
					if (Node->LastChild)
						Node->LastChild->NextSibling = NewChild;
					else
						Node->FirstChild = NewChild;
					Node->LastChild = NewChild;
					NewChild->Parent = Node;
					ParseNode(NewChild);
				}
			}
			else
			{
				VStr TextVal;
				bool HasNonWhitespace = false;
				while (CurPos < EndPos && Buf[CurPos] != '<')
				{
					if (Buf[CurPos] == '\r' && Buf[CurPos + 1] == '\n')
						CurPos++;
					TextVal += Buf[CurPos];
					if (Buf[CurPos] < 0 || Buf[CurPos] > ' ')
						HasNonWhitespace = true;
					CurPos++;
				}
				if (HasNonWhitespace)
				{
					Node->Value += HandleReferences(TextVal);
				}
			}
		}
		if (CurPos >= EndPos)
		{
			Error("Unexpected end of file");
		}
		CurPos += 2;
		VStr Test = ParseName();
		if (Node->Name != Test)
		{
			Error("Wrong end tag");
		}
		if (Buf[CurPos] != '>')
		{
			Error("Tag not closed");
		}
		CurPos++;
	}
	else
	{
		Error("Tag is not closed");
	}
	unguard;
}

//==========================================================================
//
//	VXmlDocument::HandleReferences
//
//==========================================================================

VStr VXmlDocument::HandleReferences(const VStr& AStr)
{
	guard(VXmlDocument::HandleReferences);
	VStr Ret = AStr;
	for (size_t i = 0; i < Ret.Length(); i++)
	{
		if (Ret[i] == '&')
		{
			size_t EndPos = i + 1;
			while (EndPos < Ret.Length() && Ret[EndPos] != ';')
			{
				EndPos++;
			}
			if (EndPos >= Ret.Length())
			{
				Error("Unterminated character or entity reference");
			}
			EndPos++;
			VStr Seq = VStr(Ret, i, EndPos - i);
			VStr NewVal;
			if (Seq[1] == '#' && Seq[2] == 'x')
			{
				int Val = 0;
				for (size_t j = 3; j < Seq.Length() - 1; j++)
				{
					if (Seq[j] >= '0' && Seq[j] < '9')
						Val = (Val << 4) + Seq[j] - '0';
					else if (Seq[j] >= 'a' && Seq[j] < 'f')
						Val = (Val << 4) + Seq[j] - 'a' + 10;
					else if (Seq[j] >= 'A' && Seq[j] < 'F')
						Val = (Val << 4) + Seq[j] - 'A' + 10;
					else
						Error("Bad character reference");
				}
				NewVal = VStr::FromChar(Val);
			}
			else if (Seq[1] == '#')
			{
				int Val = 0;
				for (size_t j = 2; j < Seq.Length() - 1; j++)
				{
					if (Seq[j] >= '0' && Seq[j] < '9')
						Val = Val * 10 + Seq[j] - '0';
					else
						Error("Bad character reference");
				}
				NewVal = VStr::FromChar(Val);
			}
			else if (Seq == "&amp;")
				NewVal = "&";
			else if (Seq == "&quot;")
				NewVal = "\"";
			else if (Seq == "&apos;")
				NewVal = "\'";
			else if (Seq == "&lt;")
				NewVal = "<";
			else if (Seq == "&gt;")
				NewVal = ">";
			else
				Error("Unknown entity reference");
			Ret = VStr(Ret, 0, i) + NewVal + VStr(Ret, EndPos, Ret.Length() - EndPos);
			i += NewVal.Length() - 1;
		}
	}
	return Ret;
	unguard;
}

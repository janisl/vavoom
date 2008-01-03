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

class VXmlAttribute
{
public:
	VStr					Name;
	VStr					Value;
};

class VXmlNode
{
public:
	VStr					Name;
	VStr					Value;
	VXmlNode*				Parent;
	VXmlNode*				FirstChild;
	VXmlNode*				LastChild;
	VXmlNode*				PrevSibling;
	VXmlNode*				NextSibling;
	TArray<VXmlAttribute>	Attributes;

	VXmlNode();
	~VXmlNode();

	VXmlNode* FindChild(const char*) const;
	VXmlNode* FindChild(const VStr&) const;
	VXmlNode* GetChild(const char*) const;
	VXmlNode* GetChild(const VStr&) const;
	VXmlNode* FindNext(const char*) const;
	VXmlNode* FindNext(const VStr&) const;
	VXmlNode* FindNext() const;
	bool HasAttribute(const char*) const;
	bool HasAttribute(const VStr&) const;
	VStr GetAttribute(const char*, bool = true) const;
	VStr GetAttribute(const VStr&, bool = true) const;
};

class VXmlDocument
{
public:
	VStr					Name;
	VXmlNode				Root;

	void Parse(VStream&, VStr);

private:
	char*					Buf;
	int						CurPos;
	int						EndPos;

	void SkipWhitespace();
	bool SkipComment();
	void Error(const char*);
	VStr ParseName();
	VStr ParseAttrValue(char);
	bool ParseAttribute(VStr&, VStr&);
	void ParseNode(VXmlNode*);
	VStr HandleReferences(const VStr&);
};

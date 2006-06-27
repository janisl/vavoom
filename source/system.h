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

int Sys_FileOpenRead(const VStr&);
int Sys_FileOpenWrite(const VStr&);
int Sys_FileRead(int, void*, int);
int Sys_FileWrite(int, const void*, int);
int Sys_FileSize(int);
int Sys_FileSeek(int, int);
int Sys_FileClose(int);
int Sys_FileExists(const VStr&);
int Sys_FileTime(const VStr&);

int Sys_CreateDirectory(const VStr&);
int Sys_OpenDir(const VStr&);
VStr Sys_ReadDir();
void Sys_CloseDir();
bool Sys_DirExists(const VStr&);

void __attribute__((noreturn, format(printf, 1, 2))) __declspec(noreturn) 
	Sys_Error(const char*, ...);
void __attribute__((noreturn)) __declspec(noreturn) Sys_Quit(const char*);
void Sys_Shutdown();

double Sys_Time();
void Sys_Sleep();

char *Sys_ConsoleInput();

extern "C" {

void Sys_LowFPPrecision();
void Sys_HighFPPrecision();

}

//
// memory protection
//
void Sys_MakeCodeWriteable(unsigned long, unsigned long);

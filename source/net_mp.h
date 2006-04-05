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

int  MPATH_Init();
void MPATH_Shutdown();
void MPATH_Listen(bool state);
int  MPATH_OpenSocket(int port);
int  MPATH_CloseSocket(int socket);
int  MPATH_Connect(int socket, sockaddr_t* addr);
int  MPATH_CheckNewConnections();
int  MPATH_Read(int socket, byte* buf, int len, sockaddr_t* addr);
int  MPATH_Write(int socket, byte* buf, int len, sockaddr_t* addr);
int  MPATH_Broadcast(int socket, byte* buf, int len);
char *MPATH_AddrToString(sockaddr_t* addr);
int  MPATH_StringToAddr(const char* string, sockaddr_t* addr);
int  MPATH_GetSocketAddr(int socket, sockaddr_t* addr);
int  MPATH_GetNameFromAddr(sockaddr_t* addr, char* name);
int  MPATH_GetAddrFromName(const char* name, sockaddr_t* addr);
int  MPATH_AddrCompare(sockaddr_t* addr1, sockaddr_t* addr2);
int  MPATH_GetSocketPort(sockaddr_t* addr);
int  MPATH_SetSocketPort(sockaddr_t* addr, int port);

//**************************************************************************
//
//	$Log$
//	Revision 1.5  2006/04/05 17:20:37  dj_jl
//	Merged size buffer with message class.
//
//	Revision 1.4  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************

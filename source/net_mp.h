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
//**	Copyright (C) 1999-2001 JÆnis Legzdi·ý
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
//**	$Log$
//**	Revision 1.2  2001/07/27 14:27:54  dj_jl
//**	Update with Id-s and Log-s, some fixes
//**
//**************************************************************************

int  MPATH_Init(void);
void MPATH_Shutdown(void);
void MPATH_Listen(boolean state);
int  MPATH_OpenSocket(int port);
int  MPATH_CloseSocket(int socket);
int  MPATH_Connect(int socket, sockaddr_t *addr);
int  MPATH_CheckNewConnections(void);
int  MPATH_Read(int socket, byte *buf, int len, sockaddr_t *addr);
int  MPATH_Write(int socket, byte *buf, int len, sockaddr_t *addr);
int  MPATH_Broadcast(int socket, byte *buf, int len);
char *MPATH_AddrToString(sockaddr_t *addr);
int  MPATH_StringToAddr(char *string, sockaddr_t *addr);
int  MPATH_GetSocketAddr(int socket, sockaddr_t *addr);
int  MPATH_GetNameFromAddr(sockaddr_t *addr, char *name);
int  MPATH_GetAddrFromName(char *name, sockaddr_t *addr);
int  MPATH_AddrCompare(sockaddr_t *addr1, sockaddr_t *addr2);
int  MPATH_GetSocketPort(sockaddr_t *addr);
int  MPATH_SetSocketPort(sockaddr_t *addr, int port);

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

int  IPX_Init(void);
void IPX_Shutdown(void);
void IPX_Listen(boolean state);
int  IPX_OpenSocket(int port);
int  IPX_CloseSocket(int socket);
int  IPX_Connect(int socket, sockaddr_t *addr);
int  IPX_CheckNewConnections(void);
int  IPX_Read(int socket, byte *buf, int len, sockaddr_t *addr);
int  IPX_Write(int socket, byte *buf, int len, sockaddr_t *addr);
int  IPX_Broadcast(int socket, byte *buf, int len);
char *IPX_AddrToString(sockaddr_t *addr);
int  IPX_StringToAddr(char *string, sockaddr_t *addr);
int  IPX_GetSocketAddr(int socket, sockaddr_t *addr);
int  IPX_GetNameFromAddr(sockaddr_t *addr, char *name);
int  IPX_GetAddrFromName(char *name, sockaddr_t *addr);
int  IPX_AddrCompare(sockaddr_t *addr1, sockaddr_t *addr2);
int  IPX_GetSocketPort(sockaddr_t *addr);
int  IPX_SetSocketPort(sockaddr_t *addr, int port);

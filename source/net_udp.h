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
//**************************************************************************

int  UDP_Init(void);
void UDP_Shutdown(void);
void UDP_Listen(boolean state);
int  UDP_OpenSocket(int port);
int  UDP_CloseSocket(int socket);
int  UDP_Connect(int socket, sockaddr_t *addr);
int  UDP_CheckNewConnections(void);
int  UDP_Read(int socket, byte *buf, int len, sockaddr_t *addr);
int  UDP_Write(int socket, byte *buf, int len, sockaddr_t *addr);
int  UDP_Broadcast(int socket, byte *buf, int len);
char *UDP_AddrToString(sockaddr_t *addr);
int  UDP_StringToAddr(char *string, sockaddr_t *addr);
int  UDP_GetSocketAddr(int socket, sockaddr_t *addr);
int  UDP_GetNameFromAddr(sockaddr_t *addr, char *name);
int  UDP_GetAddrFromName(char *name, sockaddr_t *addr);
int  UDP_AddrCompare(sockaddr_t *addr1, sockaddr_t *addr2);
int  UDP_GetSocketPort(sockaddr_t *addr);
int  UDP_SetSocketPort(sockaddr_t *addr, int port);

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************

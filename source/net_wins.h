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

int  WINS_Init(void);
void WINS_Shutdown(void);
void WINS_Listen(boolean state);
int  WINS_OpenSocket(int port);
int  WINS_CloseSocket(int socket);
int  WINS_Connect(int socket, sockaddr_t *addr);
int  WINS_CheckNewConnections(void);
int  WINS_Read(int socket, byte *buf, int len, sockaddr_t *addr);
int  WINS_Write(int socket, byte *buf, int len, sockaddr_t *addr);
int  WINS_Broadcast(int socket, byte *buf, int len);
char *WINS_AddrToString(sockaddr_t *addr);
int  WINS_StringToAddr(char *string, sockaddr_t *addr);
int  WINS_GetSocketAddr(int socket, sockaddr_t *addr);
int  WINS_GetNameFromAddr(sockaddr_t *addr, char *name);
int  WINS_GetAddrFromName(char *name, sockaddr_t *addr);
int  WINS_AddrCompare(sockaddr_t *addr1, sockaddr_t *addr2);
int  WINS_GetSocketPort(sockaddr_t *addr);
int  WINS_SetSocketPort(sockaddr_t *addr, int port);

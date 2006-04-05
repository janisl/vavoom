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

int			NetNull_Init();
void		NetNull_Listen(bool state);
void		NetNull_SearchForHosts(bool xmit);
qsocket_t*	NetNull_Connect(const char* host);
qsocket_t*	NetNull_CheckNewConnections();
int			NetNull_GetMessage(qsocket_t* sock);
int			NetNull_SendMessage(qsocket_t* sock, VMessage* data);
int			NetNull_SendUnreliableMessage(qsocket_t* sock, VMessage* data);
bool		NetNull_CanSendMessage(qsocket_t* sock);
bool		NetNull_CanSendUnreliableMessage(qsocket_t* sock);
void		NetNull_Close(qsocket_t* sock);
void		NetNull_Shutdown();

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2006/04/05 17:20:37  dj_jl
//	Merged size buffer with message class.
//
//	Revision 1.2  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//	
//	Revision 1.1  2001/12/01 17:40:41  dj_jl
//	Added support for bots
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************

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

//int			Serial_Init();
//void		Serial_Listen(bool state);
//void		Serial_SearchForHosts(bool xmit);
qsocket_t	*Serial_Connect(const char* host);
//qsocket_t 	*Serial_CheckNewConnections();
int			Serial_GetMessage(qsocket_t* sock);
int			Serial_SendMessage(qsocket_t* sock, VMessage* data);
int			Serial_SendUnreliableMessage(qsocket_t* sock, VMessage* data);
bool		Serial_CanSendMessage(qsocket_t* sock);
bool		Serial_CanSendUnreliableMessage(qsocket_t* sock);
//void		Serial_Close (qsocket_t* sock);
//void		Serial_Shutdown();

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

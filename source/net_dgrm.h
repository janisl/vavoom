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

int			Datagram_Init(void);
void		Datagram_Listen(boolean state);
void		Datagram_SearchForHosts(boolean xmit);
qsocket_t	*Datagram_Connect(char *host);
qsocket_t 	*Datagram_CheckNewConnections(void);
int			Datagram_GetMessage(qsocket_t *sock);
int			Datagram_SendMessage(qsocket_t *sock, TSizeBuf *data);
int			Datagram_SendUnreliableMessage(qsocket_t *sock, TSizeBuf *data);
boolean		Datagram_CanSendMessage(qsocket_t *sock);
boolean		Datagram_CanSendUnreliableMessage(qsocket_t *sock);
void		Datagram_Close(qsocket_t *sock);
void		Datagram_Shutdown(void);

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

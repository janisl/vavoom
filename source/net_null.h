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

int			NetNull_Init(void);
void		NetNull_Listen(boolean state);
void		NetNull_SearchForHosts(boolean xmit);
qsocket_t 	*NetNull_Connect(char *host);
qsocket_t 	*NetNull_CheckNewConnections(void);
int			NetNull_GetMessage(qsocket_t *sock);
int			NetNull_SendMessage(qsocket_t *sock, TSizeBuf *data);
int			NetNull_SendUnreliableMessage(qsocket_t *sock, TSizeBuf *data);
boolean		NetNull_CanSendMessage(qsocket_t *sock);
boolean		NetNull_CanSendUnreliableMessage(qsocket_t *sock);
void		NetNull_Close(qsocket_t *sock);
void		NetNull_Shutdown(void);

//**************************************************************************
//
//	$Log$
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

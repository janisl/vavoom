//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
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

int			Loop_Init(void);
void		Loop_Listen(boolean state);
void		Loop_SearchForHosts(boolean xmit);
qsocket_t 	*Loop_Connect(char *host);
qsocket_t 	*Loop_CheckNewConnections(void);
int			Loop_GetMessage(qsocket_t *sock);
int			Loop_SendMessage(qsocket_t *sock, TSizeBuf *data);
int			Loop_SendUnreliableMessage(qsocket_t *sock, TSizeBuf *data);
boolean		Loop_CanSendMessage(qsocket_t *sock);
boolean		Loop_CanSendUnreliableMessage(qsocket_t *sock);
void		Loop_Close(qsocket_t *sock);
void		Loop_Shutdown(void);

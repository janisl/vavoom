//----------------------------------------------------------------------------
//  EDGE Networking stuff
//----------------------------------------------------------------------------
// 
//  Copyright (c) 1999-2001  The EDGE Team.
// 
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//----------------------------------------------------------------------------
//
//  Based on the DOOM source code, released by Id Software under the
//  following copyright:
//
//    Copyright (C) 1993-1996 by id Software, Inc.
//
//----------------------------------------------------------------------------

#ifndef __E_NET_H__
#define __E_NET_H__

#include "e_player.h"


#define DEBUG_NET  0


//
// Network play related stuff.
// There is a data struct that stores network
//  communication related stuff, and another
//  one that defines the actual packets to
//  be transmitted.
//

#define DOOMCOM_ID		0x12345678l

// Max computers/players in a game.
#define MAXNETNODES		8

typedef enum
{
  CMD_SEND = 1,
  CMD_GET = 2

}
command_t;

//
// Network packet data.
//
typedef struct
{
  // High bit is retransmit request.
  unsigned checksum;
  // Only valid if NCMD_RETRANSMIT.
  byte retransmitfrom;
  byte starttic;
  byte player;
  byte numtics;

  ticcmd_t cmds[BACKUPTICS];
}
doomdata_t;

typedef struct setup_info_s
{
  byte version;
  byte skill;
  byte deathmatch;
  byte drone;
  gameflags_t setupflags;
  long random_seed;
  // variable length
  char startmap[1];
}
setup_info_t;

typedef struct
{
  unsigned checksum;
  byte retransmitfrom;
  byte starttic;
  byte player;
  byte numtics;

  setup_info_t info;
}
setupdata_t;

typedef struct
{
  // Supposed to be DOOMCOM_ID?
  long id;

  // DOOM executes an int to execute commands.
  short intnum;
  // Communication between DOOM and the driver.
  // Is CMD_SEND or CMD_GET.
  short command;
  // Is dest for send, set by get (-1 = no packet).
  short remotenode;

  // Number of bytes in doomdata to be sent
  short datalength;

  // Info common to all nodes.
  // Console is allways node 0.
  short numnodes;
  // Flag: 1 = no duplication, 2-5 = dup for slow nets.
  short ticdup;
  // Flag: 1 = send a backup tic in every packet.
  short extratics;
  // Flag: 1 = deathmatch.
  short deathmatch;
  // Flag: -1 = new game, 0-5 = load savegame
  short savegame;
  short episode;  // 1-3

  short map;  // 1-9

  short skill;  // 1-5

  // Info specific to this node.
  short consoleplayer;
  short numplayers;

  // These are related to the 3-display mode,
  //  in which two drones looking left and right
  //  were used to render two additional views
  //  on two additional computers.
  // Probably not operational anymore.
  // 1 = left, 0 = center, -1 = right
  short angleoffset;
  // 1 = drone
  short drone;

  // The packet data to be sent.
  doomdata_t data;

}
doomcom_t;

// The current time. Used by bot code.
extern int gametime;

// Create any new ticcmds and broadcast to other players.
void E_NetUpdate(void);

// Broadcasts special packets to other players
//  to notify of game exit
void E_QuitNetGame(void);

// Get the random seed to use for a game.
long E_NetGetRandomSeed(void);

//? how many ticks to run?
void E_TryRunTics(void);

// Sends a setup packet to sync settings globally.
void E_SendSetupPacket(int flags);

#endif

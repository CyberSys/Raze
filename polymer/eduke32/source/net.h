//-------------------------------------------------------------------------
/*
Copyright (C) 2010 EDuke32 developers and contributors

This file is part of EDuke32.

EDuke32 is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 2
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
//-------------------------------------------------------------------------

#ifndef __netplay_h__
#define __netplay_h__

#include "enet/enet.h"

#define NET_REVISIONS 64

enum netchan_t
{
    CHAN_MOVE,      // unreliable movement packets
    CHAN_GAMESTATE, // gamestate changes... frags, respawns, player names, etc
    CHAN_SYNC,      // client join sync packets
    CHAN_CHAT,      // chat and RTS
    CHAN_MISC,      // whatever else
    CHAN_MAX
};

enum DukePacket_t
{
    PACKET_MASTER_TO_SLAVE,
    PACKET_SLAVE_TO_MASTER,

    PACKET_NUM_PLAYERS,
    PACKET_PLAYER_INDEX,
    PACKET_PLAYER_DISCONNECTED,
    PACKET_PLAYER_SPAWN,
    PACKET_FRAG,
    PACKET_REQUEST_GAMESTATE,
    PACKET_VERSION,
    PACKET_AUTH,
    PACKET_PLAYER_PING,
    PACKET_PLAYER_READY,
    PACKET_MAP_STREAM,

    // any packet with an ID higher than PACKET_BROADCAST is rebroadcast by server
    // so hacked clients can't create fake server packets and get the server to
    // send them to everyone
    // newer versions of the netcode also make this determination based on which
    // channel the packet was broadcast on

    PACKET_BROADCAST,
    PACKET_NEW_GAME,
    PACKET_RTS,
    PACKET_CLIENT_INFO,
    PACKET_MESSAGE,
    PACKET_USER_MAP,

    PACKET_MAP_VOTE,
    PACKET_MAP_VOTE_INITIATE,
    PACKET_MAP_VOTE_CANCEL,
};

enum netdisconnect_t
{
    DISC_BAD_PASSWORD = 1,
    DISC_KICKED,
    DISC_BANNED
};

enum netmode_t
{
    NET_CLIENT = 0,
    NET_SERVER,
    NET_DEDICATED_CLIENT, // client on dedicated server
    NET_DEDICATED_SERVER
};

#pragma pack(push,1)
typedef struct {
    uint32_t revision;
    int32_t animategoal[MAXANIMATES], animatevel[MAXANIMATES], g_animateCount;
    int32_t animateptr[MAXANIMATES];
    int32_t msx[2048], msy[2048];
    int32_t randomseed, g_globalRandom;

    int16_t SpriteDeletionQueue[1024],g_spriteDeleteQueuePos;
    int16_t animatesect[MAXANIMATES];
    int16_t cyclers[MAXCYCLERS][6];
    int16_t g_mirrorWall[64], g_mirrorSector[64], g_mirrorCount;
    int16_t g_numAnimWalls;
    int16_t g_numCyclers;
    int16_t headspritesect[MAXSECTORS+1];
    int16_t headspritestat[MAXSTATUS+1];
    int16_t nextspritesect[MAXSPRITES];
    int16_t nextspritestat[MAXSPRITES];
    int16_t numsectors;
    int16_t numwalls;
    int16_t prevspritesect[MAXSPRITES];
    int16_t prevspritestat[MAXSPRITES];

    uint8_t g_earthquakeTime;
    int8_t g_numPlayerSprites;
    uint8_t scriptptrs[MAXSPRITES];

    netactor_t actor[MAXSPRITES];
    playerspawn_t g_playerSpawnPoints[MAXPLAYERS];
    animwalltype animwall[MAXANIMWALLS];
    sectortype sector[MAXSECTORS];
    spriteext_t spriteext[MAXSPRITES];
    spritetype sprite[MAXSPRITES];
    walltype wall[MAXWALLS];
    uint32_t crc;
} netmapstate_t;

extern netmapstate_t  *g_multiMapState[MAXPLAYERS];
extern netmapstate_t  *g_multiMapRevisions[NET_REVISIONS];
#pragma pack(pop)

extern uint32_t        g_netMapRevision;
extern ENetHost       *g_netClient;
extern ENetHost       *g_netServer;
extern ENetPeer       *g_netClientPeer;
extern char           g_netPassword[32];
extern int32_t        g_netDisconnect;
extern int32_t        g_netPlayersWaiting;
extern int32_t        g_netPort;
extern int32_t        g_networkMode;
extern int32_t        lastsectupdate[MAXSECTORS];
extern int32_t        lastupdate[MAXSPRITES];
extern int32_t        lastwallupdate[MAXWALLS];
extern int16_t        g_netStatnums[10];

#pragma pack(push,1)
typedef struct {
	vec3_t pos;
	vec3_t opos;
	vec3_t vel;
	int16_t ang;
	int16_t horiz;
	int16_t horizoff;
	int16_t ping;
	int16_t playerindex;
	int16_t deadflag;
	int16_t playerquitflag;
} playerupdate_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
	int8_t header;
	int8_t connection;
	int8_t level_number;
	int8_t volume_number;
	int8_t player_skill;
	int8_t monsters_off;
	int8_t respawn_monsters;
	int8_t respawn_items;
	int8_t respawn_inventory;
	int8_t marker;
	int8_t ffire;
	int8_t noexits;
	int8_t coop;
} newgame_t;
#pragma pack(pop)

newgame_t pendingnewgame;

// Connect/Disconnect
void    Net_Connect(const char *srvaddr);
void    Net_Disconnect(void);
void    Net_ReceiveDisconnect(ENetEvent *event);

// Packet Handlers
void    Net_GetPackets(void);
void    Net_HandleServerPackets(void);
void    Net_HandleClientPackets(void);
void    Net_ParseClientPacket(ENetEvent *event);
void    Net_ParseServerPacket(ENetEvent *event);
void    Net_ParsePacketCommon(uint8_t *pbuf, int32_t packbufleng, int32_t serverpacketp);

void    Net_SendVersion(ENetPeer *client);
void    Net_RecieveVersion(uint8_t *pbuf, int32_t packbufleng);

void    Net_SendChallenge();
void    Net_RecieveChallenge(uint8_t *pbuf, int32_t packbufleng, ENetEvent *event);

void    Net_SendNewPlayer(int32_t newplayerindex);
void    Net_RecieveNewPlayer(uint8_t *pbuf, int32_t packbufleng);

void    Net_SendPlayerIndex(int32_t index, ENetPeer *peer);
void    Net_RecievePlayerIndex(uint8_t *pbuf, int32_t packbufleng);

void    Net_SendClientInfo(void);
void    Net_ReceiveClientInfo(uint8_t *pbuf, int32_t packbufleng, int32_t fromserver);

void    Net_SendUserMapName(void);
void    Net_ReceiveUserMapName(uint8_t *pbuf, int32_t packbufleng);

void    Net_SendClientSync(ENetEvent *event, int32_t player);
void    Net_ReceiveClientSync(ENetEvent *event);

void    Net_SendMapUpdate(void);
void    Net_ReceiveMapUpdate(uint8_t *pbuf, int32_t packbufleng);

void    Net_FillPlayerUpdate(playerupdate_t *update, int32_t player);
void    Net_ExtractPlayerUpdate(playerupdate_t *update);

void    Net_SendServerUpdates(void);
void    Net_ReceiveServerUpdate(ENetEvent *event);

void    Net_SendClientUpdate(void);
void    Net_ReceiveClientUpdate(ENetEvent *event);

void    Net_SendMessage(void);
void    Net_ReceiveMessage(uint8_t *pbuf, int32_t packbufleng);

void    Net_StartNewGame();
void    Net_SendNewGame(int32_t frommenu, ENetPeer *peer);
void    Net_ReceiveNewGame(ENetEvent *event);

void    Net_FillNewGame(newgame_t* newgame, int32_t frommenu);
void    Net_ExtractNewGame(newgame_t* newgame, int32_t menuonly);

void    Net_SendMapVoteInitiate(void);
void    Net_RecieveMapVoteInitiate(uint8_t *pbuf);

void    Net_SendMapVote(int32_t votefor);
void    Net_RecieveMapVote(uint8_t *pbuf);
void    Net_CheckForEnoughVotes();

void    Net_SendMapVoteCancel(int32_t failed);
void    Net_RecieveMapVoteCancel(uint8_t *pbuf);

//////////

void    Net_ResetPrediction(void);
void    Net_RestoreMapState(netmapstate_t *save);
void    Net_SyncPlayer(ENetEvent *event);
void    Net_WaitForServer(void);
void    faketimerhandler(void);

#endif // __netplay_h__

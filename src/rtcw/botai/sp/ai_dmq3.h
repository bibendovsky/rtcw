/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		ai_dmq3.h
 *
 * desc:		Quake3 bot AI
 *
 *
 *****************************************************************************/

//setup the deathmatch AI
void BotSetupDeathmatchAI( void );
//shutdown the deathmatch AI
void BotShutdownDeathmatchAI( void );
//let the bot live within it's deathmatch AI net
void BotDeathmatchAI( bot_state_t *bs, float thinktime );
//free waypoints
void BotFreeWaypoints( bot_waypoint_t *wp );
//choose a weapon
void BotChooseWeapon( bot_state_t *bs );
//setup movement stuff
void BotSetupForMovement( bot_state_t *bs );
//update the inventory
void BotUpdateInventory( bot_state_t *bs );
//update the inventory during battle
void BotUpdateBattleInventory( bot_state_t *bs, int enemy );
//use holdable items during battle
void BotBattleUseItems( bot_state_t *bs );
//return true if the bot is dead
qboolean BotIsDead( bot_state_t *bs );
//returns true if the bot is in observer mode
qboolean BotIsObserver( bot_state_t *bs );
//returns true if the bot is in the intermission
qboolean BotIntermission( bot_state_t *bs );
//returns true if the bot is in lava
qboolean BotInLava( bot_state_t *bs );
//returns true if the bot is in slime
qboolean BotInSlime( bot_state_t *bs );
//returns true if the entity is dead
qboolean EntityIsDead( aas_entityinfo_t *entinfo );
//returns true if the entity is invisible
qboolean EntityIsInvisible( aas_entityinfo_t *entinfo );
//returns true if the entity is shooting
qboolean EntityIsShooting( aas_entityinfo_t *entinfo );
//returns the name of the client
const char *ClientName( int client, char *name, int size );
//returns an simplyfied client name
const char *EasyClientName( int client, char *name, int size );
//returns the skin used by the client
const char *ClientSkin( int client, char *skin, int size );
//returns the aggression of the bot in the range [0, 100]
float BotAggression( bot_state_t *bs );
//returns true if the bot wants to retreat
int BotWantsToRetreat( bot_state_t *bs );
//returns true if the bot wants to chase
int BotWantsToChase( bot_state_t *bs );
//returns true if the bot wants to help
int BotWantsToHelp( bot_state_t *bs );
//returns true if the bot can and wants to rocketjump
int BotCanAndWantsToRocketJump( bot_state_t *bs );
//returns true if the bot wants to and goes camping
int BotWantsToCamp( bot_state_t *bs );
//the bot will perform attack movements
bot_moveresult_t BotAttackMove( bot_state_t *bs, int tfl );
//returns true if the bot and the entity are in the same team
int BotSameTeam( bot_state_t *bs, int entnum );
//returns true if teamplay is on
int TeamPlayIsOn( void );
//returns true and sets the .enemy field when an enemy is found
int BotFindEnemy( bot_state_t *bs, int curenemy );
//returns a roam goal
void BotRoamGoal( bot_state_t *bs, vec3_t goal );
//returns entity visibility in the range [0, 1]
float BotEntityVisible( int viewer, vec3_t eye, vec3_t viewangles, float fov, int ent );
//the bot will aim at the current enemy
void BotAimAtEnemy( bot_state_t *bs );
//check if the bot should attack
void BotCheckAttack( bot_state_t *bs );
//AI when the bot is blocked
void BotAIBlocked( bot_state_t *bs, bot_moveresult_t *moveresult, int activate );
//returns the CTF team the bot is in
int BotCTFTeam( bot_state_t *bs );
//returns the flag the bot is carrying (CTFFLAG_?)
int BotCTFCarryingFlag( bot_state_t *bs );
//set ctf goals (defend base, get enemy flag) during seek
void BotCTFSeekGoals( bot_state_t *bs );
//set ctf goals (defend base, get enemy flag) during retreat
void BotCTFRetreatGoals( bot_state_t *bs );
//create a new waypoint
bot_waypoint_t *BotCreateWayPoint( char *name, vec3_t origin, int areanum );
//find a waypoint with the given name
bot_waypoint_t *BotFindWayPoint( bot_waypoint_t *waypoints, char *name );
//strstr but case insensitive
char *stristr( char *str, char *charset );
//returns the number of the client with the given name
int ClientFromName( char *name );
//
int BotPointAreaNum( vec3_t origin );
//
void BotMapScripts( bot_state_t *bs );

//ctf flags
#define CTF_FLAG_NONE       0
#define CTF_FLAG_RED        1
#define CTF_FLAG_BLUE       2
//CTF skins
#define CTF_SKIN_REDTEAM    "red"
#define CTF_SKIN_BLUETEAM   "blue"
//CTF teams
#define CTF_TEAM_NONE       0
#define CTF_TEAM_RED        1
#define CTF_TEAM_BLUE       2

extern int dmflags;         //deathmatch flags
extern int gametype;        //game type

// Rafael gameskill
extern int gameskill;
// done

extern vmCvar_t bot_grapple;
extern vmCvar_t bot_rocketjump;
extern vmCvar_t bot_fastchat;
extern vmCvar_t bot_nochat;
extern vmCvar_t bot_testrchat;

extern bot_goal_t ctf_redflag;
extern bot_goal_t ctf_blueflag;


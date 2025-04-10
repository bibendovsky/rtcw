/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		ai_dmnet.h
 *
 * desc:		Quake3 bot AI
 *
 *
 *****************************************************************************/

#define MAX_NODESWITCHES    50

void AIEnter_Intermission( bot_state_t *bs );
void AIEnter_Observer( bot_state_t *bs );
void AIEnter_Respawn( bot_state_t *bs );
void AIEnter_Stand( bot_state_t *bs );
void AIEnter_Seek_ActivateEntity( bot_state_t *bs );
void AIEnter_Seek_NBG( bot_state_t *bs );
void AIEnter_Seek_LTG( bot_state_t *bs );
void AIEnter_Seek_Camp( bot_state_t *bs );
void AIEnter_Battle_Fight( bot_state_t *bs );
void AIEnter_Battle_Chase( bot_state_t *bs );
void AIEnter_Battle_Retreat( bot_state_t *bs );
void AIEnter_Battle_NBG( bot_state_t *bs );
int AINode_Intermission( bot_state_t *bs );
int AINode_Observer( bot_state_t *bs );
int AINode_Respawn( bot_state_t *bs );
int AINode_Stand( bot_state_t *bs );
int AINode_Seek_ActivateEntity( bot_state_t *bs );
int AINode_Seek_NBG( bot_state_t *bs );
int AINode_Seek_LTG( bot_state_t *bs );
int AINode_Battle_Fight( bot_state_t *bs );
int AINode_Battle_Chase( bot_state_t *bs );
int AINode_Battle_Retreat( bot_state_t *bs );
int AINode_Battle_NBG( bot_state_t *bs );

void BotResetNodeSwitches( void );
void BotDumpNodeSwitches( bot_state_t *bs );


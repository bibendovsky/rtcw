/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		ai_chat.h
 *
 * desc:		Quake3 bot AI
 *
 *
 *****************************************************************************/

//
int BotChat_EnterGame( bot_state_t *bs );
//
int BotChat_ExitGame( bot_state_t *bs );
//
int BotChat_StartLevel( bot_state_t *bs );
//
int BotChat_EndLevel( bot_state_t *bs );
//
int BotChat_HitTalking( bot_state_t *bs );
//
int BotChat_HitNoDeath( bot_state_t *bs );
//
int BotChat_HitNoKill( bot_state_t *bs );
//
int BotChat_Death( bot_state_t *bs );
//
int BotChat_Kill( bot_state_t *bs );
//
int BotChat_EnemySuicide( bot_state_t *bs );
//
int BotChat_Random( bot_state_t *bs );
// time the selected chat takes to type in
float BotChatTime( bot_state_t *bs );
// returns true if the bot can chat at the current position
int BotValidChatPosition( bot_state_t *bs );
// test the initial bot chats
void BotChatTest( bot_state_t *bs );


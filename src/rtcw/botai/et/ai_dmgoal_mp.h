/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		ai_dmgoal_mp.h
 *
 * desc:		Wolf bot AI
 *
 *
 *****************************************************************************/

//
// MULTIPLAYER GOAL AI
//

qboolean BotMP_CheckEmergencyGoals( bot_state_t *bs );
qboolean BotMP_FindGoal( bot_state_t *bs );
// Gordon: new version
qboolean BotMP_FindGoal_New( bot_state_t *bs );


/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		be_ai_weap.h
 *
 * desc:		weapon AI
 *
 *
 *****************************************************************************/

//projectile flags
#define PFL_WINDOWDAMAGE            1       //projectile damages through window
#define PFL_RETURN                  2       //set when projectile returns to owner
//weapon flags
#define WFL_FIRERELEASED            1       //set when projectile is fired with key-up event
//damage types
#define DAMAGETYPE_IMPACT           1       //damage on impact
#define DAMAGETYPE_RADIAL           2       //radial damage
#define DAMAGETYPE_VISIBLE          4       //damage to all entities visible to the projectile

typedef struct projectileinfo_s
{
	char name[MAX_STRINGFIELD];
	char model[MAX_STRINGFIELD];
	int flags;
	float gravity;
	int damage;
	float radius;
	int visdamage;
	int damagetype;
	int healthinc;
	float push;
	float detonation;
	float bounce;
	float bouncefric;
	float bouncestop;
} projectileinfo_t;

typedef struct weaponinfo_s
{
	int valid;                  //true if the weapon info is valid
	int number;                                 //number of the weapon
	char name[MAX_STRINGFIELD];
	char model[MAX_STRINGFIELD];
	int level;
	int weaponindex;
	int flags;
	char projectile[MAX_STRINGFIELD];
	int numprojectiles;
	float hspread;
	float vspread;
	float speed;
	float acceleration;
	vec3_t recoil;
	vec3_t offset;
	vec3_t angleoffset;
	float extrazvelocity;
	int ammoamount;
	int ammoindex;
	float activate;
	float reload;
	float spinup;
	float spindown;
	projectileinfo_t proj;                      //pointer to the used projectile
} weaponinfo_t;

//setup the weapon AI
int BotSetupWeaponAI( void );
//shut down the weapon AI
void BotShutdownWeaponAI( void );
//returns the best weapon to fight with
int BotChooseBestFightWeapon( int weaponstate, int *inventory );
//returns the information of the current weapon
void BotGetWeaponInfo( int weaponstate, int weapon, weaponinfo_t *weaponinfo );
//loads the weapon weights
int BotLoadWeaponWeights( int weaponstate, char *filename );
//returns a handle to a newly allocated weapon state
int BotAllocWeaponState( void );
//frees the weapon state
void BotFreeWeaponState( int weaponstate );
//resets the whole weapon state
void BotResetWeaponState( int weaponstate );

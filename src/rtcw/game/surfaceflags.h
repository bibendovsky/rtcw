/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

// This file must be identical in the quake and utils directories






// contents flags are seperate bits
// a given brush can contribute multiple content bits

// these definitions also need to be in q_shared.h!

#if !defined RTCW_ET
#define CONTENTS_SOLID          1       // an eye is never valid in a solid

#define CONTENTS_LIGHTGRID      4   //----(SA)	added

#define CONTENTS_LAVA           8
#define CONTENTS_SLIME          16
#define CONTENTS_WATER          32
#define CONTENTS_FOG            64


//----(SA) added
#define CONTENTS_MISSILECLIP    128 // 0x80
#define CONTENTS_ITEM           256 // 0x100
//----(SA) end

#if defined RTCW_SP
// RF, AI sight/nosight & bullet/nobullet
#define CONTENTS_AI_NOSIGHT     0x1000  // AI cannot see through this
#define CONTENTS_CLIPSHOT       0x2000  // bullets hit this
// RF, end
#endif // RTCW_XX

#define CONTENTS_MOVER          0x4000
#define CONTENTS_AREAPORTAL     0x8000

#define CONTENTS_PLAYERCLIP     0x10000
#define CONTENTS_MONSTERCLIP    0x20000

//bot specific contents types
#define CONTENTS_TELEPORTER     0x40000
#define CONTENTS_JUMPPAD        0x80000
#define CONTENTS_CLUSTERPORTAL  0x100000
#define CONTENTS_DONOTENTER     0x200000
#define CONTENTS_DONOTENTER_LARGE       0x400000


#define CONTENTS_ORIGIN         0x1000000   // removed before bsping an entity

#define CONTENTS_BODY           0x2000000   // should never be on a brush, only in game
#define CONTENTS_CORPSE         0x4000000
#define CONTENTS_DETAIL         0x8000000   // brushes not used for the bsp

#define CONTENTS_STRUCTURAL     0x10000000  // brushes used for the bsp
#define CONTENTS_TRANSLUCENT    0x20000000  // don't consume surface fragments inside
#define CONTENTS_TRIGGER        0x40000000

#if defined RTCW_SP
#define CONTENTS_NODROP         0x80000000  // don't leave bodies or items (death fog, lava)zz
#elif defined RTCW_MP
#define CONTENTS_NODROP         0x80000000  // don't leave bodies or items (death fog, lava)
#endif // RTCW_XX

#define SURF_NODAMAGE           0x1     // never give falling damage
#define SURF_SLICK              0x2     // effects game physics
#define SURF_SKY                0x4     // lighting from environment map
#define SURF_LADDER             0x8
#define SURF_NOIMPACT           0x10    // don't make missile explosions
#define SURF_NOMARKS            0x20    // don't leave missile marks
//#define	SURF_FLESH			0x40	// make flesh sounds and effects
#define SURF_CERAMIC            0x40    // out of surf's, so replacing unused 'SURF_FLESH'
#define SURF_NODRAW             0x80    // don't generate a drawsurface at all
#define SURF_HINT               0x100   // make a primary bsp splitter
#define SURF_SKIP               0x200   // completely ignore, allowing non-closed brushes
#define SURF_NOLIGHTMAP         0x400   // surface doesn't need a lightmap
#define SURF_POINTLIGHT         0x800   // generate lighting info at vertexes
// JOSEPH 9-16-99
#define SURF_METAL              0x1000  // clanking footsteps
// END JOSEPH
#define SURF_NOSTEPS            0x2000  // no footstep sounds
#define SURF_NONSOLID           0x4000  // don't collide against curves with this set
#define SURF_LIGHTFILTER        0x8000  // act as a light filter during q3map -light
#define SURF_ALPHASHADOW        0x10000 // do per-pixel light shadow casting in q3map
#define SURF_NODLIGHT           0x20000 // don't dlight even if solid (solid lava, skies)
// JOSEPH 9-16-99
// Ridah, 11-01-99 (Q3 merge)
#define SURF_WOOD               0x40000
#define SURF_GRASS              0x80000
#define SURF_GRAVEL             0x100000
// END JOSEPH

// (SA)
//#define SURF_SMGROUP			0x200000
#define SURF_GLASS              0x200000    // out of surf's, so replacing unused 'SURF_SMGROUP'
#define SURF_SNOW               0x400000
#define SURF_ROOF               0x800000

//#define	SURF_RUBBLE				0x1000000	// stole 'rubble' for
#define SURF_RUBBLE             0x1000000
#define SURF_CARPET             0x2000000

#define SURF_MONSTERSLICK       0x4000000   // slick surf that only affects ai's
// #define SURF_DUST				0x8000000 // leave a dust trail when walking on this surface
#define SURF_MONSLICK_W         0x8000000

#define SURF_MONSLICK_N         0x10000000
#define SURF_MONSLICK_E         0x20000000
#define SURF_MONSLICK_S         0x40000000
#else
#define CONTENTS_SOLID              0x00000001
#define CONTENTS_LIGHTGRID          0x00000004
#define CONTENTS_LAVA               0x00000008
#define CONTENTS_SLIME              0x00000010
#define CONTENTS_WATER              0x00000020
#define CONTENTS_FOG                0x00000040
#define CONTENTS_MISSILECLIP        0x00000080
#define CONTENTS_ITEM               0x00000100
#define CONTENTS_MOVER              0x00004000
#define CONTENTS_AREAPORTAL         0x00008000
#define CONTENTS_PLAYERCLIP         0x00010000
#define CONTENTS_MONSTERCLIP        0x00020000
#define CONTENTS_TELEPORTER         0x00040000
#define CONTENTS_JUMPPAD            0x00080000
#define CONTENTS_CLUSTERPORTAL      0x00100000
#define CONTENTS_DONOTENTER         0x00200000
#define CONTENTS_DONOTENTER_LARGE   0x00400000
#define CONTENTS_ORIGIN             0x01000000  // removed before bsping an entity
#define CONTENTS_BODY               0x02000000  // should never be on a brush, only in game
#define CONTENTS_CORPSE             0x04000000
#define CONTENTS_DETAIL             0x08000000  // brushes not used for the bsp

#define CONTENTS_STRUCTURAL     0x10000000  // brushes used for the bsp
#define CONTENTS_TRANSLUCENT    0x20000000  // don't consume surface fragments inside
#define CONTENTS_TRIGGER        0x40000000
#define CONTENTS_NODROP         0x80000000  // don't leave bodies or items (death fog, lava)

#define SURF_NODAMAGE           0x00000001  // never give falling damage
#define SURF_SLICK              0x00000002  // effects game physics
#define SURF_SKY                0x00000004  // lighting from environment map
#define SURF_LADDER             0x00000008
#define SURF_NOIMPACT           0x00000010  // don't make missile explosions
#define SURF_NOMARKS            0x00000020  // don't leave missile marks
#define SURF_SPLASH             0x00000040  // out of surf's, so replacing unused 'SURF_FLESH' - and as SURF_CERAMIC wasn't used, it's now SURF_SPLASH
#define SURF_NODRAW             0x00000080  // don't generate a drawsurface at all
#define SURF_HINT               0x00000100  // make a primary bsp splitter
#define SURF_SKIP               0x00000200  // completely ignore, allowing non-closed brushes
#define SURF_NOLIGHTMAP         0x00000400  // surface doesn't need a lightmap
#define SURF_POINTLIGHT         0x00000800  // generate lighting info at vertexes
#define SURF_METAL              0x00001000  // clanking footsteps
#define SURF_NOSTEPS            0x00002000  // no footstep sounds
#define SURF_NONSOLID           0x00004000  // don't collide against curves with this set
#define SURF_LIGHTFILTER        0x00008000  // act as a light filter during q3map -light
#define SURF_ALPHASHADOW        0x00010000  // do per-pixel light shadow casting in q3map
#define SURF_NODLIGHT           0x00020000  // don't dlight even if solid (solid lava, skies)
#define SURF_WOOD               0x00040000
#define SURF_GRASS              0x00080000
#define SURF_GRAVEL             0x00100000
#define SURF_GLASS              0x00200000  // out of surf's, so replacing unused 'SURF_SMGROUP'
#define SURF_SNOW               0x00400000
#define SURF_ROOF               0x00800000
#define SURF_RUBBLE             0x01000000
#define SURF_CARPET             0x02000000
#define SURF_MONSTERSLICK       0x04000000  // slick surf that only affects ai's
#define SURF_MONSLICK_W         0x08000000
#define SURF_MONSLICK_N         0x10000000
#define SURF_MONSLICK_E         0x20000000
#define SURF_MONSLICK_S         0x40000000

#define SURF_LANDMINE           0x80000000  // ydnar: ok to place landmines on this surface
#endif // RTCW_XX


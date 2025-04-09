/*
===========================================================================

Return to Castle Wolfenstein single player GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Return to Castle Wolfenstein single player GPL Source Code (RTCW SP Source Code).  

RTCW SP Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RTCW SP Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RTCW SP Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the RTCW SP Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the RTCW SP Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#ifndef __Q_SHARED_H
#define __Q_SHARED_H

// q_shared.h -- included first by ALL program modules.
// A user mod should never modify this file

// BBi
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include <cstddef>

#include <algorithm>

#include "rtcw_c.h"
// BBi

// BBi
//#if defined RTCW_SP
//// BBi
////#define Q3_VERSION      "Wolf 1.41"
//#define Q3_VERSION "RTCW-SP 0.99b (1.41)"
//// BBi
//
//// ver 1.0.0	- release
//// ver 1.0.1	- post-release work
//// ver 1.1.0	- patch 1 (12/12/01)
//// ver 1.1b		- TTimo SP linux release (+ MP update)
//// ver 1.2.b5	- Mac code merge in
//// ver 1.3		- patch 2 (02/13/02)
//#elif defined RTCW_MP
//// BBi
////#define Q3_VERSION      "Wolf 1.41b-MP"
//#define Q3_VERSION "RTCW-MP 0.99b (1.41b)"
//// BBi
//
//// 1.41b-MP: fix autodl sploit
//// 1.4-MP : (== 1.34)
//// 1.3-MP : final for release
//// 1.1b - TTimo SP linux release (+ MP updates)
//// 1.1b5 - Mac update merge in
//#else
////#define PRE_RELEASE_DEMO
//
//#ifndef PRE_RELEASE_DEMO
//// BBi
////#define Q3_VERSION      "ET 2.60d"
//#define Q3_VERSION "RTCW-ET 0.99b (2.60d)"
//// BBi
//#else
//// BBi
////#define Q3_VERSION      "ET 2.32"
//#define Q3_VERSION "RTCW-ET 0.99b (2.32)"
//// BBi
//#endif // PRE_RELEASE_DEMO
//// 2.60d: Mac OSX universal binaries
//// 2.60c: Mac OSX universal binaries
//// 2.60b: CVE-2006-2082 fix
//// 2.6x: Enemy Territory - ETPro team maintenance release
//// 2.5x: Enemy Territory FINAL
//// 2.4x: Enemy Territory RC's
//// 2.3x: Enemy Territory TEST
//// 2.2+: post SP removal
//// 2.1+: post Enemy Territory moved standalone
//// 2.x: post Enemy Territory
//// 1.x: pre Enemy Territory
//////
//// 1.3-MP : final for release
//// 1.1b - TTimo SP linux release (+ MP updates)
//// 1.1b5 - Mac update merge in
//
//#define CONFIG_NAME     "etconfig.cfg"
//
////#define LOCALIZATION_SUPPORT
//#endif // RTCW_XX

#if defined RTCW_SP
#ifdef RTCW_SP_DEMO
	#define RTCW_VERSION "RTCW-SP-D 0.99b (1.41)"
#else
	#define RTCW_VERSION "RTCW-SP 0.99b (1.41)"
#endif
#elif defined RTCW_MP
#define RTCW_VERSION "RTCW-MP 0.99b (1.41b)"
#else
#ifndef PRE_RELEASE_DEMO
#define RTCW_VERSION "RTCW-ET 0.99b (2.60d)"
#else
#define RTCW_VERSION "RTCW-ET 0.99b (2.32)"
#endif // PRE_RELEASE_DEMO

#define CONFIG_NAME "etconfig.cfg"
#endif // RTCW_XX
// BBi

#define NEW_ANIMS
#define MAX_TEAMNAME    32

#if defined RTCW_MP
// DHM - Nerve
//#define PRE_RELEASE_DEMO
#endif // RTCW_XX

// BBi
//#if defined( ppc ) || defined( __ppc ) || defined( __ppc__ ) || defined( __POWERPC__ )
//#define idppc 1
//#endif
// BBi

// BBi
///**********************************************************************
//  VM Considerations
//
//  The VM can not use the standard system headers because we aren't really
//  using the compiler they were meant for.  We use bg_lib.h which contains
//  prototypes for the functions we define for our own use in bg_lib.c.
//
//  When writing mods, please add needed headers HERE, do not start including
//  stuff like <stdio.h> in the various .c files that make up each of the VMs
//  since you will be including system headers files can will have issues.
//
//  Remember, if you use a C library function that is not defined in bg_lib.c,
//  you will have to add your own version for support in the VM.
//
// **********************************************************************/
//
//#ifdef Q3_VM
//
//#include "bg_lib.h"
//
//#else
//
//#include <assert.h>
//#include <math.h>
//#include <stdio.h>
//#include <stdarg.h>
//#include <string.h>
//#include <stdlib.h>
//#include <time.h>
//#include <ctype.h>
//#include <limits.h>
//
//#if defined RTCW_ET
//#include <sys/stat.h> // rain
//#include <float.h>
//#endif // RTCW_XX
//
//#endif

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <cctype>
#include <climits>

#if defined RTCW_ET
#include <float.h>
#endif // RTCW_XX
// BBi


#ifdef _WIN32

#define QDECL __cdecl
#define PATH_SEP '\\'

#else

#define QDECL
#define PATH_SEP '/'

#endif

typedef uint8_t byte;

typedef int32_t qboolean;
const qboolean qfalse = 0;
const qboolean qtrue = 1;


typedef int qhandle_t;
typedef int sfxHandle_t;
typedef int fileHandle_t;
typedef int clipHandle_t;

#define ID_INLINE inline

//#define	SND_NORMAL			0x000	// (default) Allow sound to be cut off only by the same sound on this channel
#define     SND_OKTOCUT         0x001   // Allow sound to be cut off by any following sounds on this channel
#define     SND_REQUESTCUT      0x002   // Allow sound to be cut off by following sounds on this channel only for sounds who request cutoff
#define     SND_CUTOFF          0x004   // Cut off sounds on this channel that are marked 'SND_REQUESTCUT'
#define     SND_CUTOFF_ALL      0x008   // Cut off all sounds on this channel
#define     SND_NOCUT           0x010   // Don't cut off.  Always let finish (overridden by SND_CUTOFF_ALL)

#if defined RTCW_ET
#define     SND_NO_ATTENUATION  0x020   // don't attenuate (even though the sound is in voice channel, for example)
#endif // RTCW_XX

// angle indexes
#define PITCH               0       // up / down
#define YAW                 1       // left / right
#define ROLL                2       // fall over

// RF, this is just here so different elements of the engine can be aware of this setting as it changes
#define MAX_SP_CLIENTS      64      // increasing this will increase memory usage significantly

// the game guarantees that no string from the network will ever
// exceed MAX_STRING_CHARS
#define MAX_STRING_CHARS    1024    // max length of a string passed to Cmd_TokenizeString
#define MAX_STRING_TOKENS   256     // max tokens resulting from Cmd_TokenizeString
#define MAX_TOKEN_CHARS     1024    // max length of an individual token

#define MAX_INFO_STRING     1024
#define MAX_INFO_KEY        1024
#define MAX_INFO_VALUE      1024

#define BIG_INFO_STRING     8192    // used for system info key only
#define BIG_INFO_KEY        8192
#define BIG_INFO_VALUE      8192

#define MAX_QPATH           64      // max length of a quake game pathname
#define MAX_OSPATH          256     // max length of a filesystem pathname

#if !defined RTCW_ET
#define MAX_NAME_LENGTH     32      // max length of a client name
#else
// rain - increased to 36 to match MAX_NETNAME, fixes #13 - UI stuff breaks
// with very long names
#define MAX_NAME_LENGTH     36      // max length of a client name
#endif // RTCW_XX

#define MAX_SAY_TEXT        150

#if defined RTCW_ET
#define MAX_BINARY_MESSAGE  32768   // max length of binary message

typedef enum {
	MESSAGE_EMPTY = 0,
	MESSAGE_WAITING,        // rate/packet limited
	MESSAGE_WAITING_OVERFLOW,   // packet too large with message
} messageStatus_t;
#endif // RTCW_XX

// paramters for command buffer stuffing
typedef enum {
	EXEC_NOW,           // don't return until completed, a VM should NEVER use this,
						// because some commands might cause the VM to be unloaded...
	EXEC_INSERT,        // insert at current position, but don't run yet
	EXEC_APPEND         // add to end of the command buffer (normal case)
} cbufExec_t;


//
// these aren't needed by any of the VMs.  put in another header?
//
#define MAX_MAP_AREA_BYTES      32      // bit vector of area visibility


// print levels from renderer (FIXME: set up for game / cgame?)
typedef enum {
	PRINT_ALL,
	PRINT_DEVELOPER,        // only print when "developer 1"
	PRINT_WARNING,
	PRINT_ERROR
} printParm_t;

#ifdef  ERR_FATAL
#undef  ERR_FATAL               // this is be defined in malloc.h
#endif

// parameters to the main Error routine
typedef enum {
	ERR_FATAL,                  // exit the entire game with a popup window

#if defined RTCW_ET
	ERR_VID_FATAL,              // exit the entire game with a popup window and doesn't delete profile.pid
#endif // RTCW_XX

	ERR_DROP,                   // print to console and disconnect from game
	ERR_SERVERDISCONNECT,       // don't kill server
	ERR_DISCONNECT,             // client disconnected from the server

#if defined RTCW_SP
	ERR_NEED_CD,                // pop up the need-cd dialog
	ERR_ENDGAME                 // not an error.  just clean up properly, exit to the menu, and start up the "endgame" menu  //----(SA)	added
#elif defined RTCW_MP
	ERR_NEED_CD                 // pop up the need-cd dialog
#else
	ERR_NEED_CD,                // pop up the need-cd dialog
	ERR_AUTOUPDATE
#endif // RTCW_XX

} errorParm_t;


// font rendering values used by ui and cgame

#define PROP_GAP_WIDTH          3
#define PROP_SPACE_WIDTH        8
#define PROP_HEIGHT             27
#define PROP_SMALL_SIZE_SCALE   0.75

#define BLINK_DIVISOR           200
#define PULSE_DIVISOR           75

#define UI_LEFT         0x00000000  // default
#define UI_CENTER       0x00000001
#define UI_RIGHT        0x00000002
#define UI_FORMATMASK   0x00000007
#define UI_SMALLFONT    0x00000010
#define UI_BIGFONT      0x00000020  // default
#define UI_GIANTFONT    0x00000040
#define UI_DROPSHADOW   0x00000800
#define UI_BLINK        0x00001000
#define UI_INVERSE      0x00002000
#define UI_PULSE        0x00004000
// JOSEPH 10-24-99
#define UI_MENULEFT     0x00008000
#define UI_MENURIGHT    0x00010000
#define UI_EXSMALLFONT  0x00020000
#define UI_MENUFULL     0x00080000
// END JOSEPH

#define UI_SMALLFONT75  0x00100000

#if defined( _DEBUG ) && !defined( BSPC )
	#define HUNK_DEBUG
#endif

typedef enum {
	h_high,
	h_low,
	h_dontcare
} ha_pref;

#ifdef HUNK_DEBUG
#define Hunk_Alloc( size, preference )              Hunk_AllocDebug( size, preference, # size, __FILE__, __LINE__ )
void *Hunk_AllocDebug( int size, ha_pref preference, char *label, char *file, int line );
#else
void *Hunk_Alloc( int size, ha_pref preference );
#endif

#define Snd_Memset Com_Memset

void Com_Memset( void* dest, const int val, const size_t count );
void Com_Memcpy( void* dest, const void* src, const size_t count );


#define CIN_system      (0x01)
#define CIN_loop        (0x02)
#define CIN_hold        (0x04)
#define CIN_silent      (0x08)
#define CIN_shader      (0x10)

#ifdef RTCW_SP
#define CIN_letterBox   (0x20)
#endif // RTCW_XX


/*
==============================================================

MATHLIB

==============================================================
*/


typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef vec_t vec5_t[5];


#ifndef M_PI
#define M_PI 3.14159265358979323846F // matches value in gcc v2 math.h
#endif

#define NUMVERTEXNORMALS    162
extern vec3_t bytedirs[NUMVERTEXNORMALS];

// all drawing is done to a 640*480 virtual screen size
// and will be automatically scaled to the real resolution
#define SCREEN_WIDTH        640
#define SCREEN_HEIGHT       480

#define TINYCHAR_WIDTH      ( SMALLCHAR_WIDTH )

#if !defined RTCW_ET
#define TINYCHAR_HEIGHT     ( SMALLCHAR_HEIGHT / 2 )
#else
#define TINYCHAR_HEIGHT     ( SMALLCHAR_HEIGHT )
#endif // RTCW_XX

#if defined RTCW_ET
#define MINICHAR_WIDTH      8
#define MINICHAR_HEIGHT     12
#endif // RTCW_XX

#define SMALLCHAR_WIDTH     8
#define SMALLCHAR_HEIGHT    16

#define BIGCHAR_WIDTH       16
#define BIGCHAR_HEIGHT      16

#define GIANTCHAR_WIDTH     32
#define GIANTCHAR_HEIGHT    48

extern vec4_t colorBlack;
extern vec4_t colorRed;
extern vec4_t colorGreen;
extern vec4_t colorBlue;
extern vec4_t colorYellow;

#if defined RTCW_ET
extern vec4_t colorOrange;
#endif // RTCW_XX

extern vec4_t colorMagenta;
extern vec4_t colorCyan;
extern vec4_t colorWhite;
extern vec4_t colorLtGrey;
extern vec4_t colorMdGrey;
extern vec4_t colorDkGrey;

#if defined RTCW_ET
extern vec4_t colorMdRed;
extern vec4_t colorMdGreen;
extern vec4_t colorDkGreen;
extern vec4_t colorMdCyan;
extern vec4_t colorMdYellow;
extern vec4_t colorMdOrange;
extern vec4_t colorMdBlue;

extern vec4_t clrBrown;
extern vec4_t clrBrownDk;
extern vec4_t clrBrownLine;
extern vec4_t clrBrownText;
extern vec4_t clrBrownTextDk;
extern vec4_t clrBrownTextDk2;
extern vec4_t clrBrownTextLt;
extern vec4_t clrBrownTextLt2;
extern vec4_t clrBrownLineFull;

#define GAME_INIT_FRAMES    6
#define FRAMETIME           100                 // msec
#endif // RTCW_XX


#define Q_COLOR_ESCAPE  '^'
#define Q_IsColorString( p )  ( p && *( p ) == Q_COLOR_ESCAPE && *( ( p ) + 1 ) && *( ( p ) + 1 ) != Q_COLOR_ESCAPE )

#define COLOR_BLACK     '0'
#define COLOR_RED       '1'
#define COLOR_GREEN     '2'
#define COLOR_YELLOW    '3'
#define COLOR_BLUE      '4'
#define COLOR_CYAN      '5'
#define COLOR_MAGENTA   '6'
#define COLOR_WHITE     '7'

#if !defined RTCW_ET
#define ColorIndex( c )   ( ( ( c ) - '0' ) & 7 )
#endif // RTCW_XX

#if defined RTCW_ET
#define COLOR_ORANGE    '8'
#define COLOR_MDGREY    '9'
#define COLOR_LTGREY    ':'
//#define COLOR_LTGREY	';'
#define COLOR_MDGREEN   '<'
#define COLOR_MDYELLOW  '='
#define COLOR_MDBLUE    '>'
#define COLOR_MDRED     '?'
#define COLOR_LTORANGE  'A'
#define COLOR_MDCYAN    'B'
#define COLOR_MDPURPLE  'C'
#define COLOR_NULL      '*'


#define COLOR_BITS  31
#define ColorIndex( c )   ( ( ( c ) - '0' ) & COLOR_BITS )
#endif // RTCW_XX

#define S_COLOR_BLACK   "^0"
#define S_COLOR_RED     "^1"
#define S_COLOR_GREEN   "^2"
#define S_COLOR_YELLOW  "^3"
#define S_COLOR_BLUE    "^4"
#define S_COLOR_CYAN    "^5"
#define S_COLOR_MAGENTA "^6"
#define S_COLOR_WHITE   "^7"

#if defined RTCW_ET
#define S_COLOR_ORANGE      "^8"
#define S_COLOR_MDGREY      "^9"
#define S_COLOR_LTGREY      "^:"
//#define S_COLOR_LTGREY		"^;"
#define S_COLOR_MDGREEN     "^<"
#define S_COLOR_MDYELLOW    "^="
#define S_COLOR_MDBLUE      "^>"
#define S_COLOR_MDRED       "^?"
#define S_COLOR_LTORANGE    "^A"
#define S_COLOR_MDCYAN      "^B"
#define S_COLOR_MDPURPLE    "^C"
#define S_COLOR_NULL        "^*"
#endif // RTCW_XX

#if !defined RTCW_ET
extern vec4_t g_color_table[8];
#else
extern vec4_t g_color_table[32];
#endif // RTCW_XX

#define MAKERGB( v, r, g, b ) v[0] = r; v[1] = g; v[2] = b
#define MAKERGBA( v, r, g, b, a ) v[0] = r; v[1] = g; v[2] = b; v[3] = a

#if defined RTCW_ET
// Hex Color string support
#define gethex( ch ) ( ( ch ) > '9' ? ( ( ch ) >= 'a' ? ( ( ch ) - 'a' + 10 ) : ( ( ch ) - '7' ) ) : ( ( ch ) - '0' ) )
#define ishex( ch )  ( ( ch ) && ( ( ( ch ) >= '0' && ( ch ) <= '9' ) || ( ( ch ) >= 'A' && ( ch ) <= 'F' ) || ( ( ch ) >= 'a' && ( ch ) <= 'f' ) ) )
// check if it's format rrggbb r,g,b e {0..9} U {A...F}
#define Q_IsHexColorString( p ) ( ishex( *( p ) ) && ishex( *( ( p ) + 1 ) ) && ishex( *( ( p ) + 2 ) ) && ishex( *( ( p ) + 3 ) ) && ishex( *( ( p ) + 4 ) ) && ishex( *( ( p ) + 5 ) ) )
#define Q_HexColorStringHasAlpha( p ) ( ishex( *( ( p ) + 6 ) ) && ishex( *( ( p ) + 7 ) ) )
#endif // RTCW_XX

#define DEG2RAD( a ) ( ( ( a ) * M_PI ) / 180.0F )
#define RAD2DEG( a ) ( ( ( a ) * 180.0f ) / M_PI )

struct cplane_s;

extern vec3_t vec3_origin;
extern vec3_t axisDefault[3];

// BBi
//#define nanmask ( 255 << 23 )
//
//#define IS_NAN( x ) ( ( ( *(int *)&x ) & nanmask ) == nanmask )
//
//
//#if defined RTCW_SP
//// TTimo
//// handy stuff when tracking isnan problems
//#ifndef NDEBUG
//#define CHECK_NAN( x ) assert( !IS_NAN( x ) )
//#define CHECK_NAN_VEC( v ) assert( !IS_NAN( v[0] ) && !IS_NAN( v[1] ) && !IS_NAN( v[2] ) )
//#else
//#define CHECK_NAN
//#define CHECK_NAN_VEC
//#endif
//#endif // RTCW_XX
// BBi

float Q_fabs( float f );
float Q_rsqrt( float f );       // reciprocal square root

#define SQRTFAST( x ) ( 1.0f / Q_rsqrt( x ) )

template<typename T>
int myftol(
	const T x)
{
	return static_cast<int>(x);
}

signed char ClampChar( int i );
signed short ClampShort( int i );

// this isn't a real cheap function to call!
int DirToByte( vec3_t dir );
void ByteToDir( int b, vec3_t dir );

#if 1

#define DotProduct( x,y )         ( ( x )[0] * ( y )[0] + ( x )[1] * ( y )[1] + ( x )[2] * ( y )[2] )
#define VectorSubtract( a,b,c )   ( ( c )[0] = ( a )[0] - ( b )[0],( c )[1] = ( a )[1] - ( b )[1],( c )[2] = ( a )[2] - ( b )[2] )
#define VectorAdd( a,b,c )        ( ( c )[0] = ( a )[0] + ( b )[0],( c )[1] = ( a )[1] + ( b )[1],( c )[2] = ( a )[2] + ( b )[2] )
#define VectorCopy( a,b )         ( ( b )[0] = ( a )[0],( b )[1] = ( a )[1],( b )[2] = ( a )[2] )

#if defined RTCW_SP
#define VectorCopy4( a,b )        ( ( b )[0] = ( a )[0],( b )[1] = ( a )[1],( b )[2] = ( a )[2],( b )[3] = ( a )[3] )
#endif // RTCW_XX

#define VectorScale( v, s, o )    ( ( o )[0] = ( v )[0] * ( s ),( o )[1] = ( v )[1] * ( s ),( o )[2] = ( v )[2] * ( s ) )
#define VectorMA( v, s, b, o )    ( ( o )[0] = ( v )[0] + ( b )[0] * ( s ),( o )[1] = ( v )[1] + ( b )[1] * ( s ),( o )[2] = ( v )[2] + ( b )[2] * ( s ) )

#else

#define DotProduct( x,y )         _DotProduct( x,y )
#define VectorSubtract( a,b,c )   _VectorSubtract( a,b,c )
#define VectorAdd( a,b,c )        _VectorAdd( a,b,c )
#define VectorCopy( a,b )         _VectorCopy( a,b )
#define VectorScale( v, s, o )    _VectorScale( v,s,o )
#define VectorMA( v, s, b, o )    _VectorMA( v,s,b,o )

#endif

// BBi
//#ifdef __LCC__
//#ifdef VectorCopy
//#undef VectorCopy
//// this is a little hack to get more efficient copies in our interpreter
//typedef struct {
//	float v[3];
//} vec3struct_t;
//#define VectorCopy( a,b ) * (vec3struct_t *)b = *(vec3struct_t *)a;
//#endif
//#endif
// BBi

#define VectorClear( a )              ( ( a )[0] = ( a )[1] = ( a )[2] = 0 )
#define VectorNegate( a,b )           ( ( b )[0] = -( a )[0],( b )[1] = -( a )[1],( b )[2] = -( a )[2] )
#define VectorSet( v, x, y, z )       ( ( v )[0] = ( x ), ( v )[1] = ( y ), ( v )[2] = ( z ) )

#if defined RTCW_ET
#define Vector2Set( v, x, y )         ( ( v )[0] = ( x ),( v )[1] = ( y ) )
#define Vector2Copy( a,b )            ( ( b )[0] = ( a )[0],( b )[1] = ( a )[1] )
#define Vector2Subtract( a,b,c )      ( ( c )[0] = ( a )[0] - ( b )[0],( c )[1] = ( a )[1] - ( b )[1] )
#endif // RTCW_XX

#define Vector4Set( v, x, y, z, n )   ( ( v )[0] = ( x ),( v )[1] = ( y ),( v )[2] = ( z ),( v )[3] = ( n ) )
#define Vector4Copy( a,b )            ( ( b )[0] = ( a )[0],( b )[1] = ( a )[1],( b )[2] = ( a )[2],( b )[3] = ( a )[3] )
#define Vector4MA( v, s, b, o )       ( ( o )[0] = ( v )[0] + ( b )[0] * ( s ),( o )[1] = ( v )[1] + ( b )[1] * ( s ),( o )[2] = ( v )[2] + ( b )[2] * ( s ),( o )[3] = ( v )[3] + ( b )[3] * ( s ) )
#define Vector4Average( v, b, s, o )  ( ( o )[0] = ( ( v )[0] * ( 1 - ( s ) ) ) + ( ( b )[0] * ( s ) ),( o )[1] = ( ( v )[1] * ( 1 - ( s ) ) ) + ( ( b )[1] * ( s ) ),( o )[2] = ( ( v )[2] * ( 1 - ( s ) ) ) + ( ( b )[2] * ( s ) ),( o )[3] = ( ( v )[3] * ( 1 - ( s ) ) ) + ( ( b )[3] * ( s ) ) )

#define SnapVector( v ) {v[0] = ( (int)( v[0] ) ); v[1] = ( (int)( v[1] ) ); v[2] = ( (int)( v[2] ) );}

// just in case you do't want to use the macros
vec_t _DotProduct( const vec3_t v1, const vec3_t v2 );
void _VectorSubtract( const vec3_t veca, const vec3_t vecb, vec3_t out );
void _VectorAdd( const vec3_t veca, const vec3_t vecb, vec3_t out );
void _VectorCopy( const vec3_t in, vec3_t out );
void _VectorScale( const vec3_t in, float scale, vec3_t out );
void _VectorMA( const vec3_t veca, float scale, const vec3_t vecb, vec3_t vecc );

unsigned ColorBytes3( float r, float g, float b );
unsigned ColorBytes4( float r, float g, float b, float a );

float NormalizeColor( const vec3_t in, vec3_t out );

float RadiusFromBounds( const vec3_t mins, const vec3_t maxs );
void ClearBounds( vec3_t mins, vec3_t maxs );
void AddPointToBounds( const vec3_t v, vec3_t mins, vec3_t maxs );

#if defined RTCW_ET
qboolean PointInBounds( const vec3_t v, const vec3_t mins, const vec3_t maxs );
#endif // RTCW_XX

int VectorCompare( const vec3_t v1, const vec3_t v2 );
vec_t VectorLength( const vec3_t v );
vec_t VectorLengthSquared( const vec3_t v );
vec_t Distance( const vec3_t p1, const vec3_t p2 );
vec_t DistanceSquared( const vec3_t p1, const vec3_t p2 );
void CrossProduct( const vec3_t v1, const vec3_t v2, vec3_t cross );
vec_t VectorNormalize( vec3_t v );       // returns vector length
void VectorNormalizeFast( vec3_t v );     // does NOT return vector length, uses rsqrt approximation
vec_t VectorNormalize2( const vec3_t v, vec3_t out );
void VectorInverse( vec3_t v );
void Vector4Scale( const vec4_t in, vec_t scale, vec4_t out );
void VectorRotate( vec3_t in, vec3_t matrix[3], vec3_t out );
int Q_log2( int val );

float Q_acos( float c );

int     Q_rand( int *seed );
float   Q_random( int *seed );
float   Q_crandom( int *seed );

#define random()    ( ( rand() & 0x7fff ) / ( (float)0x7fff ) )
#define crandom()   ( 2.0 * ( random() - 0.5 ) )

void vectoangles( const vec3_t value1, vec3_t angles );
float vectoyaw( const vec3_t vec );
void AnglesToAxis( const vec3_t angles, vec3_t axis[3] );
// TTimo: const vec_t ** would require explicit casts for ANSI C conformance
// see unix/const-arg.c
void AxisToAngles( /*const*/ vec3_t axis[3], vec3_t angles );
float VectorDistance( vec3_t v1, vec3_t v2 );

#if defined RTCW_ET
float VectorDistanceSquared( vec3_t v1, vec3_t v2 );
#endif // RTCW_XX

void AxisClear( vec3_t axis[3] );
void AxisCopy( vec3_t in[3], vec3_t out[3] );

void SetPlaneSignbits( struct cplane_s *out );
int BoxOnPlaneSide( vec3_t emins, vec3_t emaxs, struct cplane_s *plane );

float   AngleMod( float a );
float   LerpAngle( float from, float to, float frac );

#if !defined RTCW_SP
void    LerpPosition( vec3_t start, vec3_t end, float frac, vec3_t out );
#endif // RTCW_XX

float   AngleSubtract( float a1, float a2 );
void    AnglesSubtract( vec3_t v1, vec3_t v2, vec3_t v3 );

#if defined RTCW_ET
float AngleNormalize2Pi( float angle );
#endif // RTCW_XX

float AngleNormalize360( float angle );
float AngleNormalize180( float angle );
float AngleDelta( float angle1, float angle2 );

qboolean PlaneFromPoints( vec4_t plane, const vec3_t a, const vec3_t b, const vec3_t c );
void ProjectPointOnPlane( vec3_t dst, const vec3_t p, const vec3_t normal );
void RotatePointAroundVector( vec3_t dst, const vec3_t dir, const vec3_t point, float degrees );

#if defined RTCW_ET
void RotatePointAroundVertex( vec3_t pnt, float rot_x, float rot_y, float rot_z, const vec3_t origin );
#endif // RTCW_XX

void RotateAroundDirection( vec3_t axis[3], float yaw );
void MakeNormalVectors( const vec3_t forward, vec3_t right, vec3_t up );
// perpendicular vector could be replaced by this

int PlaneTypeForNormal( vec3_t normal );

void MatrixMultiply( float in1[3][3], float in2[3][3], float out[3][3] );
void AngleVectors( const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up );
void PerpendicularVector( vec3_t dst, const vec3_t src );

// Ridah
void GetPerpendicularViewVector( const vec3_t point, const vec3_t p1, const vec3_t p2, vec3_t up );
void ProjectPointOntoVector( vec3_t point, vec3_t vStart, vec3_t vEnd, vec3_t vProj );

#if defined RTCW_ET
void ProjectPointOntoVectorBounded( vec3_t point, vec3_t vStart, vec3_t vEnd, vec3_t vProj );
float DistanceFromLineSquared( vec3_t p, vec3_t lp1, vec3_t lp2 );
float DistanceFromVectorSquared( vec3_t p, vec3_t lp1, vec3_t lp2 );
#endif // RTCW_XX

// done.

//=============================================

float Com_Clamp( float min, float max, float value );

char    *COM_SkipPath( char *pathname );

#if defined RTCW_ET
void    COM_FixPath( char *pathname );
#endif // RTCW_XX

void    COM_StripExtension( const char *in, char *out );

#if !defined RTCW_SP
void    COM_StripExtension2( const char *in, char *out, int destsize );
#endif // RTCW_XX

void    COM_StripFilename( const char *in, char *out );
void    COM_DefaultExtension( char *path, int maxSize, const char *extension );

void    COM_BeginParseSession( const char *name );
void    COM_RestoreParseSession( const char **data_p );
void    COM_SetCurrentParseLine( int line );
int     COM_GetCurrentParseLine( void );
char    *COM_Parse( const char **data_p );
char    *COM_ParseExt( const char **data_p, qboolean allowLineBreak );
int     COM_Compress( char *data_p );

// BBi
//#if !defined RTCW_ET
//void    COM_ParseError( char *format, ... );
//void    COM_ParseWarning( char *format, ... );
//#else
//void    COM_ParseError( char *format, ... ) _attribute( ( format( printf,1,2 ) ) );
//void    COM_ParseWarning( char *format, ... ) _attribute( ( format( printf,1,2 ) ) );
//#endif // RTCW_XX

void COM_ParseError (const char* format, ...);
void COM_ParseWarning (const char* format, ...);
// BBi

#if defined RTCW_ET
int Com_ParseInfos( const char *buf, int max, char infos[][MAX_INFO_STRING] );
#endif // RTCW_XX

// BBi
//#if defined RTCW_SP
//// TTimo
//#elif defined RTCW_MP
////int		COM_ParseInfos( char *buf, int max, char infos[][MAX_INFO_STRING] );
//#endif // RTCW_XX
// BBi

qboolean COM_BitCheck( const int array[], int bitNum );
void COM_BitSet( int array[], int bitNum );
void COM_BitClear( int array[], int bitNum );

#define MAX_TOKENLENGTH     1024

#ifndef TT_STRING
//token types
#define TT_STRING                   1           // string
#define TT_LITERAL                  2           // literal
#define TT_NUMBER                   3           // number
#define TT_NAME                     4           // name
#define TT_PUNCTUATION              5           // punctuation
#endif

typedef struct pc_token_s
{
	int type;
	int subtype;
	int intvalue;
	float floatvalue;
	char string[MAX_TOKENLENGTH];

#if defined RTCW_ET
	int line;
	int linescrossed;
#endif // RTCW_XX

} pc_token_t;

// data is an in/out parm, returns a parsed out token

void    COM_MatchToken( const char**buf_p, const char *match );

void SkipBracedSection( const char **program );

#if !defined RTCW_SP
void SkipBracedSection_Depth( const char **program, int depth ); // start at given depth if already matching stuff
#endif // RTCW_XX

void SkipRestOfLine( const char **data );

void Parse1DMatrix( const char **buf_p, int x, float *m );
void Parse2DMatrix( const char **buf_p, int y, int x, float *m );
void Parse3DMatrix( const char **buf_p, int z, int y, int x, float *m );

// BBi
//#if !defined RTCW_ET
//void QDECL Com_sprintf( char *dest, int size, const char *fmt, ... );
//#else
//void QDECL Com_sprintf( char *dest, int size, const char *fmt, ... ) _attribute( ( format( printf,3,4 ) ) );
//#endif // RTCW_XX

void QDECL Com_sprintf (char* dest, int size, const char* fmt, ...);
// BBi

// mode parm for FS_FOpenFile
typedef enum {
	FS_READ,
	FS_WRITE,
	FS_APPEND,
	FS_APPEND_SYNC
} fsMode_t;

typedef enum {
	FS_SEEK_CUR,
	FS_SEEK_END,
	FS_SEEK_SET
} fsOrigin_t;

//=============================================

int Q_isprint( int c );
int Q_islower( int c );
int Q_isupper( int c );
int Q_isalpha( int c );

#if !defined RTCW_MP
int Q_isnumeric( int c );       //----(SA)	added
int Q_isalphanumeric( int c );  //----(SA)	added
int Q_isforfilename( int c );       //----(SA)	added
#endif // RTCW_XX

// portable case insensitive compare
int     Q_stricmp( const char *s1, const char *s2 );
int     Q_strncmp( const char *s1, const char *s2, int n );
int     Q_stricmpn( const char *s1, const char *s2, int n );
char    *Q_strlwr( char *s1 );
char    *Q_strupr( char *s1 );
char    *Q_strrchr( const char* string, int c );

#ifdef _WIN32
#define Q_putenv _putenv
#else
#define Q_putenv putenv
#endif

// buffer size safe library replacements
void    Q_strncpyz( char *dest, const char *src, int destsize );
void    Q_strcat( char *dest, int size, const char *src );

// strlen that discounts Quake color sequences
int Q_PrintStrlen( const char *string );
// removes color sequences from string
char *Q_CleanStr( char *string );

#if !defined RTCW_ET
// Ridah
int Q_strncasecmp( const char *s1, const char *s2, int n );
int Q_strcasecmp( const char *s1, const char *s2 );
// done.
#endif // RTCW_XX

#if defined RTCW_ET
// removes whitespaces and other bad directory characters
char *Q_CleanDirName( char *dirname );
#endif // RTCW_XX

#if defined RTCW_MP
// TTimo
// vsnprintf is ISO/IEC 9899:1999
// abstracting this to make it portable
#ifdef _WIN32
#define Q_vsnprintf _vsnprintf
#else
// TODO: Mac define?
#define Q_vsnprintf vsnprintf
#endif
#elif defined RTCW_ET
#define _vsnprintf use_Q_vsnprintf
#define vsnprintf use_Q_vsnprintf
int Q_vsnprintf( char *dest, int size, const char *fmt, va_list argptr );
#endif // RTCW_XX

// BBi
#if 0
// BBi
////=============================================
//
//// 64-bit integers for global rankings interface
//// implemented as a struct for qvm compatibility
//typedef struct
//{
//	byte b0;
//	byte b1;
//	byte b2;
//	byte b3;
//	byte b4;
//	byte b5;
//	byte b6;
//	byte b7;
//} qint64;
//
////=============================================
// BBi

#if !defined RTCW_ET
short   BigShort( short l );
#endif // RTCW_XX

short   LittleShort( short l );

#if !defined RTCW_ET
int     BigLong( int l );
#endif // RTCW_XX

int     LittleLong( int l );

// BBi
//#if !defined RTCW_ET
//qint64  BigLong64( qint64 l );
//#endif // RTCW_XX
//
//qint64  LittleLong64( qint64 l );
// BBi

#if !defined RTCW_ET
float   BigFloat( float l );
#endif // RTCW_XX

float   LittleFloat( float l );

#if defined RTCW_ET
short   BigShort( short l );
int BigLong( int l );

// BBi
//qint64  BigLong64( qint64 l );
// BBi

float   BigFloat( float l );
#endif // RTCW_XX

void    Swap_Init( void );
#endif // 0
// BBi

// BBi
//#if !defined RTCW_ET
//char    * QDECL va( char *format, ... );
//#else
//char    * QDECL va( char *format, ... ) _attribute( ( format( printf,1,2 ) ) );
//#endif // RTCW_XX

char* QDECL va (const char* format, ...);
// BBi

float   *tv( float x, float y, float z );

//=============================================

//
// key / value info strings
//
const char *Info_ValueForKey( const char *s, const char *key );
void Info_RemoveKey( char *s, const char *key );
void Info_RemoveKey_big( char *s, const char *key );
void Info_SetValueForKey( char *s, const char *key, const char *value );
void Info_SetValueForKey_Big( char *s, const char *key, const char *value );
qboolean Info_Validate( const char *s );
void Info_NextPair( const char **s, char *key, char *value );

// this is only here so the functions in q_shared.c and bg_*.c can link

// BBi
//#if !defined RTCW_ET
//void QDECL Com_Error( int level, const char *error, ... );
//void QDECL Com_Printf( const char *msg, ... );
//#else
//void QDECL Com_Error( int level, const char *error, ... ) _attribute( ( format( printf,2,3 ) ) );
//void QDECL Com_Printf( const char *msg, ... ) _attribute( ( format( printf,1,2 ) ) );
//#endif // RTCW_XX

void QDECL Com_Error (int level, const char* error, ...);
void QDECL Com_Printf (const char* msg, ...);
// BBi

#if !defined RTCW_MP

#if !defined RTCW_ET
/*
==============================================================

SAVE

	12 -
	13 - (SA) added 'episode' tracking to savegame
	14 - RF added 'skill'
	15 - (SA) moved time info above the main game reading
	16 - (SA) added fog
	17 - (SA) rats, changed fog.
  18 - TTimo targetdeath fix
	   show_bug.cgi?id=434

==============================================================
*/

#define SAVE_VERSION    18
#define SAVE_INFOSTRING_LENGTH  256
#endif // RTCW_XX

/*
==========================================================

  RELOAD STATES

==========================================================
*/

#define RELOAD_SAVEGAME         0x01
#define RELOAD_NEXTMAP          0x02
#define RELOAD_NEXTMAP_WAITING  0x04
#define RELOAD_FAILED           0x08
#define RELOAD_ENDGAME          0x10
#endif // RTCW_XX

/*
==========================================================

CVARS (console variables)

Many variables can be used for cheating purposes, so when
cheats is zero, force all unspecified variables to their
default values.
==========================================================
*/

#define CVAR_ARCHIVE        1   // set to cause it to be saved to vars.rc
								// used for system variables, not for player
								// specific configurations
#define CVAR_USERINFO       2   // sent to server on connect or change
#define CVAR_SERVERINFO     4   // sent in response to front end requests
#define CVAR_SYSTEMINFO     8   // these cvars will be duplicated on all clients
#define CVAR_INIT           16  // don't allow change from console at all,
								// but can be set from the command line
#define CVAR_LATCH          32  // will only change when C code next does
								// a Cvar_Get(), so it can't be changed
								// without proper initialization.  modified
								// will be set, even though the value hasn't
								// changed yet
#define CVAR_ROM            64  // display only, cannot be set by user at all
#define CVAR_USER_CREATED   128 // created by a set command
#define CVAR_TEMP           256 // can be set even when cheats are disabled, but is not archived
#define CVAR_CHEAT          512 // can not be changed if cheats are disabled
#define CVAR_NORESTART      1024    // do not clear when a cvar_restart is issued

#if !defined RTCW_SP
#define CVAR_WOLFINFO       2048    // DHM - NERVE :: Like userinfo, but for wolf multiplayer info
#endif // RTCW_XX

#if defined RTCW_ET
#define CVAR_UNSAFE         4096    // ydnar: unsafe system cvars (renderer, sound settings, anything that might cause a crash)
#define CVAR_SERVERINFO_NOUPDATE        8192    // gordon: WONT automatically send this to clients, but server browsers will see it
#endif // RTCW_XX

// nothing outside the Cvar_*() functions should modify these fields!
typedef struct cvar_s {
	char        *name;
	char        *string;
	char        *resetString;       // cvar_restart will reset to this value
	char        *latchedString;     // for CVAR_LATCH vars
	int flags;
	qboolean modified;              // set each time the cvar is changed
	int modificationCount;          // incremented each time the cvar is changed
	float value;                    // atof( string )
	int integer;                    // atoi( string )
	struct cvar_s *next;
	struct cvar_s *hashNext;
} cvar_t;

#define MAX_CVAR_VALUE_STRING   256

typedef int cvarHandle_t;

// the modules that run in the virtual machine can't access the cvar_t directly,
// so they must ask for structured updates
typedef struct {
	cvarHandle_t handle;
	int modificationCount;
	float value;
	int integer;
	char string[MAX_CVAR_VALUE_STRING];
} vmCvar_t;

/*
==============================================================

COLLISION DETECTION

==============================================================
*/

#include "surfaceflags.h"            // shared with the q3map utility

// plane types are used to speed some tests
// 0-2 are axial planes
#define PLANE_X         0
#define PLANE_Y         1
#define PLANE_Z         2
#define PLANE_NON_AXIAL 3

#if defined RTCW_ET
#define PLANE_NON_PLANAR    4
#endif // RTCW_XX

/*
=================
PlaneTypeForNormal
=================
*/

#if !defined RTCW_ET
#define PlaneTypeForNormal( x ) ( x[0] == 1.0 ? PLANE_X : ( x[1] == 1.0 ? PLANE_Y : ( x[2] == 1.0 ? PLANE_Z : PLANE_NON_AXIAL ) ) )
#else
//#define PlaneTypeForNormal(x) (x[0] == 1.0 ? PLANE_X : (x[1] == 1.0 ? PLANE_Y : (x[2] == 1.0 ? PLANE_Z : PLANE_NON_AXIAL) ) )
#define PlaneTypeForNormal( x ) ( x[0] == 1.0 ? PLANE_X : ( x[1] == 1.0 ? PLANE_Y : ( x[2] == 1.0 ? PLANE_Z : ( x[0] == 0.f && x[1] == 0.f && x[2] == 0.f ? PLANE_NON_PLANAR : PLANE_NON_AXIAL ) ) ) )
#endif // RTCW_XX

// plane_t structure
// !!! if this is changed, it must be changed in asm code too !!!
typedef struct cplane_s {
	vec3_t normal;
	float dist;
	uint8_t type;              // for fast side tests: 0,1,2 = axial, 3 = nonaxial
	uint8_t signbits;          // signx + (signy<<1) + (signz<<2), used as lookup during collision
	uint8_t pad[2];
} cplane_t;

#if defined RTCW_ET
#define CPLANE
#endif // RTCW_XX

// a trace is returned when a box is swept through the world
typedef struct {
	qboolean allsolid;      // if true, plane is not valid
	qboolean startsolid;    // if true, the initial point was in a solid area
	float fraction;         // time completed, 1.0 = didn't hit anything
	vec3_t endpos;          // final position
	cplane_t plane;         // surface normal at impact, transformed to world space
	int surfaceFlags;           // surface hit
	int contents;           // contents on other side of surface hit
	int entityNum;          // entity the contacted sirface is a part of
} trace_t;

// trace->entityNum can also be 0 to (MAX_GENTITIES-1)
// or ENTITYNUM_NONE, ENTITYNUM_WORLD


// markfragments are returned by CM_MarkFragments()
typedef struct {
	int firstPoint;
	int numPoints;
} markFragment_t;



typedef struct {
	vec3_t origin;
	vec3_t axis[3];
} orientation_t;

//=====================================================================


// in order from highest priority to lowest
// if none of the catchers are active, bound key strings will be executed
#define KEYCATCH_CONSOLE        0x0001
#define KEYCATCH_UI                 0x0002
#define KEYCATCH_MESSAGE        0x0004
#define KEYCATCH_CGAME          0x0008


// sound channels
// channel 0 never willingly overrides
// other channels will allways override a playing sound on that channel
typedef enum {
	CHAN_AUTO,
	CHAN_LOCAL,     // menu sounds, etc
	CHAN_WEAPON,
	CHAN_VOICE,
	CHAN_ITEM,
	CHAN_BODY,
	CHAN_LOCAL_SOUND,   // chat messages, etc

#if !defined RTCW_ET
	CHAN_ANNOUNCER      // announcer voices, etc
#else
	CHAN_ANNOUNCER,     // announcer voices, etc
#endif // RTCW_XX

#if defined RTCW_ET
	CHAN_VOICE_BG,  // xkan - background sound for voice (radio static, etc.)
#endif // RTCW_XX

} soundChannel_t;


/*
========================================================================

  ELEMENTS COMMUNICATED ACROSS THE NET

========================================================================
*/
#define ANIM_BITS       10

#define ANGLE2SHORT( x )  ( (int)( ( x ) * 65536 / 360 ) & 65535 )
#define SHORT2ANGLE( x )  ( ( x ) * ( 360.0 / 65536 ) )

#define SNAPFLAG_RATE_DELAYED   1
#define SNAPFLAG_NOT_ACTIVE     2   // snapshot used during connection and for zombies
#define SNAPFLAG_SERVERCOUNT    4   // toggled every map_restart so transitions can be detected

//
// per-level limits
//

#if defined RTCW_SP
#define MAX_CLIENTS         128     // absolute limit
#else
#define MAX_CLIENTS         64 // JPW NERVE back to q3ta default was 128		// absolute limit
#endif // RTCW_XX

#if !defined RTCW_ET
#define MAX_LOCATIONS       64
#endif // RTCW_XX

#if defined RTCW_SP
#define GENTITYNUM_BITS     10      // don't need to send any more
#else
#define GENTITYNUM_BITS     10  // JPW NERVE put q3ta default back for testing	// don't need to send any more
#endif // RTCW_XX

#if !defined RTCW_ET
//#define	GENTITYNUM_BITS		11		// don't need to send any more		(SA) upped 4/21/2001 adjusted: tr_local.h (802-822), tr_main.c (1501), sv_snapshot (206)
#endif // RTCW_XX

#define MAX_GENTITIES       ( 1 << GENTITYNUM_BITS )

// entitynums are communicated with GENTITY_BITS, so any reserved
// values thatare going to be communcated over the net need to
// also be in this range
#define ENTITYNUM_NONE      ( MAX_GENTITIES - 1 )
#define ENTITYNUM_WORLD     ( MAX_GENTITIES - 2 )
#define ENTITYNUM_MAX_NORMAL    ( MAX_GENTITIES - 2 )


#define MAX_MODELS          256     // these are sent over the net as 8 bits
#define MAX_SOUNDS          256     // so they cannot be blindly increased

#if !defined RTCW_ET
#define MAX_PARTICLES_AREAS     128
#endif // RTCW_XX

#if defined RTCW_ET
#define MAX_CS_SKINS        64
#define MAX_CSSTRINGS       32

#define MAX_CS_SHADERS      32
#define MAX_SERVER_TAGS     256
#define MAX_TAG_FILES       64
#endif // RTCW_XX

#define MAX_MULTI_SPAWNTARGETS  16 // JPW NERVE

#if !defined RTCW_ET
//#define	MAX_CONFIGSTRINGS	1024
#else
#define MAX_CONFIGSTRINGS   1024
#endif // RTCW_XX

#if !defined RTCW_ET
#define MAX_CONFIGSTRINGS   2048

#define MAX_DLIGHT_CONFIGSTRINGS    128
#define MAX_CLIPBOARD_CONFIGSTRINGS 64
#define MAX_SPLINE_CONFIGSTRINGS    64
#endif // RTCW_XX

#if defined RTCW_ET
#define MAX_DLIGHT_CONFIGSTRINGS    16
#define MAX_SPLINE_CONFIGSTRINGS    8
#endif // RTCW_XX

#define PARTICLE_SNOW128    1
#define PARTICLE_SNOW64     2
#define PARTICLE_SNOW32     3
#define PARTICLE_SNOW256    0

#define PARTICLE_BUBBLE8    4
#define PARTICLE_BUBBLE16   5
#define PARTICLE_BUBBLE32   6
#define PARTICLE_BUBBLE64   7

// these are the only configstrings that the system reserves, all the
// other ones are strictly for servergame to clientgame communication
#define CS_SERVERINFO       0       // an info string with all the serverinfo cvars
#define CS_SYSTEMINFO       1       // an info string for server system to client system configuration (timescale, etc)

#define RESERVED_CONFIGSTRINGS  2   // game can't modify below this, only the system can

#define MAX_GAMESTATE_CHARS 16000
typedef struct {
	int stringOffsets[MAX_CONFIGSTRINGS];
	char stringData[MAX_GAMESTATE_CHARS];
	int dataCount;
} gameState_t;

#if !defined RTCW_ET
#define REF_FORCE_DLIGHT    ( 1 << 31 ) // RF, passed in through overdraw parameter, force this dlight under all conditions
#define REF_JUNIOR_DLIGHT   ( 1 << 30 ) // (SA) this dlight does not light surfaces.  it only affects dynamic light grid

//=========================================================
// shared by AI and animation scripting
//
#endif // RTCW_XX

#if defined RTCW_ET
// xkan, 1/10/2003 - adapted from original SP
#endif // RTCW_XX

typedef enum
{

#if defined RTCW_MP
	// TTimo gcc: enums don't go <=0 unless you force a value
	AISTATE_NULL = -1,
#endif // RTCW_XX

	AISTATE_RELAXED,
	AISTATE_QUERY,
	AISTATE_ALERT,
	AISTATE_COMBAT,

	MAX_AISTATES
} aistateEnum_t;

#if !defined RTCW_ET
//=========================================================


// weapon grouping
#define MAX_WEAP_BANKS      12
#define MAX_WEAPS_IN_BANK   3
// JPW NERVE
#define MAX_WEAPS_IN_BANK_MP    8
#define MAX_WEAP_BANKS_MP   7
// jpw
#endif // RTCW_XX

#if defined RTCW_ET
#define REF_FORCE_DLIGHT    ( 1 << 31 ) // RF, passed in through overdraw parameter, force this dlight under all conditions
#define REF_JUNIOR_DLIGHT   ( 1 << 30 ) // (SA) this dlight does not light surfaces.  it only affects dynamic light grid
#define REF_DIRECTED_DLIGHT ( 1 << 29 ) // ydnar: global directional light, origin should be interpreted as a normal vector
#endif // RTCW_XX

#if defined RTCW_SP
#define MAX_WEAP_ALTS       WP_DYNAMITE
#elif defined RTCW_MP
#define MAX_WEAP_ALTS       WP_DYNAMITE2
#endif // RTCW_XX

// bit field limits
#define MAX_STATS               16
#define MAX_PERSISTANT          16
#define MAX_POWERUPS            16
#define MAX_WEAPONS             64  // (SA) and yet more!

#if defined RTCW_SP
#define MAX_HOLDABLE            16
#endif // RTCW_XX

// Ridah, increased this
//#define	MAX_PS_EVENTS			2
// ACK: I'd really like to make this 4, but that seems to cause network problems
#define MAX_EVENTS              4   // max events per frame before we drop events
//#define	MAX_EVENTS				2	// max events per frame before we drop events


#define PS_PMOVEFRAMECOUNTBITS  6

// playerState_t is the information needed by both the client and server
// to predict player motion and actions
// nothing outside of pmove should modify these, or some degree of prediction error
// will occur

// you can't add anything to this without modifying the code in msg.c

#if defined RTCW_ET
// (Gordon: unless it doesnt need transmitted over the network, in which case it should prolly go in the new pmext struct anyway)
#endif // RTCW_XX

// playerState_t is a full superset of entityState_t as it is used by players,
// so if a playerState_t is transmitted, the entityState_t can be fully derived
// from it.
//
// NOTE: all fields in here must be 32 bits (or those within sub-structures)
typedef struct playerState_s {
	int commandTime;            // cmd->serverTime of last executed command
	int pm_type;
	int bobCycle;               // for view bobbing and footstep generation
	int pm_flags;               // ducked, jump_held, etc
	int pm_time;

	vec3_t origin;
	vec3_t velocity;
	int weaponTime;
	int weaponDelay;            // for weapons that don't fire immediately when 'fire' is hit (grenades, venom, ...)
	int grenadeTimeLeft;            // for delayed grenade throwing.  this is set to a #define for grenade
									// lifetime when the attack button goes down, then when attack is released
									// this is the amount of time left before the grenade goes off (or if it
									// gets to 0 while in players hand, it explodes)


	int gravity;
	float leanf;                // amount of 'lean' when player is looking around corner //----(SA)	added

	int speed;
	int delta_angles[3];            // add to command angles to get view direction
									// changed by spawns, rotating objects, and teleporters

	int groundEntityNum;        // ENTITYNUM_NONE = in air

	int legsTimer;              // don't change low priority animations until this runs out
	int legsAnim;               // mask off ANIM_TOGGLEBIT

	int torsoTimer;             // don't change low priority animations until this runs out
	int torsoAnim;              // mask off ANIM_TOGGLEBIT

	int movementDir;            // a number 0 to 7 that represents the reletive angle
								// of movement to the view angle (axial and diagonals)
								// when at rest, the value will remain unchanged
								// used to twist the legs during strafing



	int eFlags;                 // copied to entityState_t->eFlags

	int eventSequence;          // pmove generated events
	int events[MAX_EVENTS];
	int eventParms[MAX_EVENTS];
	int oldEventSequence;           // so we can see which events have been added since we last converted to entityState_t

	int externalEvent;          // events set on player from another source
	int externalEventParm;
	int externalEventTime;

	int clientNum;              // ranges from 0 to MAX_CLIENTS-1

	// weapon info
	int weapon;                 // copied to entityState_t->weapon
	int weaponstate;

	// item info
	int item;

	vec3_t viewangles;          // for fixed views
	int viewheight;

	// damage feedback
	int damageEvent;            // when it changes, latch the other parms
	int damageYaw;
	int damagePitch;
	int damageCount;

	int stats[MAX_STATS];
	int persistant[MAX_PERSISTANT];         // stats that aren't cleared on death
	int powerups[MAX_POWERUPS];         // level.time that the powerup runs out
	int ammo[MAX_WEAPONS];              // total amount of ammo
	int ammoclip[MAX_WEAPONS];          // ammo in clip

#if defined RTCW_SP
	int holdable[MAX_HOLDABLE];
#else
	int holdable[16];
#endif // RTCW_XX

	int holding;                        // the current item in holdable[] that is selected (held)
	int weapons[MAX_WEAPONS / ( sizeof( int ) * 8 )];   // 64 bits for weapons held

	// Ridah, allow for individual bounding boxes
	vec3_t mins, maxs;
	float crouchMaxZ;
	float crouchViewHeight, standViewHeight, deadViewHeight;
	// variable movement speed
	float runSpeedScale, sprintSpeedScale, crouchSpeedScale;
	// done.

	// Ridah, view locking for mg42
	int viewlocked;
	int viewlocked_entNum;

#if !defined RTCW_ET
	// Ridah, need this to fix friction problems with slow zombie's whereby
	// the friction prevents them from accelerating to their full potential
#endif // RTCW_XX

	float friction;

#if !defined RTCW_ET
	// Ridah, AI character id is used for weapon association
	int aiChar;
	int teamNum;
#endif // RTCW_XX

#if defined RTCW_ET
	int nextWeapon;
	int teamNum;                        // Arnout: doesn't seem to be communicated over the net
#endif // RTCW_XX

	// Rafael

#if !defined RTCW_ET
	int gunfx;
#else
	//int			gunfx;
#endif // RTCW_XX

	// RF, burning effect is required for view blending effect
	int onFireStart;

	int serverCursorHint;               // what type of cursor hint the server is dictating
	int serverCursorHintVal;            // a value (0-255) associated with the above

	trace_t serverCursorHintTrace;      // not communicated over net, but used to store the current server-side cursorhint trace

	// ----------------------------------------------------------------------

#if !defined RTCW_ET
	// not communicated over the net at all
	// FIXME: this doesn't get saved between predicted frames on the clients-side (cg.predictedPlayerState)
#endif // RTCW_XX

	// So to use persistent variables here, which don't need to come from the server,
	// we could use a marker variable, and use that to store everything after it
	// before we read in the new values for the predictedPlayerState, then restore them
	// after copying the structure recieved from the server.

#if !defined RTCW_ET
	// (SA) yeah.  this is causing me a little bit of trouble too.  can we go ahead with the above suggestion or find an alternative?
#else
	// Arnout: use the pmoveExt_t structure in bg_public.h to store this kind of data now (presistant on client, not network transmitted)
#endif // RTCW_XX

	int ping;                   // server to game info for scoreboard
	int pmove_framecount;           // FIXME: don't transmit over the network
	int entityEventSequence;

#if !defined RTCW_ET
	int sprintTime;
#endif // RTCW_XX

	int sprintExertTime;

	// JPW NERVE -- value for all multiplayer classes with regenerating "class weapons" -- ie LT artillery, medic medpack, engineer build points, etc
	int classWeaponTime;

#if defined RTCW_SP
	int jumpTime;         // used in SP/MP to prevent jump accel
	// jpw

	int weapAnimTimer;              // don't change low priority animations until this runs out
	int weapAnim;               // mask off ANIM_TOGGLEBIT
#else
	int jumpTime;         // used in MP to prevent jump accel
	// jpw

#if !defined RTCW_ET
	int weapAnimTimer;              // don't change low priority animations until this runs out		//----(SA)	added
#endif // RTCW_XX

	int weapAnim;               // mask off ANIM_TOGGLEBIT										//----(SA)	added
#endif // RTCW_XX

	qboolean releasedFire;

	float aimSpreadScaleFloat;          // (SA) the server-side aimspreadscale that lets it track finer changes but still only
										// transmit the 8bit int to the client
	int aimSpreadScale;         // 0 - 255 increases with angular movement
	int lastFireTime;           // used by server to hold last firing frame briefly when randomly releasing trigger (AI)

	int quickGrenTime;

	int leanStopDebounceTime;

#if !defined RTCW_SP
//----(SA)	added

	// seems like heat and aimspread could be tied together somehow, however, they (appear to) change at different rates and
	// I can't currently see how to optimize this to one server->client transmission "weapstatus" value.
#endif // RTCW_XX

	int weapHeat[MAX_WEAPONS];          // some weapons can overheat.  this tracks (server-side) how hot each weapon currently is.
	int curWeapHeat;                    // value for the currently selected weapon (for transmission to client)

#if !defined RTCW_ET
	int venomTime;
#endif // RTCW_XX

#if defined RTCW_SP
//----(SA)	added
	int accShowBits;            // RF (changed from short), these should all be 32 bit
	int accHideBits;
#endif // RTCW_XX

//----(SA)	end

#if !defined RTCW_ET
	aistateEnum_t aiState;
#endif // RTCW_XX

#if defined RTCW_SP
	float footstepCount;
#else
	int identifyClient;                 // NERVE - SMF
#endif // RTCW_XX

#if defined RTCW_ET
	int identifyClientHealth;

	aistateEnum_t aiState;          // xkan, 1/10/2003
#endif // RTCW_XX

} playerState_t;


//====================================================================


//
// usercmd_t->button bits, many of which are generated by the client system,
// so they aren't game/cgame only definitions
//
#define BUTTON_ATTACK       1
#define BUTTON_TALK         2           // displays talk balloon and disables actions

#if !defined RTCW_ET
#define BUTTON_USE_HOLDABLE 4
#else
//#define	BUTTON_USE_HOLDABLE	4
#endif // RTCW_XX

#define BUTTON_GESTURE      8
#define BUTTON_WALKING      16          // walking can't just be infered from MOVE_RUN
										// because a key pressed late in the frame will
										// only generate a small move value for that frame
										// walking will use different animations and
										// won't generate footsteps
//----(SA)	added
#define BUTTON_SPRINT       32
#define BUTTON_ACTIVATE     64
//----(SA)	end

#define BUTTON_ANY          128         // any key whatsoever




//----(SA) wolf buttons
#define WBUTTON_ATTACK2     1
#define WBUTTON_ZOOM        2

#if !defined RTCW_ET
#define WBUTTON_QUICKGREN   4
#endif // RTCW_XX

#define WBUTTON_RELOAD      8
#define WBUTTON_LEANLEFT    16
#define WBUTTON_LEANRIGHT   32

#if !defined RTCW_SP
#define WBUTTON_DROP        64 // JPW NERVE
#endif // RTCW_XX

#if defined RTCW_ET
#define WBUTTON_PRONE       128 // Arnout: wbutton now
#endif // RTCW_XX

// unused

#if defined RTCW_SP
#define WBUTTON_EXTRA6      64
#endif // RTCW_XX

#if !defined RTCW_ET
#define WBUTTON_EXTRA7      128
#endif // RTCW_XX

//----(SA) end

#define MOVE_RUN            120         // if forwardmove or rightmove are >= MOVE_RUN,
										// then BUTTON_WALKING should be set

#if defined RTCW_MP
#define MP_TEAM_OFFSET      6
#define MP_CLASS_OFFSET     4
#define MP_WEAPON_OFFSET    0

#define MP_TEAM_BITS        2
#define MP_CLASS_BITS       2
#define MP_WEAPON_BITS      4

#define MP_TEAM_MASK        0xC0
#define MP_CLASS_MASK       0x30
#define MP_WEAPON_MASK      0x0F
#endif // RTCW_XX

#if defined RTCW_ET
// Arnout: doubleTap buttons - DT_NUM can be max 8
typedef enum {
	DT_NONE,
	DT_MOVELEFT,
	DT_MOVERIGHT,
	DT_FORWARD,
	DT_BACK,
	DT_LEANLEFT,
	DT_LEANRIGHT,
	DT_UP,
	DT_NUM
} dtType_t;
#endif // RTCW_XX

// usercmd_t is sent to the server each client frame
typedef struct usercmd_s {
	int serverTime;
	uint8_t buttons;
	uint8_t wbuttons;
	uint8_t weapon;

#if !defined RTCW_ET
	uint8_t holdable;          //----(SA)	added
#endif // RTCW_XX

#if defined RTCW_ET
	uint8_t flags;
#endif // RTCW_XX

	int angles[3];

	signed char forwardmove, rightmove, upmove;

#if !defined RTCW_ET
	signed char wolfkick;       // RF, we should move this over to a wbutton, this is a huge waste of bandwidth
#endif // RTCW_XX

#if defined RTCW_ET
	uint8_t doubleTap;             // Arnout: only 3 bits used
#endif // RTCW_XX

#if defined RTCW_SP
	unsigned short cld;         // NERVE - SMF - send client damage in usercmd instead of as a server command
#elif defined RTCW_MP
	char mpSetup;               // NERVE - SMF
	char identClient;           // NERVE - SMF
#else
	// rain - in ET, this can be any entity, and it's used as an array
	// index, so make sure it's unsigned
	uint8_t identClient;           // NERVE - SMF
#endif // RTCW_XX

} usercmd_t;

//===================================================================

// if entityState->solid == SOLID_BMODEL, modelindex is an inline model number
#define SOLID_BMODEL    0xffffff

typedef enum {
	TR_STATIONARY,
	TR_INTERPOLATE,             // non-parametric, but interpolate between snapshots
	TR_LINEAR,
	TR_LINEAR_STOP,
	TR_LINEAR_STOP_BACK,        //----(SA)	added.  so reverse movement can be different than forward
	TR_SINE,                    // value = base + sin( time / duration ) * delta
	TR_GRAVITY,
	// Ridah
	TR_GRAVITY_LOW,
	TR_GRAVITY_FLOAT,           // super low grav with no gravity acceleration (floating feathers/fabric/leaves/...)
	TR_GRAVITY_PAUSED,          //----(SA)	has stopped, but will still do a short trace to see if it should be switched back to TR_GRAVITY
	TR_ACCELERATE,

#if !defined RTCW_ET
	TR_DECCELERATE
#else
	TR_DECCELERATE,
	// Gordon
	TR_SPLINE,
	TR_LINEAR_PATH
#endif // RTCW_XX

} trType_t;

typedef struct {
	trType_t trType;
	int trTime;
	int trDuration;             // if non 0, trTime + trDuration = stop time
//----(SA)	removed
	vec3_t trBase;
	vec3_t trDelta;             // velocity, etc
//----(SA)	removed
} trajectory_t;

#if !defined RTCW_ET
// RF, put this here so we have a central means of defining a Zombie (kind of a hack, but this is to minimize bandwidth usage)
#define SET_FLAMING_ZOMBIE( x,y ) ( x.frame = y )
#define IS_FLAMING_ZOMBIE( x )    ( x.frame == 1 )
#endif // RTCW_XX

// entityState_t is the information conveyed from the server
// in an update message about entities that the client will
// need to render in some way
// Different eTypes may use the information in different ways
// The messages are delta compressed, so it doesn't really matter if
// the structure size is fairly large
//
// NOTE: all fields in here must be 32 bits (or those within sub-structures)

#if defined RTCW_ET
typedef enum {
	ET_GENERAL,
	ET_PLAYER,
	ET_ITEM,
	ET_MISSILE,
	ET_MOVER,
	ET_BEAM,
	ET_PORTAL,
	ET_SPEAKER,
	ET_PUSH_TRIGGER,
	ET_TELEPORT_TRIGGER,
	ET_INVISIBLE,
	ET_CONCUSSIVE_TRIGGER,  // JPW NERVE trigger for concussive dust particles
	ET_OID_TRIGGER,         // DHM - Nerve :: Objective Info Display
	ET_EXPLOSIVE_INDICATOR, // NERVE - SMF

	//---- (SA) Wolf
	ET_EXPLOSIVE,           // brush that will break into smaller bits when damaged
	ET_EF_SPOTLIGHT,
	ET_ALARMBOX,
	ET_CORONA,
	ET_TRAP,

	ET_GAMEMODEL,           // misc_gamemodel.  similar to misc_model, but it's a dynamic model so we have LOD
	ET_FOOTLOCKER,  //----(SA)	added
	//---- end

	ET_FLAMEBARREL,
	ET_FP_PARTS,

	// FIRE PROPS
	ET_FIRE_COLUMN,
	ET_FIRE_COLUMN_SMOKE,
	ET_RAMJET,

	ET_FLAMETHROWER_CHUNK,      // DHM - NERVE :: Used in server side collision detection for flamethrower

	ET_EXPLO_PART,

	ET_PROP,

	ET_AI_EFFECT,

	ET_CAMERA,
	ET_MOVERSCALED,

	ET_CONSTRUCTIBLE_INDICATOR,
	ET_CONSTRUCTIBLE,
	ET_CONSTRUCTIBLE_MARKER,
	ET_BOMB,
	ET_WAYPOINT,
	ET_BEAM_2,
	ET_TANK_INDICATOR,
	ET_TANK_INDICATOR_DEAD,
	// Start - TAT - 8/29/2002
	// An indicator object created by the bot code to show where the bots are moving to
	ET_BOTGOAL_INDICATOR,
	// End - TA - 8/29/2002
	ET_CORPSE,              // Arnout: dead player
	ET_SMOKER,              // Arnout: target_smoke entity

	ET_TEMPHEAD,            // Gordon: temporary head for clients for bullet traces
	ET_MG42_BARREL,         // Arnout: MG42 barrel
	ET_TEMPLEGS,            // Arnout: temporary leg for clients for bullet traces
	ET_TRIGGER_MULTIPLE,
	ET_TRIGGER_FLAGONLY,
	ET_TRIGGER_FLAGONLY_MULTIPLE,
	ET_GAMEMANAGER,
	ET_AAGUN,
	ET_CABINET_H,
	ET_CABINET_A,
	ET_HEALER,
	ET_SUPPLIER,

	ET_LANDMINE_HINT,       // Gordon: landmine hint for botsetgoalstate filter
	ET_ATTRACTOR_HINT,      // Gordon: attractor hint for botsetgoalstate filter
	ET_SNIPER_HINT,         // Gordon: sniper hint for botsetgoalstate filter
	ET_LANDMINESPOT_HINT,   // Gordon: landminespot hint for botsetgoalstate filter

	ET_COMMANDMAP_MARKER,

	ET_WOLF_OBJECTIVE,

	ET_EVENTS               // any of the EV_* events can be added freestanding
							// by setting eType to ET_EVENTS + eventNum
							// this avoids having to set eFlags and eventNum
} entityType_t;
#endif // RTCW_XX

typedef struct entityState_s {
	int number;             // entity index

#if !defined RTCW_ET
	int eType;              // entityType_t
#endif // RTCW_XX

#if defined RTCW_ET
	entityType_t eType;             // entityType_t
#endif // RTCW_XX

	int eFlags;

	trajectory_t pos;       // for calculating position
	trajectory_t apos;      // for calculating angles

	int time;
	int time2;

	vec3_t origin;
	vec3_t origin2;

	vec3_t angles;
	vec3_t angles2;

	int otherEntityNum;     // shotgun sources, etc
	int otherEntityNum2;

	int groundEntityNum;        // -1 = in air

	int constantLight;      // r + (g<<8) + (b<<16) + (intensity<<24)
	int dl_intensity;       // used for coronas
	int loopSound;          // constantly loop this sound

	int modelindex;
	int modelindex2;
	int clientNum;          // 0 to (MAX_CLIENTS - 1), for players and corpses
	int frame;

	int solid;              // for client side prediction, trap_linkentity sets this properly

	// old style events, in for compatibility only
	int event;
	int eventParm;

	int eventSequence;      // pmove generated events
	int events[MAX_EVENTS];
	int eventParms[MAX_EVENTS];

	// for players

#if !defined RTCW_ET
	int powerups;           // bit flags
#else
	int powerups;           // bit flags	// Arnout: used to store entState_t for non-player entities (so we know to draw them translucent clientsided)
#endif // RTCW_XX

	int weapon;             // determines weapon and flash model, etc
	int legsAnim;           // mask off ANIM_TOGGLEBIT
	int torsoAnim;          // mask off ANIM_TOGGLEBIT
//	int		weapAnim;		// mask off ANIM_TOGGLEBIT	//----(SA)	removed (weap anims will be client-side only)

	int density;            // for particle effects

	int dmgFlags;           // to pass along additional information for damage effects for players/ Also used for cursorhints for non-player entities

	// Ridah
	int onFireStart, onFireEnd;

#if !defined RTCW_ET
	int aiChar, teamNum;
#else
	int nextWeapon;
	int teamNum;
#endif // RTCW_XX

	int effect1Time, effect2Time, effect3Time;

#if !defined RTCW_ET
	aistateEnum_t aiState;
#else
	aistateEnum_t aiState;      // xkan, 1/10/2003
#endif // RTCW_XX

	int animMovetype;       // clients can't derive movetype of other clients for anim scripting system

} entityState_t;

typedef enum {
	CA_UNINITIALIZED,
	CA_DISCONNECTED,    // not talking to a server
	CA_AUTHORIZING,     // not used any more, was checking cd key
	CA_CONNECTING,      // sending request packets to the server
	CA_CHALLENGING,     // sending challenge packets to the server
	CA_CONNECTED,       // netchan_t established, getting gamestate
	CA_LOADING,         // only during cgame initialization, never during main loop
	CA_PRIMED,          // got gamestate, waiting for first frame
	CA_ACTIVE,          // game views should be displayed
	CA_CINEMATIC        // playing a cinematic or a static pic, not connected to a server
} connstate_t;

// font support

#define GLYPH_START 0
#define GLYPH_END 255
#define GLYPH_CHARSTART 32
#define GLYPH_CHAREND 127
#define GLYPHS_PER_FONT GLYPH_END - GLYPH_START + 1
typedef struct {
	int height;     // number of scan lines
	int top;        // top of glyph in buffer
	int bottom;     // bottom of glyph in buffer
	int pitch;      // width for copying
	int xSkip;      // x adjustment
	int imageWidth; // width of actual image
	int imageHeight; // height of actual image
	float s;        // x offset in image where glyph starts
	float t;        // y offset in image where glyph starts
	float s2;
	float t2;
	qhandle_t glyph; // handle to the shader with the glyph
	char shaderName[32];
} glyphInfo_t;

typedef struct {
	glyphInfo_t glyphs [GLYPHS_PER_FONT];
	float glyphScale;
	char name[MAX_QPATH];
} fontInfo_t;

#define Square( x ) ( ( x ) * ( x ) )

// real time
//=============================================


typedef struct qtime_s {
	int tm_sec;     /* seconds after the minute - [0,59] */
	int tm_min;     /* minutes after the hour - [0,59] */
	int tm_hour;    /* hours since midnight - [0,23] */
	int tm_mday;    /* day of the month - [1,31] */
	int tm_mon;     /* months since January - [0,11] */
	int tm_year;    /* years since 1900 */
	int tm_wday;    /* days since Sunday - [0,6] */
	int tm_yday;    /* days since January 1 - [0,365] */
	int tm_isdst;   /* daylight savings time flag */
} qtime_t;


// server browser sources

#if defined RTCW_SP
#define AS_LOCAL            0
#define AS_MPLAYER      1
#define AS_GLOBAL           2
#define AS_FAVORITES    3
#else
#define AS_LOCAL        0
#define AS_GLOBAL       1           // NERVE - SMF - modified
#define AS_FAVORITES    2

#if !defined RTCW_ET
#define AS_MPLAYER      3
#endif // RTCW_XX

#endif // RTCW_XX

// cinematic states
typedef enum {
	FMV_IDLE,
	FMV_PLAY,       // play
	FMV_EOF,        // all other conditions, i.e. stop/EOF/abort
	FMV_ID_BLT,
	FMV_ID_IDLE,
	FMV_LOOPED,
	FMV_ID_WAIT
} e_status;

typedef enum _flag_status {
	FLAG_ATBASE = 0,
	FLAG_TAKEN,         // CTF
	FLAG_TAKEN_RED,     // One Flag CTF
	FLAG_TAKEN_BLUE,    // One Flag CTF
	FLAG_DROPPED
} flagStatus_t;


#if !defined RTCW_ET
#define MAX_GLOBAL_SERVERS          2048
#else
#define MAX_GLOBAL_SERVERS          4096
#endif // RTCW_XX

#define MAX_OTHER_SERVERS           128
#define MAX_PINGREQUESTS            16
#define MAX_SERVERSTATUSREQUESTS    16

#if !defined RTCW_ET
#define SAY_ALL     0
#define SAY_TEAM    1
#define SAY_TELL    2
#endif // RTCW_XX

#define CDKEY_LEN 16
#define CDCHKSUM_LEN 2

// NERVE - SMF - localization
typedef enum {
	LANGUAGE_FRENCH = 0,
	LANGUAGE_GERMAN,
	LANGUAGE_ITALIAN,
	LANGUAGE_SPANISH,

	MAX_LANGUAGES
} languages_t;

#if !defined RTCW_SP
// NERVE - SMF - wolf server/game states
typedef enum {
	GS_INITIALIZE = -1,
	GS_PLAYING,
	GS_WARMUP_COUNTDOWN,
	GS_WARMUP,
	GS_INTERMISSION,
	GS_WAITING_FOR_PLAYERS,
	GS_RESET
} gamestate_t;

#if !defined RTCW_ET
// TTimo - voting config flags
#define VOTEFLAGS_RESTART           ( 1 << 0 )
#define VOTEFLAGS_RESETMATCH    ( 1 << 1 )
#define VOTEFLAGS_STARTMATCH    ( 1 << 2 )
#define VOTEFLAGS_NEXTMAP           ( 1 << 3 )
#define VOTEFLAGS_SWAP              ( 1 << 4 )
#define VOTEFLAGS_TYPE              ( 1 << 5 )
#define VOTEFLAGS_KICK              ( 1 << 6 )
#define VOTEFLAGS_MAP                   ( 1 << 7 )
#endif // RTCW_XX

#if defined RTCW_ET
#define SQR( a ) ( ( a ) * ( a ) )
#endif // RTCW_XX

#endif // RTCW_XX

#endif  // __Q_SHARED_H

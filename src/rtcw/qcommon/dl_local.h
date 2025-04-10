/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#ifndef _DL_LOCAL_H_
#define _DL_LOCAL_H_

//bani
#ifdef __GNUC__
#define _attribute( x ) __attribute__( x )
#else
#define _attribute( x )
#endif

// system API
// only the restricted subset we need

int Com_VPrintf( const char *fmt, va_list pArgs ) _attribute( ( format( printf,1,0 ) ) );
int Com_DPrintf( const char *fmt, ... ) _attribute( ( format( printf,1,2 ) ) );
int Com_Printf( const char *fmt, ... ) _attribute( ( format( printf,1,2 ) ) );
void Com_Error( int code, const char *fmt, ... ) _attribute( ( format( printf,2,3 ) ) ); // watch out, we don't define ERR_FATAL and stuff
void    Cvar_SetValue( const char *var_name, float value );
void    Cvar_Set( const char *var_name, const char *value );
char    * va( char *format, ... ) _attribute( ( format( printf,1,2 ) ) );

#ifdef WIN32
  #define Q_stricmp stricmp
#else
  #define Q_stricmp strcasecmp
#endif

extern int com_errorEntered;

#endif

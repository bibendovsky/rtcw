/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

#if defined RTCW_SP
// Copyright (C) 1999-2000 Id Software, Inc.
//
#endif // RTCW_XX

// q_shared.c -- stateless support routines that are included in each code dll
#include "q_shared.h"

/*
============
Com_Clamp
============
*/
float Com_Clamp( float min, float max, float value ) {
	if ( value < min ) {
		return min;
	}
	if ( value > max ) {
		return max;
	}
	return value;
}


#if defined RTCW_ET
/*
COM_FixPath()
unixifies a pathname
*/

void COM_FixPath( char *pathname ) {
	while ( *pathname )
	{
		if ( *pathname == '\\' ) {
			*pathname = '/';
		}
		pathname++;
	}
}
#endif // RTCW_XX

/*
============
COM_SkipPath
============
*/
char *COM_SkipPath( char *pathname ) {
	char    *last;

	last = pathname;
	while ( *pathname )
	{
		if ( *pathname == '/' ) {
			last = pathname + 1;
		}
		pathname++;
	}
	return last;
}

/*
============
COM_StripExtension
============
*/
void COM_StripExtension( const char *in, char *out ) {
	while ( *in && *in != '.' ) {
		*out++ = *in++;
	}
	*out = 0;
}

#if !defined RTCW_SP
/*
============
COM_StripExtension2
a safer version
============
*/
void COM_StripExtension2( const char *in, char *out, int destsize ) {
	int len = 0;
	while ( len < destsize - 1 && *in && *in != '.' ) {
		*out++ = *in++;
		len++;
	}
	*out = 0;
}
#endif // RTCW_XX


#if !defined RTCW_ET
/*
============
COM_StripFilename
============
*/
#endif // RTCW_XX

void COM_StripFilename( const char *in, char *out ) {
	char *end;

#if !defined RTCW_ET
	Q_strncpyz( out, in, strlen( in ) );
#else
	Q_strncpyz( out, in, strlen( in ) + 1 );
#endif // RTCW_XX

	end = COM_SkipPath( out );
	*end = 0;
}


/*
==================
COM_DefaultExtension
==================
*/
void COM_DefaultExtension( char *path, int maxSize, const char *extension ) {
	char oldPath[MAX_QPATH];
	char    *src;

//
// if path doesn't have a .EXT, append extension
// (extension should include the .)
//
	src = path + strlen( path ) - 1;

	while ( *src != '/' && src != path ) {
		if ( *src == '.' ) {
			return;                 // it has an extension
		}
		src--;
	}

	Q_strncpyz( oldPath, path, sizeof( oldPath ) );
	Com_sprintf( path, maxSize, "%s%s", oldPath, extension );
}

//============================================================================
/*
==================
COM_BitCheck

  Allows bit-wise checks on arrays with more than one item (> 32 bits)
==================
*/
qboolean COM_BitCheck( const int array[], int bitNum ) {
	int i;

	i = 0;
	while ( bitNum > 31 ) {
		i++;
		bitNum -= 32;
	}

	return ( ( array[i] & ( 1 << bitNum ) ) != 0 );  // (SA) heh, whoops. :)
}

/*
==================
COM_BitSet

  Allows bit-wise SETS on arrays with more than one item (> 32 bits)
==================
*/
void COM_BitSet( int array[], int bitNum ) {
	int i;

	i = 0;
	while ( bitNum > 31 ) {
		i++;
		bitNum -= 32;
	}

	array[i] |= ( 1 << bitNum );
}

/*
==================
COM_BitClear

  Allows bit-wise CLEAR on arrays with more than one item (> 32 bits)
==================
*/
void COM_BitClear( int array[], int bitNum ) {
	int i;

	i = 0;
	while ( bitNum > 31 ) {
		i++;
		bitNum -= 32;
	}

	array[i] &= ~( 1 << bitNum );
}
//============================================================================


/*
============================================================================

PARSING

============================================================================
*/

static char com_token[MAX_TOKEN_CHARS];
static char com_parsename[MAX_TOKEN_CHARS];
static int com_lines;

static int backup_lines;
static const char* backup_text;

#if !defined RTCW_ET
/*
================
COM_BeginParseSession
================
*/
#endif // RTCW_XX

void COM_BeginParseSession( const char *name ) {
	com_lines = 0;
	Com_sprintf( com_parsename, sizeof( com_parsename ), "%s", name );
}

#if !defined RTCW_SP
void COM_BackupParseSession( const char **data_p ) {
	backup_lines = com_lines;
	backup_text = *data_p;
}
#endif // RTCW_XX

#if !defined RTCW_ET
/*
================
COM_RestoreParseSession
================
*/
#endif // RTCW_XX

void COM_RestoreParseSession( const char **data_p ) {
	com_lines = backup_lines;
	*data_p = backup_text;
}

#if !defined RTCW_ET
/*
================
COM_SetCurrentParseLine
================
*/
#endif // RTCW_XX

void COM_SetCurrentParseLine( int line ) {
	com_lines = line;
}

#if !defined RTCW_ET
/*
================
COM_GetCurrentParseLine
================
*/
#endif // RTCW_XX

int COM_GetCurrentParseLine( void ) {
	return com_lines;
}

#if !defined RTCW_ET
/*
================
COM_Parse
================
*/
#endif // RTCW_XX

char *COM_Parse( const char **data_p ) {
	return COM_ParseExt( data_p, qtrue );
}

#if !defined RTCW_ET
/*
================
COM_ParseError
================
*/
#endif // RTCW_XX

void COM_ParseError( const char *format, ... ) {
	va_list argptr;
	static char string[4096];

	va_start( argptr, format );

#if defined RTCW_SP
	vsprintf( string, format, argptr );
#else
	Q_vsnprintf( string, sizeof( string ), format, argptr );
#endif // RTCW_XX

	va_end( argptr );

	Com_Printf( "ERROR: %s, line %d: %s\n", com_parsename, com_lines, string );
}

#if !defined RTCW_ET
/*
================
COM_ParseWarning
================
*/
#endif // RTCW_XX

void COM_ParseWarning( const char *format, ... ) {
	va_list argptr;
	static char string[4096];

	va_start( argptr, format );

#if defined RTCW_SP
	vsprintf( string, format, argptr );
#else
	Q_vsnprintf( string, sizeof( string ), format, argptr );
#endif // RTCW_XX

	va_end( argptr );

	Com_Printf( "WARNING: %s, line %d: %s\n", com_parsename, com_lines, string );
}

/*
==============
SkipWhitespace

Parse a token out of a string
Will never return NULL, just empty strings

If "allowLineBreaks" is qtrue then an empty
string will be returned if the next token is
a newline.
==============
*/

#if defined RTCW_SP
const char *SkipWhitespace( const char *data, qboolean *hasNewLines ) {
#else
static const char *SkipWhitespace( const char *data, qboolean *hasNewLines ) {
#endif // RTCW_XX

	int c;

	while ( ( c = *data ) <= ' ' ) {

#if !defined RTCW_SP
		if ( !c ) {
			return NULL;
		}
#endif // RTCW_XX

		if ( c == '\n' ) {
			com_lines++;
			*hasNewLines = qtrue;

#if defined RTCW_SP
		} else if ( !c )   {
			return NULL;
#endif // RTCW_XX

		}
		data++;
	}

	return data;
}

#if !defined RTCW_ET
/*
================
COM_Compress
================
*/
#endif // RTCW_XX

int COM_Compress( char *data_p ) {
	char *datai, *datao;
	int c, pc, size;
	qboolean ws = qfalse;

	size = 0;
	pc = 0;
	datai = datao = data_p;
	if ( datai ) {
		while ( ( c = *datai ) != 0 ) {
			if ( c == 13 || c == 10 ) {
				*datao = c;
				datao++;
				ws = qfalse;
				pc = c;
				datai++;
				size++;
				// skip double slash comments
			} else if ( c == '/' && datai[1] == '/' ) {
				while ( *datai && *datai != '\n' ) {
					datai++;
				}
				ws = qfalse;
				// skip /* */ comments
			} else if ( c == '/' && datai[1] == '*' ) {

#if defined RTCW_ET
				datai += 2; // Arnout: skip over '/*'
#endif // RTCW_XX

				while ( *datai && ( *datai != '*' || datai[1] != '/' ) ) {
					datai++;
				}
				if ( *datai ) {
					datai += 2;
				}
				ws = qfalse;
			} else {
				if ( ws ) {
					*datao = ' ';
					datao++;
				}
				*datao = c;
				datao++;
				datai++;
				ws = qfalse;
				pc = c;
				size++;
			}
		}
	}
	*datao = 0;
	return size;
}

#if !defined RTCW_ET
/*
================
COM_ParseExt
================
*/
#endif // RTCW_XX

char *COM_ParseExt( const char **data_p, qboolean allowLineBreaks ) {
	int c = 0, len;
	qboolean hasNewLines = qfalse;
	const char *data;

	data = *data_p;
	len = 0;
	com_token[0] = 0;

	// make sure incoming data is valid
	if ( !data ) {
		*data_p = NULL;
		return com_token;
	}

	// RF, backup the session data so we can unget easily

#if defined RTCW_SP
	backup_lines = com_lines;
	backup_text = *data_p;
#else
	COM_BackupParseSession( data_p );
#endif // RTCW_XX

	while ( 1 )
	{
		// skip whitespace
		data = SkipWhitespace( data, &hasNewLines );
		if ( !data ) {
			*data_p = NULL;
			return com_token;
		}
		if ( hasNewLines && !allowLineBreaks ) {
			*data_p = data;
			return com_token;
		}

		c = *data;

		// skip double slash comments
		if ( c == '/' && data[1] == '/' ) {
			data += 2;
			while ( *data && *data != '\n' ) {
				data++;
			}

#if defined RTCW_ET
//			com_lines++;
#endif // RTCW_XX

		}
		// skip /* */ comments
		else if ( c == '/' && data[1] == '*' ) {
			data += 2;
			while ( *data && ( *data != '*' || data[1] != '/' ) )
			{
				data++;

#if defined RTCW_ET
				if ( *data == '\n' ) {
//					com_lines++;
				}
#endif // RTCW_XX

			}
			if ( *data ) {
				data += 2;
			}
		} else
		{
			break;
		}
	}

	// handle quoted strings
	if ( c == '\"' ) {
		data++;
		while ( 1 )
		{
			c = *data++;

#if defined RTCW_ET
			if ( c == '\\' && *( data ) == '\"' ) {
				// Arnout: string-in-string
				if ( len < MAX_TOKEN_CHARS ) {
					com_token[len] = '\"';
					len++;
				}
				data++;

				while ( 1 ) {
					c = *data++;

					if ( !c ) {
						com_token[len] = 0;
						*data_p = ( char * ) data;
						break;
					}
					if ( ( c == '\\' && *( data ) == '\"' ) ) {
						if ( len < MAX_TOKEN_CHARS ) {
							com_token[len] = '\"';
							len++;
						}
						data++;
						c = *data++;
						break;
					}
					if ( len < MAX_TOKEN_CHARS ) {
						com_token[len] = c;
						len++;
					}
				}
			}
#endif // RTCW_XX

			if ( c == '\"' || !c ) {
				com_token[len] = 0;
				*data_p = ( char * ) data;
				return com_token;
			}
			if ( len < MAX_TOKEN_CHARS ) {
				com_token[len] = c;
				len++;
			}
		}
	}

	// parse a regular word
	do
	{
		if ( len < MAX_TOKEN_CHARS ) {
			com_token[len] = c;
			len++;
		}
		data++;
		c = *data;
		if ( c == '\n' ) {
			com_lines++;
		}

#if defined RTCW_SP
	} while ( c > ' ' );
#else
	} while ( c > 32 );
#endif // RTCW_XX

	if ( len == MAX_TOKEN_CHARS ) {
//		Com_Printf ("Token exceeded %i chars, discarded.\n", MAX_TOKEN_CHARS);
		len = 0;
	}

#if defined RTCW_SP
	com_token[len] = '\0';
#else
	com_token[len] = 0;
#endif // RTCW_XX

	*data_p = ( char * ) data;
	return com_token;
}




/*
==================
COM_MatchToken
==================
*/
void COM_MatchToken( const char **buf_p, const char *match ) {
	char    *token;

	token = COM_Parse( buf_p );
	if ( strcmp( token, match ) ) {
		Com_Error( ERR_DROP, "MatchToken: %s != %s", token, match );
	}
}

#if defined RTCW_ET
/*
=================
SkipBracedSection_Depth

=================
*/
void SkipBracedSection_Depth( const char **program, int depth ) {
	char            *token;

	do {
		token = COM_ParseExt( program, qtrue );
		if ( token[1] == 0 ) {
			if ( token[0] == '{' ) {
				depth++;
			} else if ( token[0] == '}' )     {
				depth--;
			}
		}
	} while ( depth && *program );
}
#endif // RTCW_XX

/*
=================
SkipBracedSection

The next token should be an open brace.
Skips until a matching close brace is found.
Internal brace depths are properly skipped.
=================
*/

#if !defined RTCW_MP
void SkipBracedSection( const char **program ) {
#else
void SkipBracedSection_Depth( const char **program, int depth ) {
#endif // RTCW_XX

	char            *token;

#if !defined RTCW_MP
	int depth;

	depth = 0;
#endif // RTCW_XX

	do {
		token = COM_ParseExt( program, qtrue );
		if ( token[1] == 0 ) {
			if ( token[0] == '{' ) {
				depth++;
			} else if ( token[0] == '}' )     {
				depth--;
			}
		}
	} while ( depth && *program );
}

#if defined RTCW_MP
/*
=================
SkipBracedSection

The next token should be an open brace.
Skips until a matching close brace is found.
Internal brace depths are properly skipped.
=================
*/
void SkipBracedSection( const char **program ) {
	SkipBracedSection_Depth( program, 0 );
}
#endif // RTCW_XX

/*
=================
SkipRestOfLine
=================
*/
void SkipRestOfLine( const char **data ) {
	const char    *p;
	int c;

	p = *data;
	while ( ( c = *p++ ) != 0 ) {
		if ( c == '\n' ) {
			com_lines++;
			break;
		}
	}

	*data = p;
}

#if !defined RTCW_ET
/*
================
Parse1DMatrix
================
*/
#endif // RTCW_XX

void Parse1DMatrix( const char **buf_p, int x, float *m ) {
	char    *token;
	int i;

	COM_MatchToken( buf_p, "(" );

	for ( i = 0 ; i < x ; i++ ) {
		token = COM_Parse( buf_p );
		m[i] = atof( token );
	}

	COM_MatchToken( buf_p, ")" );
}

#if !defined RTCW_ET
/*
================
Parse2DMatrix
================
*/
#endif // RTCW_XX

void Parse2DMatrix( const char **buf_p, int y, int x, float *m ) {
	int i;

	COM_MatchToken( buf_p, "(" );

	for ( i = 0 ; i < y ; i++ ) {
		Parse1DMatrix( buf_p, x, m + i * x );
	}

	COM_MatchToken( buf_p, ")" );
}

#if !defined RTCW_ET
/*
================
Parse3DMatrix
================
*/
#endif // RTCW_XX

void Parse3DMatrix( const char **buf_p, int z, int y, int x, float *m ) {
	int i;

	COM_MatchToken( buf_p, "(" );

	for ( i = 0 ; i < z ; i++ ) {
		Parse2DMatrix( buf_p, y, x, m + i * x * y );
	}

	COM_MatchToken( buf_p, ")" );
}


#if defined RTCW_ET
/*
===============
Com_ParseInfos
===============
*/
int Com_ParseInfos( const char *buf, int max, char infos[][MAX_INFO_STRING] ) {
	const char  *token;
	int count;
	char key[MAX_TOKEN_CHARS];

	count = 0;

	while ( 1 ) {
		token = COM_Parse( &buf );
		if ( !token[0] ) {
			break;
		}
		if ( strcmp( token, "{" ) ) {
			Com_Printf( "Missing { in info file\n" );
			break;
		}

		if ( count == max ) {
			Com_Printf( "Max infos exceeded\n" );
			break;
		}

		infos[count][0] = 0;
		while ( 1 ) {
			token = COM_Parse( &buf );
			if ( !token[0] ) {
				Com_Printf( "Unexpected end of info file\n" );
				break;
			}
			if ( !strcmp( token, "}" ) ) {
				break;
			}
			Q_strncpyz( key, token, sizeof( key ) );

			token = COM_ParseExt( &buf, qfalse );
			if ( !token[0] ) {
				token = "<NULL>";
			}
			Info_SetValueForKey( infos[count], key, token );
		}
		count++;
	}

	return count;
}
#endif // RTCW_XX

/*
============================================================================

					LIBRARY REPLACEMENT FUNCTIONS

============================================================================
*/

int Q_isprint( int c ) {
	if ( c >= 0x20 && c <= 0x7E ) {
		return ( 1 );
	}
	return ( 0 );
}

int Q_islower( int c ) {
	if ( c >= 'a' && c <= 'z' ) {
		return ( 1 );
	}
	return ( 0 );
}

int Q_isupper( int c ) {
	if ( c >= 'A' && c <= 'Z' ) {
		return ( 1 );
	}
	return ( 0 );
}

int Q_isalpha( int c ) {
	if ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) ) {
		return ( 1 );
	}
	return ( 0 );
}

#if !defined RTCW_MP
//----(SA)	added
int Q_isnumeric( int c ) {
	if ( c >= '0' && c <= '9' ) {
		return ( 1 );
	}
	return ( 0 );
}

int Q_isalphanumeric( int c ) {
	if ( Q_isalpha( c ) ||
		 Q_isnumeric( c ) ) {
		return( 1 );
	}
	return ( 0 );
}

int Q_isforfilename( int c ) {
	if ( ( Q_isalphanumeric( c ) || c == '_' ) && c != ' ' ) { // space not allowed in filename
		return( 1 );
	}
	return ( 0 );
}
//----(SA)	end
#endif // RTCW_XX

char* Q_strrchr( const char* string, int c ) {
	char cc = c;
	char *s;
	char *sp = (char *)0;

	s = (char*)string;

	while ( *s )
	{
		if ( *s == cc ) {
			sp = s;
		}
		s++;
	}
	if ( cc == 0 ) {
		sp = s;
	}

	return sp;
}

/*
=============
Q_strncpyz

Safe strncpy that ensures a trailing zero
=============
*/
void Q_strncpyz( char *dest, const char *src, int destsize ) {
	if ( !src ) {
		Com_Error( ERR_FATAL, "Q_strncpyz: NULL src" );
	}
	if ( destsize < 1 ) {
		Com_Error( ERR_FATAL,"Q_strncpyz: destsize < 1" );
	}

	strncpy( dest, src, destsize - 1 );
	dest[destsize - 1] = 0;
}

int Q_stricmpn( const char *s1, const char *s2, int n ) {
	int c1, c2;

	do {
		c1 = *s1++;
		c2 = *s2++;

		if ( !n-- ) {
			return 0;       // strings are equal until end point
		}

		if ( c1 != c2 ) {

#if defined RTCW_SP
			if ( Q_islower( c1 ) ) {
#else
			if ( c1 >= 'a' && c1 <= 'z' ) {
#endif // RTCW_XX

				c1 -= ( 'a' - 'A' );
			}

#if defined RTCW_SP
			if ( Q_islower( c2 ) ) {
#else
			if ( c2 >= 'a' && c2 <= 'z' ) {
#endif // RTCW_XX

				c2 -= ( 'a' - 'A' );
			}
			if ( c1 != c2 ) {
				return c1 < c2 ? -1 : 1;
			}
		}
	} while ( c1 );

	return 0;       // strings are equal
}

int Q_strncmp( const char *s1, const char *s2, int n ) {
	int c1, c2;

	do {
		c1 = *s1++;
		c2 = *s2++;

		if ( !n-- ) {
			return 0;       // strings are equal until end point
		}

		if ( c1 != c2 ) {
			return c1 < c2 ? -1 : 1;
		}
	} while ( c1 );

	return 0;       // strings are equal
}

int Q_stricmp( const char *s1, const char *s2 ) {
	return ( s1 && s2 ) ? Q_stricmpn( s1, s2, 99999 ) : -1;
}


char *Q_strlwr( char *s1 ) {
	char    *s;

#if !defined RTCW_ET
	s = s1;
	while ( *s ) {
		*s = tolower( *s );
		s++;
	}
#else
	for ( s = s1; *s; ++s ) {
		if ( ( 'A' <= *s ) && ( *s <= 'Z' ) ) {
			*s -= 'A' - 'a';
		}
	}
#endif // RTCW_XX
	return s1;
}

char *Q_strupr( char *s1 ) {

#if !defined RTCW_ET
	char    *s;

	s = s1;
	while ( *s ) {
		*s = toupper( *s );
		s++;
	}
#else
	char* cp;

	for ( cp = s1 ; *cp ; ++cp ) {
		if ( ( 'a' <= *cp ) && ( *cp <= 'z' ) ) {
			*cp += 'A' - 'a';
		}
	}
#endif // RTCW_XX

	return s1;
}


// never goes past bounds or leaves without a terminating 0
void Q_strcat( char *dest, int size, const char *src ) {
	intptr_t l1;

	l1 = strlen( dest );
	if ( l1 >= size ) {
		Com_Error( ERR_FATAL, "Q_strcat: already overflowed" );
	}
	Q_strncpyz( dest + l1, src, size - l1 );
}


int Q_PrintStrlen( const char *string ) {
	int len;
	const char  *p;

	if ( !string ) {
		return 0;
	}

	len = 0;
	p = string;
	while ( *p ) {
		if ( Q_IsColorString( p ) ) {
			p += 2;
			continue;
		}
		p++;
		len++;
	}

	return len;
}


char *Q_CleanStr( char *string ) {
	char*   d;
	char*   s;
	int c;

	s = string;
	d = string;
	while ( ( c = *s ) != 0 ) {
		if ( Q_IsColorString( s ) ) {
			s++;
		} else if ( c >= 0x20 && c <= 0x7E )   {
			*d++ = c;
		}
		s++;
	}
	*d = '\0';

	return string;
}

#if defined RTCW_ET
// strips whitespaces and bad characters
qboolean Q_isBadDirChar( char c ) {
	char badchars[] = { ';', '&', '(', ')', '|', '<', '>', '*', '?', '[', ']', '~', '+', '@', '!', '\\', '/', ' ', '\'', '\"', '\0' };
	int i;

	for ( i = 0; badchars[i] != '\0'; i++ ) {
		if ( c == badchars[i] ) {
			return qtrue;
		}
	}

	return qfalse;
}

char *Q_CleanDirName( char *dirname ) {
	char*   d;
	char*   s;

	s = dirname;
	d = dirname;

	// clear trailing .'s
	while ( *s == '.' ) {
		s++;
	}

	while ( *s != '\0' ) {
		if ( !Q_isBadDirChar( *s ) ) {
			*d++ = *s;
		}
		s++;
	}
	*d = '\0';

	return dirname;
}
#endif // RTCW_XX

void QDECL Com_sprintf( char *dest, int size, const char *fmt, ... ) {

#if !defined RTCW_ET
	int len;
#else
	int ret;
#endif // RTCW_XX

	va_list argptr;

#if defined RTCW_SP
	char bigbuffer[32000];      // big, but small enough to fit in PPC stack
#endif // RTCW_XX

#if defined RTCW_MP
	/*
	C99 for vsnprintf:
	return the number of characters  (excluding  the  trailing  '\0')
	which would have been written to the final string if enough space had been available.
	*/
#endif // RTCW_XX

	va_start( argptr,fmt );

#if defined RTCW_SP
	len = vsprintf( bigbuffer,fmt,argptr );
#elif defined RTCW_MP
	len = Q_vsnprintf( dest, size, fmt, argptr );
#else
	ret = Q_vsnprintf( dest, size, fmt, argptr );
#endif // RTCW_XX

	va_end( argptr );

#if defined RTCW_SP
	if ( len >= sizeof( bigbuffer ) ) {
		Com_Error( ERR_FATAL, "Com_sprintf: overflowed bigbuffer" );
	}
#endif // RTCW_XX

#if !defined RTCW_ET
	if ( len >= size ) {
		Com_Printf( "Com_sprintf: overflow of %i in %i\n", len, size );
	}
#endif // RTCW_XX

#if defined RTCW_SP
	Q_strncpyz( dest, bigbuffer, size );
#endif // RTCW_XX

#if defined RTCW_ET
	if ( ret == -1 ) {
		Com_Printf( "Com_sprintf: overflow of %i bytes buffer\n", size );
	}
#endif // RTCW_XX

}

#if !defined RTCW_ET
// Ridah, ripped from l_bsp.c
int Q_strncasecmp( const char *s1, const char *s2, int n ) {
	int c1, c2;

	do
	{
		c1 = *s1++;
		c2 = *s2++;

		if ( !n-- ) {
			return 0;       // strings are equal until end point

		}
		if ( c1 != c2 ) {

#if defined RTCW_SP
			if ( Q_islower( c1 ) ) {
#elif defined RTCW_MP
			if ( c1 >= 'a' && c1 <= 'z' ) {
#endif // RTCW_XX

				c1 -= ( 'a' - 'A' );
			}

#if defined RTCW_SP
			if ( Q_islower( c2 ) ) {
#elif defined RTCW_MP
			if ( c2 >= 'a' && c2 <= 'z' ) {
#endif // RTCW_XX

				c2 -= ( 'a' - 'A' );
			}
			if ( c1 != c2 ) {
				return -1;      // strings not equal
			}
		}
	} while ( c1 );

	return 0;       // strings are equal
}

int Q_strcasecmp( const char *s1, const char *s2 ) {
	return Q_strncasecmp( s1, s2, 99999 );
}
// done.
#endif // RTCW_XX

/*
============
va

does a varargs printf into a temp buffer, so I don't need to have
varargs versions of all text functions.
FIXME: make this buffer size safe someday

Ridah, modified this into a circular list, to further prevent stepping on
previous strings
============
*/
char    * QDECL va( const char *format, ... ) {
	va_list argptr;
	#define MAX_VA_STRING   32000
	static char temp_buffer[MAX_VA_STRING];
	static char string[MAX_VA_STRING];      // in case va is called by nested functions
	static int index = 0;
	char    *buf;
	int len;


	va_start( argptr, format );
	vsprintf( temp_buffer, format,argptr );
	va_end( argptr );

	if ( ( len = strlen( temp_buffer ) ) >= MAX_VA_STRING ) {
		Com_Error( ERR_DROP, "Attempted to overrun string in call to va()\n" );
	}

	if ( len + index >= MAX_VA_STRING - 1 ) {
		index = 0;
	}

	buf = &string[index];
	memcpy( buf, temp_buffer, len + 1 );

	index += len + 1;

	return buf;
}

/*
=============
TempVector

(SA) this is straight out of g_utils.c around line 210

This is just a convenience function
for making temporary vectors for function calls
=============
*/
float   *tv( float x, float y, float z ) {
	static int index;
	static vec3_t vecs[8];
	float   *v;

	// use an array so that multiple tempvectors won't collide
	// for a while
	v = vecs[index];
	index = ( index + 1 ) & 7;

	v[0] = x;
	v[1] = y;
	v[2] = z;

	return v;
}

/*
=====================================================================

  INFO STRINGS

=====================================================================
*/

/*
===============
Info_ValueForKey

Searches the string for the given
key and returns the associated value, or an empty string.
FIXME: overflow check?
===============
*/
const char *Info_ValueForKey( const char *s, const char *key ) {
	char pkey[BIG_INFO_KEY];
	static char value[2][BIG_INFO_VALUE];   // use two buffers so compares
											// work without stomping on each other
	static int valueindex = 0;
	char    *o;

	if ( !s || !key ) {
		return "";
	}

	if ( strlen( s ) >= BIG_INFO_STRING ) {

#if !defined RTCW_ET
		Com_Error( ERR_DROP, "Info_ValueForKey: oversize infostring" );
#else
		Com_Error( ERR_DROP, "Info_ValueForKey: oversize infostring [%s] [%s]", s, key );
#endif // RTCW_XX

	}

	valueindex ^= 1;
	if ( *s == '\\' ) {
		s++;
	}
	while ( 1 )
	{
		o = pkey;
		while ( *s != '\\' )
		{
			if ( !*s ) {
				return "";
			}
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value[valueindex];

		while ( *s != '\\' && *s )
		{
			*o++ = *s++;
		}
		*o = 0;

		if ( !Q_stricmp( key, pkey ) ) {
			return value[valueindex];
		}

		if ( !*s ) {
			break;
		}
		s++;
	}

	return "";
}


/*
===================
Info_NextPair

Used to itterate through all the key/value pairs in an info string
===================
*/
void Info_NextPair( const char **head, char *key, char *value ) {
	char    *o;
	const char  *s;

	s = *head;

	if ( *s == '\\' ) {
		s++;
	}
	key[0] = 0;
	value[0] = 0;

	o = key;
	while ( *s != '\\' ) {
		if ( !*s ) {
			*o = 0;
			*head = s;
			return;
		}
		*o++ = *s++;
	}
	*o = 0;
	s++;

	o = value;
	while ( *s != '\\' && *s ) {
		*o++ = *s++;
	}
	*o = 0;

	*head = s;
}


/*
===================
Info_RemoveKey
===================
*/
void Info_RemoveKey( char *s, const char *key ) {
	char    *start;
	char pkey[MAX_INFO_KEY];
	char value[MAX_INFO_VALUE];
	char    *o;

	if ( strlen( s ) >= MAX_INFO_STRING ) {

#if !defined RTCW_ET
		Com_Error( ERR_DROP, "Info_RemoveKey: oversize infostring" );
#else
		Com_Error( ERR_DROP, "Info_RemoveKey: oversize infostring [%s] [%s]", s, key );
#endif // RTCW_XX

	}

	if ( strchr( key, '\\' ) ) {
		return;
	}

	while ( 1 )
	{
		start = s;
		if ( *s == '\\' ) {
			s++;
		}
		o = pkey;
		while ( *s != '\\' )
		{
			if ( !*s ) {
				return;
			}
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while ( *s != '\\' && *s )
		{
			if ( !*s ) {
				return;
			}
			*o++ = *s++;
		}
		*o = 0;

#if !defined RTCW_ET
		if ( !strcmp( key, pkey ) ) {
			strcpy( start, s );  // remove this part
#else
		if ( !Q_stricmp( key, pkey ) ) {
			// rain - arguments to strcpy must not overlap
			//strcpy (start, s);	// remove this part
			memmove( start, s, strlen( s ) + 1 ); // remove this part
#endif // RTCW_XX

			return;
		}

		if ( !*s ) {
			return;
		}
	}

}

/*
===================
Info_RemoveKey_Big
===================
*/
void Info_RemoveKey_Big( char *s, const char *key ) {
	char    *start;
	char pkey[BIG_INFO_KEY];
	char value[BIG_INFO_VALUE];
	char    *o;

	if ( strlen( s ) >= BIG_INFO_STRING ) {

#if !defined RTCW_ET
		Com_Error( ERR_DROP, "Info_RemoveKey_Big: oversize infostring" );
#else
		Com_Error( ERR_DROP, "Info_RemoveKey_Big: oversize infostring [%s] [%s]", s, key );
#endif // RTCW_XX

	}

	if ( strchr( key, '\\' ) ) {
		return;
	}

	while ( 1 )
	{
		start = s;
		if ( *s == '\\' ) {
			s++;
		}
		o = pkey;
		while ( *s != '\\' )
		{
			if ( !*s ) {
				return;
			}
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while ( *s != '\\' && *s )
		{
			if ( !*s ) {
				return;
			}
			*o++ = *s++;
		}
		*o = 0;

#if !defined RTCW_ET
		if ( !strcmp( key, pkey ) ) {
#else
		if ( !Q_stricmp( key, pkey ) ) {
#endif // RTCW_XX

			strcpy( start, s );  // remove this part
			return;
		}

		if ( !*s ) {
			return;
		}
	}

}




/*
==================
Info_Validate

Some characters are illegal in info strings because they
can mess up the server's parsing
==================
*/
qboolean Info_Validate( const char *s ) {
	if ( strchr( s, '\"' ) ) {
		return qfalse;
	}
	if ( strchr( s, ';' ) ) {
		return qfalse;
	}
	return qtrue;
}

/*
==================
Info_SetValueForKey

Changes or adds a key/value pair
==================
*/
void Info_SetValueForKey( char *s, const char *key, const char *value ) {
	char newi[MAX_INFO_STRING];

	if ( strlen( s ) >= MAX_INFO_STRING ) {

#if !defined RTCW_ET
		Com_Error( ERR_DROP, "Info_SetValueForKey: oversize infostring" );
#else
		Com_Error( ERR_DROP, "Info_SetValueForKey: oversize infostring [%s] [%s] [%s]", s, key, value );
#endif // RTCW_XX

	}

	if ( strchr( key, '\\' ) || strchr( value, '\\' ) ) {
		Com_Printf( "Can't use keys or values with a \\\n" );
		return;
	}

	if ( strchr( key, ';' ) || strchr( value, ';' ) ) {
		Com_Printf( "Can't use keys or values with a semicolon\n" );
		return;
	}

	if ( strchr( key, '\"' ) || strchr( value, '\"' ) ) {
		Com_Printf( "Can't use keys or values with a \"\n" );
		return;
	}

	Info_RemoveKey( s, key );
	if ( !value || !strlen( value ) ) {
		return;
	}

	Com_sprintf( newi, sizeof( newi ), "\\%s\\%s", key, value );

	if ( strlen( newi ) + strlen( s ) > MAX_INFO_STRING ) {
		Com_Printf( "Info string length exceeded\n" );
		return;
	}

	strcat( s, newi );
}

/*
==================
Info_SetValueForKey_Big

Changes or adds a key/value pair
==================
*/
void Info_SetValueForKey_Big( char *s, const char *key, const char *value ) {
	char newi[BIG_INFO_STRING];

	if ( strlen( s ) >= BIG_INFO_STRING ) {

#if !defined RTCW_ET
		Com_Error( ERR_DROP, "Info_SetValueForKey: oversize infostring" );
#else
		Com_Error( ERR_DROP, "Info_SetValueForKey: oversize infostring [%s] [%s] [%s]", s, key, value );
#endif // RTCW_XX

	}

	if ( strchr( key, '\\' ) || strchr( value, '\\' ) ) {
		Com_Printf( "Can't use keys or values with a \\\n" );
		return;
	}

	if ( strchr( key, ';' ) || strchr( value, ';' ) ) {
		Com_Printf( "Can't use keys or values with a semicolon\n" );
		return;
	}

	if ( strchr( key, '\"' ) || strchr( value, '\"' ) ) {
		Com_Printf( "Can't use keys or values with a \"\n" );
		return;
	}

	Info_RemoveKey_Big( s, key );
	if ( !value || !strlen( value ) ) {
		return;
	}

	Com_sprintf( newi, sizeof( newi ), "\\%s\\%s", key, value );

	if ( strlen( newi ) + strlen( s ) > BIG_INFO_STRING ) {
		Com_Printf( "BIG Info string length exceeded\n" );
		return;
	}

	strcat( s, newi );
}




//====================================================================

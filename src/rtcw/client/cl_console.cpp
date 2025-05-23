/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

// console.c

#include "client.h"


int g_console_field_width = 78;

#define COLNSOLE_COLOR  COLOR_WHITE //COLOR_BLACK

#if !defined RTCW_ET
#define NUM_CON_TIMES 4

//#define		CON_TEXTSIZE	32768
#define     CON_TEXTSIZE    65536   // (SA) DM want's more console...

typedef struct {
	qboolean initialized;

	short text[CON_TEXTSIZE];
	int current;            // line where next message will be printed
	int x;                  // offset in current line for next print
	int display;            // bottom of console displays this line

	int linewidth;          // characters across screen
	int totallines;         // total lines in console scrollback

	float xadjust;          // for wide aspect screens

	float displayFrac;      // aproaches finalFrac at scr_conspeed
	float finalFrac;        // 0.0 to 1.0 lines of console to display

	int vislines;           // in scanlines

	int times[NUM_CON_TIMES];       // cls.realtime time the line was generated
	// for transparent notify lines
	vec4_t color;
} console_t;

extern console_t con;
#endif // RTCW_XX

console_t con;

cvar_t      *con_debug;
cvar_t      *con_conspeed;
cvar_t      *con_notifytime;

#if defined RTCW_ET
cvar_t      *con_autoclear;
#endif // RTCW_XX


#if !defined RTCW_SP
// DHM - Nerve :: Must hold CTRL + SHIFT + ~ to get console
cvar_t      *con_restricted;
#endif // RTCW_XX

#define DEFAULT_CONSOLE_WIDTH   78

vec4_t console_color = {1.0, 1.0, 1.0, 1.0};

#if defined RTCW_ET
vec4_t console_highlightcolor = {0.5, 0.5, 0.2, 0.45};
#endif // RTCW_XX



/*
================
Con_ToggleConsole_f
================
*/
void Con_ToggleConsole_f( void ) {

#if !defined RTCW_ET
	// closing a full screen console restarts the demo loop
	if ( cls.state == CA_DISCONNECTED && cls.keyCatchers == KEYCATCH_CONSOLE ) {
		CL_StartDemoLoop();
		return;
	}

#if defined RTCW_MP
	if ( con_restricted->integer && ( !keys[K_CTRL].down || !keys[K_SHIFT].down ) ) {
		return;
	}
#endif // RTCW_XX

	Field_Clear( &g_consoleField );
	g_consoleField.widthInChars = g_console_field_width;

	Con_ClearNotify();
	cls.keyCatchers ^= KEYCATCH_CONSOLE;
#else
	con.acLength = 0;

	if ( con_restricted->integer && ( !keys[K_CTRL].down || !keys[K_SHIFT].down ) ) {
		return;
	}

	// ydnar: persistent console input is more useful
	// Arnout: added cvar
	if ( con_autoclear->integer ) {
		Field_Clear( &g_consoleField );
	}

	g_consoleField.widthInChars = g_console_field_width;

	Con_ClearNotify();

	// ydnar: multiple console size support
	if ( cls.keyCatchers & KEYCATCH_CONSOLE ) {
		cls.keyCatchers &= ~KEYCATCH_CONSOLE;
		con.desiredFrac = 0.0;
	} else
	{
		cls.keyCatchers |= KEYCATCH_CONSOLE;

		// short console
		if ( keys[ K_CTRL ].down ) {
			con.desiredFrac = ( 5.0 * SMALLCHAR_HEIGHT ) / cls.glconfig.vidHeight;
		}
		// full console
		else if ( keys[ K_ALT ].down ) {
			con.desiredFrac = 1.0;
		}
		// normal half-screen console
		else {
			con.desiredFrac = 0.5;
		}
	}
#endif // RTCW_XX

}

/*
================
Con_MessageMode_f
================
*/
void Con_MessageMode_f( void ) {

#if !defined RTCW_ET
	chat_playerNum = -1;
#endif // RTCW_XX

	chat_team = qfalse;

#if defined RTCW_SP
//	chat_limbo = qfalse;		// NERVE - SMF
#endif // RTCW_XX

	Field_Clear( &chatField );
	chatField.widthInChars = 30;

	cls.keyCatchers ^= KEYCATCH_MESSAGE;
}

/*
================
Con_MessageMode2_f
================
*/
void Con_MessageMode2_f( void ) {

#if !defined RTCW_ET
	chat_playerNum = -1;
#endif // RTCW_XX

	chat_team = qtrue;

#if defined RTCW_SP
//	chat_limbo = qfalse;		// NERVE - SMF
#endif // RTCW_XX

	Field_Clear( &chatField );
	chatField.widthInChars = 25;
	cls.keyCatchers ^= KEYCATCH_MESSAGE;
}

/*
================
Con_MessageMode3_f
================
*/
void Con_MessageMode3_f( void ) {

#if !defined RTCW_ET
	chat_playerNum = VM_Call(cgvm, CG_CROSSHAIR_PLAYER);
	if ( chat_playerNum < 0 || chat_playerNum >= MAX_CLIENTS ) {
		chat_playerNum = -1;
		return;
	}
	chat_team = qfalse;

#if defined RTCW_SP
//	chat_limbo = qfalse;		// NERVE - SMF
#endif // RTCW_XX

	Field_Clear( &chatField );
	chatField.widthInChars = 30;
	cls.keyCatchers ^= KEYCATCH_MESSAGE;
#else
	chat_team = qfalse;
	chat_buddy = qtrue;
	Field_Clear( &chatField );
	chatField.widthInChars = 26;
	cls.keyCatchers ^= KEYCATCH_MESSAGE;
#endif // RTCW_XX

}

#if !defined RTCW_ET
/*
================
Con_MessageMode4_f
================
*/
void Con_MessageMode4_f( void ) {
	chat_playerNum = VM_Call(cgvm, CG_LAST_ATTACKER);
	if ( chat_playerNum < 0 || chat_playerNum >= MAX_CLIENTS ) {
		chat_playerNum = -1;
		return;
	}
	chat_team = qfalse;

#if defined RTCW_SP
//	chat_limbo = qfalse;		// NERVE - SMF
#endif // RTCW_XX

	Field_Clear( &chatField );
	chatField.widthInChars = 30;
	cls.keyCatchers ^= KEYCATCH_MESSAGE;
}

// NERVE - SMF
/*
================
Con_StartLimboMode_f
================
*/
void Con_StartLimboMode_f( void ) {

#if defined RTCW_SP
//	chat_playerNum = -1;
//	chat_team = qfalse;
	chat_limbo = qtrue;     // NERVE - SMF
//	Field_Clear( &chatField );
//	chatField.widthInChars = 30;

//	cls.keyCatchers ^= KEYCATCH_MESSAGE;
#elif defined RTCW_MP
	chat_limbo = qtrue;
#endif // RTCW_XX

}

/*
================
Con_StopLimboMode_f
================
*/
void Con_StopLimboMode_f( void ) {

#if defined RTCW_SP
//	chat_playerNum = -1;
//	chat_team = qfalse;
	chat_limbo = qfalse;        // NERVE - SMF
//	Field_Clear( &chatField );
//	chatField.widthInChars = 30;

//	cls.keyCatchers &= ~KEYCATCH_MESSAGE;
#elif defined RTCW_MP
	chat_limbo = qfalse;
#endif // RTCW_XX

}
// -NERVE - SMF
#endif // RTCW_XX

/*
================
Con_Clear_f
================
*/
void Con_Clear_f( void ) {
	int i;

	for ( i = 0 ; i < CON_TEXTSIZE ; i++ ) {
		con.text[i] = ( ColorIndex( COLNSOLE_COLOR ) << 8 ) | ' ';
	}

	Con_Bottom();       // go to end
}

/*
================
Con_Dump_f

Save the console contents out to a file
================
*/
void Con_Dump_f( void ) {
	int l, x, i;
	short   *line;
	fileHandle_t f;
	char buffer[1024];

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "usage: condump <filename>\n" );
		return;
	}

	Com_Printf( "Dumped console text to %s.\n", Cmd_Argv( 1 ) );

	f = FS_FOpenFileWrite( Cmd_Argv( 1 ) );
	if ( !f ) {
		Com_Printf( "ERROR: couldn't open.\n" );
		return;
	}

	// skip empty lines
	for ( l = con.current - con.totallines + 1 ; l <= con.current ; l++ )
	{
		line = con.text + ( l % con.totallines ) * con.linewidth;
		for ( x = 0 ; x < con.linewidth ; x++ )
			if ( ( line[x] & 0xff ) != ' ' ) {
				break;
			}
		if ( x != con.linewidth ) {
			break;
		}
	}

	// write the remaining lines
	buffer[con.linewidth] = 0;
	for ( ; l <= con.current ; l++ )
	{
		line = con.text + ( l % con.totallines ) * con.linewidth;
		for ( i = 0; i < con.linewidth; i++ )
			buffer[i] = line[i] & 0xff;
		for ( x = con.linewidth - 1 ; x >= 0 ; x-- )
		{
			if ( buffer[x] == ' ' ) {
				buffer[x] = 0;
			} else {
				break;
			}
		}
		strcat( buffer, "\n" );
		FS_Write( buffer, strlen( buffer ), f );
	}

	FS_FCloseFile( f );
}


/*
================
Con_ClearNotify
================
*/
void Con_ClearNotify( void ) {
	int i;

	for ( i = 0 ; i < NUM_CON_TIMES ; i++ ) {
		con.times[i] = 0;
	}
}



/*
================
Con_CheckResize

If the line width has changed, reformat the buffer.
================
*/
void Con_CheckResize( void ) {
	int i, j, width, oldwidth, oldtotallines, numlines, numchars;
	short tbuf[CON_TEXTSIZE];

#if !defined RTCW_ET
	width = ( SCREEN_WIDTH / SMALLCHAR_WIDTH ) - 2;
#else
	// ydnar: wasn't allowing for larger consoles
	// width = (SCREEN_WIDTH / SMALLCHAR_WIDTH) - 2;
	width = ( cls.glconfig.vidWidth / SMALLCHAR_WIDTH ) - 2;
#endif // RTCW_XX


	if ( width == con.linewidth ) {
		return;
	}

	if ( width < 1 ) {        // video hasn't been initialized yet
		width = DEFAULT_CONSOLE_WIDTH;
		con.linewidth = width;
		con.totallines = CON_TEXTSIZE / con.linewidth;
		for ( i = 0; i < CON_TEXTSIZE; i++ )

			con.text[i] = ( ColorIndex( COLNSOLE_COLOR ) << 8 ) | ' ';
	} else
	{
		oldwidth = con.linewidth;
		con.linewidth = width;
		oldtotallines = con.totallines;
		con.totallines = CON_TEXTSIZE / con.linewidth;
		numlines = oldtotallines;

		if ( con.totallines < numlines ) {
			numlines = con.totallines;
		}

		numchars = oldwidth;

		if ( con.linewidth < numchars ) {
			numchars = con.linewidth;
		}

		memcpy( tbuf, con.text, CON_TEXTSIZE * sizeof( short ) );
		for ( i = 0; i < CON_TEXTSIZE; i++ )

			con.text[i] = ( ColorIndex( COLNSOLE_COLOR ) << 8 ) | ' ';


		for ( i = 0 ; i < numlines ; i++ )
		{
			for ( j = 0 ; j < numchars ; j++ )
			{
				con.text[( con.totallines - 1 - i ) * con.linewidth + j] =
					tbuf[( ( con.current - i + oldtotallines ) %
						   oldtotallines ) * oldwidth + j];
			}
		}

		Con_ClearNotify();
	}

	con.current = con.totallines - 1;
	con.display = con.current;
}


/*
================
Con_Init
================
*/
void Con_Init( void ) {
	int i;

#if defined RTCW_SP
	con_notifytime = Cvar_Get( "con_notifytime", "3", 0 );
#else
	con_notifytime = Cvar_Get( "con_notifytime", "7", 0 ); // JPW NERVE increased per id req for obits
#endif // RTCW_XX

	con_conspeed = Cvar_Get( "scr_conspeed", "3", 0 );
	con_debug = Cvar_Get( "con_debug", "0", CVAR_ARCHIVE ); //----(SA)	added

#if defined RTCW_ET
	con_autoclear = Cvar_Get( "con_autoclear", "1", CVAR_ARCHIVE );
#endif // RTCW_XX

#if !defined RTCW_SP
	con_restricted = Cvar_Get( "con_restricted", "0", CVAR_INIT );      // DHM - Nerve
#endif // RTCW_XX

	Field_Clear( &g_consoleField );
	g_consoleField.widthInChars = g_console_field_width;
	for ( i = 0 ; i < COMMAND_HISTORY ; i++ ) {
		Field_Clear( &historyEditLines[i] );
		historyEditLines[i].widthInChars = g_console_field_width;
	}

#if !defined RTCW_ET
	Cmd_AddCommand( "toggleconsole", Con_ToggleConsole_f );
	Cmd_AddCommand( "messagemode", Con_MessageMode_f );
	Cmd_AddCommand( "messagemode2", Con_MessageMode2_f );
	Cmd_AddCommand( "messagemode3", Con_MessageMode3_f );
	Cmd_AddCommand( "messagemode4", Con_MessageMode4_f );
	Cmd_AddCommand( "startLimboMode", Con_StartLimboMode_f );     // NERVE - SMF
	Cmd_AddCommand( "stopLimboMode", Con_StopLimboMode_f );           // NERVE - SMF
#else
	Cmd_AddCommand( "toggleConsole", Con_ToggleConsole_f );
#endif // RTCW_XX

	Cmd_AddCommand( "clear", Con_Clear_f );
	Cmd_AddCommand( "condump", Con_Dump_f );

#if defined RTCW_ET
	// ydnar: these are deprecated in favor of cgame/ui based version
	Cmd_AddCommand( "clMessageMode", Con_MessageMode_f );
	Cmd_AddCommand( "clMessageMode2", Con_MessageMode2_f );
	Cmd_AddCommand( "clMessageMode3", Con_MessageMode3_f );
#endif // RTCW_XX

}


/*
===============
Con_Linefeed
===============
*/
#if defined RTCW_SP
void Con_Linefeed( void ) {
#else
void Con_Linefeed( qboolean skipnotify ) {
#endif // RTCW_XX

	int i;

	// mark time for transparent overlay
	if ( con.current >= 0 ) {

#if defined RTCW_SP
		con.times[con.current % NUM_CON_TIMES] = cls.realtime;
#else
		if ( skipnotify ) {
			con.times[con.current % NUM_CON_TIMES] = 0;
		} else {
			con.times[con.current % NUM_CON_TIMES] = cls.realtime;
		}
#endif // RTCW_XX

	}

	con.x = 0;
	if ( con.display == con.current ) {
		con.display++;
	}
	con.current++;
	for ( i = 0; i < con.linewidth; i++ )
		con.text[( con.current % con.totallines ) * con.linewidth + i] = ( ColorIndex( COLNSOLE_COLOR ) << 8 ) | ' ';
}

/*
================
CL_ConsolePrint

Handles cursor positioning, line wrapping, etc
All console printing must go through this in order to be logged to disk
If no console is visible, the text will appear at the top of the game window
================
*/

void CL_ConsolePrint( char *txt ) {
	int y;
	int c, l;
	int color;

#if !defined RTCW_SP
	qboolean skipnotify = qfalse;       // NERVE - SMF
	int prev;                           // NERVE - SMF

	// NERVE - SMF - work around for text that shows up in console but not in notify
	if ( !Q_strncmp( txt, "[skipnotify]", 12 ) ) {
		skipnotify = qtrue;
		txt += 12;
	}
#endif // RTCW_XX

	// for some demos we don't want to ever show anything on the console
	if ( cl_noprint && cl_noprint->integer ) {
		return;
	}

	if ( !con.initialized ) {
		con.color[0] =
			con.color[1] =
				con.color[2] =
					con.color[3] = 1.0f;
		con.linewidth = -1;
		Con_CheckResize();
		con.initialized = qtrue;
	}

	color = ColorIndex( COLNSOLE_COLOR );

	while ( ( c = *txt ) != 0 ) {
		if ( Q_IsColorString( txt ) ) {

#if !defined RTCW_ET
			color = ColorIndex( *( txt + 1 ) );
#else
			if ( *( txt + 1 ) == COLOR_NULL ) {
				color = ColorIndex( COLNSOLE_COLOR );
			} else {
				color = ColorIndex( *( txt + 1 ) );
			}
#endif // RTCW_XX

			txt += 2;
			continue;
		}

		// count word length
		for ( l = 0 ; l < con.linewidth ; l++ ) {
			if ( txt[l] <= ' ' ) {
				break;
			}

		}

		// word wrap
		if ( l != con.linewidth && ( con.x + l >= con.linewidth ) ) {

#if defined RTCW_SP
			Con_Linefeed();
#else
			Con_Linefeed( skipnotify );
#endif // RTCW_XX

		}

		txt++;

		switch ( c )
		{
		case '\n':

#if defined RTCW_SP
			Con_Linefeed();
#else
			Con_Linefeed( skipnotify );
#endif // RTCW_XX

			break;
		case '\r':
			con.x = 0;
			break;
		default:    // display character and advance
			y = con.current % con.totallines;

#if !defined RTCW_ET
			con.text[y * con.linewidth + con.x] = ( color << 8 ) | c;
#else
			// rain - sign extension caused the character to carry over
			// into the color info for high ascii chars; casting c to unsigned
			con.text[y * con.linewidth + con.x] = ( color << 8 ) | (unsigned char)c;
#endif // RTCW_XX

			con.x++;
			if ( con.x >= con.linewidth ) {

#if defined RTCW_SP
				Con_Linefeed();
#else
				Con_Linefeed( skipnotify );
#endif // RTCW_XX

				con.x = 0;
			}
			break;
		}
	}

	// mark time for transparent overlay
	if ( con.current >= 0 ) {

#if defined RTCW_SP
		con.times[con.current % NUM_CON_TIMES] = cls.realtime;
#else
		// NERVE - SMF
		if ( skipnotify ) {
			prev = con.current % NUM_CON_TIMES - 1;
			if ( prev < 0 ) {
				prev = NUM_CON_TIMES - 1;
			}
			con.times[prev] = 0;
		} else {
			// -NERVE - SMF
			con.times[con.current % NUM_CON_TIMES] = cls.realtime;
		}
#endif // RTCW_XX

	}
}

/*
==============================================================================

DRAWING

==============================================================================
*/


/*
================
Con_DrawInput

Draw the editline after a ] prompt
================
*/
void Con_DrawInput( void ) {
	int y;

	if ( cls.state != CA_DISCONNECTED && !( cls.keyCatchers & KEYCATCH_CONSOLE ) ) {
		return;
	}

	y = con.vislines - ( SMALLCHAR_HEIGHT * 2 );

#if defined RTCW_ET
	// hightlight the current autocompleted part
	if ( con.acLength ) {
		Cmd_TokenizeString( g_consoleField.buffer );

		if ( strlen( Cmd_Argv( 0 ) ) - con.acLength > 0 ) {
			re.SetColor( console_highlightcolor );
			re.DrawStretchPic( con.xadjust + ( 2 + con.acLength ) * SMALLCHAR_WIDTH,
							   y + 2,
							   ( strlen( Cmd_Argv( 0 ) ) - con.acLength ) * SMALLCHAR_WIDTH,
							   SMALLCHAR_HEIGHT - 2, 0, 0, 0, 0, cls.whiteShader );
		}
	}
#endif // RTCW_XX

	re.SetColor( con.color );

	SCR_DrawSmallChar( con.xadjust + 1 * SMALLCHAR_WIDTH, y, ']' );

	Field_Draw( &g_consoleField, con.xadjust + 2 * SMALLCHAR_WIDTH, y,
				SCREEN_WIDTH - 3 * SMALLCHAR_WIDTH, qtrue );
}


/*
================
Con_DrawNotify

Draws the last few lines of output transparently over the game top
================
*/
void Con_DrawNotify( void ) {
	int x, v;
	short   *text;
	int i;
	int time;
	int skip;
	int currentColor;

#if defined RTCW_MP
	// NERVE - SMF - we dont want draw notify in limbo mode
	if ( Cvar_VariableIntegerValue( "ui_limboMode" ) ) {
		return;
	}
#endif // RTCW_XX

	currentColor = 7;
	re.SetColor( g_color_table[currentColor] );

	v = 0;
	for ( i = con.current - NUM_CON_TIMES + 1 ; i <= con.current ; i++ )
	{
		if ( i < 0 ) {
			continue;
		}
		time = con.times[i % NUM_CON_TIMES];
		if ( time == 0 ) {
			continue;
		}
		time = cls.realtime - time;
		if ( time > con_notifytime->value * 1000 ) {
			continue;
		}
		text = con.text + ( i % con.totallines ) * con.linewidth;

		if ( cl.snap.ps.pm_type != PM_INTERMISSION && cls.keyCatchers & ( KEYCATCH_UI | KEYCATCH_CGAME ) ) {
			continue;
		}

		for ( x = 0 ; x < con.linewidth ; x++ ) {
			if ( ( text[x] & 0xff ) == ' ' ) {
				continue;
			}

#if !defined RTCW_ET
			if ( ( ( text[x] >> 8 ) & 7 ) != currentColor ) {
				currentColor = ( text[x] >> 8 ) & 7;
#else
			if ( ( ( text[x] >> 8 ) & COLOR_BITS ) != currentColor ) {
				currentColor = ( text[x] >> 8 ) & COLOR_BITS;
#endif // RTCW_XX

				re.SetColor( g_color_table[currentColor] );
			}
			SCR_DrawSmallChar( cl_conXOffset->integer + con.xadjust + ( x + 1 ) * SMALLCHAR_WIDTH, v, text[x] & 0xff );
		}

		v += SMALLCHAR_HEIGHT;
	}

	re.SetColor( NULL );

	if ( cls.keyCatchers & ( KEYCATCH_UI | KEYCATCH_CGAME ) ) {
		return;
	}

	// draw the chat line
	if ( cls.keyCatchers & KEYCATCH_MESSAGE ) {

#if defined RTCW_SP
		if ( chat_team ) {
			SCR_DrawBigString( 8, v, "say_team:", 1.0f );
			skip = 11;
		} else
		{
			SCR_DrawBigString( 8, v, "say:", 1.0f );
			skip = 5;
		}
#else
		if ( chat_team ) {
			char buf[128];
			CL_TranslateString( "say_team:", buf );
			SCR_DrawBigString( 8, v, buf, 1.0f );
			skip = strlen( buf ) + 2;

#if !defined RTCW_ET
		} else
		{
#else
		} else if ( chat_buddy ) {
			char buf[128];
			CL_TranslateString( "say_fireteam:", buf );
			SCR_DrawBigString( 8, v, buf, 1.0f );
			skip = strlen( buf ) + 2;
		} else {
#endif // RTCW_XX

			char buf[128];
			CL_TranslateString( "say:", buf );
			SCR_DrawBigString( 8, v, buf, 1.0f );
			skip = strlen( buf ) + 1;
		}
#endif // RTCW_XX

		Field_BigDraw( &chatField, skip * BIGCHAR_WIDTH, v,
					   SCREEN_WIDTH - ( skip + 1 ) * BIGCHAR_WIDTH, qtrue );

		v += BIGCHAR_HEIGHT;
	}

}

/*
================
Con_DrawSolidConsole

Draws the console with the solid background
================
*/

void Con_DrawSolidConsole( float frac ) {
	int i, x, y;
	int rows;
	short           *text;
	int row;
	int lines;

#if defined RTCW_SP
//	qhandle_t		conShader;
#endif // RTCW_XX

	int currentColor;
	vec4_t color;

	lines = cls.glconfig.vidHeight * frac;
	if ( lines <= 0 ) {
		return;
	}

	if ( lines > cls.glconfig.vidHeight ) {
		lines = cls.glconfig.vidHeight;
	}

	// on wide screens, we will center the text
	con.xadjust = 0;
	SCR_AdjustFrom640( &con.xadjust, NULL, NULL, NULL );

	// draw the background
	y = frac * SCREEN_HEIGHT - 2;
	if ( y < 1 ) {
		y = 0;
	} else {
		SCR_DrawPic( 0, 0, SCREEN_WIDTH, y, cls.consoleShader );

#if defined RTCW_SP
		if ( frac >= 0.5f ) {  // only draw when the console is down all the way (for now)
#else
		// NERVE - SMF - merged from WolfSP
		if ( frac >= 0.5f ) {
#endif // RTCW_XX

			color[0] = color[1] = color[2] = frac * 2.0f;
			color[3] = 1.0f;
			re.SetColor( color );

			// draw the logo
			SCR_DrawPic( 192, 70, 256, 128, cls.consoleShader2 );
			re.SetColor( NULL );
		}

#if !defined RTCW_SP
		// -NERVE - SMF
#endif // RTCW_XX

	}

#if !defined RTCW_ET
	color[0] = 0;
	color[1] = 0;
	color[2] = 0;

#if defined RTCW_MP
//	color[3] = 1;
#endif // RTCW_XX

	color[3] = 0.6f;
	SCR_FillRect( 0, y, SCREEN_WIDTH, 2, color );
#else
	// ydnar: matching light text
	color[0] = 0.75;
	color[1] = 0.75;
	color[2] = 0.75;
	color[3] = 1.0f;
	if ( frac < 1.0 ) {
		SCR_FillRect( 0, y, SCREEN_WIDTH, 1.25, color );
	}
#endif // RTCW_XX


	// draw the version number

	re.SetColor( g_color_table[ColorIndex( COLNSOLE_COLOR )] );

	// BBi
	//i = strlen( Q3_VERSION );
	i = strlen (RTCW_VERSION);
	// BBi

	for ( x = 0 ; x < i ; x++ ) {

		// BBi
		//SCR_DrawSmallChar( cls.glconfig.vidWidth - ( i - x ) * SMALLCHAR_WIDTH,
		//				   ( lines - ( SMALLCHAR_HEIGHT + SMALLCHAR_HEIGHT / 2 ) ), Q3_VERSION[x] );
		SCR_DrawSmallChar (cls.glconfig.vidWidth - (i - x) * SMALLCHAR_WIDTH,
			(lines - (SMALLCHAR_HEIGHT + SMALLCHAR_HEIGHT / 2)), RTCW_VERSION[x]);
		// BBi

	}


	// draw the text
	con.vislines = lines;
	rows = ( lines - SMALLCHAR_WIDTH ) / SMALLCHAR_WIDTH;     // rows of text to draw

	y = lines - ( SMALLCHAR_HEIGHT * 3 );

	// draw from the bottom up
	if ( con.display != con.current ) {
		// draw arrows to show the buffer is backscrolled
		re.SetColor( g_color_table[ColorIndex( COLOR_WHITE )] );
		for ( x = 0 ; x < con.linewidth ; x += 4 )
			SCR_DrawSmallChar( con.xadjust + ( x + 1 ) * SMALLCHAR_WIDTH, y, '^' );
		y -= SMALLCHAR_HEIGHT;
		rows--;
	}

	row = con.display;

	if ( con.x == 0 ) {
		row--;
	}

	currentColor = 7;
	re.SetColor( g_color_table[currentColor] );

	for ( i = 0 ; i < rows ; i++, y -= SMALLCHAR_HEIGHT, row-- )
	{
		if ( row < 0 ) {
			break;
		}
		if ( con.current - row >= con.totallines ) {
			// past scrollback wrap point
			continue;
		}

		text = con.text + ( row % con.totallines ) * con.linewidth;

		for ( x = 0 ; x < con.linewidth ; x++ ) {
			if ( ( text[x] & 0xff ) == ' ' ) {
				continue;
			}

#if !defined RTCW_ET
			if ( ( ( text[x] >> 8 ) & 7 ) != currentColor ) {
				currentColor = ( text[x] >> 8 ) & 7;
#else
			if ( ( ( text[x] >> 8 ) & COLOR_BITS ) != currentColor ) {
				currentColor = ( text[x] >> 8 ) & COLOR_BITS;
#endif // RTCW_XX

				re.SetColor( g_color_table[currentColor] );
			}
			SCR_DrawSmallChar(  con.xadjust + ( x + 1 ) * SMALLCHAR_WIDTH, y, text[x] & 0xff );
		}
	}

	// draw the input prompt, user text, and cursor if desired
	Con_DrawInput();

	re.SetColor( NULL );
}

#if defined RTCW_ET
extern cvar_t   *con_drawnotify;
#endif // RTCW_XX

/*
==================
Con_DrawConsole
==================
*/
void Con_DrawConsole( void ) {
	// check for console width changes from a vid mode change
	Con_CheckResize();

	// if disconnected, render console full screen

#if defined RTCW_SP
	switch ( cls.state ) {
	case CA_UNINITIALIZED:
	case CA_CONNECTING:         // sending request packets to the server
	case CA_CHALLENGING:        // sending challenge packets to the server
	case CA_CONNECTED:          // netchan_t established, getting gamestate
	case CA_PRIMED:             // got gamestate, waiting for first frame
	case CA_LOADING:            // only during cgame initialization, never during main loop
		if ( !con_debug->integer ) { // these are all 'no console at all' when con_debug is not set
			return;
		}

		if ( cls.keyCatchers & KEYCATCH_UI ) {
			return;
		}

		Con_DrawSolidConsole( 1.0 );
		return;

	case CA_DISCONNECTED:       // not talking to a server
		if ( !( cls.keyCatchers & KEYCATCH_UI ) ) {
			Con_DrawSolidConsole( 1.0 );
			return;
		}
		break;
#else
	if ( cls.state == CA_DISCONNECTED ) {
		if ( !( cls.keyCatchers & ( KEYCATCH_UI | KEYCATCH_CGAME ) ) ) {
			Con_DrawSolidConsole( 1.0 );
			return;
		}
#endif // RTCW_XX

#if defined RTCW_SP
	case CA_ACTIVE:             // game views should be displayed
		if ( con.displayFrac ) {
			if ( con_debug->integer == 2 ) {    // 2 means draw full screen console at '~'
//					Con_DrawSolidConsole( 1.0f );
				Con_DrawSolidConsole( con.displayFrac * 2.0f );
				return;
			}
		}

		break;


	case CA_CINEMATIC:          // playing a cinematic or a static pic, not connected to a server
	default:
		break;
#endif // RTCW_XX

	}

	if ( con.displayFrac ) {
		Con_DrawSolidConsole( con.displayFrac );
	} else {

#if defined RTCW_SP
		Con_DrawNotify();       // draw notify lines
#elif defined RTCW_MP
		// draw notify lines
		if ( cls.state == CA_ACTIVE ) {
			Con_DrawNotify();
		}
#else
		// draw notify lines
		if ( cls.state == CA_ACTIVE && con_drawnotify->integer ) {
			Con_DrawNotify();
		}
#endif // RTCW_XX

	}
}

//================================================================

/*
==================
Con_RunConsole

Scroll it up or down
==================
*/
void Con_RunConsole( void ) {
	// decide on the destination height of the console

#if !defined RTCW_ET
	if ( cls.keyCatchers & KEYCATCH_CONSOLE ) {
		con.finalFrac = 0.5;        // half screen
	} else {
		con.finalFrac = 0;              // none visible
#else
	// ydnar: added short console support (via shift+~)
	if ( cls.keyCatchers & KEYCATCH_CONSOLE ) {
		con.finalFrac = con.desiredFrac;
	} else {
		con.finalFrac = 0;  // none visible
#endif // RTCW_XX

	}
	// scroll towards the destination height
	if ( con.finalFrac < con.displayFrac ) {
		con.displayFrac -= con_conspeed->value * cls.realFrametime * 0.001;
		if ( con.finalFrac > con.displayFrac ) {
			con.displayFrac = con.finalFrac;
		}

	} else if ( con.finalFrac > con.displayFrac )     {
		con.displayFrac += con_conspeed->value * cls.realFrametime * 0.001;
		if ( con.finalFrac < con.displayFrac ) {
			con.displayFrac = con.finalFrac;
		}
	}

}


void Con_PageUp( void ) {
	con.display -= 2;
	if ( con.current - con.display >= con.totallines ) {
		con.display = con.current - con.totallines + 1;
	}
}

void Con_PageDown( void ) {
	con.display += 2;
	if ( con.display > con.current ) {
		con.display = con.current;
	}
}

void Con_Top( void ) {
	con.display = con.totallines;
	if ( con.current - con.display >= con.totallines ) {
		con.display = con.current - con.totallines + 1;
	}
}

void Con_Bottom( void ) {
	con.display = con.current;
}


void Con_Close( void ) {
	if ( !com_cl_running->integer ) {
		return;
	}
	Field_Clear( &g_consoleField );
	Con_ClearNotify();
	cls.keyCatchers &= ~KEYCATCH_CONSOLE;
	con.finalFrac = 0;              // none visible
	con.displayFrac = 0;
}

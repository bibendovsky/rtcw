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

// cg_draw.c -- draw all of the graphical elements during
// active (after loading) gameplay

#include "cg_local.h"
#include "../ui/ui_shared.h"

//----(SA) added to make it easier to raise/lower our statsubar by only changing one thing
#define STATUSBARHEIGHT 452
//----(SA) end

extern displayContextDef_t cgDC;
menuDef_t *menuScoreboard = NULL;

int sortedTeamPlayers[TEAM_MAXOVERLAY];
int numSortedTeamPlayers;

char systemChat[256];
char teamChat1[256];
char teamChat2[256];

#if defined RTCW_MP
// NERVE - SMF
void Controls_GetKeyAssignment( char *command, int *twokeys );
char* BindingFromName( const char *cvar );
void Controls_GetConfig( void );
// -NERVE - SMF
#endif RTCW_XX

////////////////////////
////////////////////////
////// new hud stuff
///////////////////////
///////////////////////

#if defined RTCW_SP
int CG_Text_Width( const char *text, int font, float scale, int limit ) {
#elif defined RTCW_MP
int CG_Text_Width( const char *text, float scale, int limit ) {
#endif RTCW_XX

	int count,len;
	float out;
	glyphInfo_t *glyph;
	float useScale;
	const char *s = text;

#if defined RTCW_SP
	fontInfo_t *fnt = &cgDC.Assets.textFont;

	if ( font == UI_FONT_DEFAULT ) {
		if ( scale <= cg_smallFont.value ) {
			fnt = &cgDC.Assets.smallFont;
		} else if ( scale > cg_bigFont.value ) {
			fnt = &cgDC.Assets.bigFont;
		}
	} else if ( font == UI_FONT_BIG ) {
		fnt = &cgDC.Assets.bigFont;
	} else if ( font == UI_FONT_SMALL ) {
		fnt = &cgDC.Assets.smallFont;
	} else if ( font == UI_FONT_HANDWRITING ) {
		fnt = &cgDC.Assets.handwritingFont;
	}

	useScale = scale * fnt->glyphScale;
#elif defined RTCW_MP
	fontInfo_t *font = &cgDC.Assets.textFont;
	if ( scale <= cg_smallFont.value ) {
		font = &cgDC.Assets.smallFont;
	} else if ( scale > cg_bigFont.value ) {
		font = &cgDC.Assets.bigFont;
	}
	useScale = scale * font->glyphScale;
#endif RTCW_XX

	out = 0;
	if ( text ) {
		len = strlen( text );
		if ( limit > 0 && len > limit ) {
			len = limit;
		}
		count = 0;
		while ( s && *s && count < len ) {
			if ( Q_IsColorString( s ) ) {
				s += 2;
				continue;
			} else {

#if defined RTCW_SP
				glyph = &fnt->glyphs[(int)*s];
#elif defined RTCW_MP
				glyph = &font->glyphs[(int)*s];
#endif RTCW_XX

				out += glyph->xSkip;
				s++;
				count++;
			}
		}
	}
	return out * useScale;
}

#if defined RTCW_SP
int CG_Text_Height( const char *text, int font, float scale, int limit ) {
#elif defined RTCW_MP
int CG_Text_Height( const char *text, float scale, int limit ) {
#endif RTCW_XX

	int len, count;
	float max;
	glyphInfo_t *glyph;
	float useScale;
	const char *s = text;

#if defined RTCW_SP
	fontInfo_t *fnt = &cgDC.Assets.textFont;
	if ( font == UI_FONT_DEFAULT ) {
		if ( scale <= cg_smallFont.value ) {
			fnt = &cgDC.Assets.smallFont;
		} else if ( scale > cg_bigFont.value ) {
			fnt = &cgDC.Assets.bigFont;
		}
	} else if ( font == UI_FONT_BIG ) {
		fnt = &cgDC.Assets.bigFont;
	} else if ( font == UI_FONT_SMALL ) {
		fnt = &cgDC.Assets.smallFont;
	} else if ( font == UI_FONT_HANDWRITING ) {
		fnt = &cgDC.Assets.handwritingFont;
	}

	useScale = scale * fnt->glyphScale;
#elif defined RTCW_MP
	fontInfo_t *font = &cgDC.Assets.textFont;
	if ( scale <= cg_smallFont.value ) {
		font = &cgDC.Assets.smallFont;
	} else if ( scale > cg_bigFont.value ) {
		font = &cgDC.Assets.bigFont;
	}
	useScale = scale * font->glyphScale;
#endif RTCW_XX

	max = 0;
	if ( text ) {
		len = strlen( text );
		if ( limit > 0 && len > limit ) {
			len = limit;
		}
		count = 0;
		while ( s && *s && count < len ) {
			if ( Q_IsColorString( s ) ) {
				s += 2;
				continue;
			} else {

#if defined RTCW_SP
				glyph = &fnt->glyphs[(int)*s];
#elif defined RTCW_MP
				glyph = &font->glyphs[(int)*s];
#endif RTCW_XX

				if ( max < glyph->height ) {
					max = glyph->height;
				}
				s++;
				count++;
			}
		}
	}
	return max * useScale;
}

void CG_Text_PaintChar( float x, float y, float width, float height, float scale, float s, float t, float s2, float t2, qhandle_t hShader ) {
	float w, h;
	w = width * scale;
	h = height * scale;
	CG_AdjustFrom640( &x, &y, &w, &h );
	trap_R_DrawStretchPic( x, y, w, h, s, t, s2, t2, hShader );
}

#if defined RTCW_SP
void CG_Text_Paint( float x, float y, int font, float scale, vec4_t color, const char *text, float adjust, int limit, int style ) {
#elif defined RTCW_MP
void CG_Text_Paint( float x, float y, float scale, vec4_t color, const char *text, float adjust, int limit, int style ) {
#endif RTCW_XX

	int len, count;
	vec4_t newColor;
	glyphInfo_t *glyph;
	float useScale;

#if defined RTCW_SP
	fontInfo_t *fnt = &cgDC.Assets.textFont;

	if ( font == UI_FONT_DEFAULT ) {
		if ( scale <= cg_smallFont.value ) {
			fnt = &cgDC.Assets.smallFont;
		} else if ( scale > cg_bigFont.value ) {
			fnt = &cgDC.Assets.bigFont;
		}
	} else if ( font == UI_FONT_BIG ) {
		fnt = &cgDC.Assets.bigFont;
	} else if ( font == UI_FONT_SMALL ) {
		fnt = &cgDC.Assets.smallFont;
	} else if ( font == UI_FONT_HANDWRITING ) {
		fnt = &cgDC.Assets.handwritingFont;
	}

	useScale = scale * fnt->glyphScale;
#elif defined RTCW_MP
	fontInfo_t *font = &cgDC.Assets.textFont;
	if ( scale <= cg_smallFont.value ) {
		font = &cgDC.Assets.smallFont;
	} else if ( scale > cg_bigFont.value ) {
		font = &cgDC.Assets.bigFont;
	}
	useScale = scale * font->glyphScale;
#endif RTCW_XX

	color[3] *= cg_hudAlpha.value;  // (SA) adjust for cg_hudalpha

	if ( text ) {
		const char *s = text;
		trap_R_SetColor( color );
		memcpy( &newColor[0], &color[0], sizeof( vec4_t ) );
		len = strlen( text );
		if ( limit > 0 && len > limit ) {
			len = limit;
		}
		count = 0;
		while ( s && *s && count < len ) {

#if defined RTCW_SP
			glyph = &fnt->glyphs[(int)*s];
#elif defined RTCW_MP
			glyph = &font->glyphs[(int)*s];
#endif RTCW_XX

			//int yadj = Assets.textFont.glyphs[text[i]].bottom + Assets.textFont.glyphs[text[i]].top;
			//float yadj = scale * (Assets.textFont.glyphs[text[i]].imageHeight - Assets.textFont.glyphs[text[i]].height);
			if ( Q_IsColorString( s ) ) {
				memcpy( newColor, g_color_table[ColorIndex( *( s + 1 ) )], sizeof( newColor ) );
				newColor[3] = color[3];
				trap_R_SetColor( newColor );
				s += 2;
				continue;
			} else {
				float yadj = useScale * glyph->top;
				if ( style == ITEM_TEXTSTYLE_SHADOWED || style == ITEM_TEXTSTYLE_SHADOWEDMORE ) {
					int ofs = style == ITEM_TEXTSTYLE_SHADOWED ? 1 : 2;
					colorBlack[3] = newColor[3];
					trap_R_SetColor( colorBlack );
					CG_Text_PaintChar( x + ofs, y - yadj + ofs,
									   glyph->imageWidth,
									   glyph->imageHeight,
									   useScale,
									   glyph->s,
									   glyph->t,
									   glyph->s2,
									   glyph->t2,
									   glyph->glyph );
					colorBlack[3] = 1.0;
					trap_R_SetColor( newColor );
				}
				CG_Text_PaintChar( x, y - yadj,
								   glyph->imageWidth,
								   glyph->imageHeight,
								   useScale,
								   glyph->s,
								   glyph->t,
								   glyph->s2,
								   glyph->t2,
								   glyph->glyph );
				// CG_DrawPic(x, y - yadj, scale * cgDC.Assets.textFont.glyphs[text[i]].imageWidth, scale * cgDC.Assets.textFont.glyphs[text[i]].imageHeight, cgDC.Assets.textFont.glyphs[text[i]].glyph);
				x += ( glyph->xSkip * useScale ) + adjust;
				s++;
				count++;
			}
		}
		trap_R_SetColor( NULL );
	}
}

#if defined RTCW_MP
// NERVE - SMF - added back in
int CG_DrawFieldWidth( int x, int y, int width, int value, int charWidth, int charHeight ) {
	char num[16], *ptr;
	int l;
	int frame;
	int totalwidth = 0;

	if ( width < 1 ) {
		return 0;
	}

	// draw number string
	if ( width > 5 ) {
		width = 5;
	}

	switch ( width ) {
	case 1:
		value = value > 9 ? 9 : value;
		value = value < 0 ? 0 : value;
		break;
	case 2:
		value = value > 99 ? 99 : value;
		value = value < -9 ? -9 : value;
		break;
	case 3:
		value = value > 999 ? 999 : value;
		value = value < -99 ? -99 : value;
		break;
	case 4:
		value = value > 9999 ? 9999 : value;
		value = value < -999 ? -999 : value;
		break;
	}

	Com_sprintf( num, sizeof( num ), "%i", value );
	l = strlen( num );
	if ( l > width ) {
		l = width;
	}

	ptr = num;
	while ( *ptr && l )
	{
		if ( *ptr == '-' ) {
			frame = STAT_MINUS;
		} else {
			frame = *ptr - '0';
		}

		totalwidth += charWidth;
		ptr++;
		l--;
	}

	return totalwidth;
}
#endif RTCW_XX


#if defined RTCW_SP
#ifdef OLDWOLFUI
static void CG_DrawField( int x, int y, int width, int value ) {
	char num[16], *ptr;
	int l;
	int frame;

	if ( width < 1 ) {
		return;
	}

	// draw number string
	if ( width > 5 ) {
		width = 5;
	}

	switch ( width ) {
	case 1:
		value = value > 9 ? 9 : value;
		value = value < 0 ? 0 : value;
		break;
	case 2:
		value = value > 99 ? 99 : value;
		value = value < -9 ? -9 : value;
		break;
	case 3:
		value = value > 999 ? 999 : value;
		value = value < -99 ? -99 : value;
		break;
	case 4:
		value = value > 9999 ? 9999 : value;
		value = value < -999 ? -999 : value;
		break;
	}

	Com_sprintf( num, sizeof( num ), "%i", value );
	l = strlen( num );
	if ( l > width ) {
		l = width;
	}
	x += 2 + CHAR_WIDTH * ( width - l );

	ptr = num;
	while ( *ptr && l )
	{
		if ( *ptr == '-' ) {
			frame = STAT_MINUS;
		} else {
			frame = *ptr - '0';
		}

		CG_DrawPic( x,y, CHAR_WIDTH, CHAR_HEIGHT, cgs.media.numberShaders[frame] );
		x += CHAR_WIDTH;
		ptr++;
		l--;
	}
}
#endif  // #ifdef OLDWOLFUI
#elif defined RTCW_MP
int CG_DrawField( int x, int y, int width, int value, int charWidth, int charHeight, qboolean dodrawpic, qboolean leftAlign ) {
	char num[16], *ptr;
	int l;
	int frame;
	int startx;

	if ( width < 1 ) {
		return 0;
	}

	// draw number string
	if ( width > 5 ) {
		width = 5;
	}

	switch ( width ) {
	case 1:
		value = value > 9 ? 9 : value;
		value = value < 0 ? 0 : value;
		break;
	case 2:
		value = value > 99 ? 99 : value;
		value = value < -9 ? -9 : value;
		break;
	case 3:
		value = value > 999 ? 999 : value;
		value = value < -99 ? -99 : value;
		break;
	case 4:
		value = value > 9999 ? 9999 : value;
		value = value < -999 ? -999 : value;
		break;
	}

	Com_sprintf( num, sizeof( num ), "%i", value );
	l = strlen( num );
	if ( l > width ) {
		l = width;
	}

	// NERVE - SMF
	if ( !leftAlign ) {
		x -= 2 + charWidth * ( l );
	}

	startx = x;

	ptr = num;
	while ( *ptr && l )
	{
		if ( *ptr == '-' ) {
			frame = STAT_MINUS;
		} else {
			frame = *ptr - '0';
		}

		if ( dodrawpic ) {
			CG_DrawPic( x,y, charWidth, charHeight, cgs.media.numberShaders[frame] );
		}
		x += charWidth;
		ptr++;
		l--;
	}

	return startx;
}
// -NERVE - SMF
#endif RTCW_XX

/*
================
CG_Draw3DModel

================
*/
void CG_Draw3DModel( float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles ) {
	refdef_t refdef;
	refEntity_t ent;

	if ( !cg_draw3dIcons.integer || !cg_drawIcons.integer ) {
		return;
	}

	CG_AdjustFrom640( &x, &y, &w, &h );

	memset( &refdef, 0, sizeof( refdef ) );

	memset( &ent, 0, sizeof( ent ) );
	AnglesToAxis( angles, ent.axis );
	VectorCopy( origin, ent.origin );
	ent.hModel = model;
	ent.customSkin = skin;
	ent.renderfx = RF_NOSHADOW;     // no stencil shadows

	refdef.rdflags = RDF_NOWORLDMODEL;

	AxisClear( refdef.viewaxis );

	refdef.fov_x = 30;
	refdef.fov_y = 30;

	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	refdef.time = cg.time;

#if defined RTCW_SP
	refdef.rdflags |= RDF_DRAWSKYBOX;
	if ( !cg_skybox.integer ) {
		refdef.rdflags &= ~RDF_DRAWSKYBOX;
	}
#endif RTCW_XX

	trap_R_ClearScene();
	trap_R_AddRefEntityToScene( &ent );
	trap_R_RenderScene( &refdef );
}

/*
================
CG_DrawHead

Used for both the status bar and the scoreboard
================
*/
void CG_DrawHead( float x, float y, float w, float h, int clientNum, vec3_t headAngles ) {
	clipHandle_t cm;
	clientInfo_t    *ci;
	float len;
	vec3_t origin;
	vec3_t mins, maxs;

	ci = &cgs.clientinfo[ clientNum ];

	if ( cg_draw3dIcons.integer ) {
		cm = ci->headModel;
		if ( !cm ) {
			return;
		}

		// offset the origin y and z to center the head
		trap_R_ModelBounds( cm, mins, maxs );

		origin[2] = -0.5 * ( mins[2] + maxs[2] );
		origin[1] = 0.5 * ( mins[1] + maxs[1] );

		// calculate distance so the head nearly fills the box
		// assume heads are taller than wide
		len = 0.7 * ( maxs[2] - mins[2] );
		origin[0] = len / 0.268;    // len / tan( fov/2 )

		// allow per-model tweaking
		VectorAdd( origin, ci->modelInfo->headOffset, origin );

		CG_Draw3DModel( x, y, w, h, ci->headModel, ci->headSkin, origin, headAngles );
//	} else if ( cg_drawIcons.integer ) {
//		CG_DrawPic( x, y, w, h, ci->modelIcon );
	}

	// if they are deferred, draw a cross out
	if ( ci->deferred ) {
		CG_DrawPic( x, y, w, h, cgs.media.deferShader );
	}
}

/*
================
CG_DrawFlagModel

Used for both the status bar and the scoreboard
================
*/
void CG_DrawFlagModel( float x, float y, float w, float h, int team ) {
	qhandle_t cm;
	float len;
	vec3_t origin, angles;
	vec3_t mins, maxs;

	VectorClear( angles );

	cm = cgs.media.redFlagModel;

	// offset the origin y and z to center the flag
	trap_R_ModelBounds( cm, mins, maxs );

	origin[2] = -0.5 * ( mins[2] + maxs[2] );
	origin[1] = 0.5 * ( mins[1] + maxs[1] );

	// calculate distance so the flag nearly fills the box
	// assume heads are taller than wide
	len = 0.5 * ( maxs[2] - mins[2] );
	origin[0] = len / 0.268;    // len / tan( fov/2 )

	angles[YAW] = 60 * sin( cg.time / 2000.0 );;

	CG_Draw3DModel( x, y, w, h,
					team == TEAM_RED ? cgs.media.redFlagModel : cgs.media.blueFlagModel,
					0, origin, angles );
}


/*
==============
CG_DrawKeyModel
==============
*/
void CG_DrawKeyModel( int keynum, float x, float y, float w, float h, int fadetime ) {
	qhandle_t cm;
	float len;
	vec3_t origin, angles;
	vec3_t mins, maxs;

	VectorClear( angles );

	cm = cg_items[keynum].models[0];

	// offset the origin y and z to center the model
	trap_R_ModelBounds( cm, mins, maxs );

	origin[2] = -0.5 * ( mins[2] + maxs[2] );
	origin[1] = 0.5 * ( mins[1] + maxs[1] );

//	len = 0.5 * ( maxs[2] - mins[2] );
	len = 0.75 * ( maxs[2] - mins[2] );
	origin[0] = len / 0.268;    // len / tan( fov/2 )

	angles[YAW] = 30 * sin( cg.time / 2000.0 );;

	CG_Draw3DModel( x, y, w, h, cg_items[keynum].models[0], 0, origin, angles );
}

#if defined RTCW_SP
/*
================
CG_DrawStatusBarHead

================
*/
#ifdef OLDWOLFUI
static void CG_DrawStatusBarHead( float x ) {
	vec3_t angles;
	float size, stretch;
	float frac;

	VectorClear( angles );

	if ( cg.damageTime && cg.time - cg.damageTime < DAMAGE_TIME ) {
		frac = (float)( cg.time - cg.damageTime ) / DAMAGE_TIME;
		size = ICON_SIZE * 1.25 * ( 1.5 - frac * 0.5 );

		stretch = size - ICON_SIZE * 1.25;
		// kick in the direction of damage
		x -= stretch * 0.5 + cg.viewDamage[cg.damageIndex].damageX * stretch * 0.5;

		cg.headStartYaw = 180 + cg.viewDamage[cg.damageIndex].damageX * 45;

		cg.headEndYaw = 180 + 20 * cos( crandom() * M_PI );
		cg.headEndPitch = 5 * cos( crandom() * M_PI );

		cg.headStartTime = cg.time;
		cg.headEndTime = cg.time + 100 + random() * 2000;
	} else {
		if ( cg.time >= cg.headEndTime ) {
			// select a new head angle
			cg.headStartYaw = cg.headEndYaw;
			cg.headStartPitch = cg.headEndPitch;
			cg.headStartTime = cg.headEndTime;
			cg.headEndTime = cg.time + 100 + random() * 2000;

			cg.headEndYaw = 180 + 20 * cos( crandom() * M_PI );
			cg.headEndPitch = 5 * cos( crandom() * M_PI );
		}

		size = ICON_SIZE * 1.25;
	}

	// if the server was frozen for a while we may have a bad head start time
	if ( cg.headStartTime > cg.time ) {
		cg.headStartTime = cg.time;
	}

	frac = ( cg.time - cg.headStartTime ) / (float)( cg.headEndTime - cg.headStartTime );
	frac = frac * frac * ( 3 - 2 * frac );
	angles[YAW] = cg.headStartYaw + ( cg.headEndYaw - cg.headStartYaw ) * frac;
	angles[PITCH] = cg.headStartPitch + ( cg.headEndPitch - cg.headStartPitch ) * frac;

	CG_DrawHead( x, 480 - size, size, size,
				 cg.snap->ps.clientNum, angles );
}
#endif  // #ifdef OLDWOLFUI

/*
==============
CG_DrawStatusBarKeys
IT_KEY (this makes this routine easier to find in files...) (SA)
==============
*/
#ifdef OLDWOLFUI
static void CG_DrawStatusBarKeys() {
	int i;
	float y = 0;    // start height is
	gitem_t *gi;
	int itemnum;
//	int		fadetime = 0;
	float   *fadeColor;


//----(SA)	added
	if ( cg.showItems ) {
		fadeColor = colorWhite;
	} else {
		fadeColor = CG_FadeColor( cg.itemFadeTime, 1000 );
	}

	if ( !fadeColor ) {
		return;
	}


	// (SA) just don't draw this stuff for now.  It's got fog issues I need to clean up

	return;



	for ( i = 1; i < KEY_NUM_KEYS; i++ )
	{
		gi = BG_FindItemForKey( i, &itemnum );
		// if i've got the key...

		if ( cg.snap->ps.stats[STAT_KEYS] & ( 1 << gi->giTag ) ) {
			y += ICON_SIZE + 5;
			CG_DrawKeyModel( itemnum, 640 - ( 1.5 * ICON_SIZE ), y, ICON_SIZE, ICON_SIZE, cg.time + fadeColor[0] * 1000 );
		}
	}
}
#endif  // #ifdef OLDWOLFUI

/*
================
CG_DrawStatusBarFlag

================
*/
#ifdef OLDWOLFUI
static void CG_DrawStatusBarFlag( float x, int team ) {
	CG_DrawFlagModel( x, 480 - ICON_SIZE, ICON_SIZE, ICON_SIZE, team );
}
#endif  // #ifdef OLDWOFLUI
#endif RTCW_XX

/*
================
CG_DrawTeamBackground

================
*/
void CG_DrawTeamBackground( int x, int y, int w, int h, float alpha, int team ) {
	vec4_t hcolor;

	hcolor[3] = alpha;
	if ( team == TEAM_RED ) {
		hcolor[0] = 1;
		hcolor[1] = 0;
		hcolor[2] = 0;
	} else if ( team == TEAM_BLUE ) {
		hcolor[0] = 0;
		hcolor[1] = 0;
		hcolor[2] = 1;
	} else {
		return;
	}
	trap_R_SetColor( hcolor );
	CG_DrawPic( x, y, w, h, cgs.media.teamStatusBar );
	trap_R_SetColor( NULL );
}

#if defined RTCW_SP
//////////////////////
////// end new hud stuff
//////////////////////





// JOSEPH 4-25-00
/*
================
CG_DrawStatusBar

================
*/
#ifdef OLDWOLFUI
static void CG_DrawStatusBar( void ) {
	int color;
	centity_t   *cent;
	playerState_t   *ps;
	int value, inclip;
	vec4_t hcolor;
	vec3_t angles;
//	vec3_t		origin;
	static float colors[4][4] = {
//		{ 0.2, 1.0, 0.2, 1.0 } , { 1.0, 0.2, 0.2, 1.0 }, {0.5, 0.5, 0.5, 1} };
		{ 1, 0.69, 0, 1.0 },        // normal
		{ 1.0, 0.2, 0.2, 1.0 },     // low health
		{0.5, 0.5, 0.5, 1},         // weapon firing
		{ 1, 1, 1, 1 }
	};                              // health > 100

	if ( cg_drawStatus.integer == 0 ) {
		return;
	}

	// draw the team background
	if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED ) {
		hcolor[0] = 1;
		hcolor[1] = 0;
		hcolor[2] = 0;
		hcolor[3] = 0.33;
		trap_R_SetColor( hcolor );
		CG_DrawPic( 0, 420, 640, 60, cgs.media.teamStatusBar );
		trap_R_SetColor( NULL );
	} else if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE ) {
		hcolor[0] = 0;
		hcolor[1] = 0;
		hcolor[2] = 1;
		hcolor[3] = 0.33;
		trap_R_SetColor( hcolor );
		CG_DrawPic( 0, 420, 640, 60, cgs.media.teamStatusBar );
		trap_R_SetColor( NULL );
	}

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	VectorClear( angles );

	// draw any 3D icons first, so the changes back to 2D are minimized

	//----(SA) further change... we don't need to draw the ammo 3d model do we?
/*
	if ( cent->currentState.weapon && cg_weapons[ cent->currentState.weapon ].ammoModel ) {
		origin[0] = 70;
		origin[1] = 0;
		origin[2] = 0;
		angles[YAW] = 90 + 20 * sin( cg.time / 1000.0 );;
//----(SA) Wolf statusbar change
		CG_Draw3DModel( CHAR_WIDTH*3 + TEXT_ICON_SPACE, STATUSBARHEIGHT -20, ICON_SIZE, ICON_SIZE,
					   cg_weapons[ cent->currentState.weapon ].ammoModel, 0, origin, angles );
//----(SA) end
	}
*/
	//CG_DrawStatusBarHead( 185 + CHAR_WIDTH*3 + TEXT_ICON_SPACE );

	CG_DrawStatusBarKeys();

	if ( cg.predictedPlayerState.powerups[PW_REDFLAG] ) {
		CG_DrawStatusBarFlag( 185 + CHAR_WIDTH * 3 + TEXT_ICON_SPACE + ICON_SIZE, TEAM_RED );
	} else if ( cg.predictedPlayerState.powerups[PW_BLUEFLAG] ) {
		CG_DrawStatusBarFlag( 185 + CHAR_WIDTH * 3 + TEXT_ICON_SPACE + ICON_SIZE, TEAM_BLUE );
	}

	//----(SA) further change... we don't need to draw the armor do we?
/*
	if ( ps->stats[ STAT_ARMOR ] ) {
		origin[0] = 90;
		origin[1] = 0;
		origin[2] = -10;
		angles[YAW] = ( cg.time & 2047 ) * 360 / 2048.0;
//----(SA) Wolf statusbar change
//		CG_Draw3DModel( 370 + CHAR_WIDTH*3 + TEXT_ICON_SPACE, STATUSBARHEIGHT -20, ICON_SIZE, ICON_SIZE,
//					   cgs.media.armorModel, 0, origin, angles );
//----(SA) end
	}
*/
	//
	// ammo
	//
	if ( cent->currentState.weapon ) {
		qhandle_t icon;
		float scale,halfScale;
		float wideOffset;

		value = ps->ammo[BG_FindAmmoForWeapon( cent->currentState.weapon )];
		inclip = ps->ammoclip[BG_FindClipForWeapon( cent->currentState.weapon )];

		if ( value > -1 ) {
			if ( ( cg.predictedPlayerState.weaponstate == WEAPON_FIRING || cg.predictedPlayerState.weaponstate == WEAPON_FIRINGALT )
				 && cg.predictedPlayerState.weaponTime > 100 ) {
				// draw as dark grey when reloading
				color = 2;  // dark grey
			} else {
				if ( value >= 0 ) {
					color = 0;  // green
				} else {
					color = 1;  // red
				}
			}
			trap_R_SetColor( colors[color] );

			// pulsing grenade icon to help the player 'count' in their head
			if ( ps->grenadeTimeLeft ) {
				if ( ps->weapon == WP_DYNAMITE ) {

				} else {
					if ( ( ( cg.grenLastTime ) % 1000 ) < ( ( ps->grenadeTimeLeft ) % 1000 ) ) {
						switch ( ps->grenadeTimeLeft / 1000 ) {
						case 3:
							trap_S_StartLocalSound( cgs.media.grenadePulseSound4, CHAN_LOCAL_SOUND );
							break;
						case 2:
							trap_S_StartLocalSound( cgs.media.grenadePulseSound3, CHAN_LOCAL_SOUND );
							break;
						case 1:
							trap_S_StartLocalSound( cgs.media.grenadePulseSound2, CHAN_LOCAL_SOUND );
							break;
						case 0:
							trap_S_StartLocalSound( cgs.media.grenadePulseSound1, CHAN_LOCAL_SOUND );
							break;
						}
					}
				}

				scale = (float)( ( ps->grenadeTimeLeft ) % 1000 ) / 100.0f;
				halfScale = scale * 0.5f;

				cg.grenLastTime = ps->grenadeTimeLeft;
			} else {
				scale = halfScale = 0;
			}


			switch ( cg.predictedPlayerState.weapon ) {
			case WP_THOMPSON:
			case WP_MP40:
			case WP_STEN:
			case WP_MAUSER:
			case WP_GARAND:
			case WP_VENOM:
			case WP_TESLA:
			case WP_PANZERFAUST:
			case WP_FLAMETHROWER:
				wideOffset = -38;
				break;
			default:
				wideOffset = 0;
				break;
			}

			// don't draw ammo value for knife
			if ( cg.predictedPlayerState.weapon != WP_KNIFE ) {
				if ( cgs.dmflags & DF_NO_WEAPRELOAD ) {
					CG_DrawBigString2( ( 580 - 23 + 35 ) + wideOffset, STATUSBARHEIGHT, va( "%d.", value ), cg_hudAlpha.value );
				} else if ( value ) {
					CG_DrawBigString2( ( 580 - 23 + 35 ) + wideOffset, STATUSBARHEIGHT, va( "%d/%d", inclip, value ), cg_hudAlpha.value );
				} else {
					CG_DrawBigString2( ( 580 - 23 + 35 ) + wideOffset, STATUSBARHEIGHT, va( "%d", inclip ), cg_hudAlpha.value );
				}
			}

			icon = cg_weapons[ cg.predictedPlayerState.weapon ].weaponIcon[0];
			if ( icon ) {
				CG_DrawPic( ( ( 530 + 68 ) - halfScale ) + wideOffset,  ( 446 - 10 ) - halfScale, ( 38 + scale ) - wideOffset, 38 + scale, icon );
			}

			trap_R_SetColor( NULL );

			// if we didn't draw a 3D icon, draw a 2D icon for ammo
			if ( !cg_draw3dIcons.integer ) {
				qhandle_t icon;

				icon = cg_weapons[ cg.predictedPlayerState.weapon ].ammoIcon;
				if ( icon ) {
					CG_DrawPic( CHAR_WIDTH * 3 + TEXT_ICON_SPACE, STATUSBARHEIGHT, ICON_SIZE, ICON_SIZE, icon );
				}
			}
		}
	}

	//
	// health
	//
	value = ps->stats[STAT_HEALTH];
	if ( value > 100 ) {
		trap_R_SetColor( colors[3] );       // white
	} else if ( value > 25 ) {
		trap_R_SetColor( colors[0] );   // green
	} else if ( value > 0 ) {
		color = ( cg.time >> 8 ) & 1; // flash
		trap_R_SetColor( colors[color] );
	} else {
		trap_R_SetColor( colors[1] );   // red
	}

	// stretch the health up when taking damage
//----(SA) Wolf statusbar change
//	CG_DrawField ( 185, STATUSBARHEIGHT, 3, value);
	{
		char printme[16];
		sprintf( printme, "%d", value );
		//CG_DrawBigString( 185, STATUSBARHEIGHT, printme, cg_hudAlpha.value );
		CG_DrawBigString2( 16 + 23 + 43, STATUSBARHEIGHT, printme, cg_hudAlpha.value );
	}
//----(SA) end
	CG_ColorForHealth( hcolor );
	trap_R_SetColor( hcolor );


	//
	// armor
	//
	value = ps->stats[STAT_ARMOR];
	if ( value > 0 ) {
		trap_R_SetColor( colors[0] );
//----(SA) Wolf statusbar change
//		CG_DrawField (370, STATUSBARHEIGHT, 3, value);
		{
			char printme[16];
			sprintf( printme, "%d", value );
			//CG_DrawBigString( 370, STATUSBARHEIGHT, printme, cg_hudAlpha.value );
			CG_DrawBigString2( 200, STATUSBARHEIGHT, printme, cg_hudAlpha.value );
		}
//----(SA) end
		trap_R_SetColor( NULL );
//----(SA) Wolf statusbar change
//		CG_DrawPic( 370 + CHAR_WIDTH*3 + TEXT_ICON_SPACE, STATUSBARHEIGHT, ICON_SIZE, ICON_SIZE, cgs.media.armorIcon );
//----(SA) end
	}
}
// END JOSEPH
#endif  // #ifdef OLDWOLFUI
#endif RTCW_XX

/*
===========================================================================================

  UPPER RIGHT CORNER

===========================================================================================
*/

#if defined RTCW_SP
/*
================
CG_DrawAttacker

================
*/
#ifdef OLDWOLFUI
static float CG_DrawAttacker( float y ) {
	int t;
	float size;
	vec3_t angles;
	const char  *info;
	const char  *name;
	int clientNum;

	if ( cg.predictedPlayerState.stats[STAT_HEALTH] <= 0 ) {
		return y;
	}

	if ( !cg.attackerTime ) {
		return y;
	}

	clientNum = cg.predictedPlayerState.persistant[PERS_ATTACKER];
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS || clientNum == cg.snap->ps.clientNum ) {
		return y;
	}

	t = cg.time - cg.attackerTime;
	if ( t > ATTACKER_HEAD_TIME ) {
		cg.attackerTime = 0;
		return y;
	}

	size = ICON_SIZE * 1.25;

	angles[PITCH] = 0;
	angles[YAW] = 180;
	angles[ROLL] = 0;
	CG_DrawHead( 640 - size, y, size, size, clientNum, angles );

	info = CG_ConfigString( CS_PLAYERS + clientNum );
	name = Info_ValueForKey(  info, "n" );
	y += size;
	CG_DrawBigString( 640 - ( Q_PrintStrlen( name ) * BIGCHAR_WIDTH ), y, name, 0.5 );

	return y + BIGCHAR_HEIGHT + 2;
}
#endif  // #ifdef OLDWOLFUI
#endif RTCW_XX

#define UPPERRIGHT_X 500
/*
==================
CG_DrawSnapshot
==================
*/
static float CG_DrawSnapshot( float y ) {
	char        *s;
	int w;

	s = va( "time:%i snap:%i cmd:%i", cg.snap->serverTime,
			cg.latestSnapshotNum, cgs.serverCommandSequence );
	w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;

	CG_DrawBigString( UPPERRIGHT_X - w, y + 2, s, 1.0F );

	return y + BIGCHAR_HEIGHT + 4;
}

/*
==================
CG_DrawFPS
==================
*/
#define FPS_FRAMES  4
static float CG_DrawFPS( float y ) {
	char        *s;
	int w;
	static int previousTimes[FPS_FRAMES];
	static int index;
	int i, total;
	int fps;
	static int previous;
	int t, frameTime;

	// don't use serverTime, because that will be drifting to
	// correct for internet lag changes, timescales, timedemos, etc
	t = trap_Milliseconds();
	frameTime = t - previous;
	previous = t;

	previousTimes[index % FPS_FRAMES] = frameTime;
	index++;
	if ( index > FPS_FRAMES ) {
		// average multiple frames together to smooth changes out a bit
		total = 0;
		for ( i = 0 ; i < FPS_FRAMES ; i++ ) {
			total += previousTimes[i];
		}
		if ( !total ) {
			total = 1;
		}
		fps = 1000 * FPS_FRAMES / total;

		s = va( "%ifps", fps );
		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;

		CG_DrawBigString( UPPERRIGHT_X - w, y + 2, s, 1.0F );
	}

	return y + BIGCHAR_HEIGHT + 4;
}

/*
=================
CG_DrawTimer
=================
*/
static float CG_DrawTimer( float y ) {
	char        *s;
	int w;
	int mins, seconds, tens;
	int msec;

	// NERVE - SMF - draw time remaining in multiplayer

#if defined RTCW_SP
	if ( cgs.gametype == GT_WOLF ) {
#elif defined RTCW_MP
	if ( cgs.gametype >= GT_WOLF ) {
#endif RTCW_XX

		msec = ( cgs.timelimit * 60.f * 1000.f ) - ( cg.time - cgs.levelStartTime );
	} else {
		msec = cg.time - cgs.levelStartTime;
	}
	// -NERVE - SMF

	seconds = msec / 1000;
	mins = seconds / 60;
	seconds -= mins * 60;
	tens = seconds / 10;
	seconds -= tens * 10;

#if defined RTCW_SP
	s = va( "%i:%i%i", mins, tens, seconds );
#elif defined RTCW_MP
	if ( msec < 0 ) {
		s = va( "Sudden Death" );
	} else {
		s = va( "%i:%i%i", mins, tens, seconds );
	}
#endif RTCW_XX

	w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;

	CG_DrawBigString( UPPERRIGHT_X - w, y + 2, s, 1.0F );

	return y + BIGCHAR_HEIGHT + 4;
}


/*
=================
CG_DrawTeamOverlay
=================
*/

#if defined RTCW_MP
int maxCharsBeforeOverlay;
#endif RTCW_XX

// set in CG_ParseTeamInfo
int sortedTeamPlayers[TEAM_MAXOVERLAY];
int numSortedTeamPlayers;

#define TEAM_OVERLAY_MAXNAME_WIDTH  16
#define TEAM_OVERLAY_MAXLOCATION_WIDTH  20

static float CG_DrawTeamOverlay( float y ) {
	int x, w, h, xx;

#if defined RTCW_SP
	int i, j, len;
#elif defined RTCW_MP
	int i, len;
#endif RTCW_XX

	const char *p;
	vec4_t hcolor;
	int pwidth, lwidth;
	int plyrs;
	char st[16];
	clientInfo_t *ci;

#if defined RTCW_MP
	// NERVE - SMF
	char classType[2] = { 0, 0 };
	int val;
	vec4_t deathcolor, damagecolor;      // JPW NERVE
	float       *pcolor;
	// -NERVE - SMF

	deathcolor[0] = 1;
	deathcolor[1] = 0;
	deathcolor[2] = 0;
	deathcolor[3] = cg_hudAlpha.value;
	damagecolor[0] = 1;
	damagecolor[1] = 1;
	damagecolor[2] = 0;
	damagecolor[3] = cg_hudAlpha.value;
	maxCharsBeforeOverlay = 80;
#endif RTCW_XX

	if ( !cg_drawTeamOverlay.integer ) {
		return y;
	}

	if ( cg.snap->ps.persistant[PERS_TEAM] != TEAM_RED &&
		 cg.snap->ps.persistant[PERS_TEAM] != TEAM_BLUE ) {
		return y; // Not on any team

	}
	plyrs = 0;

	// max player name width
	pwidth = 0;
	for ( i = 0; i < numSortedTeamPlayers; i++ ) {
		ci = cgs.clientinfo + sortedTeamPlayers[i];
		if ( ci->infoValid && ci->team == cg.snap->ps.persistant[PERS_TEAM] ) {
			plyrs++;

#if defined RTCW_SP
			len = CG_DrawStrlen( cgs.clientinfo[i].name );
#elif defined RTCW_MP
			len = CG_DrawStrlen( ci->name );
#endif RTCW_XX

			if ( len > pwidth ) {
				pwidth = len;
			}
		}
	}

	if ( !plyrs ) {
		return y;
	}

	if ( pwidth > TEAM_OVERLAY_MAXNAME_WIDTH ) {
		pwidth = TEAM_OVERLAY_MAXNAME_WIDTH;
	}

#if defined RTCW_SP
#if 0
	// max location name width
	lwidth = 0;
	for ( i = 0; i < numSortedTeamPlayers; i++ ) {
		ci = cgs.clientinfo + sortedTeamPlayers[i];
		if ( ci->infoValid &&
			 ci->team == cg.snap->ps.persistant[PERS_TEAM] &&
			 CG_ConfigString( CS_LOCATIONS + ci->location ) ) {
			len = CG_DrawStrlen( CG_ConfigString( CS_LOCATIONS + ci->location ) );
			if ( len > lwidth ) {
				lwidth = len;
			}
		}
	}
#else
	// max location name width
	lwidth = 0;
	for ( i = 1; i < MAX_LOCATIONS; i++ ) {
		p = CG_ConfigString( CS_LOCATIONS + i );
		if ( p && *p ) {
			len = CG_DrawStrlen( p );
			if ( len > lwidth ) {
				lwidth = len;
			}
		}
	}
#endif
#elif defined RTCW_MP
#if 1
	// max location name width
	lwidth = 0;
	if ( cg_drawTeamOverlay.integer > 1 ) {
		for ( i = 0; i < numSortedTeamPlayers; i++ ) {
			ci = cgs.clientinfo + sortedTeamPlayers[i];
			if ( ci->infoValid &&
				 ci->team == cg.snap->ps.persistant[PERS_TEAM] &&
				 CG_ConfigString( CS_LOCATIONS + ci->location ) ) {
				len = CG_DrawStrlen( CG_TranslateString( CG_ConfigString( CS_LOCATIONS + ci->location ) ) );
				if ( len > lwidth ) {
					lwidth = len;
				}
			}
		}
	}
#else
	// max location name width
	lwidth = 0;
	for ( i = 1; i < MAX_LOCATIONS; i++ ) {
		p = CG_TranslateString( CG_ConfigString( CS_LOCATIONS + i ) );
		if ( p && *p ) {
			len = CG_DrawStrlen( p );
			if ( len > lwidth ) {
				lwidth = len;
			}
		}
	}
#endif
#endif RTCW_XX

	if ( lwidth > TEAM_OVERLAY_MAXLOCATION_WIDTH ) {
		lwidth = TEAM_OVERLAY_MAXLOCATION_WIDTH;
	}

#if defined RTCW_SP
	w = ( pwidth + lwidth + 4 + 7 ) * TINYCHAR_WIDTH;
	x = 640 - w - 32;
	h = plyrs * TINYCHAR_HEIGHT;

	if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED ) {
		hcolor[0] = 1;
		hcolor[1] = 0;
		hcolor[2] = 0;
		hcolor[3] = 0.33;
	} else { // if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE )
		hcolor[0] = 0;
		hcolor[1] = 0;
		hcolor[2] = 1;
		hcolor[3] = 0.33;
	}
	trap_R_SetColor( hcolor );
	CG_DrawPic( x, y, w, h, cgs.media.teamStatusBar );
	trap_R_SetColor( NULL );


	for ( i = 0; i < numSortedTeamPlayers; i++ ) {
		ci = cgs.clientinfo + sortedTeamPlayers[i];
		if ( ci->infoValid && ci->team == cg.snap->ps.persistant[PERS_TEAM] ) {

			hcolor[0] = hcolor[1] = hcolor[2] = hcolor[3] = 1.0;

			xx = x + TINYCHAR_WIDTH;

			CG_DrawStringExt( xx, y,
							  ci->name, hcolor, qfalse, qfalse,
							  TINYCHAR_WIDTH, TINYCHAR_HEIGHT, TEAM_OVERLAY_MAXNAME_WIDTH );

			if ( lwidth ) {
				p = CG_ConfigString( CS_LOCATIONS + ci->location );
				if ( !p || !*p ) {
					p = "unknown";
				}
				len = CG_DrawStrlen( p );
				if ( len > lwidth ) {
					len = lwidth;
				}

				xx = x + TINYCHAR_WIDTH * 2 + TINYCHAR_WIDTH * pwidth +
					 ( ( lwidth / 2 - len / 2 ) * TINYCHAR_WIDTH );
				CG_DrawStringExt( xx, y,
								  p, hcolor, qfalse, qfalse, TINYCHAR_WIDTH, TINYCHAR_HEIGHT,
								  TEAM_OVERLAY_MAXLOCATION_WIDTH );
			}

			CG_ColorForHealth( hcolor );

			Com_sprintf( st, sizeof( st ), "%3i %3i", ci->health,  ci->armor );

			xx = x + TINYCHAR_WIDTH * 3 +
				 TINYCHAR_WIDTH * pwidth + TINYCHAR_WIDTH * lwidth;

			CG_DrawStringExt( xx, y,
							  st, hcolor, qfalse, qfalse,
							  TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0 );

			// draw weapon icon
			xx += TINYCHAR_WIDTH * 3;

			CG_DrawPic( xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT,
						cg_weapons[ci->curWeapon].weaponIcon[0] );

			// Draw powerup icons
			xx = x;
			for ( j = 0; j < PW_NUM_POWERUPS; j++ ) {
				if ( ci->powerups & ( 1 << j ) ) {
					gitem_t *item;

					item = BG_FindItemForPowerup( j );

					if ( item != NULL ) { // JPW NERVE added for invulnerability powerup at beginning of map
						CG_DrawPic( xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT,
									trap_R_RegisterShader( item->icon ) );
						xx -= TINYCHAR_WIDTH;
					} // jpw
				}
			}

			y += TINYCHAR_HEIGHT;
		}
	}

	return y;
#elif defined RTCW_MP
	if ( cg_drawTeamOverlay.integer > 1 ) {
		w = ( pwidth + lwidth + 3 + 7 ) * TINYCHAR_WIDTH; // JPW NERVE was +4+7
	} else {
		w = ( pwidth + lwidth + 8 ) * TINYCHAR_WIDTH; // JPW NERVE was +4+7

	}
	x = 640 - w - 4; // JPW was -32
	h = plyrs * TINYCHAR_HEIGHT;

	// DHM - Nerve :: Set the max characters that can be printed before the left edge
	maxCharsBeforeOverlay = ( x / TINYCHAR_WIDTH ) - 1;

	if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED ) {
		hcolor[0] = 0.5f; // was 0.38 instead of 0.5 JPW NERVE
		hcolor[1] = 0.25f;
		hcolor[2] = 0.25f;
		hcolor[3] = 0.25f * cg_hudAlpha.value;
	} else { // if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE )
		hcolor[0] = 0.25f;
		hcolor[1] = 0.25f;
		hcolor[2] = 0.5f;
		hcolor[3] = 0.25f * cg_hudAlpha.value;
	}

	CG_FillRect( x,y,w,h,hcolor );
	VectorSet( hcolor, 0.4f, 0.4f, 0.4f );
	hcolor[3] = cg_hudAlpha.value;
	CG_DrawRect( x - 1, y, w + 2, h + 2, 1, hcolor );


	for ( i = 0; i < numSortedTeamPlayers; i++ ) {
		ci = cgs.clientinfo + sortedTeamPlayers[i];
		if ( ci->infoValid && ci->team == cg.snap->ps.persistant[PERS_TEAM] ) {

			// NERVE - SMF
			// determine class type
			val = cg_entities[ ci->clientNum ].currentState.teamNum;
			if ( val == 0 ) {
				classType[0] = 'S';
			} else if ( val == 1 ) {
				classType[0] = 'M';
			} else if ( val == 2 ) {
				classType[0] = 'E';
			} else if ( val == 3 ) {
				classType[0] = 'L';
			} else {
				classType[0] = 'S';
			}

			Com_sprintf( st, sizeof( st ), "%s", CG_TranslateString( classType ) );

			xx = x + TINYCHAR_WIDTH;

			hcolor[0] = hcolor[1] = 1.0;
			hcolor[2] = 0.0;
			hcolor[3] = cg_hudAlpha.value;

			CG_DrawStringExt( xx, y,
							  st, hcolor, qtrue, qfalse,
							  TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 1 );

			hcolor[0] = hcolor[1] = hcolor[2] = 1.0;
			hcolor[3] = cg_hudAlpha.value;

			xx = x + 3 * TINYCHAR_WIDTH;

			// JPW NERVE
			if ( ci->health > 80 ) {
				pcolor = hcolor;
			} else if ( ci->health > 0 ) {
				pcolor = damagecolor;
			} else {
				pcolor = deathcolor;
			}
			// jpw

			CG_DrawStringExt( xx, y,
							  ci->name, pcolor, qtrue, qfalse,
							  TINYCHAR_WIDTH, TINYCHAR_HEIGHT, TEAM_OVERLAY_MAXNAME_WIDTH );

			if ( lwidth ) {
				p = CG_ConfigString( CS_LOCATIONS + ci->location );
				if ( !p || !*p ) {
					p = "unknown";
				}
				p = CG_TranslateString( p );
				len = CG_DrawStrlen( p );
				if ( len > lwidth ) {
					len = lwidth;
				}

				xx = x + TINYCHAR_WIDTH * 5 + TINYCHAR_WIDTH * pwidth +
					 ( ( lwidth / 2 - len / 2 ) * TINYCHAR_WIDTH );
				CG_DrawStringExt( xx, y,
								  p, hcolor, qfalse, qfalse, TINYCHAR_WIDTH, TINYCHAR_HEIGHT,
								  TEAM_OVERLAY_MAXLOCATION_WIDTH );
			}


			Com_sprintf( st, sizeof( st ), "%3i", ci->health ); // JPW NERVE pulled class stuff since it's at top now

			if ( cg_drawTeamOverlay.integer > 1 ) {
				xx = x + TINYCHAR_WIDTH * 6 + TINYCHAR_WIDTH * pwidth + TINYCHAR_WIDTH * lwidth;
			} else {
				xx = x + TINYCHAR_WIDTH * 4 + TINYCHAR_WIDTH * pwidth + TINYCHAR_WIDTH * lwidth;
			}

			CG_DrawStringExt( xx, y,
							  st, pcolor, qfalse, qfalse,
							  TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 3 );

			y += TINYCHAR_HEIGHT;
		}
	}

	return y;
#endif RTCW_XX

}


/*
=====================
CG_DrawUpperRight

=====================
*/
static void CG_DrawUpperRight( void ) {
	float y;

#if defined RTCW_SP
	y = 0;
#elif defined RTCW_MP
	y = 0; // JPW NERVE move team overlay below obits, even with timer on left
#endif RTCW_XX

	if ( cgs.gametype >= GT_TEAM ) {
		y = CG_DrawTeamOverlay( y );
	}
	if ( cg_drawSnapshot.integer ) {
		y = CG_DrawSnapshot( y );
	}
	if ( cg_drawFPS.integer ) {
		y = CG_DrawFPS( y );
	}
	if ( cg_drawTimer.integer ) {
		y = CG_DrawTimer( y );
	}
// (SA) disabling drawattacker for the time being
//	if ( cg_drawAttacker.integer ) {
//		y = CG_DrawAttacker( y );
//	}
//----(SA)	end
}

/*
===========================================================================================

  LOWER RIGHT CORNER

===========================================================================================
*/

#if defined RTCW_SP
/*
=================
CG_DrawScores

Draw the small two score display
=================
*/
#ifdef OLDWOLFUI
static float CG_DrawScores( float y ) {
	const char  *s;
	int s1, s2, score;
	int x, w;
	int v;
	vec4_t color;

	s = CG_ConfigString( CS_SCORES1 );
	s1 = cgs.scores1;
	s = CG_ConfigString( CS_SCORES2 );
	s2 = cgs.scores2;

	y -=  BIGCHAR_HEIGHT + 8;

	// draw from the right side to left
	if ( cgs.gametype >= GT_TEAM ) {
		x = 640;

		color[0] = 0;
		color[1] = 0;
		color[2] = 1;
		color[3] = 0.33;
		s = va( "%2i", s2 );
		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
		x -= w;
		CG_FillRect( x, y - 4,  w, BIGCHAR_HEIGHT + 8, color );
		if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE ) {
			CG_DrawPic( x, y - 4, w, BIGCHAR_HEIGHT + 8, cgs.media.selectShader );
		}
		CG_DrawBigString( x + 4, y, s, 1.0F );


		color[0] = 1;
		color[1] = 0;
		color[2] = 0;
		color[3] = 0.33;
		s = va( "%2i", s1 );
		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
		x -= w;
		CG_FillRect( x, y - 4,  w, BIGCHAR_HEIGHT + 8, color );
		if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED ) {
			CG_DrawPic( x, y - 4, w, BIGCHAR_HEIGHT + 8, cgs.media.selectShader );
		}
		CG_DrawBigString( x + 4, y, s, 1.0F );

		if ( cgs.gametype == GT_CTF ) {
			v = cgs.capturelimit;
		} else {
			v = cgs.fraglimit;
		}
		if ( v ) {
			s = va( "%2i", v );
			w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
			x -= w;
			CG_DrawBigString( x + 4, y, s, 1.0F );
		}

//----(SA) don't show frag count/limit in sp
	} else if ( cgs.gametype != GT_SINGLE_PLAYER && cg_drawFrags.integer ) {
//----(SA) end
		qboolean spectator;

		x = 640;
		score = cg.snap->ps.persistant[PERS_SCORE];
		spectator = ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR );

		// always show your score in the second box if not in first place
		if ( s1 != score ) {
			s2 = score;
		}
		if ( s2 != SCORE_NOT_PRESENT ) {
			s = va( "%2i", s2 );
			w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
			x -= w;
			if ( !spectator && score == s2 && score != s1 ) {
				color[0] = 1;
				color[1] = 0;
				color[2] = 0;
				color[3] = 0.33;
				CG_FillRect( x, y - 4,  w, BIGCHAR_HEIGHT + 8, color );
				CG_DrawPic( x, y - 4, w, BIGCHAR_HEIGHT + 8, cgs.media.selectShader );
			} else {
				color[0] = 0.5;
				color[1] = 0.5;
				color[2] = 0.5;
				color[3] = 0.33;
				CG_FillRect( x, y - 4,  w, BIGCHAR_HEIGHT + 8, color );
			}
			CG_DrawBigString( x + 4, y, s, 1.0F );
		}

		// first place
		if ( s1 != SCORE_NOT_PRESENT ) {
			s = va( "%2i", s1 );
			w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
			x -= w;
			if ( !spectator && score == s1 ) {
				color[0] = 0;
				color[1] = 0;
				color[2] = 1;
				color[3] = 0.33;
				CG_FillRect( x, y - 4,  w, BIGCHAR_HEIGHT + 8, color );
				CG_DrawPic( x, y - 4, w, BIGCHAR_HEIGHT + 8, cgs.media.selectShader );
			} else {
				color[0] = 0.5;
				color[1] = 0.5;
				color[2] = 0.5;
				color[3] = 0.33;
				CG_FillRect( x, y - 4,  w, BIGCHAR_HEIGHT + 8, color );
			}
			CG_DrawBigString( x + 4, y, s, 1.0F );
		}

		if ( cgs.fraglimit ) {
			s = va( "%2i", cgs.fraglimit );
			w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
			x -= w;
			CG_DrawBigString( x + 4, y, s, 1.0F );
		}

	}

	return y - 8;
}
#endif  // #ifdef OLDWOLFUI

/*
================
CG_DrawPowerups
================
*/
#ifdef OLDWOLFUI
static float CG_DrawPowerups( float y ) {
	int sorted[MAX_POWERUPS];
	int sortedTime[MAX_POWERUPS];
	int i, j, k;
	int active;
	playerState_t   *ps;
	int t;
	gitem_t *item;
	int x;
	int color;
	float size;
	float f;
	static float colors[2][4] = {
		{ 0.2, 1.0, 0.2, 1.0 }, { 1.0, 0.2, 0.2, 1.0 }
	};

	ps = &cg.snap->ps;

	if ( ps->stats[STAT_HEALTH] <= 0 ) {
		return y;
	}

	// sort the list by time remaining
	active = 0;
	for ( i = 0 ; i < MAX_POWERUPS ; i++ ) {
		if ( !ps->powerups[ i ] ) {
			continue;
		}
		t = ps->powerups[ i ] - cg.time;
		// ZOID--don't draw if the power up has unlimited time (999 seconds)
		// This is true of the CTF flags
		if ( t < 0 || t > 999000 ) {
			continue;
		}

		// insert into the list
		for ( j = 0 ; j < active ; j++ ) {
			if ( sortedTime[j] >= t ) {
				for ( k = active - 1 ; k >= j ; k-- ) {
					sorted[k + 1] = sorted[k];
					sortedTime[k + 1] = sortedTime[k];
				}
				break;
			}
		}
		sorted[j] = i;
		sortedTime[j] = t;
		active++;
	}

	// draw the icons and timers
	x = 640 - ICON_SIZE - CHAR_WIDTH * 2;
	for ( i = 0 ; i < active ; i++ ) {

		continue;   // (SA) FIXME: TEMP: as I'm getting powerup business going

		item = BG_FindItemForPowerup( sorted[i] );

		color = 1;

		y -= ICON_SIZE;

		trap_R_SetColor( colors[color] );
		CG_DrawField( x, y, 2, sortedTime[ i ] / 1000 );

		t = ps->powerups[ sorted[i] ];
		if ( t - cg.time >= POWERUP_BLINKS * POWERUP_BLINK_TIME ) {
			trap_R_SetColor( NULL );
		} else {
			vec4_t modulate;

			f = (float)( t - cg.time ) / POWERUP_BLINK_TIME;
			f -= (int)f;
			modulate[0] = modulate[1] = modulate[2] = modulate[3] = f;
			trap_R_SetColor( modulate );
		}

		if ( cg.powerupActive == sorted[i] &&
			 cg.time - cg.powerupTime < PULSE_TIME ) {
			f = 1.0 - ( ( (float)cg.time - cg.powerupTime ) / PULSE_TIME );
			size = ICON_SIZE * ( 1.0 + ( PULSE_SCALE - 1.0 ) * f );
		} else {
			size = ICON_SIZE;
		}

		CG_DrawPic( 640 - size, y + ICON_SIZE / 2 - size / 2,
					size, size, trap_R_RegisterShader( item->icon ) );
	}
	trap_R_SetColor( NULL );

	return y;
}
#endif  // #ifdef OLDWOLFUI


/*
=====================
CG_DrawLowerRight

=====================
*/
#ifdef OLDWOLFUI
static void CG_DrawLowerRight( void ) {
	float y;

	y = 480 - ICON_SIZE;

	y = CG_DrawScores( y );
	y = CG_DrawPowerups( y );
}
#endif  // #ifdef OLDWOLFUI

//===========================================================================================
#endif RTCW_XX

/*
=================
CG_DrawTeamInfo
=================
*/
static void CG_DrawTeamInfo( void ) {
	int w, h;
	int i, len;
	vec4_t hcolor;
	int chatHeight;

#if defined RTCW_MP
	float alphapercent;
#endif RTCW_XX

#if defined RTCW_SP
#define CHATLOC_Y 420 // bottom end
#elif defined RTCW_MP
#define CHATLOC_Y 385 // bottom end
#endif RTCW_XX

#define CHATLOC_X 0

	if ( cg_teamChatHeight.integer < TEAMCHAT_HEIGHT ) {
		chatHeight = cg_teamChatHeight.integer;
	} else {
		chatHeight = TEAMCHAT_HEIGHT;
	}
	if ( chatHeight <= 0 ) {
		return; // disabled

	}
	if ( cgs.teamLastChatPos != cgs.teamChatPos ) {
		if ( cg.time - cgs.teamChatMsgTimes[cgs.teamLastChatPos % chatHeight] > cg_teamChatTime.integer ) {
			cgs.teamLastChatPos++;
		}

		h = ( cgs.teamChatPos - cgs.teamLastChatPos ) * TINYCHAR_HEIGHT;

		w = 0;

		for ( i = cgs.teamLastChatPos; i < cgs.teamChatPos; i++ ) {
			len = CG_DrawStrlen( cgs.teamChatMsgs[i % chatHeight] );
			if ( len > w ) {
				w = len;
			}
		}
		w *= TINYCHAR_WIDTH;
		w += TINYCHAR_WIDTH * 2;

#if defined RTCW_SP
		if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED ) {
			hcolor[0] = 1;
			hcolor[1] = 0;
			hcolor[2] = 0;
			hcolor[3] = 0.33;
		} else if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE ) {
			hcolor[0] = 0;
			hcolor[1] = 0;
			hcolor[2] = 1;
			hcolor[3] = 0.33;
		} else {
			hcolor[0] = 0;
			hcolor[1] = 1;
			hcolor[2] = 0;
			hcolor[3] = 0.33;
		}

		trap_R_SetColor( hcolor );
		CG_DrawPic( CHATLOC_X, CHATLOC_Y - h, 640, h, cgs.media.teamStatusBar );
		trap_R_SetColor( NULL );

		hcolor[0] = hcolor[1] = hcolor[2] = 1.0;
		hcolor[3] = 1.0;

		for ( i = cgs.teamChatPos - 1; i >= cgs.teamLastChatPos; i-- ) {
			CG_DrawStringExt( CHATLOC_X + TINYCHAR_WIDTH,
							  CHATLOC_Y - ( cgs.teamChatPos - i ) * TINYCHAR_HEIGHT,
							  cgs.teamChatMsgs[i % chatHeight], hcolor, qfalse, qfalse,
							  TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0 );
//			CG_DrawSmallString( CHATLOC_X + SMALLCHAR_WIDTH,
//				CHATLOC_Y - (cgs.teamChatPos - i)*SMALLCHAR_HEIGHT,
//				cgs.teamChatMsgs[i % TEAMCHAT_HEIGHT], 1.0F );
		}
	}
#elif defined RTCW_MP
// JPW NERVE rewritten to support first pass at fading chat messages
		for ( i = cgs.teamChatPos - 1; i >= cgs.teamLastChatPos; i-- ) {
			alphapercent = 1.0f - ( cg.time - cgs.teamChatMsgTimes[i % chatHeight] ) / (float)( cg_teamChatTime.integer );
			if ( alphapercent > 1.0f ) {
				alphapercent = 1.0f;
			} else if ( alphapercent < 0.f ) {
				alphapercent = 0.f;
			}

			if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED ) {
				hcolor[0] = 1;
				hcolor[1] = 0;
				hcolor[2] = 0;
//			hcolor[3] = 0.33;
			} else if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE ) {
				hcolor[0] = 0;
				hcolor[1] = 0;
				hcolor[2] = 1;
//			hcolor[3] = 0.33;
			} else {
				hcolor[0] = 0;
				hcolor[1] = 1;
				hcolor[2] = 0;
//			hcolor[3] = 0.33;
			}

			hcolor[3] = 0.33f * alphapercent;

			trap_R_SetColor( hcolor );
			CG_DrawPic( CHATLOC_X, CHATLOC_Y - ( cgs.teamChatPos - i ) * TINYCHAR_HEIGHT, 640, TINYCHAR_HEIGHT, cgs.media.teamStatusBar );

			hcolor[0] = hcolor[1] = hcolor[2] = 1.0;
			hcolor[3] = alphapercent;
			trap_R_SetColor( hcolor );

			CG_DrawStringExt( CHATLOC_X + TINYCHAR_WIDTH,
							  CHATLOC_Y - ( cgs.teamChatPos - i ) * TINYCHAR_HEIGHT,
							  cgs.teamChatMsgs[i % chatHeight], hcolor, qfalse, qfalse,
							  TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0 );
//			CG_DrawSmallString( CHATLOC_X + SMALLCHAR_WIDTH,
//				CHATLOC_Y - (cgs.teamChatPos - i)*SMALLCHAR_HEIGHT,
//				cgs.teamChatMsgs[i % TEAMCHAT_HEIGHT], 1.0F );
		}
// jpw
	}
#endif RTCW_XX

}

//----(SA)	modified
/*
===================
CG_DrawPickupItem
===================
*/
static void CG_DrawPickupItem( void ) {
	int value;
	float   *fadeColor;
	char pickupText[256];
	float color[4];

#if defined RTCW_MP
	const char *s;
#endif RTCW_XX

	value = cg.itemPickup;
	if ( value ) {
		fadeColor = CG_FadeColor( cg.itemPickupTime, 3000 );
		if ( fadeColor ) {
			CG_RegisterItemVisuals( value );

#if defined RTCW_SP
//----(SA) commented out
//			trap_R_SetColor( fadeColor );
//			CG_DrawPic( 8, 380, ICON_SIZE, ICON_SIZE, cg_items[ value ].icon );
//----(SA) end
#endif RTCW_XX

			//----(SA)	so we don't pick up all sorts of items and have it print "0 <itemname>"
			if ( bg_itemlist[ value ].giType == IT_AMMO || bg_itemlist[ value ].giType == IT_HEALTH || bg_itemlist[value].giType == IT_POWERUP ) {
				if ( bg_itemlist[ value ].world_model[2] ) {   // this is a multi-stage item
					// FIXME: print the correct amount for multi-stage

#if defined RTCW_SP
					Com_sprintf( pickupText, sizeof( pickupText ), "%s", cgs.itemPrintNames[ value ] );
				} else {
					if ( bg_itemlist[ value ].gameskillnumber[cg_gameSkill.integer] > 1 ) {
						Com_sprintf( pickupText, sizeof( pickupText ), "%i  %s", bg_itemlist[ value ].gameskillnumber[cg_gameSkill.integer], cgs.itemPrintNames[ value ] );
					} else {
						Com_sprintf( pickupText, sizeof( pickupText ), "%s", cgs.itemPrintNames[value] );
					}
				}
			} else {
				Com_sprintf( pickupText, sizeof( pickupText ), "%s", cgs.itemPrintNames[value] );
			}

			//----(SA)	trying smaller text
			color[0] = color[1] = color[2] = 1.0;
			color[3] = fadeColor[0];
			CG_DrawStringExt2( ICON_SIZE + 16, 398, pickupText, color, qfalse, qtrue, 10, 10, 0 );
//			CG_Text_Paint(ICON_SIZE + 16, 398, 2, 0.3f, color, pickupText, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE);
#elif defined RTCW_MP
					Com_sprintf( pickupText, sizeof( pickupText ), "%s", bg_itemlist[ value ].pickup_name );
				} else {
					Com_sprintf( pickupText, sizeof( pickupText ), "%i  %s", bg_itemlist[ value ].gameskillnumber[( cg_gameSkill.integer ) - 1], bg_itemlist[ value ].pickup_name );
				}
			} else {
				if ( !Q_stricmp( "Blue Flag", bg_itemlist[ value ].pickup_name ) ) {
					Com_sprintf( pickupText, sizeof( pickupText ), "%s", "Objective" );
				} else {
					Com_sprintf( pickupText, sizeof( pickupText ), "%s", bg_itemlist[ value ].pickup_name );
				}
			}

			s = CG_TranslateString( pickupText );

			color[0] = color[1] = color[2] = 1.0;
			color[3] = fadeColor[0];
			CG_DrawStringExt2( 34, 388, s, color, qfalse, qtrue, 10, 10, 0 ); // JPW NERVE moved per atvi req
#endif RTCW_XX

			trap_R_SetColor( NULL );
		}
	}
}
//----(SA)	end

#if defined RTCW_SP
/*
===================
CG_DrawHoldableItem
===================
*/
void CG_DrawHoldableItem_old( void ) {
	int value;
	gitem_t *item;

	if ( !cg.holdableSelect ) {
		return;
	}

	item    = BG_FindItemForHoldable( cg.holdableSelect );

	if ( !item ) {
		return;
	}

	value   = cg.predictedPlayerState.holdable[cg.holdableSelect];

	if ( value ) {

		trap_R_SetColor( NULL );

		CG_RegisterItemVisuals( item - bg_itemlist );

		if ( cg.holdableSelect == HI_WINE ) {
			if ( value > 3 ) {
				value = 3;  // 3 stages to icon, just draw full if beyond 'full'

			}
			//----(SA)	trying smaller text
			//----(SA)	and off to the right side of the HUD
//			CG_DrawPic( 100, (SCREEN_HEIGHT-ICON_SIZE)-8, ICON_SIZE/2, ICON_SIZE, cg_items[item - bg_itemlist].icons[2-(value-1)] );
			CG_DrawPic( 606, 366, 24, 48, cg_items[item - bg_itemlist].icons[2 - ( value - 1 )] );

		} else {
//			CG_DrawPic( 100, (SCREEN_HEIGHT-ICON_SIZE)-8, ICON_SIZE/2, ICON_SIZE, cg_items[item - bg_itemlist].icons[0] );
			CG_DrawPic( 606, 366, 24, 48, cg_items[item - bg_itemlist].icons[0] );

		}

		// draw the selection box so it's not just floating in space
		CG_DrawPic( 606 - 4, 366 - 4, 32, 56, cgs.media.selectShader );
	}
}
/*
		if(cg.holdableSelect == HI_WINE) {
			if(value > 3)
				value = 3;	// 3 stages to icon, just draw full if beyond 'full'

			CG_DrawPic( 598 + 16, 366, 16, 32, cg_items[item - bg_itemlist].icons[2-(value-1)] );
			CG_DrawPic( (598 + 16)-4, 366-4, 24, 40, cgs.media.selectShader );

		} else {
			CG_DrawPic( 598, 366, 32, 32, cg_items[item - bg_itemlist].icons[0] );
			CG_DrawPic( 598-4, 366-4, 40, 40, cgs.media.selectShader );
		}
*/

/*
===================
CG_DrawReward
===================
*/
static void CG_DrawReward( void ) {
	float   *color;
	int i;
	float x, y;

	if ( !cg_drawRewards.integer ) {
		return;
	}
	color = CG_FadeColor( cg.rewardTime, REWARD_TIME );
	if ( !color ) {
		return;
	}

	trap_R_SetColor( color );
	y = 56;
	x = 320 - cg.rewardCount * ICON_SIZE / 2;
	for ( i = 0 ; i < cg.rewardCount ; i++ ) {
		CG_DrawPic( x, y, ICON_SIZE - 4, ICON_SIZE - 4, cg.rewardShader );
		x += ICON_SIZE;
	}
	trap_R_SetColor( NULL );
}
#elif defined RTCW_MP
/*
=================
CG_DrawNotify
=================
*/
#define NOTIFYLOC_Y 42 // bottom end
#define NOTIFYLOC_X 0

static void CG_DrawNotify( void ) {
	int w, h;
	int i, len;
	vec4_t hcolor;
	int chatHeight;
	float alphapercent;
	char var[MAX_TOKEN_CHARS];
	float notifytime = 1.0f;

	trap_Cvar_VariableStringBuffer( "con_notifytime", var, sizeof( var ) );
	notifytime = atof( var ) * 1000;

	if ( notifytime <= 100.f ) {
		notifytime = 100.0f;
	}

	chatHeight = NOTIFY_HEIGHT;

	if ( cgs.notifyLastPos != cgs.notifyPos ) {
		if ( cg.time - cgs.notifyMsgTimes[cgs.notifyLastPos % chatHeight] > notifytime ) {
			cgs.notifyLastPos++;
		}

		h = ( cgs.notifyPos - cgs.notifyLastPos ) * TINYCHAR_HEIGHT;

		w = 0;

		for ( i = cgs.notifyLastPos; i < cgs.notifyPos; i++ ) {
			len = CG_DrawStrlen( cgs.notifyMsgs[i % chatHeight] );
			if ( len > w ) {
				w = len;
			}
		}
		w *= TINYCHAR_WIDTH;
		w += TINYCHAR_WIDTH * 2;

		if ( maxCharsBeforeOverlay <= 0 ) {
			maxCharsBeforeOverlay = 80;
		}

		for ( i = cgs.notifyPos - 1; i >= cgs.notifyLastPos; i-- ) {
			alphapercent = 1.0f - ( ( cg.time - cgs.notifyMsgTimes[i % chatHeight] ) / notifytime );
			if ( alphapercent > 0.5f ) {
				alphapercent = 1.0f;
			} else {
				alphapercent *= 2;
			}

			if ( alphapercent < 0.f ) {
				alphapercent = 0.f;
			}

			hcolor[0] = hcolor[1] = hcolor[2] = 1.0;
			hcolor[3] = alphapercent;
			trap_R_SetColor( hcolor );

			CG_DrawStringExt( NOTIFYLOC_X + TINYCHAR_WIDTH,
							  NOTIFYLOC_Y - ( cgs.notifyPos - i ) * TINYCHAR_HEIGHT,
							  cgs.notifyMsgs[i % chatHeight], hcolor, qfalse, qfalse,
							  TINYCHAR_WIDTH, TINYCHAR_HEIGHT, maxCharsBeforeOverlay );
		}
	}
}
#endif RTCW_XX

/*
===============================================================================

LAGOMETER

===============================================================================
*/

#define LAG_SAMPLES     128


typedef struct {
	int frameSamples[LAG_SAMPLES];
	int frameCount;
	int snapshotFlags[LAG_SAMPLES];
	int snapshotSamples[LAG_SAMPLES];
	int snapshotCount;
} lagometer_t;

lagometer_t lagometer;

/*
==============
CG_AddLagometerFrameInfo

Adds the current interpolate / extrapolate bar for this frame
==============
*/
void CG_AddLagometerFrameInfo( void ) {
	int offset;

	offset = cg.time - cg.latestSnapshotTime;
	lagometer.frameSamples[ lagometer.frameCount & ( LAG_SAMPLES - 1 ) ] = offset;
	lagometer.frameCount++;
}

/*
==============
CG_AddLagometerSnapshotInfo

Each time a snapshot is received, log its ping time and
the number of snapshots that were dropped before it.

Pass NULL for a dropped packet.
==============
*/
void CG_AddLagometerSnapshotInfo( snapshot_t *snap ) {
	// dropped packet
	if ( !snap ) {
		lagometer.snapshotSamples[ lagometer.snapshotCount & ( LAG_SAMPLES - 1 ) ] = -1;
		lagometer.snapshotCount++;
		return;
	}

	// add this snapshot's info
	lagometer.snapshotSamples[ lagometer.snapshotCount & ( LAG_SAMPLES - 1 ) ] = snap->ping;
	lagometer.snapshotFlags[ lagometer.snapshotCount & ( LAG_SAMPLES - 1 ) ] = snap->snapFlags;
	lagometer.snapshotCount++;
}

/*
==============
CG_DrawDisconnect

Should we draw something differnet for long lag vs no packets?
==============
*/
static void CG_DrawDisconnect( void ) {
	float x, y;
	int cmdNum;
	usercmd_t cmd;
	const char      *s;
	int w;          // bk010215 - FIXME char message[1024];

	// draw the phone jack if we are completely past our buffers
	cmdNum = trap_GetCurrentCmdNumber() - CMD_BACKUP + 1;
	trap_GetUserCmd( cmdNum, &cmd );
	if ( cmd.serverTime <= cg.snap->ps.commandTime
		 || cmd.serverTime > cg.time ) { // special check for map_restart // bk 0102165 - FIXME
		return;
	}

	// also add text in center of screen

#if defined RTCW_SP
	s = "Connection Interrupted"; // bk 010215 - FIXME
#elif defined RTCW_MP
	s = CG_TranslateString( "Connection Interrupted" ); // bk 010215 - FIXME
#endif RTCW_XX

	w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
	CG_DrawBigString( 320 - w / 2, 100, s, 1.0F );

	// blink the icon
	if ( ( cg.time >> 9 ) & 1 ) {
		return;
	}

#if defined RTCW_SP
	x = 640 - 48;
	y = 480 - 48;
#elif defined RTCW_MP
	x = 640 - 72;
	y = 480 - 52;
#endif RTCW_XX

	CG_DrawPic( x, y, 48, 48, trap_R_RegisterShader( "gfx/2d/net.tga" ) );
}


#define MAX_LAGOMETER_PING  900
#define MAX_LAGOMETER_RANGE 300

/*
==============
CG_DrawLagometer
==============
*/
static void CG_DrawLagometer( void ) {
	int a, x, y, i;
	float v;
	float ax, ay, aw, ah, mid, range;
	int color;
	float vscale;

	if ( !cg_lagometer.integer || cgs.localServer ) {
//	if(0) {
		CG_DrawDisconnect();
		return;
	}

	//
	// draw the graph
	//

#if defined RTCW_SP
	x = 640 - 48;
	y = 480 - 48;
#elif defined RTCW_MP
	x = 640 - 55;
	y = 480 - 140;
#endif RTCW_XX

	trap_R_SetColor( NULL );
	CG_DrawPic( x, y, 48, 48, cgs.media.lagometerShader );

	ax = x;
	ay = y;
	aw = 48;
	ah = 48;
	CG_AdjustFrom640( &ax, &ay, &aw, &ah );

	color = -1;
	range = ah / 3;
	mid = ay + range;

	vscale = range / MAX_LAGOMETER_RANGE;

	// draw the frame interpoalte / extrapolate graph
	for ( a = 0 ; a < aw ; a++ ) {
		i = ( lagometer.frameCount - 1 - a ) & ( LAG_SAMPLES - 1 );
		v = lagometer.frameSamples[i];
		v *= vscale;
		if ( v > 0 ) {
			if ( color != 1 ) {
				color = 1;
				trap_R_SetColor( g_color_table[ColorIndex( COLOR_YELLOW )] );
			}
			if ( v > range ) {
				v = range;
			}
			trap_R_DrawStretchPic( ax + aw - a, mid - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
		} else if ( v < 0 ) {
			if ( color != 2 ) {
				color = 2;
				trap_R_SetColor( g_color_table[ColorIndex( COLOR_BLUE )] );
			}
			v = -v;
			if ( v > range ) {
				v = range;
			}
			trap_R_DrawStretchPic( ax + aw - a, mid, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
		}
	}

	// draw the snapshot latency / drop graph
	range = ah / 2;
	vscale = range / MAX_LAGOMETER_PING;

	for ( a = 0 ; a < aw ; a++ ) {
		i = ( lagometer.snapshotCount - 1 - a ) & ( LAG_SAMPLES - 1 );
		v = lagometer.snapshotSamples[i];
		if ( v > 0 ) {
			if ( lagometer.snapshotFlags[i] & SNAPFLAG_RATE_DELAYED ) {
				if ( color != 5 ) {
					color = 5;  // YELLOW for rate delay
					trap_R_SetColor( g_color_table[ColorIndex( COLOR_YELLOW )] );
				}
			} else {
				if ( color != 3 ) {
					color = 3;
					trap_R_SetColor( g_color_table[ColorIndex( COLOR_GREEN )] );
				}
			}
			v = v * vscale;
			if ( v > range ) {
				v = range;
			}
			trap_R_DrawStretchPic( ax + aw - a, ay + ah - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
		} else if ( v < 0 ) {
			if ( color != 4 ) {
				color = 4;      // RED for dropped snapshots
				trap_R_SetColor( g_color_table[ColorIndex( COLOR_RED )] );
			}
			trap_R_DrawStretchPic( ax + aw - a, ay + ah - range, 1, range, 0, 0, 0, 0, cgs.media.whiteShader );
		}
	}

	trap_R_SetColor( NULL );

	if ( cg_nopredict.integer || cg_synchronousClients.integer ) {
		CG_DrawBigString( ax, ay, "snc", 1.0 );
	}

	CG_DrawDisconnect();
}


/*
===============================================================================

CENTER PRINTING

===============================================================================
*/


/*
==============
CG_CenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/

#if defined RTCW_MP
#define CP_LINEWIDTH 55         // NERVE - SMF
#endif RTCW_XX

void CG_CenterPrint( const char *str, int y, int charWidth ) {

#if defined RTCW_SP
	unsigned char   *s;

//----(SA)	added translation lookup
	Q_strncpyz( cg.centerPrint, CG_translateString( (char*)str ), sizeof( cg.centerPrint ) );
//----(SA)	end

#elif defined RTCW_MP
	char    *s;
	int i, len;                         // NERVE - SMF
	qboolean neednewline = qfalse;      // NERVE - SMF
	int priority = 0;

	// NERVE - SMF - don't draw if this print message is less important
	if ( cg.centerPrintTime && priority < cg.centerPrintPriority ) {
		return;
	}

	Q_strncpyz( cg.centerPrint, str, sizeof( cg.centerPrint ) );
	cg.centerPrintPriority = priority;  // NERVE - SMF

	// NERVE - SMF - turn spaces into newlines, if we've run over the linewidth
	len = strlen( cg.centerPrint );
	for ( i = 0; i < len; i++ ) {

		// NOTE: subtract a few chars here so long words still get displayed properly
		if ( i % ( CP_LINEWIDTH - 20 ) == 0 && i > 0 ) {
			neednewline = qtrue;
		}
		if ( cg.centerPrint[i] == ' ' && neednewline ) {
			cg.centerPrint[i] = '\n';
			neednewline = qfalse;
		}
	}
	// -NERVE - SMF
#endif RTCW_XX

	cg.centerPrintTime = cg.time;
	cg.centerPrintY = y;
	cg.centerPrintCharWidth = charWidth;

	// count the number of lines for centering
	cg.centerPrintLines = 1;
	s = cg.centerPrint;
	while ( *s ) {
		if ( *s == '\n' ) {
			cg.centerPrintLines++;
		}

#if defined RTCW_SP
		if ( !Q_strncmp( s, "\\n", 1 ) ) {
			cg.centerPrintLines++;
			s++;
		}
#endif RTCW_XX

		s++;
	}
}

#if defined RTCW_MP
// NERVE - SMF
/*
==============
CG_PriorityCenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void CG_PriorityCenterPrint( const char *str, int y, int charWidth, int priority ) {
	char    *s;
	int i, len;                         // NERVE - SMF
	qboolean neednewline = qfalse;      // NERVE - SMF

	// NERVE - SMF - don't draw if this print message is less important
	if ( cg.centerPrintTime && priority < cg.centerPrintPriority ) {
		return;
	}

	Q_strncpyz( cg.centerPrint, str, sizeof( cg.centerPrint ) );
	cg.centerPrintPriority = priority;  // NERVE - SMF

	// NERVE - SMF - turn spaces into newlines, if we've run over the linewidth
	len = strlen( cg.centerPrint );
	for ( i = 0; i < len; i++ ) {

		// NOTE: subtract a few chars here so long words still get displayed properly
		if ( i % ( CP_LINEWIDTH - 20 ) == 0 && i > 0 ) {
			neednewline = qtrue;
		}
		if ( cg.centerPrint[i] == ' ' && neednewline ) {
			cg.centerPrint[i] = '\n';
			neednewline = qfalse;
		}
	}
	// -NERVE - SMF

	cg.centerPrintTime = cg.time + 2000;
	cg.centerPrintY = y;
	cg.centerPrintCharWidth = charWidth;

	// count the number of lines for centering
	cg.centerPrintLines = 1;
	s = cg.centerPrint;
	while ( *s ) {
		if ( *s == '\n' ) {
			cg.centerPrintLines++;
		}
		s++;
	}
}
// -NERVE - SMF
#endif RTCW_XX

/*
===================
CG_DrawCenterString
===================
*/
static void CG_DrawCenterString( void ) {
	char    *start;
	int l;
	int x, y, w;
	float   *color;

	if ( !cg.centerPrintTime ) {
		return;
	}

	color = CG_FadeColor( cg.centerPrintTime, 1000 * cg_centertime.value );
	if ( !color ) {

#if defined RTCW_MP
		cg.centerPrintTime = 0;
		cg.centerPrintPriority = 0;
#endif RTCW_XX

		return;
	}

	trap_R_SetColor( color );

	start = cg.centerPrint;

	y = cg.centerPrintY - cg.centerPrintLines * BIGCHAR_HEIGHT / 2;

	while ( 1 ) {
		char linebuffer[1024];

#if defined RTCW_SP
		for ( l = 0; l < 40; l++ ) {
			if ( !start[l] || start[l] == '\n' || !Q_strncmp( &start[l], "\\n", 1 ) ) {
#elif defined RTCW_MP
		for ( l = 0; l < CP_LINEWIDTH; l++ ) {          // NERVE - SMF - added CP_LINEWIDTH
			if ( !start[l] || start[l] == '\n' ) {
#endif RTCW_XX

				break;
			}
			linebuffer[l] = start[l];
		}
		linebuffer[l] = 0;

		w = cg.centerPrintCharWidth * CG_DrawStrlen( linebuffer );

		x = ( SCREEN_WIDTH - w ) / 2;

#if defined RTCW_SP
		CG_DrawStringExt( x, y, linebuffer, color, qfalse, qtrue, cg.centerPrintCharWidth, (int)( cg.centerPrintCharWidth * 1.5 ), 0 );
//		CG_Text_Paint(x, y, 2, 0.3f, color, linebuffer, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE);

//		y += cg.centerPrintCharWidth * 1.5;
		y += cg.centerPrintCharWidth * 2;

//		while ( *start && ( *start != '\n' ) && !Q_strncmp(start, "\\n", 1) ) {
		while ( *start && ( *start != '\n' ) ) {
			if ( !Q_strncmp( start, "\\n", 1 ) ) {
				start++;
				break;
			}
#elif defined RTCW_MP
		CG_DrawStringExt( x, y, linebuffer, color, qfalse, qtrue,
						  cg.centerPrintCharWidth, (int)( cg.centerPrintCharWidth * 1.5 ), 0 );

		y += cg.centerPrintCharWidth * 1.5;

		while ( *start && ( *start != '\n' ) ) {
#endif RTCW_XX

			start++;
		}
		if ( !*start ) {
			break;
		}
		start++;
	}

	trap_R_SetColor( NULL );
}



/*
================================================================================

CROSSHAIRS

================================================================================
*/

/*
==============
CG_DrawWeapReticle
==============
*/
static void CG_DrawWeapReticle( void ) {

#if defined RTCW_SP
	int weap;
	vec4_t color = {0, 0, 0, 1};
	vec4_t snoopercolor = {0.7, .8, 0.7, 0};    // greenish
	float snooperBrightness;
	float x = 80, y, w = 240, h = 240;

	CG_AdjustFrom640( &x, &y, &w, &h );

	weap = cg.weaponSelect;

	// DHM - Nerve :: So that we will draw reticle
	if ( cgs.gametype == GT_WOLF && cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		weap = cg.snap->ps.weapon;
	}


	if ( weap == WP_SNIPERRIFLE ) {


		// sides
		CG_FillRect( 0, 0, 80, 480, color );
		CG_FillRect( 560, 0, 80, 480, color );

		// center
		if ( cgs.media.reticleShaderSimpleQ ) {
			trap_R_DrawStretchPic( x, 0, w, h, 0, 0, 1, 1, cgs.media.reticleShaderSimpleQ );    // tl
			trap_R_DrawStretchPic( x + w, 0, w, h, 1, 0, 0, 1, cgs.media.reticleShaderSimpleQ );  // tr
			trap_R_DrawStretchPic( x, h, w, h, 0, 1, 1, 0, cgs.media.reticleShaderSimpleQ );    // bl
			trap_R_DrawStretchPic( x + w, h, w, h, 1, 1, 0, 0, cgs.media.reticleShaderSimpleQ );  // br
		}

		// hairs
		CG_FillRect( 84, 239, 177, 2, color );   // left
		CG_FillRect( 320, 242, 1, 58, color );   // center top
		CG_FillRect( 319, 300, 2, 178, color );  // center bot
		CG_FillRect( 380, 239, 177, 2, color );  // right
	} else if ( weap == WP_SNOOPERSCOPE ) {
		// sides
		CG_FillRect( 0, 0, 80, 480, color );
		CG_FillRect( 560, 0, 80, 480, color );

		// center

//----(SA)	added
		// DM didn't like how bright it gets
		snooperBrightness = Com_Clamp( 0.0f, 1.0f, cg_reticleBrightness.value );
		snoopercolor[0] *= snooperBrightness;
		snoopercolor[1] *= snooperBrightness;
		snoopercolor[2] *= snooperBrightness;
		trap_R_SetColor( snoopercolor );
//----(SA)	end

		if ( cgs.media.snooperShaderSimple ) {
			CG_DrawPic( 80, 0, 480, 480, cgs.media.snooperShaderSimple );
		}

		// hairs

		CG_FillRect( 310, 120, 20, 1, color );   //					-----
		CG_FillRect( 300, 160, 40, 1, color );   //				-------------
		CG_FillRect( 310, 200, 20, 1, color );   //					-----

		CG_FillRect( 140, 239, 360, 1, color );  // horiz ---------------------------

		CG_FillRect( 310, 280, 20, 1, color );   //					-----
		CG_FillRect( 300, 320, 40, 1, color );   //				-------------
		CG_FillRect( 310, 360, 20, 1, color );   //					-----



		CG_FillRect( 400, 220, 1, 40, color );   // l

		CG_FillRect( 319, 60, 1, 360, color );   // vert

		CG_FillRect( 240, 220, 1, 40, color );   // r
	} else if ( weap == WP_FG42SCOPE ) {
		// sides
		CG_FillRect( 0, 0, 80, 480, color );
		CG_FillRect( 560, 0, 80, 480, color );

		// center
		if ( cgs.media.reticleShaderSimpleQ ) {
			trap_R_DrawStretchPic( x,   0, w, h, 0, 0, 1, 1, cgs.media.reticleShaderSimpleQ );  // tl
			trap_R_DrawStretchPic( x + w, 0, w, h, 1, 0, 0, 1, cgs.media.reticleShaderSimpleQ );  // tr
			trap_R_DrawStretchPic( x,   h, w, h, 0, 1, 1, 0, cgs.media.reticleShaderSimpleQ );  // bl
			trap_R_DrawStretchPic( x + w, h, w, h, 1, 1, 0, 0, cgs.media.reticleShaderSimpleQ );  // br
		}

		// hairs
		CG_FillRect( 84, 239, 150, 3, color );   // left
		CG_FillRect( 234, 240, 173, 1, color );  // horiz center
		CG_FillRect( 407, 239, 150, 3, color );  // right


		CG_FillRect( 319, 2,   3, 151, color );  // top center top
		CG_FillRect( 320, 153, 1, 114, color );  // top center bot

		CG_FillRect( 320, 241, 1, 87, color );   // bot center top
		CG_FillRect( 319, 327, 3, 151, color );  // bot center bot
	}
#elif defined RTCW_MP
	qboolean snooper, sniper, fg;
	vec4_t color = {0, 0, 0, 1};

	// DHM - Nerve :: So that we will draw reticle
	if ( cgs.gametype >= GT_WOLF && ( ( cg.snap->ps.pm_flags & PMF_FOLLOW ) || cg.demoPlayback ) ) {
		sniper = (qboolean)( cg.snap->ps.weapon == WP_SNIPERRIFLE );
		snooper = (qboolean)( cg.snap->ps.weapon == WP_SNOOPERSCOPE );
		fg = (qboolean)( cg.snap->ps.weapon == WP_FG42SCOPE );
	} else {
		sniper = (qboolean)( cg.weaponSelect == WP_SNIPERRIFLE );
		snooper = (qboolean)( cg.weaponSelect == WP_SNOOPERSCOPE );
		fg = (qboolean)( cg.weaponSelect == WP_FG42SCOPE );
	}

	if ( sniper ) {
		if ( cg_reticles.integer ) {

			// sides
			CG_FillRect( 0, 0, 80, 480, color );
			CG_FillRect( 560, 0, 80, 480, color );

			// center
			if ( cgs.media.reticleShaderSimple ) {
				CG_DrawPic( 80, 0, 480, 480, cgs.media.reticleShaderSimple );
			}

			// hairs
			CG_FillRect( 84, 239, 177, 2, color );   // left
			CG_FillRect( 320, 242, 1, 58, color );   // center top
			CG_FillRect( 319, 300, 2, 178, color );  // center bot
			CG_FillRect( 380, 239, 177, 2, color );  // right
		}
	} else if ( snooper )     {
		if ( cg_reticles.integer ) {

			// sides
			CG_FillRect( 0, 0, 80, 480, color );
			CG_FillRect( 560, 0, 80, 480, color );

			// center

//----(SA)	added
			// DM didn't like how bright it gets
			{
				vec4_t hcolor = {0.7, .8, 0.7, 0}; // greenish
				float brt;

				brt = cg_reticleBrightness.value;
				Com_Clamp( 0.0f, 1.0f, brt );
				hcolor[0] *= brt;
				hcolor[1] *= brt;
				hcolor[2] *= brt;
				trap_R_SetColor( hcolor );
			}
//----(SA)	end

			if ( cgs.media.snooperShaderSimple ) {
				CG_DrawPic( 80, 0, 480, 480, cgs.media.snooperShaderSimple );
			}

			// hairs

			CG_FillRect( 310, 120, 20, 1, color );   //					-----
			CG_FillRect( 300, 160, 40, 1, color );   //				-------------
			CG_FillRect( 310, 200, 20, 1, color );   //					-----

			CG_FillRect( 140, 239, 360, 2, color );  // horiz ---------------------------

			CG_FillRect( 310, 280, 20, 1, color );   //					-----
			CG_FillRect( 300, 320, 40, 1, color );   //				-------------
			CG_FillRect( 310, 360, 20, 1, color );   //					-----



			CG_FillRect( 400, 220, 1, 40, color );   // l

			CG_FillRect( 319, 60, 2, 360, color );   // vert

			CG_FillRect( 240, 220, 1, 40, color );   // r
		}
	}
#endif RTCW_XX

}

#if defined RTCW_SP
//----(SA)	removed (9/8/2001)
#endif RTCW_XX

/*
==============
CG_DrawBinocReticle
==============
*/
static void CG_DrawBinocReticle( void ) {

#if defined RTCW_SP
	// an alternative.  This gives nice sharp lines at the expense of a few extra polys
	vec4_t color = {0, 0, 0, 1};
	float x, y, w = 320, h = 240;

	if ( cgs.media.binocShaderSimpleQ ) {
		CG_AdjustFrom640( &x, &y, &w, &h );
		trap_R_DrawStretchPic( 0, 0, w, h, 0, 0, 1, 1, cgs.media.binocShaderSimpleQ );  // tl
		trap_R_DrawStretchPic( w, 0, w, h, 1, 0, 0, 1, cgs.media.binocShaderSimpleQ );  // tr
		trap_R_DrawStretchPic( 0, h, w, h, 0, 1, 1, 0, cgs.media.binocShaderSimpleQ );  // bl
		trap_R_DrawStretchPic( w, h, w, h, 1, 1, 0, 0, cgs.media.binocShaderSimpleQ );  // br
	}

	CG_FillRect( 146, 239, 348, 1, color );

	CG_FillRect( 188, 234, 1, 13, color );   // ll
	CG_FillRect( 234, 226, 1, 29, color );   // l
	CG_FillRect( 274, 234, 1, 13, color );   // lr
	CG_FillRect( 320, 213, 1, 55, color );   // center
	CG_FillRect( 360, 234, 1, 13, color );   // rl
	CG_FillRect( 406, 226, 1, 29, color );   // r
	CG_FillRect( 452, 234, 1, 13, color );   // rr
#elif defined RTCW_MP
	if ( cg_reticles.integer ) {
		if ( cg_reticleType.integer == 0 ) {
			if ( cgs.media.binocShader ) {
				CG_DrawPic( 0, 0, 640, 480, cgs.media.binocShader );
			}
		} else if ( cg_reticleType.integer == 1 ) {
			// an alternative.  This gives nice sharp lines at the expense of a few extra polys
			vec4_t color;
			color[0] = color[1] = color[2] = 0;
			color[3] = 1;

			if ( cgs.media.binocShaderSimple ) {
				CG_DrawPic( 0, 0, 640, 480, cgs.media.binocShaderSimple );
			}

			CG_FillRect( 146, 239, 348, 1, color );

			CG_FillRect( 188, 234, 1, 13, color );   // ll
			CG_FillRect( 234, 226, 1, 29, color );   // l
			CG_FillRect( 274, 234, 1, 13, color );   // lr
			CG_FillRect( 320, 213, 1, 55, color );   // center
			CG_FillRect( 360, 234, 1, 13, color );   // rl
			CG_FillRect( 406, 226, 1, 29, color );   // r
			CG_FillRect( 452, 234, 1, 13, color );   // rr
		}
	}
#endif RTCW_XX

}

void CG_FinishWeaponChange( int lastweap, int newweap ); // JPW NERVE


/*
=================
CG_DrawCrosshair
=================
*/
static void CG_DrawCrosshair( void ) {
	float w, h;
	qhandle_t hShader;
	float f;
	float x, y;
	int weapnum;                // DHM - Nerve

#if defined RTCW_SP
	vec4_t hcolor = {1, 1, 1, 0};
	qboolean friendInSights = qfalse;
#endif RTCW_XX

	if ( cg.renderingThirdPerson ) {
		return;
	}

#if defined RTCW_SP
	if ( cg_crosshairHealth.integer ) {
		CG_ColorForHealth( hcolor );
	}

	hcolor[3] = cg_crosshairAlpha.value;    //----(SA)	added


	// on mg42
	if ( cg.snap->ps.eFlags & EF_MG42_ACTIVE ) {
		hcolor[0] = hcolor[1] = hcolor[2] = 0.0f;
		hcolor[3] = 0.6f;
		// option 1
//		CG_FillRect (300, 240, 40, 2, hcolor);	// horizontal
//		CG_FillRect (319, 242, 2, 16, hcolor);	// vertical

		// option 2
		CG_FillRect( 305, 240, 30, 2, hcolor );  // horizontal
		CG_FillRect( 314, 256, 12, 2, hcolor );  // horizontal2
		CG_FillRect( 319, 242, 2, 32, hcolor );  // vertical

		return;
	}

	friendInSights = (qboolean)( cg.snap->ps.serverCursorHint == HINT_PLYR_FRIEND );  //----(SA)	added

	// DHM - Nerve :: show reticle in limbo and spectator
	if ( cgs.gametype == GT_WOLF && cg.snap->ps.pm_flags & PMF_FOLLOW ) {
#elif defined RTCW_MP
	// DHM - Nerve :: show reticle in limbo and spectator
	if ( cgs.gametype >= GT_WOLF && ( ( cg.snap->ps.pm_flags & PMF_FOLLOW ) || cg.demoPlayback ) ) {
#endif RTCW_XX

		weapnum = cg.snap->ps.weapon;
	} else {
		weapnum = cg.weaponSelect;
	}

	switch ( weapnum ) {

		// weapons that get no reticle
	case WP_NONE:       // no weapon, no crosshair

#if defined RTCW_SP
	case WP_GARAND:
		if ( cg.zoomedBinoc ) {
			CG_DrawBinocReticle();
		}
		return;
		break;

		// special reticle for weapon
	case WP_KNIFE:
#endif RTCW_XX

		if ( cg.zoomedBinoc ) {
			CG_DrawBinocReticle();

#if defined RTCW_SP
			return;
		}

		// no crosshair when looking at exits
		if ( cg.snap->ps.serverCursorHint >= HINT_EXIT && cg.snap->ps.serverCursorHint <= HINT_NOEXIT_FAR ) {
			return;
		}

		if ( !friendInSights ) {
			if ( !cg.snap->ps.leanf ) {     // no crosshair while leaning
				CG_FillRect( 319, 239, 2, 2, hcolor );      // dot
			}
#elif defined RTCW_MP
		}

		if ( cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR ) {
#endif RTCW_XX

			return;
		}
		break;

#if defined RTCW_MP
		// special reticle for weapon
#endif RTCW_XX

	case WP_SNIPERRIFLE:

#if defined RTCW_SP
	case WP_SNOOPERSCOPE:
	case WP_FG42SCOPE:

// JPW NERVE -- don't let players run with rifles -- speed 80 == crouch, 128 == walk, 256 == run
		if ( cg_gameType.integer != GT_SINGLE_PLAYER ) {
			if ( VectorLength( cg.snap->ps.velocity ) > 127.0f ) {
				if ( cg.snap->ps.weapon == WP_SNIPERRIFLE ) {
					CG_FinishWeaponChange( WP_SNIPERRIFLE, WP_MAUSER );
				}
				if ( cg.snap->ps.weapon == WP_SNOOPERSCOPE ) {
					CG_FinishWeaponChange( WP_SNOOPERSCOPE, WP_GARAND );
				}
			}
		}
// jpw

		CG_DrawWeapReticle();
		return;
#elif defined RTCW_MP
		if ( !( cg.snap->ps.eFlags & EF_MG42_ACTIVE ) ) {

			// JPW NERVE -- don't let players run with rifles -- speed 80 == crouch, 128 == walk, 256 == run
			if ( cg_gameType.integer != GT_SINGLE_PLAYER ) {
				if ( VectorLength( cg.snap->ps.velocity ) > 127.0f ) {
					if ( cg.snap->ps.weapon == WP_SNIPERRIFLE ) {
						CG_FinishWeaponChange( WP_SNIPERRIFLE, WP_MAUSER );
					}
					if ( cg.snap->ps.weapon == WP_SNOOPERSCOPE ) {
						CG_FinishWeaponChange( WP_SNOOPERSCOPE, WP_GARAND );
					}
				}
			}
			// jpw

			CG_DrawWeapReticle();
			return;
		}
		break;
#endif RTCW_XX

	default:
		break;
	}

	// using binoculars
	if ( cg.zoomedBinoc ) {
		CG_DrawBinocReticle();
		return;
	}

#if defined RTCW_SP
	// mauser only gets crosshair if you don't have the scope (I don't like this, but it's a test)
	if ( cg.weaponSelect == WP_MAUSER ) {
		if ( COM_BitCheck( cg.predictedPlayerState.weapons, WP_SNIPERRIFLE ) ) {
			return;
		}
	}


	if ( !cg_drawCrosshair.integer ) {  //----(SA)	moved down so it doesn't keep the scoped weaps from drawing reticles
		return;
	}

	// no crosshair while leaning
	if ( cg.snap->ps.leanf ) {
		return;
	}

	// no crosshair when looking at exits
	if ( cg.snap->ps.serverCursorHint >= HINT_EXIT && cg.snap->ps.serverCursorHint <= HINT_NOEXIT_FAR ) {
#elif defined RTCW_MP
	if ( cg_drawCrosshair.integer < 0 ) { //----(SA)	moved down so it doesn't keep the scoped weaps from drawing reticles
#endif RTCW_XX

		return;
	}

#if defined RTCW_SP
	if ( cg_paused.integer ) {
		// no draw if any menu's are up	 (or otherwise paused)
#elif defined RTCW_MP
	if ( cg.snap->ps.leanf ) { // no crosshair while leaning
#endif RTCW_XX

		return;
	}

	// set color based on health
	if ( cg_crosshairHealth.integer ) {

#if defined RTCW_MP
		vec4_t hcolor;

		CG_ColorForHealth( hcolor );
#endif RTCW_XX

		trap_R_SetColor( hcolor );
	} else {
		trap_R_SetColor( NULL );
	}

	w = h = cg_crosshairSize.value;

#if defined RTCW_SP
/*
	// pulse the size of the crosshair when picking up items
	f = cg.time - cg.itemPickupBlendTime;
	if ( f > 0 && f < ITEM_BLOB_TIME ) {
		f /= ITEM_BLOB_TIME;
		w *= ( 1 + f );
		h *= ( 1 + f );
	}
*/
#endif RTCW_XX

	// RF, crosshair size represents aim spread
	f = (float)cg.snap->ps.aimSpreadScale / 255.0;
	w *= ( 1 + f * 2.0 );
	h *= ( 1 + f * 2.0 );

	x = cg_crosshairX.integer;
	y = cg_crosshairY.integer;
	CG_AdjustFrom640( &x, &y, &w, &h );

#if defined RTCW_SP
//----(SA)	modified
	if ( friendInSights ) {
		hShader = cgs.media.crosshairFriendly;
	} else {
		hShader = cgs.media.crosshairShader[ cg_drawCrosshair.integer % NUM_CROSSHAIRS ];
	}
//----(SA)	end

	// NERVE - SMF - modified, fixes crosshair offset in shifted/scaled 3d views
	// (SA) also breaks scaled view...
	trap_R_DrawStretchPic(  x + cg.refdef.x + 0.5 * ( cg.refdef.width - w ),
							y + cg.refdef.y + 0.5 * ( cg.refdef.height - h ),
							w, h, 0, 0, 1, 1, hShader );
#elif defined RTCW_MP
	hShader = cgs.media.crosshairShader[ cg_drawCrosshair.integer % NUM_CROSSHAIRS ];

	// NERVE - SMF - modified, fixes crosshair offset in shifted/scaled 3d views
	if ( cg.limboMenu ) { // JPW NERVE
		trap_R_DrawStretchPic( x /*+ cg.refdef.x*/ + 0.5 * ( cg.refdef.width - w ),
							   y /*+ cg.refdef.y*/ + 0.5 * ( cg.refdef.height - h ),
							   w, h, 0, 0, 1, 1, hShader );
	} else {
		trap_R_DrawStretchPic( x + 0.5 * ( cgs.glconfig.vidWidth - w ), // JPW NERVE for scaled-down main windows
							   y + 0.5 * ( cgs.glconfig.vidHeight - h ),
							   w, h, 0, 0, 1, 1, hShader );
	}
	// NERVE - SMF
	if ( cg.crosshairShaderAlt[ cg_drawCrosshair.integer % NUM_CROSSHAIRS ] ) {
		w = h = cg_crosshairSize.value;
		x = cg_crosshairX.integer;
		y = cg_crosshairY.integer;
		CG_AdjustFrom640( &x, &y, &w, &h );

		if ( cg.limboMenu ) { // JPW NERVE
			trap_R_DrawStretchPic( x + 0.5 * ( cg.refdef.width - w ), y + 0.5 * ( cg.refdef.height - h ),
								   w, h, 0, 0, 1, 1, cg.crosshairShaderAlt[ cg_drawCrosshair.integer % NUM_CROSSHAIRS ] );
		} else {
			trap_R_DrawStretchPic( x + 0.5 * ( cgs.glconfig.vidWidth - w ), y + 0.5 * ( cgs.glconfig.vidHeight - h ), // JPW NERVE fix for small main windows (dunno why people still do this, but it's supported)
								   w, h, 0, 0, 1, 1, cg.crosshairShaderAlt[ cg_drawCrosshair.integer % NUM_CROSSHAIRS ] );
		}
	}
	// -NERVE - SMF
#endif RTCW_XX

}



/*
=================
CG_ScanForCrosshairEntity
=================
*/
static void CG_ScanForCrosshairEntity( void ) {
	trace_t trace;
//	gentity_t	*traceEnt;
	vec3_t start, end;
	int content;

	// DHM - Nerve :: We want this in multiplayer
	if ( cgs.gametype == GT_SINGLE_PLAYER ) {
		return; //----(SA)	don't use any scanning at the moment.

	}
	VectorCopy( cg.refdef.vieworg, start );

#if defined RTCW_SP
	VectorMA( start, 4096, cg.refdef.viewaxis[0], end );    //----(SA)	changed from 8192
#elif defined RTCW_MP
	VectorMA( start, 8192, cg.refdef.viewaxis[0], end );    //----(SA)	changed from 8192
#endif RTCW_XX

	CG_Trace( &trace, start, vec3_origin, vec3_origin, end,
			  cg.snap->ps.clientNum, CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_ITEM );

#if defined RTCW_SP
//----(SA)	allow targets that aren't clients
//	if ( trace.entityNum >= MAX_CLIENTS ) {
//		return;
//	}
#elif defined RTCW_MP
	if ( trace.entityNum >= MAX_CLIENTS ) {
		return;
	}
#endif RTCW_XX

//	traceEnt = &g_entities[trace.entityNum];


	// if the player is in fog, don't show it
	content = trap_CM_PointContents( trace.endpos, 0 );
	if ( content & CONTENTS_FOG ) {
		return;
	}

	// if the player is invisible, don't show it
	if ( cg_entities[ trace.entityNum ].currentState.powerups & ( 1 << PW_INVIS ) ) {
		return;
	}

	// update the fade timer
	cg.crosshairClientNum = trace.entityNum;
	cg.crosshairClientTime = cg.time;

#if defined RTCW_MP
	if ( cg.crosshairClientNum != cg.identifyClientNum && cg.crosshairClientNum != ENTITYNUM_WORLD ) {
		cg.identifyClientRequest = cg.crosshairClientNum;
	}
#endif RTCW_XX

}



/*
==============
CG_DrawDynamiteStatus
==============
*/
static void CG_DrawDynamiteStatus( void ) {
	float color[4];
	char        *name;
	int timeleft;
	float w;

#if defined RTCW_SP
	if ( cg.snap->ps.weapon != WP_DYNAMITE ) {
#elif defined RTCW_MP
	if ( cg.snap->ps.weapon != WP_DYNAMITE && cg.snap->ps.weapon != WP_DYNAMITE2 ) {
#endif RTCW_XX

		return;
	}

	if ( cg.snap->ps.grenadeTimeLeft <= 0 ) {
		return;
	}

	timeleft = cg.snap->ps.grenadeTimeLeft;

//	color = g_color_table[ColorIndex(COLOR_RED)];
	color[0] = color[3] = 1.0f;

	// fade red as it pulses past seconds
	color[1] = color[2] = 1.0f - ( (float)( timeleft % 1000 ) * 0.001f );

	if ( timeleft < 300 ) {        // fade up the text
		color[3] = (float)timeleft / 300.0f;
	}

	trap_R_SetColor( color );

	timeleft *= 5;
	timeleft -= ( timeleft % 5000 );
	timeleft += 5000;
	timeleft /= 1000;

#if defined RTCW_SP
	name = va( "Timer: %d", timeleft );
#elif defined RTCW_MP
//	name = va("Timer: %d", timeleft);
	name = va( "Timer: 30" );
#endif RTCW_XX

	w = CG_DrawStrlen( name ) * BIGCHAR_WIDTH;

	color[3] *= cg_hudAlpha.value;
	CG_DrawBigStringColor( 320 - w / 2, 170, name, color );

	trap_R_SetColor( NULL );
}


#if defined RTCW_MP
#define CH_KNIFE_DIST       48  // from g_weapon.c
#define CH_LADDER_DIST      100
#define CH_WATER_DIST       100
#define CH_BREAKABLE_DIST   64
#define CH_DOOR_DIST        96

#define CH_DIST             100 //128		// use the largest value from above
#endif RTCW_XX

#if defined RTCW_SP
/*
==============
CG_CheckForCursorHints
==============
*/
#elif defined RTCW_MP
/*
==============
CG_CheckForCursorHints
	concept in progress...
==============
*/
#endif RTCW_XX

void CG_CheckForCursorHints( void ) {

#if defined RTCW_SP
	if ( cg.renderingThirdPerson ) {
		return;
	}

	if ( cg.snap->ps.serverCursorHint != HINT_NONE ) { // let the client remember what was last looked at (for fading out)
		cg.cursorHintTime = cg.time;
		cg.cursorHintFade = cg_hintFadeTime.integer;    // fade out time
		cg.cursorHintIcon = cg.snap->ps.serverCursorHint;
		cg.cursorHintValue = cg.snap->ps.serverCursorHintVal;
	}

	// (SA) (8/14/01) removed all the client-side stuff.  don't think it's really necessary anymore
#elif defined RTCW_MP
	trace_t trace;
	vec3_t start, end;
	centity_t   *tracent;
	vec3_t pforward, eforward;
	float dist;


	if ( cg.renderingThirdPerson ) {
		return;
	}

	if ( cg.snap->ps.serverCursorHint ) {  // server is dictating a cursor hint, use it.
		cg.cursorHintTime = cg.time;
		cg.cursorHintFade = 500;    // fade out time
		cg.cursorHintIcon = cg.snap->ps.serverCursorHint;
		cg.cursorHintValue = cg.snap->ps.serverCursorHintVal;
		return;
	}


	// From here on it's client-side cursor hints.  So if the server isn't sending that info (as an option)
	// then it falls into here and you can get basic cursorhint info if you want, but not the detailed info
	// the server sends.

	// the trace
	VectorCopy( cg.refdef.vieworg, start );
	VectorMA( start, CH_DIST, cg.refdef.viewaxis[0], end );

//	CG_Trace( &trace, start, vec3_origin, vec3_origin, end, cg.snap->ps.clientNum, MASK_ALL &~CONTENTS_MONSTERCLIP);
	CG_Trace( &trace, start, vec3_origin, vec3_origin, end, cg.snap->ps.clientNum, MASK_PLAYERSOLID );

	if ( trace.fraction == 1 ) {
		return;
	}

	dist = trace.fraction * CH_DIST;

	tracent = &cg_entities[ trace.entityNum ];

	//
	// world
	//
	if ( trace.entityNum == ENTITYNUM_WORLD ) {
		// if looking into water, warn if you don't have a breather
		if ( ( trace.contents & CONTENTS_WATER ) && !( cg.snap->ps.powerups[PW_BREATHER] ) ) {
			if ( dist <= CH_WATER_DIST ) {
				cg.cursorHintIcon = HINT_WATER;
				cg.cursorHintTime = cg.time;
				cg.cursorHintFade = 500;
			}
		}
		// ladder
		else if ( ( trace.surfaceFlags & SURF_LADDER ) && !( cg.snap->ps.pm_flags & PMF_LADDER ) ) {
			if ( dist <= CH_LADDER_DIST ) {
				cg.cursorHintIcon = HINT_LADDER;
				cg.cursorHintTime = cg.time;
				cg.cursorHintFade = 500;
			}
		}


	}
	//
	// people
	//
	else if ( trace.entityNum < MAX_CLIENTS ) {

		// knife
		if ( trace.entityNum < MAX_CLIENTS && ( cg.snap->ps.weapon == WP_KNIFE || cg.snap->ps.weapon == WP_KNIFE2 ) ) {
			if ( dist <= CH_KNIFE_DIST ) {

				AngleVectors( cg.snap->ps.viewangles,   pforward, NULL, NULL );
				AngleVectors( tracent->lerpAngles,      eforward, NULL, NULL );

				if ( DotProduct( eforward, pforward ) > 0.9f ) {   // from behind
					cg.cursorHintIcon = HINT_KNIFE;
					cg.cursorHintTime = cg.time;
					cg.cursorHintFade = 100;
				}
			}
		}
	}
	//
	// other entities
	//
	else {
		if ( tracent->currentState.dmgFlags ) {
			switch ( tracent->currentState.dmgFlags ) {
			case HINT_DOOR:
				if ( dist < CH_DOOR_DIST ) {
					cg.cursorHintIcon = HINT_DOOR;
					cg.cursorHintTime = cg.time;
					cg.cursorHintFade = 500;
				}
				break;
				//case HINT_CHAIR:
			case HINT_MG42:
				if ( dist < CH_DOOR_DIST && !( cg.snap->ps.eFlags & EF_MG42_ACTIVE ) ) {
					cg.cursorHintIcon = HINT_ACTIVATE;
					cg.cursorHintTime = cg.time;
					cg.cursorHintFade = 500;
				}
				break;
			}
		} else {

			if ( tracent->currentState.eType == ET_EXPLOSIVE ) {
				if ( ( dist < CH_BREAKABLE_DIST ) && ( cg.cursorHintIcon != HINT_FORCENONE ) ) { // JPW NERVE so we don't get breakables in trigger_objective_info fields for wrong team (see code chunk in g_main.c)
					cg.cursorHintIcon = HINT_BREAKABLE;
					cg.cursorHintTime = cg.time;
					cg.cursorHintFade = 500;
				}

			}
		}
	}
#endif RTCW_XX

}


/*
=====================
CG_DrawCrosshairNames
=====================
*/
static void CG_DrawCrosshairNames( void ) {
	float       *color;

#if defined RTCW_SP
	vec4_t teamColor;           // NERVE - SMF
#endif RTCW_XX

	char        *name;
	float w;

#if defined RTCW_MP
	// NERVE - SMF
	const char  *s, *playerClass;
	int playerHealth, val;
	vec4_t c;
	float barFrac;
	// -NERVE - SMF
#endif RTCW_XX

#if defined RTCW_SP
	if ( !cg_drawCrosshair.integer ) {
#elif defined RTCW_MP
	if ( cg_drawCrosshair.integer < 0 ) {
#endif RTCW_XX

		return;
	}
	if ( !cg_drawCrosshairNames.integer ) {
		return;
	}
	if ( cg.renderingThirdPerson ) {
		return;
	}

#if defined RTCW_MP
	// NERVE - SMF - we don't want to do this in warmup
	if ( cgs.gamestate != GS_PLAYING && cgs.gametype == GT_WOLF_STOPWATCH ) {
		return;
	}
#endif RTCW_XX

	// Ridah
	if ( cg_gameType.integer == GT_SINGLE_PLAYER ) {
		return;
	}
	// done.

	// scan the known entities to see if the crosshair is sighted on one
	CG_ScanForCrosshairEntity();

	// draw the name of the player being looked at
	color = CG_FadeColor( cg.crosshairClientTime, 1000 );

	if ( !color ) {
		trap_R_SetColor( NULL );
		return;
	}

#if defined RTCW_SP
	// NERVE - SMF - use fade alpha but color text according to teams
	teamColor[3] = color[3];

	// NERVE - SMF - no longer identify opposing side, so just use green now
//	if ( cgs.clientinfo[ cg.crosshairClientNum ].team != cgs.clientinfo[ cg.clientNum ].team )
//		VectorSet( teamColor, 0.7608, 0.1250, 0.0859 );			// LIGHT-RED
//	else
	VectorSet( teamColor, 0.1250, 0.7608, 0.0859 );             // LIGHT-GREEN

	trap_R_SetColor( teamColor );
	// -NERVE - SMF

	name = cgs.clientinfo[ cg.crosshairClientNum ].name;
	w = CG_DrawStrlen( va( "Axis: %s", name ) ) * BIGCHAR_WIDTH;
//	CG_DrawBigString( 320 - w / 2, 170, name, color[3] * 0.5 );

	// NERVE - SMF
	if ( strlen( name ) ) {
		if ( ( cgs.clientinfo[ cg.crosshairClientNum ].team == TEAM_RED ) &&
			 ( cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_RED ) ) { // JPW NERVE -- only show same team info so people can't pan-search
			CG_DrawBigStringColor( 320 - w / 2, 170, va( "Axis: %s", name ), teamColor );
		} else if ( ( cgs.clientinfo[ cg.crosshairClientNum ].team == TEAM_BLUE ) &&
					( cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_BLUE ) ) { // JPW NERVE -- so's we can't find snipers for free
			CG_DrawBigStringColor( 320 - w / 2, 170, va( "Ally: %s", name ), teamColor );
		}
	}
	// -NERVE - SMF
#elif defined RTCW_MP
	// NERVE - SMF
	if ( cg.crosshairClientNum > MAX_CLIENTS ) {
		return;
	}

	// we only want to see players on our team
	if ( cgs.clientinfo[cg.snap->ps.clientNum].team != TEAM_SPECTATOR
		 && cgs.clientinfo[ cg.crosshairClientNum ].team != cgs.clientinfo[cg.snap->ps.clientNum].team ) {
		return;
	}

	// determine player class
	val = cg_entities[ cg.crosshairClientNum ].currentState.teamNum;
	if ( val == 0 ) {
		playerClass = "S";
	} else if ( val == 1 ) {
		playerClass = "M";
	} else if ( val == 2 ) {
		playerClass = "E";
	} else if ( val == 3 ) {
		playerClass = "L";
	} else {
		playerClass = "";
	}

	name = cgs.clientinfo[ cg.crosshairClientNum ].name;

	s = va( "[%s] %s", CG_TranslateString( playerClass ), name );
	if ( !s ) {
		return;
	}
	w = CG_DrawStrlen( s ) * SMALLCHAR_WIDTH;

	// draw the name and class
	CG_DrawSmallStringColor( 320 - w / 2, 170, s, color );

	// draw the health bar
	playerHealth = cg.identifyClientHealth;

	if ( cg.crosshairClientNum == cg.identifyClientNum ) {
		barFrac = (float)playerHealth / 100;

		if ( barFrac > 1.0 ) {
			barFrac = 1.0;
		} else if ( barFrac < 0 ) {
			barFrac = 0;
		}

		c[0] = 1.0f;
		c[1] = c[2] = barFrac;
		c[3] = 0.25 + barFrac * 0.5 * color[3];

		CG_FilledBar( 320 - w / 2, 190, 110, 10, c, NULL, NULL, barFrac, 16 );
	}
	// -NERVE - SMF
#endif RTCW_XX

	trap_R_SetColor( NULL );
}



//==============================================================================

/*
=================
CG_DrawSpectator
=================
*/
static void CG_DrawSpectator( void ) {

#if defined RTCW_SP
	CG_DrawBigString( 320 - 9 * 8, 440, "SPECTATOR", 1.0F );
#elif defined RTCW_MP
	CG_DrawBigString( 320 - 9 * 8, 440, CG_TranslateString( "SPECTATOR" ), 1.0F );
#endif RTCW_XX

	if ( cgs.gametype == GT_TOURNAMENT ) {
		CG_DrawBigString( 320 - 15 * 8, 460, "waiting to play", 1.0F );
	}
	if ( cgs.gametype == GT_TEAM || cgs.gametype == GT_CTF ) {
		CG_DrawBigString( 320 - 25 * 8, 460, "use the TEAM menu to play", 1.0F );
	}
}

/*
=================
CG_DrawVote
=================
*/
static void CG_DrawVote( void ) {
	char    *s;

#if defined RTCW_MP
	char str1[32], str2[32];
	float color[4] = { 1, 1, 0, 1 };
#endif RTCW_XX

	int sec;

#if defined RTCW_MP
	if ( cgs.complaintEndTime > cg.time ) {

		if ( cgs.complaintClient == -1 ) {
			s = "Your complaint has been filed";
			CG_DrawStringExt( 8, 200, CG_TranslateString( s ), color, qtrue, qfalse, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 80 );
			return;
		}
		if ( cgs.complaintClient == -2 ) {
			s = "Complaint dismissed";
			CG_DrawStringExt( 8, 200, CG_TranslateString( s ), color, qtrue, qfalse, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 80 );
			return;
		}
		if ( cgs.complaintClient == -3 ) {
			s = "Server Host cannot be complained against";
			CG_DrawStringExt( 8, 200, CG_TranslateString( s ), color, qtrue, qfalse, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 80 );
			return;
		}
		if ( cgs.complaintClient == -4 ) {
			s = "You were team-killed by the Server Host";
			CG_DrawStringExt( 8, 200, CG_TranslateString( s ), color, qtrue, qfalse, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 80 );
			return;
		}

		Q_strncpyz( str1, BindingFromName( "vote yes" ), 32 );
		if ( !Q_stricmp( str1, "???" ) ) {
			Q_strncpyz( str1, "vote yes", 32 );
		}
		Q_strncpyz( str2, BindingFromName( "vote no" ), 32 );
		if ( !Q_stricmp( str2, "???" ) ) {
			Q_strncpyz( str2, "vote no", 32 );
		}

		s = va( CG_TranslateString( "File complaint against %s for team-killing?" ), cgs.clientinfo[cgs.complaintClient].name );
		CG_DrawStringExt( 8, 200, s, color, qtrue, qfalse, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 80 );

		s = va( CG_TranslateString( "Press '%s' for YES, or '%s' for No" ), str1, str2 );
		CG_DrawStringExt( 8, 214, s, color, qtrue, qfalse, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 80 );
		return;
	}
#endif RTCW_XX

	if ( !cgs.voteTime ) {
		return;
	}

#if defined RTCW_MP
	Q_strncpyz( str1, BindingFromName( "vote yes" ), 32 );
	if ( !Q_stricmp( str1, "???" ) ) {
		Q_strncpyz( str1, "vote yes", 32 );
	}
	Q_strncpyz( str2, BindingFromName( "vote no" ), 32 );
	if ( !Q_stricmp( str2, "???" ) ) {
		Q_strncpyz( str2, "vote no", 32 );
	}
#endif RTCW_XX

	// play a talk beep whenever it is modified
	if ( cgs.voteModified ) {
		cgs.voteModified = qfalse;
		trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
	}

	sec = ( VOTE_TIME - ( cg.time - cgs.voteTime ) ) / 1000;
	if ( sec < 0 ) {
		sec = 0;
	}

#if defined RTCW_SP
	s = va( "VOTE(%i):%s yes(F1):%i no(F2):%i", sec, cgs.voteString, cgs.voteYes, cgs.voteNo );
	CG_DrawSmallString( 0, 58, s, 1.0F );
#elif defined RTCW_MP
	if ( !( cg.snap->ps.eFlags & EF_VOTED ) ) {
		s = va( CG_TranslateString( "VOTE(%i):%s" ), sec, cgs.voteString );
		CG_DrawStringExt( 8, 200, s, color, qtrue, qfalse, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 60 );

		s = va( CG_TranslateString( "YES(%s):%i, NO(%s):%i" ), str1, cgs.voteYes, str2, cgs.voteNo );
		CG_DrawStringExt( 8, 214, s, color, qtrue, qfalse, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 60 );
	} else {
		s = va( CG_TranslateString( "Y:%i, N:%i" ), cgs.voteYes, cgs.voteNo );
		CG_DrawStringExt( 8, 214, s, color, qtrue, qfalse, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 20 );
	}
#endif RTCW_XX

}

/*
=================
CG_DrawIntermission
=================
*/
static void CG_DrawIntermission( void ) {
	if ( cgs.gametype == GT_SINGLE_PLAYER ) {
		CG_DrawCenterString();
		return;
	}

	cg.scoreFadeTime = cg.time;
	CG_DrawScoreboard();
}


#if defined RTCW_SP
// NERVE - SMF
/*
=================
CG_ActivateLimboMenu
=================
*/
// TTimo: unused
/*
static void CG_ActivateLimboMenu( void ) {
	static qboolean latch = qfalse;
	qboolean test;
	char buf[32];

	if ( cgs.gametype != GT_WOLF )
		return;

	// should we open the limbo menu
	test = cg.snap->ps.pm_flags & PMF_LIMBO || cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR;

	if ( test && !latch ) {
		trap_SendConsoleCommand( "startLimboMode\n" );
		trap_SendConsoleCommand( "OpenLimboMenu\n" );
		latch = qtrue;
	}
	else if ( !test && latch ) {
		trap_SendConsoleCommand( "stopLimboMode\n" );
		trap_SendConsoleCommand( "CloseLimboMenu\n" );
		latch = qfalse;
	}

	// set the limbo state
	trap_Cvar_VariableStringBuffer( "ui_limboMode", buf, sizeof( buf ) );

	if ( atoi( buf ) )
		cg.limboMenu = qtrue;
	else
		cg.limboMenu = qfalse;
}
*/
// -NERVE - SMF
#elif defined RTCW_MP
/*
=================
CG_ActivateLimboMenu

NERVE - SMF
=================
*/
static void CG_ActivateLimboMenu( void ) {
	// ATVI Wolfenstein Misc #339
	// track when the UI would disable limbo, that leaves us in an inconsistent latch state
	// the inconsistent state is a good thing most of the time, except when game sends us back to free fly
	// that had the bad effect of triggering limbo again
	static qboolean ui_disable = qfalse;
	// track when we see cgame conditions change and need to emit a new limbo console command
	static qboolean latch = qfalse;
	qboolean test;
	char buf[32];

	if ( cgs.gametype < GT_WOLF ) {
		return;
	}

	// a test to detect when UI closes the limbo
	trap_Cvar_VariableStringBuffer( "ui_limboMode", buf, sizeof( buf ) );
	if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR && atoi( buf ) == 0 && latch == 1 ) {
		ui_disable = qtrue;
	}

	// should we open the limbo menu
	if ( cg.snap->ps.pm_type == PM_INTERMISSION ) {
		test = qfalse;
	} else {
		test = cg.snap->ps.pm_flags & PMF_LIMBO || cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR || cg.warmup;
	}

	// auto open/close limbo mode
	if ( cg_popupLimboMenu.integer ) {
		// we don't want to trigger limbo in this very particular case
		if ( ui_disable && cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR && test && !latch ) {
			// ATVI Wolfenstein Misc #413
			// we manually update this, otherwise there's a chance team selections won't work
			trap_Cvar_Set( "mp_currentTeam", "2" );
			latch = 1;
			ui_disable = qfalse;
		}
		if ( test && !latch ) {
			trap_SendConsoleCommand( "OpenLimboMenu\n" );
			latch = qtrue;
			ui_disable = qfalse;
		} else if ( !test && latch )   {
			trap_SendConsoleCommand( "CloseLimboMenu\n" );
			latch = qfalse;
		}
	}

	if ( atoi( buf ) ) {
		cg.limboMenu = qtrue;
	} else {
		cg.limboMenu = qfalse;
	}
}

/*
=================
CG_DrawSpectatorMessage
=================
*/
static void CG_DrawSpectatorMessage( void ) {
	float color[4] = { 1, 1, 1, 1 };
	const char *str, *str2;
	float x, y;
	char buf[32];

	if ( cgs.gametype < GT_WOLF ) {
		return;
	}

	if ( !cg_descriptiveText.integer ) {
		return;
	}

	if ( !( cg.snap->ps.pm_flags & PMF_LIMBO || cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR ) ) {
		return;
	}

	trap_Cvar_VariableStringBuffer( "ui_limboMode", buf, sizeof( buf ) );
	if ( atoi( buf ) ) {
		return;
	}

	Controls_GetConfig();

	x = 80;
	y = 408;

	str2 = BindingFromName( "OpenLimboMenu" );
	if ( !Q_stricmp( str2, "???" ) ) {
		str2 = "ESCAPE";
	}
	str = va( CG_TranslateString( "- Press %s to open Limbo Menu" ), str2 );
	CG_DrawStringExt( x, y, str, color, qtrue, 0, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0 );
	y += TINYCHAR_HEIGHT;

	str2 = BindingFromName( "mp_QuickMessage" );
	str = va( CG_TranslateString( "- Press %s to open quick message menu" ), str2 );
	CG_DrawStringExt( x, y, str, color, qtrue, 0, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0 );
	y += TINYCHAR_HEIGHT;

	str2 = BindingFromName( "+attack" );
	str = va( CG_TranslateString( "- Press %s to follow next player" ), str2 );
	CG_DrawStringExt( x, y, str, color, qtrue, 0, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0 );
	y += TINYCHAR_HEIGHT;
}

/*
=================
CG_DrawLimboMessage
=================
*/

#define INFOTEXT_STARTX 42

static void CG_DrawLimboMessage( void ) {
	float color[4] = { 1, 1, 1, 1 };
	const char *str;
	playerState_t *ps;
	//int w;

	if ( cgs.gametype < GT_WOLF ) {
		return;
	}

	ps = &cg.snap->ps;

	if ( ps->stats[STAT_HEALTH] > 0 ) {
		return;
	}

	if ( cg.snap->ps.pm_flags & PMF_LIMBO || cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR ) {
		return;
	}

	color[3] *= cg_hudAlpha.value;

	if ( cg_descriptiveText.integer ) {
		str = CG_TranslateString( "You are wounded and waiting for a medic." );
		CG_DrawSmallStringColor( INFOTEXT_STARTX, 68, str, color );

		str = CG_TranslateString( "Press JUMP to go into reinforcement queue." );
		CG_DrawSmallStringColor( INFOTEXT_STARTX, 86, str, color );
	}

	// JPW NERVE
	if ( cg.snap->ps.persistant[PERS_RESPAWNS_LEFT] == 0 ) {
		str = CG_TranslateString( "No more reinforcements this round." );
	} else if ( cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_RED ) {
		str = va( CG_TranslateString( "Reinforcements deploy in %d seconds." ),
				  (int)( 1 + (float)( cg_redlimbotime.integer - ( cg.time % cg_redlimbotime.integer ) ) * 0.001f ) );
	} else {
		str = va( CG_TranslateString( "Reinforcements deploy in %d seconds." ),
				  (int)( 1 + (float)( cg_bluelimbotime.integer - ( cg.time % cg_bluelimbotime.integer ) ) * 0.001f ) );
	}

	CG_DrawSmallStringColor( INFOTEXT_STARTX, 104, str, color );
	// jpw

	trap_R_SetColor( NULL );
}
#endif RTCW_XX

// -NERVE - SMF

/*
=================
CG_DrawFollow
=================
*/
static qboolean CG_DrawFollow( void ) {

#if defined RTCW_SP
	float x;
#elif defined RTCW_MP
	//float		x;
#endif RTCW_XX

	vec4_t color;
	const char  *name;
	char deploytime[128];        // JPW NERVE

	if ( !( cg.snap->ps.pm_flags & PMF_FOLLOW ) ) {
		return qfalse;
	}
	color[0] = 1;
	color[1] = 1;
	color[2] = 1;
	color[3] = 1;

#if defined RTCW_SP
// JPW NERVE -- if in limbo, show different follow message
	if ( cg.snap->ps.pm_flags & PMF_LIMBO ) {
//		CG_Printf("following %s\n",cgs.clientinfo[ cg.snap->ps.clientNum ].name);
		color[1] = 0;
		color[2] = 0;
		if ( cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_RED ) {
			sprintf( deploytime,"Deploying in %d seconds", (int)( (float)( cg_redlimbotime.integer - ( cg.time % cg_redlimbotime.integer ) ) * 0.001f ) );
		} else {
			sprintf( deploytime,"Deploying in %d seconds", (int)( (float)( cg_bluelimbotime.integer - ( cg.time % cg_bluelimbotime.integer ) ) * 0.001f ) );
		}

		x = 0.5 * ( 640 - BIGCHAR_WIDTH * strlen( deploytime ) ); //CG_DrawStrlen( deploytime ) );
		CG_DrawStringExt( x, 24, deploytime, color, qtrue, qtrue, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 0 );
		sprintf( deploytime,"(Following %s)",cgs.clientinfo[ cg.snap->ps.clientNum ].name );
		x = 0.5 * ( 640 - BIGCHAR_WIDTH * strlen( deploytime ) ); //CG_DrawStrlen( deploytime ) );
		CG_DrawStringExt( x, 48, deploytime, color, qtrue, qtrue, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 0 );

	} else {
// jpw
		CG_DrawBigString( 320 - 9 * 8, 24, "following", 1.0F );

		name = cgs.clientinfo[ cg.snap->ps.clientNum ].name;

		x = 0.5 * ( 640 - GIANT_WIDTH * CG_DrawStrlen( name ) );

		CG_DrawStringExt( x, 40, name, color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0 );
	} // JPW NERVE
	return qtrue;
#elif defined RTCW_MP
	// JPW NERVE -- if in limbo, show different follow message
	if ( cg.snap->ps.pm_flags & PMF_LIMBO ) {
		color[1] = 0.0;
		color[2] = 0.0;
		if ( cg.snap->ps.persistant[PERS_RESPAWNS_LEFT] == 0 ) {
			sprintf( deploytime, CG_TranslateString( "No more deployments this round" ) );
		} else if ( cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_RED ) {
			sprintf( deploytime, CG_TranslateString( "Deploying in %d seconds" ),
					 (int)( 1 + (float)( cg_redlimbotime.integer - ( cg.time % cg_redlimbotime.integer ) ) * 0.001f ) );
		} else {
			sprintf( deploytime, CG_TranslateString( "Deploying in %d seconds" ),
					 (int)( 1 + (float)( cg_bluelimbotime.integer - ( cg.time % cg_bluelimbotime.integer ) ) * 0.001f ) );
		}

		CG_DrawStringExt( INFOTEXT_STARTX, 68, deploytime, color, qtrue, qfalse, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 80 );

		// DHM - Nerve :: Don't display if you're following yourself
		if ( cg.snap->ps.clientNum != cg.clientNum ) {
			sprintf( deploytime,"(%s %s)", CG_TranslateString( "Following" ), cgs.clientinfo[ cg.snap->ps.clientNum ].name );
			CG_DrawStringExt( INFOTEXT_STARTX, 86, deploytime, color, qtrue, qfalse, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 80 );
		}
	} else {
		// jpw
		CG_DrawSmallString( INFOTEXT_STARTX, 68, CG_TranslateString( "following" ), 1.0F );

		name = cgs.clientinfo[ cg.snap->ps.clientNum ].name;

		CG_DrawStringExt( 120, 68, name, color, qtrue, qtrue, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 0 );
	} // JPW NERVE
	return qtrue;
#endif RTCW_XX

}


#if defined RTCW_SP
/*
=================
CG_DrawAmmoWarning
=================
*/
static void CG_DrawAmmoWarning( void ) {
	const char  *s;
	int w;

//----(SA)	forcing return for now
//			if we have messages to show here, comment back in
	return;


	if ( cg_drawAmmoWarning.integer == 0 ) {
		return;
	}

	if ( !cg.lowAmmoWarning ) {
		return;
	}

	if ( cg.lowAmmoWarning == 2 ) {
		s = "OUT OF AMMO";
	} else {
		s = "LOW AMMO WARNING";
	}
	w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
	CG_DrawBigString( 320 - w / 2, 64, s, 1.0F );
}
#endif RTCW_XX

/*
=================
CG_DrawWarmup
=================
*/
static void CG_DrawWarmup( void ) {
	int w;
	int sec;

#if defined RTCW_SP
	int i;
	clientInfo_t    *ci1, *ci2;
#endif RTCW_XX

	int cw;

#if defined RTCW_SP
	const char  *s;
#elif defined RTCW_MP
	const char  *s, *s1, *s2;
#endif RTCW_XX

	if ( cgs.gametype == GT_SINGLE_PLAYER ) {
		return;     // (SA) don't bother with this stuff in sp
	}

	sec = cg.warmup;
	if ( !sec ) {

#if defined RTCW_SP
		return;
	}

	if ( sec < 0 ) {
		s = "Waiting for players";
		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
		CG_DrawBigString( 320 - w / 2, 40, s, 1.0F );
		cg.warmupCount = 0;
		return;
	}


	// find the two active players
	ci1 = NULL;
	ci2 = NULL;
	for ( i = 0 ; i < cgs.maxclients ; i++ ) {
		if ( cgs.clientinfo[i].infoValid && cgs.clientinfo[i].team == TEAM_FREE ) {
			if ( !ci1 ) {
				ci1 = &cgs.clientinfo[i];
			} else {
				ci2 = &cgs.clientinfo[i];
			}
		}
	}

	if ( ci1 && ci2 ) {
		s = va( "%s vs %s", ci1->name, ci2->name );
		w = CG_DrawStrlen( s );
		if ( w > 640 / GIANT_WIDTH ) {
			cw = 640 / w;
		} else {
			cw = GIANT_WIDTH;
		}
		CG_DrawStringExt( 320 - w * cw / 2, 20,s, colorWhite,
						  qfalse, qtrue, cw, (int)( cw * 1.5 ), 0 );
	}


	sec = ( sec - cg.time ) / 1000;
	if ( sec < 0 ) {
		sec = 0;
	}
	s = va( "Starts in: %i", sec + 1 );
	if ( sec != cg.warmupCount ) {
		cg.warmupCount = sec;
		switch ( sec ) {
		case 0:
			trap_S_StartLocalSound( cgs.media.count1Sound, CHAN_ANNOUNCER );
			break;
		case 1:
			trap_S_StartLocalSound( cgs.media.count2Sound, CHAN_ANNOUNCER );
			break;
		case 2:
			trap_S_StartLocalSound( cgs.media.count3Sound, CHAN_ANNOUNCER );
			break;
		default:
			break;
		}
	}
	switch ( cg.warmupCount ) {
	case 0:
		cw = 28;
		break;
	case 1:
		cw = 24;
		break;
	case 2:
		cw = 20;
		break;
	default:
		cw = 16;
		break;
	}

	w = CG_DrawStrlen( s );
	CG_DrawStringExt( 320 - w * cw / 2, 70, s, colorWhite,
					  qfalse, qtrue, cw, (int)( cw * 1.5 ), 0 );
#elif defined RTCW_MP
		if ( cgs.gamestate == GS_WAITING_FOR_PLAYERS ) {
			cw = 10;

			s = CG_TranslateString( "Game Stopped - Waiting for more players" );

			w = CG_DrawStrlen( s );
			CG_DrawStringExt( 320 - w * 6, 120, s, colorWhite, qfalse, qtrue, 12, 18, 0 );


			s1 = va( CG_TranslateString( "Waiting for %i players" ), cgs.minclients );
			s2 = CG_TranslateString( "or call a vote to start match" );

			w = CG_DrawStrlen( s1 );
			CG_DrawStringExt( 320 - w * cw / 2, 160, s1, colorWhite,
							  qfalse, qtrue, cw, (int)( cw * 1.5 ), 0 );

			w = CG_DrawStrlen( s2 );
			CG_DrawStringExt( 320 - w * cw / 2, 180, s2, colorWhite,
							  qfalse, qtrue, cw, (int)( cw * 1.5 ), 0 );

			return;
		}

		return;
	}

	sec = ( sec - cg.time ) / 1000;
	if ( sec < 0 ) {
		sec = 0;
	}

	if ( cgs.gametype == GT_WOLF_STOPWATCH ) {
		s = va( "%s %i", CG_TranslateString( "(WARMUP) Match begins in:" ), sec + 1 );
	} else {
		s = va( "%s %i", CG_TranslateString( "(WARMUP) Match begins in:" ), sec + 1 );
	}

	w = CG_DrawStrlen( s );
	CG_DrawStringExt( 320 - w * 6, 120, s, colorWhite, qfalse, qtrue, 12, 18, 0 );

	// NERVE - SMF - stopwatch stuff
	s1 = "";
	s2 = "";

	if ( cgs.gametype == GT_WOLF_STOPWATCH ) {
		const char  *cs;
		int defender;

		s = va( "%s %i", CG_TranslateString( "Stopwatch Round" ), cgs.currentRound + 1 );

		cs = CG_ConfigString( CS_MULTI_INFO );
		defender = atoi( Info_ValueForKey( cs, "defender" ) );

		if ( !defender ) {
			if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED ) {
				if ( cgs.currentRound == 1 ) {
					s1 = "You have been switched to the Axis team";
					s2 = "Keep the Allies from beating the clock!";
				} else {
					s1 = "You are on the Axis team";
				}
			} else if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE )   {
				if ( cgs.currentRound == 1 ) {
					s1 = "You have been switched to the Allied team";
					s2 = "Try to beat the clock!";
				} else {
					s1 = "You are on the Allied team";
				}
			}
		} else {
			if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED ) {
				if ( cgs.currentRound == 1 ) {
					s1 = "You have been switched to the Axis team";
					s2 = "Try to beat the clock!";
				} else {
					s1 = "You are on the Axis team";
				}
			} else if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE )   {
				if ( cgs.currentRound == 1 ) {
					s1 = "You have been switched to the Allied team";
					s2 = "Keep the Axis from beating the clock!";
				} else {
					s1 = "You are on the Allied team";
				}
			}
		}

		if ( strlen( s1 ) ) {
			s1 = CG_TranslateString( s1 );
		}

		if ( strlen( s2 ) ) {
			s2 = CG_TranslateString( s2 );
		}

		cw = 10;

		w = CG_DrawStrlen( s );
		CG_DrawStringExt( 320 - w * cw / 2, 140, s, colorWhite,
						  qfalse, qtrue, cw, (int)( cw * 1.5 ), 0 );

		w = CG_DrawStrlen( s1 );
		CG_DrawStringExt( 320 - w * cw / 2, 160, s1, colorWhite,
						  qfalse, qtrue, cw, (int)( cw * 1.5 ), 0 );

		w = CG_DrawStrlen( s2 );
		CG_DrawStringExt( 320 - w * cw / 2, 180, s2, colorWhite,
						  qfalse, qtrue, cw, (int)( cw * 1.5 ), 0 );
	}
#endif RTCW_XX

}

//==================================================================================

/*
=================
CG_DrawFlashFade
=================
*/
static void CG_DrawFlashFade( void ) {
	static int lastTime;
	int elapsed, time;
	vec4_t col;

#if defined RTCW_SP
	if ( cgs.scrFadeStartTime + cgs.scrFadeDuration < cg.time ) {
		cgs.scrFadeAlphaCurrent = cgs.scrFadeAlpha;
	} else if ( cgs.scrFadeAlphaCurrent != cgs.scrFadeAlpha ) {
		elapsed = ( time = trap_Milliseconds() ) - lastTime;  // we need to use trap_Milliseconds() here since the cg.time gets modified upon reloading
		lastTime = time;
		if ( elapsed < 500 && elapsed > 0 ) {
			if ( cgs.scrFadeAlphaCurrent > cgs.scrFadeAlpha ) {
				cgs.scrFadeAlphaCurrent -= ( (float)elapsed / (float)cgs.scrFadeDuration );
				if ( cgs.scrFadeAlphaCurrent < cgs.scrFadeAlpha ) {
					cgs.scrFadeAlphaCurrent = cgs.scrFadeAlpha;
				}
			} else {
				cgs.scrFadeAlphaCurrent += ( (float)elapsed / (float)cgs.scrFadeDuration );
				if ( cgs.scrFadeAlphaCurrent > cgs.scrFadeAlpha ) {
					cgs.scrFadeAlphaCurrent = cgs.scrFadeAlpha;
				}
			}
		}
	}
	// now draw the fade
	if ( cgs.scrFadeAlphaCurrent > 0.0 ) {
//		CG_Printf("fade: %f\n", cgs.scrFadeAlphaCurrent);
		VectorClear( col );
		col[3] = cgs.scrFadeAlphaCurrent;
//		CG_FillRect( -10, -10, 650, 490, col );
		CG_FillRect( 0, 0, 640, 480, col ); // why do a bunch of these extend outside 640x480?
	}
#elif defined RTCW_MP
	if ( cgs.fadeStartTime + cgs.fadeDuration < cg.time ) {
		cgs.fadeAlphaCurrent = cgs.fadeAlpha;
	} else if ( cgs.fadeAlphaCurrent != cgs.fadeAlpha ) {
		elapsed = ( time = trap_Milliseconds() ) - lastTime;  // we need to use trap_Milliseconds() here since the cg.time gets modified upon reloading
		lastTime = time;
		if ( elapsed < 500 && elapsed > 0 ) {
			if ( cgs.fadeAlphaCurrent > cgs.fadeAlpha ) {
				cgs.fadeAlphaCurrent -= ( (float)elapsed / (float)cgs.fadeDuration );
				if ( cgs.fadeAlphaCurrent < cgs.fadeAlpha ) {
					cgs.fadeAlphaCurrent = cgs.fadeAlpha;
				}
			} else {
				cgs.fadeAlphaCurrent += ( (float)elapsed / (float)cgs.fadeDuration );
				if ( cgs.fadeAlphaCurrent > cgs.fadeAlpha ) {
					cgs.fadeAlphaCurrent = cgs.fadeAlpha;
				}
			}
		}
	}
	// now draw the fade
	if ( cgs.fadeAlphaCurrent > 0.0 ) {
		VectorClear( col );
		col[3] = cgs.fadeAlphaCurrent;
		CG_FillRect( -10, -10, 650, 490, col );
	}
#endif RTCW_XX

}



/*
==============
CG_DrawFlashZoomTransition
	hide the snap transition from regular view to/from zoomed

  FIXME: TODO: use cg_fade?
==============
*/
static void CG_DrawFlashZoomTransition( void ) {
	vec4_t color;
	float frac;
	int fadeTime;

	if ( !cg.snap ) {
		return;
	}

	if ( cg.snap->ps.eFlags & EF_MG42_ACTIVE ) {   // don't draw when on mg_42
		// keep the timer fresh so when you remove yourself from the mg42, it'll fade
		cg.zoomTime = cg.time;
		return;
	}

	if ( cgs.gametype != GT_SINGLE_PLAYER ) { // JPW NERVE
		fadeTime = 400;
	} else {
		if ( cg.zoomedScope ) {
			fadeTime = cg.zoomedScope;  //----(SA)
		} else {
			fadeTime = 300;
		}
	}
	// jpw

	frac = cg.time - cg.zoomTime;

	if ( frac < fadeTime ) {
		frac = frac / (float)fadeTime;

		if ( cg.weaponSelect == WP_SNOOPERSCOPE ) {
//			Vector4Set( color, 0.7f, 0.3f, 0.7f, 1.0f - frac );
//			Vector4Set( color, 1, 0.5, 1, 1.0f - frac );
//			Vector4Set( color, 0.5f, 0.3f, 0.5f, 1.0f - frac );
			Vector4Set( color, 0.7f, 0.6f, 0.7f, 1.0f - frac );
		} else {
			Vector4Set( color, 0, 0, 0, 1.0f - frac );
		}

		CG_FillRect( -10, -10, 650, 490, color );
	}
}



/*
=================
CG_DrawFlashDamage
=================
*/
static void CG_DrawFlashDamage( void ) {
	vec4_t col;
	float redFlash;

	if ( !cg.snap ) {
		return;
	}

	if ( cg.v_dmg_time > cg.time ) {
		redFlash = fabs( cg.v_dmg_pitch * ( ( cg.v_dmg_time - cg.time ) / DAMAGE_TIME ) );

		// blend the entire screen red
		if ( redFlash > 5 ) {
			redFlash = 5;
		}

		VectorSet( col, 0.2, 0, 0 );
		col[3] =  0.7 * ( redFlash / 5.0 );

		CG_FillRect( -10, -10, 650, 490, col );
	}
}


/*
=================
CG_DrawFlashFire
=================
*/
static void CG_DrawFlashFire( void ) {
	vec4_t col = {1,1,1,1};
	float alpha, max, f;

	if ( !cg.snap ) {
		return;
	}

#if defined RTCW_SP
	if ( cg_thirdPerson.integer ) {
		return;
	}

	if ( cg.cameraMode ) { // don't draw flames on camera screen.  will still do damage though, so not a potential cheat
#elif defined RTCW_MP
	if ( cg.renderingThirdPerson ) {
#endif RTCW_XX

		return;
	}

	if ( !cg.snap->ps.onFireStart ) {
		cg.v_noFireTime = cg.time;
		return;
	}

	alpha = (float)( ( FIRE_FLASH_TIME - 1000 ) - ( cg.time - cg.snap->ps.onFireStart ) ) / ( FIRE_FLASH_TIME - 1000 );
	if ( alpha > 0 ) {
		if ( alpha >= 1.0 ) {
			alpha = 1.0;
		}

		// fade in?
		f = (float)( cg.time - cg.v_noFireTime ) / FIRE_FLASH_FADEIN_TIME;
		if ( f >= 0.0 && f < 1.0 ) {
			alpha = f;
		}

		max = 0.5 + 0.5 * sin( (float)( ( cg.time / 10 ) % 1000 ) / 1000.0 );
		if ( alpha > max ) {
			alpha = max;
		}
		col[0] = alpha;
		col[1] = alpha;
		col[2] = alpha;
		col[3] = alpha;
		trap_R_SetColor( col );
		CG_DrawPic( -10, -10, 650, 490, cgs.media.viewFlashFire[( cg.time / 50 ) % 16] );
		trap_R_SetColor( NULL );

		trap_S_AddLoopingSound( cg.snap->ps.clientNum, cg.snap->ps.origin, vec3_origin, cgs.media.flameSound, (int)( 255.0 * alpha ) );
		trap_S_AddLoopingSound( cg.snap->ps.clientNum, cg.snap->ps.origin, vec3_origin, cgs.media.flameCrackSound, (int)( 255.0 * alpha ) );
	} else {
		cg.v_noFireTime = cg.time;
	}
}

/*
=================
CG_DrawFlashLightning
=================
*/
static void CG_DrawFlashLightning( void ) {

#if defined RTCW_SP
	//vec4_t		col={1,1,1,1}; // TTimo: unused
#endif RTCW_XX

	float alpha;
	centity_t *cent;
	qhandle_t shader;

	if ( !cg.snap ) {
		return;
	}

	if ( cg_thirdPerson.integer ) {
		return;
	}

	cent = &cg_entities[cg.snap->ps.clientNum];

	if ( !cent->pe.teslaDamagedTime || ( cent->pe.teslaDamagedTime > cg.time ) ) {
		return;
	}

	alpha = 1.0 - (float)( cg.time - cent->pe.teslaDamagedTime ) / LIGHTNING_FLASH_TIME;
	if ( alpha > 0 ) {
		if ( alpha >= 1.0 ) {
			alpha = 1.0;
		}

		if ( ( cg.time / 50 ) % ( 2 + ( cg.time % 2 ) ) == 0 ) {
			shader = cgs.media.viewTeslaAltDamageEffectShader;
		} else {
			shader = cgs.media.viewTeslaDamageEffectShader;
		}

		CG_DrawPic( -10, -10, 650, 490, shader );
	}
}



/*
==============
CG_DrawFlashBlendBehindHUD
	screen flash stuff drawn first (on top of world, behind HUD)
==============
*/
static void CG_DrawFlashBlendBehindHUD( void ) {
	CG_DrawFlashZoomTransition();
}


/*
=================
CG_DrawFlashBlend
	screen flash stuff drawn last (on top of everything)
=================
*/
static void CG_DrawFlashBlend( void ) {
	CG_DrawFlashLightning();
	CG_DrawFlashFire();
	CG_DrawFlashDamage();
	CG_DrawFlashFade();
}

// NERVE - SMF
/*
=================
CG_DrawObjectiveInfo
=================
*/
#define OID_LEFT    10

#if defined RTCW_SP
#define OID_TOP     65
#elif defined RTCW_MP
#define OID_TOP     360
#endif RTCW_XX

#if defined RTCW_SP
void CG_ObjectivePrint( const char *str, int charWidth, int team ) {
	char    *s;

	Q_strncpyz( cg.oidPrint, str, sizeof( cg.oidPrint ) );

	cg.oidPrintTime = cg.time;
#elif defined RTCW_MP
void CG_ObjectivePrint( const char *str, int charWidth ) {
	char    *s;
	int i, len;                         // NERVE - SMF
	qboolean neednewline = qfalse;      // NERVE - SMF

	s = CG_TranslateString( str );

	Q_strncpyz( cg.oidPrint, s, sizeof( cg.oidPrint ) );

	// NERVE - SMF - turn spaces into newlines, if we've run over the linewidth
	len = strlen( cg.oidPrint );
	for ( i = 0; i < len; i++ ) {

		// NOTE: subtract a few chars here so long words still get displayed properly
		if ( i % ( CP_LINEWIDTH - 20 ) == 0 && i > 0 ) {
			neednewline = qtrue;
		}
		if ( cg.oidPrint[i] == ' ' && neednewline ) {
			cg.oidPrint[i] = '\n';
			neednewline = qfalse;
		}
	}
	// -NERVE - SMF

	cg.oidPrintTime = cg.time;
#endif RTCW_XX

	cg.oidPrintY = OID_TOP;
	cg.oidPrintCharWidth = charWidth;

	// count the number of lines for oiding
	cg.oidPrintLines = 1;
	s = cg.oidPrint;
	while ( *s ) {
		if ( *s == '\n' ) {
			cg.oidPrintLines++;
		}
		s++;
	}
}

static void CG_DrawObjectiveInfo( void ) {
	char    *start;
	int l;

#if defined RTCW_SP
	int x, y, w;
#elif defined RTCW_MP
	int x, y, w,h;
#endif RTCW_XX

	int x1, y1, x2, y2;
	float   *color;

#if defined RTCW_SP
	vec4_t backColor = { 0.2f, 0.2f, 0.2f, 1.f };
#elif defined RTCW_MP
	vec4_t backColor;
	backColor[0] = 0.2f;
	backColor[1] = 0.2f;
	backColor[2] = 0.2f;
	backColor[2] = cg_hudAlpha.value;
#endif RTCW_XX

	if ( !cg.oidPrintTime ) {
		return;
	}

#if defined RTCW_SP
	color = CG_FadeColor( cg.oidPrintTime, 1000 * 5 );
#elif defined RTCW_MP
	color = CG_FadeColor( cg.oidPrintTime, 250 );
#endif RTCW_XX

	if ( !color ) {

#if defined RTCW_MP
		cg.oidPrintTime = 0;
#endif RTCW_XX

		return;
	}

	trap_R_SetColor( color );

	start = cg.oidPrint;

#if defined RTCW_SP
	y = cg.oidPrintY - cg.oidPrintLines * BIGCHAR_HEIGHT / 2;

	x1 = OID_LEFT - 2;
	y1 = y - 2;
	x2 = 0;
#elif defined RTCW_MP
// JPW NERVE
	//	y = cg.oidPrintY - cg.oidPrintLines * BIGCHAR_HEIGHT / 2;
	y = 415 - cg.oidPrintLines * BIGCHAR_HEIGHT / 2;

	x1 = 319;
	y1 = y - 2;
	x2 = 321;
// jpw
#endif RTCW_XX

	// first just find the bounding rect
	while ( 1 ) {
		char linebuffer[1024];

#if defined RTCW_SP
		for ( l = 0; l < 40; l++ ) {
#elif defined RTCW_MP
		for ( l = 0; l < CP_LINEWIDTH; l++ ) {
#endif RTCW_XX

			if ( !start[l] || start[l] == '\n' ) {
				break;
			}
			linebuffer[l] = start[l];
		}
		linebuffer[l] = 0;

#if defined RTCW_SP
		w = cg.oidPrintCharWidth * CG_DrawStrlen( linebuffer );
		if ( x1 + w > x2 ) {
			x2 = x1 + w;
		}

		x = OID_LEFT;

#elif defined RTCW_MP
		w = cg.oidPrintCharWidth * CG_DrawStrlen( linebuffer ) + 10;
// JPW NERVE
		if ( 320 - w / 2 < x1 ) {
			x1 = 320 - w / 2;
			x2 = 320 + w / 2;
		}

/*
		if ( x1 + w > x2 )
			x2 = x1 + w;
*/
		x = 320 - w / 2;
// jpw
#endif RTCW_XX

		y += cg.oidPrintCharWidth * 1.5;

		while ( *start && ( *start != '\n' ) ) {
			start++;
		}
		if ( !*start ) {
			break;
		}
		start++;
	}

	x2 = x2 + 4;
	y2 = y - cg.oidPrintCharWidth * 1.5 + 4;

#if defined RTCW_SP
	backColor[3] = color[3];
	CG_FillRect( x1, y1, x2 - x1, y2 - y1, backColor );
#elif defined RTCW_MP
	h = y2 - y1; // JPW NERVE

	VectorCopy( color, backColor );
	backColor[3] = 0.5 * color[3];
	trap_R_SetColor( backColor );

	CG_DrawPic( x1, y1, x2 - x1, y2 - y1, cgs.media.teamStatusBar );
#endif RTCW_XX

	VectorSet( backColor, 0, 0, 0 );
	CG_DrawRect( x1, y1, x2 - x1, y2 - y1, 1, backColor );

#if defined RTCW_MP
	trap_R_SetColor( color );
#endif RTCW_XX

	// do the actual drawing
	start = cg.oidPrint;

#if defined RTCW_SP
	y = cg.oidPrintY - cg.oidPrintLines * BIGCHAR_HEIGHT / 2;
#elif defined RTCW_MP
//	y = cg.oidPrintY - cg.oidPrintLines * BIGCHAR_HEIGHT / 2;
	y = 415 - cg.oidPrintLines * BIGCHAR_HEIGHT / 2; // JPW NERVE
#endif RTCW_XX

	while ( 1 ) {
		char linebuffer[1024];

#if defined RTCW_SP
		for ( l = 0; l < 40; l++ ) {
#elif defined RTCW_MP
		for ( l = 0; l < CP_LINEWIDTH; l++ ) {
#endif RTCW_XX

			if ( !start[l] || start[l] == '\n' ) {
				break;
			}
			linebuffer[l] = start[l];
		}
		linebuffer[l] = 0;

		w = cg.oidPrintCharWidth * CG_DrawStrlen( linebuffer );
		if ( x1 + w > x2 ) {
			x2 = x1 + w;
		}

#if defined RTCW_SP
		x = OID_LEFT;
#elif defined RTCW_MP
		x = 320 - w / 2; // JPW NERVE
#endif RTCW_XX

		CG_DrawStringExt( x, y, linebuffer, color, qfalse, qtrue,
						  cg.oidPrintCharWidth, (int)( cg.oidPrintCharWidth * 1.5 ), 0 );

		y += cg.oidPrintCharWidth * 1.5;

		while ( *start && ( *start != '\n' ) ) {
			start++;
		}
		if ( !*start ) {
			break;
		}
		start++;
	}

	trap_R_SetColor( NULL );
}

#if defined RTCW_MP
void CG_DrawObjectiveIcons() {
	float x, y, w, h, xx, fade; // JPW NERVE
	float startColor[4];
	const char *s, *buf;
	char teamstr[32];
	float captureHoldFract;
	int i, num, status,barheight;
	vec4_t hcolor = { 0.2f, 0.2f, 0.2f, 1.f };
	int msec, mins, seconds, tens; // JPW NERVE

// JPW NERVE added round timer
	y = 48;
	x = 5;
	msec = ( cgs.timelimit * 60.f * 1000.f ) - ( cg.time - cgs.levelStartTime );

	seconds = msec / 1000;
	mins = seconds / 60;
	seconds -= mins * 60;
	tens = seconds / 10;
	seconds -= tens * 10;
	if ( msec < 0 ) {
		fade = fabs( sin( cg.time * 0.002 ) ) * cg_hudAlpha.value;
		s = va( "0:00" );
	} else {
		s = va( "%i:%i%i", mins, tens, seconds ); // float cast to line up with reinforce time
		fade = cg_hudAlpha.value;
	}

	CG_DrawSmallString( x,y,s,fade );

// jpw

	x = 5;
	y = 68;
	w = 24;
	h = 14;

	// draw the stopwatch
	if ( cgs.gametype == GT_WOLF_STOPWATCH ) {
		if ( cgs.currentRound == 0 ) {
			CG_DrawPic( 3, y, 30, 30, trap_R_RegisterShader( "sprites/stopwatch1.tga" ) );
		} else {
			CG_DrawPic( 3, y, 30, 30, trap_R_RegisterShader( "sprites/stopwatch2.tga" ) );
		}
		y += 34;
	}

	// determine character's team
	if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED ) {
		strcpy( teamstr, "axis_desc" );
	} else {
		strcpy( teamstr, "allied_desc" );
	}

	s = CG_ConfigString( CS_MULTI_INFO );
	buf = Info_ValueForKey( s, "numobjectives" );

	if ( buf && atoi( buf ) ) {
		num = atoi( buf );

		VectorSet( hcolor, 0.3f, 0.3f, 0.3f );
		hcolor[3] = 0.7f * cg_hudAlpha.value; // JPW NERVE
		CG_DrawRect( x - 1, y - 1, w + 2, ( h + 4 ) * num - 4 + 2, 1, hcolor );

		VectorSet( hcolor, 1.0f, 1.0f, 1.0f );
		hcolor[3] = 0.2f * cg_hudAlpha.value; // JPW NERVE
		trap_R_SetColor( hcolor );
		CG_DrawPic( x, y, w, ( h + 4 ) * num - 4, cgs.media.teamStatusBar );
		trap_R_SetColor( NULL );

		for ( i = 0; i < num; i++ ) {
			s = CG_ConfigString( CS_MULTI_OBJECTIVE1 + i );
			buf = Info_ValueForKey( s, teamstr );

			xx = x;

			hcolor[0] = 0.25f;
			hcolor[1] = 0.38f;
			hcolor[2] = 0.38f;
			hcolor[3] = 0.5f * cg_hudAlpha.value; // JPW NERVE
			trap_R_SetColor( hcolor );
			CG_DrawPic( xx, y, w, h, cgs.media.teamStatusBar );
			trap_R_SetColor( NULL );

			// draw text
			VectorSet( hcolor, 1.f, 1.f, 1.f );
			hcolor[3] = cg_hudAlpha.value; // JPW NERVE
//			CG_DrawStringExt( xx, y, va( "%i", i+1 ), hcolor, qtrue, qfalse, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 ); // JPW NERVE pulled per id request

//			xx += 10;

			// draw status flags
			s = CG_ConfigString( CS_MULTI_OBJ1_STATUS + i );
			status = atoi( Info_ValueForKey( s, "status" ) );

			VectorSet( hcolor, 1, 1, 1 );
			hcolor[3] = 0.7f * cg_hudAlpha.value; // JPW NERVE
			trap_R_SetColor( hcolor );

			if ( status == 0 ) {
				CG_DrawPic( xx, y, w, h, trap_R_RegisterShaderNoMip( "ui_mp/assets/ger_flag.tga" ) );
			} else if ( status == 1 )   {
				CG_DrawPic( xx, y, w, h, trap_R_RegisterShaderNoMip( "ui_mp/assets/usa_flag.tga" ) );
			}

			VectorSet( hcolor, 0.2f, 0.2f, 0.2f );
			hcolor[3] = cg_hudAlpha.value; // JPW NERVE
			trap_R_SetColor( hcolor );

//			CG_DrawRect( xx, y, w, h, 2, hcolor );

			y += h + 4;
		}
	}

// JPW NERVE compute & draw hold bar
	if ( cgs.gametype == GT_WOLF_CPH ) {
		if ( cg.snap->ps.stats[STAT_CAPTUREHOLD_RED] || cg.snap->ps.stats[STAT_CAPTUREHOLD_BLUE] ) {
			captureHoldFract = (float)cg.snap->ps.stats[STAT_CAPTUREHOLD_RED] /
							   (float)( cg.snap->ps.stats[STAT_CAPTUREHOLD_RED] + cg.snap->ps.stats[STAT_CAPTUREHOLD_BLUE] );
		} else {
			captureHoldFract = 0.5;
		}

		barheight = y - 72; // (68+4)


		startColor[0] = 1;
		startColor[1] = 1;
		startColor[2] = 1;
		startColor[3] = 1;
		if ( captureHoldFract > 0.5 ) {
			startColor[3] = cg_hudAlpha.value;
		} else {
			startColor[3] = cg_hudAlpha.value * ( ( captureHoldFract ) + 0.15 );
		}
//		startColor[3] = 0.25;
		trap_R_SetColor( startColor );
		CG_DrawPic( 32,68,5,barheight * captureHoldFract,cgs.media.redColorBar );

		if ( captureHoldFract < 0.5 ) {
			startColor[3] = cg_hudAlpha.value;
		} else {
			startColor[3] = cg_hudAlpha.value * ( 0.15 + ( 1.0f - captureHoldFract ) );
		}
//		startColor[3] = 0.25;
		trap_R_SetColor( startColor );
		CG_DrawPic( 32,68 + barheight * captureHoldFract,5,barheight - barheight * captureHoldFract,cgs.media.blueColorBar );
	}
// jpw


	// draw treasure icon if we have the flag
	y += 4;

	VectorSet( hcolor, 1, 1, 1 );
	hcolor[3] = cg_hudAlpha.value;
	trap_R_SetColor( hcolor );
	if ( cgs.clientinfo[cg.snap->ps.clientNum].powerups & ( 1 << PW_REDFLAG ) ||
		 cgs.clientinfo[cg.snap->ps.clientNum].powerups & ( 1 << PW_BLUEFLAG ) ) {
		CG_DrawPic( -7, y, 48, 48, trap_R_RegisterShader( "models/multiplayer/treasure/treasure" ) );
		y += 50;
	}
}
#endif RTCW_XX

// -NERVE - SMF

//==================================================================================


void CG_DrawTimedMenus() {
	if ( cg.voiceTime ) {
		int t = cg.time - cg.voiceTime;
		if ( t > 2500 ) {
			Menus_CloseByName( "voiceMenu" );
			trap_Cvar_Set( "cl_conXOffset", "0" );
			cg.voiceTime = 0;
		}
	}
}


/*
=================
CG_Fade
=================
*/

#if defined RTCW_SP
void CG_Fade( int r, int g, int b, int a, int time, int duration ) {

	// incorporate this into the current fade scheme

	cgs.scrFadeAlpha = (float)a / 255.0f;
	cgs.scrFadeStartTime = time;
	cgs.scrFadeDuration = duration;

	if ( cgs.scrFadeStartTime + cgs.scrFadeDuration <= cg.time ) {
		cgs.scrFadeAlphaCurrent = cgs.scrFadeAlpha;
	}


	return;

#elif defined RTCW_MP
void CG_Fade( int r, int g, int b, int a, float time ) {
#endif RTCW_XX

	if ( time <= 0 ) {  // do instantly
		cg.fadeRate = 1;
		cg.fadeTime = cg.time - 1;  // set cg.fadeTime behind cg.time so it will start out 'done'
	} else {
		cg.fadeRate = 1.0f / time;
		cg.fadeTime = cg.time + time;
	}

	cg.fadeColor2[ 0 ] = ( float )r / 255.0f;
	cg.fadeColor2[ 1 ] = ( float )g / 255.0f;
	cg.fadeColor2[ 2 ] = ( float )b / 255.0f;
	cg.fadeColor2[ 3 ] = ( float )a / 255.0f;
}


#if defined RTCW_SP
/*
===============
CG_DrawGameScreenFade
===============
*/
static void CG_DrawGameScreenFade( void ) {
	vec4_t col;

	if ( cg.viewFade <= 0.0 ) {
		return;
	}

	if ( !cg.snap ) {
		return;
	}

	VectorClear( col );
	col[3] = cg.viewFade;
	CG_FillRect( 0, 0, 640, 480, col );
}
#endif RTCW_XX

/*
=================
CG_ScreenFade
=================
*/
static void CG_ScreenFade( void ) {
	int msec;
	int i;
	float t, invt;
	vec4_t color;

#if defined RTCW_SP
	// Ridah, fade the screen (in-game)
	CG_DrawGameScreenFade();
#endif RTCW_XX

	if ( !cg.fadeRate ) {
		return;
	}

	msec = cg.fadeTime - cg.time;
	if ( msec <= 0 ) {
		cg.fadeColor1[ 0 ] = cg.fadeColor2[ 0 ];
		cg.fadeColor1[ 1 ] = cg.fadeColor2[ 1 ];
		cg.fadeColor1[ 2 ] = cg.fadeColor2[ 2 ];
		cg.fadeColor1[ 3 ] = cg.fadeColor2[ 3 ];

		if ( !cg.fadeColor1[ 3 ] ) {
			cg.fadeRate = 0;
			return;
		}

		CG_FillRect( 0, 0, 640, 480, cg.fadeColor1 );

	} else {
		t = ( float )msec * cg.fadeRate;
		invt = 1.0f - t;

		for ( i = 0; i < 4; i++ ) {
			color[ i ] = cg.fadeColor1[ i ] * t + cg.fadeColor2[ i ] * invt;
		}

		if ( color[ 3 ] ) {
			CG_FillRect( 0, 0, 640, 480, color );
		}
	}
}

#if defined RTCW_MP
// JPW NERVE
void CG_Draw2D2( void ) {
	qhandle_t weapon;
	vec4_t hcolor;

	VectorSet( hcolor, 1.f,1.f,1.f );
//	VectorSet(hcolor, cg_hudAlpha.value, cg_hudAlpha.value, cg_hudAlpha.value);
	hcolor[3] = cg_hudAlpha.value;
	trap_R_SetColor( hcolor );

	CG_DrawPic( 0,480, 640, -70, cgs.media.hud1Shader );

	if ( !( cg.snap->ps.eFlags & EF_MG42_ACTIVE ) ) {
		switch ( cg.snap->ps.weapon ) {
		case WP_COLT:
		case WP_LUGER:
			weapon = cgs.media.hud2Shader;
			break;
		case WP_KNIFE:
			weapon = cgs.media.hud5Shader;
			break;
		case WP_VENOM:
			weapon = cgs.media.hud4Shader;
			break;
		default:
			weapon = cgs.media.hud3Shader;
		}
		CG_DrawPic( 220,410, 200,-200,weapon );
	}
}
// jpw

/*
=================
CG_DrawCompassIcon

NERVE - SMF
=================
*/
void CG_DrawCompassIcon( int x, int y, int w, int h, vec3_t origin, vec3_t dest, qhandle_t shader ) {
	float angle, pi2 = M_PI * 2;
	vec3_t v1, angles;
	float len;

	VectorCopy( dest, v1 );
	VectorSubtract( origin, v1, v1 );
	len = VectorLength( v1 );
	VectorNormalize( v1 );
	vectoangles( v1, angles );

	if ( v1[0] == 0 && v1[1] == 0 && v1[2] == 0 ) {
		return;
	}

	angles[YAW] = AngleSubtract( cg.snap->ps.viewangles[YAW], angles[YAW] );

	angle = ( ( angles[YAW] + 180.f ) / 360.f - ( 0.50 / 2.f ) ) * pi2;

	w /= 2;
	h /= 2;

	x += w;
	y += h;

	w = sqrt( ( w * w ) + ( h * h ) ) / 3.f * 2.f * 0.9f;

	x = x + ( cos( angle ) * w );
	y = y + ( sin( angle ) * w );

	len = 1 - min( 1.f, len / 2000.f );

	CG_DrawPic( x - ( 14 * len + 4 ) / 2, y - ( 14 * len + 4 ) / 2, 14 * len + 4, 14 * len + 4, shader );
}

/*
=================
CG_DrawCompass

NERVE - SMF
=================
*/
static void CG_DrawCompass( void ) {
	float basex = 290, basey = 420;
	float basew = 60, baseh = 60;
	snapshot_t  *snap;
	vec4_t hcolor;
	float angle;
	int i;

	if ( cgs.gametype < GT_WOLF ) {
		return;
	}

	if ( cg.snap->ps.pm_flags & PMF_LIMBO || cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR ) {
		return;
	}

	angle = ( cg.snap->ps.viewangles[YAW] + 180.f ) / 360.f - ( 0.25 / 2.f );

	VectorSet( hcolor, 1.f,1.f,1.f );
	hcolor[3] = cg_hudAlpha.value;
	trap_R_SetColor( hcolor );

	CG_DrawRotatedPic( basex, basey, basew, baseh, trap_R_RegisterShader( "gfx/2d/compass2.tga" ), angle );
	CG_DrawPic( basex, basey, basew, baseh, trap_R_RegisterShader( "gfx/2d/compass.tga" ) );

	// draw voice chats
	for ( i = 0; i < MAX_CLIENTS; i++ ) {
		centity_t *cent = &cg_entities[i];

		if ( cg.snap->ps.clientNum == i || !cgs.clientinfo[i].infoValid || cg.snap->ps.persistant[PERS_TEAM] != cgs.clientinfo[i].team ) {
			continue;
		}

		// also draw revive icons if cent is dead and player is a medic
		if ( cent->voiceChatSpriteTime < cg.time ) {
			continue;
		}

		if ( cgs.clientinfo[i].health <= 0 ) {
			// reset
			cent->voiceChatSpriteTime = cg.time;
			continue;
		}

		CG_DrawCompassIcon( basex, basey, basew, baseh, cg.snap->ps.origin, cent->lerpOrigin, cent->voiceChatSprite );
	}

	// draw explosives if an engineer
	if ( cg.snap->ps.stats[ STAT_PLAYER_CLASS ] == PC_ENGINEER ) {
		if ( cg.nextSnap && !cg.nextFrameTeleport && !cg.thisFrameTeleport ) {
			snap = cg.nextSnap;
		} else {
			snap = cg.snap;
		}

		for ( i = 0; i < snap->numEntities; i++ ) {
			centity_t *cent = &cg_entities[ snap->entities[ i ].number ];

			if ( cent->currentState.eType != ET_EXPLOSIVE_INDICATOR ) {
				continue;
			}

			if ( cent->currentState.teamNum == 1 && cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED ) {
				continue;
			} else if ( cent->currentState.teamNum == 2 && cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE ) {
				continue;
			}

			CG_DrawCompassIcon( basex, basey, basew, baseh, cg.snap->ps.origin, cent->lerpOrigin, trap_R_RegisterShader( "sprites/destroy.tga" ) );
		}
	}

	// draw revive medic icons
	if ( cg.snap->ps.stats[ STAT_PLAYER_CLASS ] == PC_MEDIC ) {
		if ( cg.nextSnap && !cg.nextFrameTeleport && !cg.thisFrameTeleport ) {
			snap = cg.nextSnap;
		} else {
			snap = cg.snap;
		}

		for ( i = 0; i < snap->numEntities; i++ ) {
			entityState_t *ent = &snap->entities[i];

			if ( ent->eType != ET_PLAYER ) {
				continue;
			}

			if ( ( ent->eFlags & EF_DEAD ) && ent->number == ent->clientNum ) {
				if ( !cgs.clientinfo[ent->clientNum].infoValid || cg.snap->ps.persistant[PERS_TEAM] != cgs.clientinfo[ent->clientNum].team ) {
					continue;
				}

				CG_DrawCompassIcon( basex, basey, basew, baseh, cg.snap->ps.origin, ent->pos.trBase, cgs.media.medicReviveShader );
			}
		}
	}
}
// -NERVE - SMF
#endif RTCW_XX


/*
=================
CG_Draw2D
=================
*/
static void CG_Draw2D( void ) {

	// if we are taking a levelshot for the menu, don't draw anything
	if ( cg.levelShot ) {
		return;
	}

#if defined RTCW_SP
	if ( cg.cameraMode ) { //----(SA)	no 2d when in camera view
		CG_DrawFlashBlend();    // (for fades)
		return;
	}
#endif RTCW_XX

	if ( cg_draw2D.integer == 0 ) {
		return;
	}

	CG_ScreenFade();

#if defined RTCW_MP
	if ( cg.cameraMode ) { //----(SA)	no 2d when in camera view
		return;
	}
#endif RTCW_XX

	if ( cg.snap->ps.pm_type == PM_INTERMISSION ) {
		CG_DrawIntermission();
		return;
	}

	CG_DrawFlashBlendBehindHUD();

#if defined RTCW_MP
#ifndef PRE_RELEASE_DEMO
	if ( cg_uselessNostalgia.integer ) {
		CG_DrawCrosshair();
		CG_Draw2D2();
		return;
	}
#endif
#endif RTCW_XX

	if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR ) {
		CG_DrawSpectator();
		CG_DrawCrosshair();
		CG_DrawCrosshairNames();

#if defined RTCW_MP
		// NERVE - SMF - we need to do this for spectators as well
		if ( cgs.gametype >= GT_TEAM ) {
			CG_DrawTeamInfo();
		}
#endif RTCW_XX

	} else {
		// don't draw any status if dead

#if defined RTCW_SP
		if ( cg.snap->ps.stats[STAT_HEALTH] > 0 ) {

			CG_DrawCrosshair();

			if ( cg_drawStatus.integer ) {
				Menu_PaintAll();
				CG_DrawTimedMenus();
			}

//			CG_DrawStatusBar();
			CG_DrawAmmoWarning();
			CG_DrawDynamiteStatus();
			CG_DrawCrosshairNames();
			CG_DrawWeaponSelect();
			CG_DrawHoldableSelect();
			CG_DrawPickupItem();
			CG_DrawReward();
		}
		if ( cgs.gametype >= GT_TEAM ) {
			CG_DrawTeamInfo();
		}
#elif defined RTCW_MP
		if ( cg.snap->ps.stats[STAT_HEALTH] > 0 || ( cg.snap->ps.pm_flags & PMF_FOLLOW ) ) {

			CG_DrawCrosshair();

			CG_DrawDynamiteStatus();
			CG_DrawCrosshairNames();

			// DHM - Nerve :: Don't draw icon in upper-right when switching weapons
			//CG_DrawWeaponSelect();

			CG_DrawPickupItem();
		}
		if ( cgs.gametype >= GT_TEAM ) {
			CG_DrawTeamInfo();
		}
		if ( cg_drawStatus.integer ) {
			Menu_PaintAll();
			CG_DrawTimedMenus();
		}
#endif RTCW_XX

	}

	CG_DrawVote();

	CG_DrawLagometer();

	if ( !cg_paused.integer ) {
		CG_DrawUpperRight();
	}

#if defined RTCW_SP
//	CG_DrawLowerRight();
	if ( !CG_DrawFollow() ) {
		CG_DrawWarmup();
	}
#endif RTCW_XX

	// don't draw center string if scoreboard is up
	if ( !CG_DrawScoreboard() ) {
		CG_DrawCenterString();

#if defined RTCW_SP
		CG_DrawObjectiveInfo();     // NERVE - SMF
	}
#elif defined RTCW_MP
		CG_DrawFollow();
		CG_DrawWarmup();

		//if ( cg_drawNotifyText.integer )
		CG_DrawNotify();

		// NERVE - SMF
		if ( cg_drawCompass.integer ) {
			CG_DrawCompass();
		}

		CG_DrawObjectiveInfo();
		CG_DrawObjectiveIcons();

		CG_DrawSpectatorMessage();

		CG_DrawLimboMessage();
		// -NERVE - SMF
	}
#endif RTCW_XX

	// Ridah, draw flash blends now
	CG_DrawFlashBlend();
}

#if defined RTCW_SP
/*
====================
CG_StartShakeCamera
====================
*/
void CG_StartShakeCamera( float p, int duration, vec3_t src, float radius ) {
	int i;

	// find a free shake slot
	for ( i = 0; i < MAX_CAMERA_SHAKE; i++ ) {
		if ( cg.cameraShake[i].time > cg.time || cg.cameraShake[i].time + cg.cameraShake[i].length <= cg.time ) {
			break;
		}
	}

	if ( i == MAX_CAMERA_SHAKE ) {
		return; // no free slots

	}
	cg.cameraShake[i].scale = p;

	cg.cameraShake[i].length = duration;
	cg.cameraShake[i].time = cg.time;
	VectorCopy( src, cg.cameraShake[i].src );
	cg.cameraShake[i].radius = radius;
}
#elif defined RTCW_MP
// NERVE - SMF
void CG_StartShakeCamera( float p ) {
	cg.cameraShakeScale = p;

	cg.cameraShakeLength = 1000 * ( p * p );
	cg.cameraShakeTime = cg.time + cg.cameraShakeLength;
	cg.cameraShakePhase = crandom() * M_PI; // start chain in random dir
}
#endif RTCW_XX

#if defined RTCW_SP
/*
====================
CG_CalcShakeCamera
====================
*/
void CG_CalcShakeCamera() {
	float val, scale, dist, x, sx;
	float bx = 0.0f; // TTimo: init
	int i;

	// build the scale
	scale = 0.0f;
	sx = (float)cg.time / 600.0; // x * (float)(cg.cameraShake[i].length) / 600.0;
	for ( i = 0; i < MAX_CAMERA_SHAKE; i++ ) {
		if ( cg.cameraShake[i].time <= cg.time && cg.cameraShake[i].time + cg.cameraShake[i].length > cg.time ) {
			dist = Distance( cg.cameraShake[i].src, cg.refdef.vieworg );
			// fade with distance
			val = cg.cameraShake[i].scale * ( 1.0f - ( dist / cg.cameraShake[i].radius ) );
			// fade with time
			x = 1.0f - ( ( cg.time - cg.cameraShake[i].time ) / cg.cameraShake[i].length );
			val *= x;
			// overwrite global scale if larger
			if ( val > scale ) {
				scale = val;
				bx = x;
			}
		}
	}

	// check the current rumble status
	if ( cg.rumbleScale > scale ) {
		scale = cg.rumbleScale;
		bx = cg.rumbleScale;
	}

	if ( scale <= 0.0f ) {
		cg.cameraShakePhase = crandom() * M_PI; // randomize the phase
		return;
	}

	if ( scale > 1.0f ) {
		scale = 1.0f;
	}

	// up/down
	val = sin( M_PI * 8 * sx + cg.cameraShakePhase ) * bx * 18.0f * scale;
	cg.cameraShakeAngles[0] = val;
	//cg.refdefViewAngles[0] += val;

	// left/right
	val = sin( M_PI * 15 * sx + cg.cameraShakePhase ) * bx * 16.0f * scale;
	cg.cameraShakeAngles[1] = val;
	//cg.refdefViewAngles[1] += val;

	// roll
	val = sin( M_PI * 12 * sx + cg.cameraShakePhase ) * bx * 10.0f * scale;
	cg.cameraShakeAngles[2] = val;
	//cg.refdefViewAngles[2] += val;
}

/*
====================
CG_ApplyShakeCamera
====================
*/
void CG_ApplyShakeCamera() {
	VectorAdd( cg.refdefViewAngles, cg.cameraShakeAngles, cg.refdefViewAngles );
	AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );
}
#elif defined RTCW_MP
void CG_ShakeCamera() {
	float x, val;

	if ( cg.time > cg.cameraShakeTime ) {
		cg.cameraShakeScale = 0; // JPW NERVE all pending explosions resolved, so reset shakescale
		return;
	}

	// JPW NERVE starts at 1, approaches 0 over time
	x = ( cg.cameraShakeTime - cg.time ) / cg.cameraShakeLength;

	// up/down
	val = sin( M_PI * 8 * x + cg.cameraShakePhase ) * x * 18.0f * cg.cameraShakeScale;
	cg.refdefViewAngles[0] += val;

	// left/right
	val = sin( M_PI * 15 * x + cg.cameraShakePhase ) * x * 16.0f * cg.cameraShakeScale;
	cg.refdefViewAngles[1] += val;

	AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );
}
// -NERVE - SMF
#endif RTCW_XX

/*
=====================
CG_DrawActive

Perform all drawing needed to completely fill the screen
=====================
*/
void CG_DrawActive( stereoFrame_t stereoView ) {
	float separation;
	vec3_t baseOrg;

	// optionally draw the info screen instead
	if ( !cg.snap ) {
		CG_DrawInformation();
		return;
	}

	// if they are waiting at the mission stats screen, show the stats
	if ( cg_gameType.integer == GT_SINGLE_PLAYER ) {
		if ( strlen( cg_missionStats.string ) > 1 ) {
			trap_Cvar_Set( "com_expectedhunkusage", "-2" );
			CG_DrawInformation();
			return;
		}
	}

	// optionally draw the tournement scoreboard instead
	if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR &&
		 ( cg.snap->ps.pm_flags & PMF_SCOREBOARD ) ) {
		CG_DrawTourneyScoreboard();
		return;
	}

	switch ( stereoView ) {
	case STEREO_CENTER:
		separation = 0;
		break;
	case STEREO_LEFT:
		separation = -cg_stereoSeparation.value / 2;
		break;
	case STEREO_RIGHT:
		separation = cg_stereoSeparation.value / 2;
		break;
	default:
		separation = 0;
		CG_Error( "CG_DrawActive: Undefined stereoView" );
	}


	// clear around the rendered view if sized down

#if defined RTCW_SP
//	CG_TileClear();	// (SA) moved down
#elif defined RTCW_MP
	CG_TileClear();
#endif RTCW_XX

	// offset vieworg appropriately if we're doing stereo separation
	VectorCopy( cg.refdef.vieworg, baseOrg );
	if ( separation != 0 ) {
		VectorMA( cg.refdef.vieworg, -separation, cg.refdef.viewaxis[1], cg.refdef.vieworg );
	}

	cg.refdef.glfog.registered = 0; // make sure it doesn't use fog from another scene

#if defined RTCW_SP
/*
	// NERVE - SMF - activate limbo menu and draw small 3d window
	CG_ActivateLimboMenu();

	if ( cg.limboMenu ) {
		float x, y, w, h;
		x = LIMBO_3D_X;
		y = LIMBO_3D_Y;
		w = LIMBO_3D_W;
		h = LIMBO_3D_H;

		cg.refdef.width = 0;
		CG_AdjustFrom640( &x, &y, &w, &h );

		cg.refdef.x = x;
		cg.refdef.y = y;
		cg.refdef.width = w;
		cg.refdef.height = h;
	}
	// -NERVE - SMF
*/
#elif defined RTCW_MP
	// NERVE - SMF - activate limbo menu and draw small 3d window
	CG_ActivateLimboMenu();

	if ( cg.limboMenu ) {
		float x, y, w, h;
		x = LIMBO_3D_X;
		y = LIMBO_3D_Y;
		w = LIMBO_3D_W;
		h = LIMBO_3D_H;

		cg.refdef.width = 0;
		CG_AdjustFrom640( &x, &y, &w, &h );

		cg.refdef.x = x;
		cg.refdef.y = y;
		cg.refdef.width = w;
		cg.refdef.height = h;
	}
	// -NERVE - SMF
#endif RTCW_XX

#if defined RTCW_SP
	cg.refdef.rdflags |= RDF_DRAWSKYBOX;
	if ( !cg_skybox.integer ) {
		cg.refdef.rdflags &= ~RDF_DRAWSKYBOX;
	}
#elif defined RTCW_MP
	CG_ShakeCamera();       // NERVE - SMF
#endif RTCW_XX

	trap_R_RenderScene( &cg.refdef );

	// restore original viewpoint if running stereo
	if ( separation != 0 ) {
		VectorCopy( baseOrg, cg.refdef.vieworg );
	}

#if defined RTCW_SP
	// clear around the rendered view if sized down
	CG_TileClear();     //----(SA)	moved to 2d section to avoid 2d/3d fog-state problems
#endif RTCW_XX

	// draw status bar and other floating elements
	CG_Draw2D();
}



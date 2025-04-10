/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

// cg_syscalls.c -- this file is only included when building a dll
// cg_syscalls.asm is included instead when building a qvm


#include "cg_local.h"

#include "rtcw_vm_args.h"


static int32_t ( QDECL * syscall )( intptr_t arg, ... ) = ( int32_t ( QDECL * )( intptr_t, ... ) ) - 1;

extern "C" void dllEntry( int32_t ( QDECL  *syscallptr )( intptr_t arg,... ) ) {
	syscall = syscallptr;
}

#if FIXME
int PASSFLOAT( float x ) {
	float floatTemp;
	floatTemp = x;
	return *(int *)&floatTemp;
}
#endif // FIXME

void    trap_Print( const char *fmt ) {
	syscall(
		CG_PRINT,
		rtcw::to_vm_arg(fmt)
	);
}

void    trap_Error( const char *fmt ) {
	syscall(
		CG_ERROR,
		rtcw::to_vm_arg(fmt)
	);
}

int     trap_Milliseconds( void ) {
	return syscall(CG_MILLISECONDS);
}

void    trap_Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags ) {
	syscall(
		CG_CVAR_REGISTER,
		rtcw::to_vm_arg(vmCvar),
		rtcw::to_vm_arg(varName),
		rtcw::to_vm_arg(defaultValue),
		rtcw::to_vm_arg(flags)
	);
}

void    trap_Cvar_Update( vmCvar_t *vmCvar ) {
	syscall(
		CG_CVAR_UPDATE,
		rtcw::to_vm_arg(vmCvar)
	);
}

void    trap_Cvar_Set( const char *var_name, const char *value ) {
	syscall(
		CG_CVAR_SET,
		rtcw::to_vm_arg(var_name),
		rtcw::to_vm_arg(value)
	);
}

void trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	syscall(
		CG_CVAR_VARIABLESTRINGBUFFER,
		rtcw::to_vm_arg(var_name),
		rtcw::to_vm_arg(buffer),
		rtcw::to_vm_arg(bufsize)
	);
}

int     trap_Argc( void ) {
	return syscall(CG_ARGC);
}

void    trap_Argv( int n, char *buffer, int bufferLength ) {
	syscall(
		CG_ARGV,
		rtcw::to_vm_arg(n),
		rtcw::to_vm_arg(buffer),
		rtcw::to_vm_arg(bufferLength)
	);
}

void    trap_Args( char *buffer, int bufferLength ) {
	syscall(
		CG_ARGS,
		rtcw::to_vm_arg(buffer),
		rtcw::to_vm_arg(bufferLength)
	);
}

int     trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode ) {
	return syscall(
		CG_FS_FOPENFILE,
		rtcw::to_vm_arg(qpath),
		rtcw::to_vm_arg(f),
		rtcw::to_vm_arg(mode)
	);
}

void    trap_FS_Read( void *buffer, int len, fileHandle_t f ) {
	syscall(
		CG_FS_READ,
		rtcw::to_vm_arg(buffer),
		rtcw::to_vm_arg(len),
		rtcw::to_vm_arg(f)
	);
}

void    trap_FS_Write( const void *buffer, int len, fileHandle_t f ) {
	syscall(
		CG_FS_WRITE,
		rtcw::to_vm_arg(buffer),
		rtcw::to_vm_arg(len),
		rtcw::to_vm_arg(f)
	);
}

void    trap_FS_FCloseFile( fileHandle_t f ) {
	syscall(
		CG_FS_FCLOSEFILE,
		rtcw::to_vm_arg(f)
	);
}

void    trap_SendConsoleCommand( const char *text ) {
	syscall(
		CG_SENDCONSOLECOMMAND,
		rtcw::to_vm_arg(text)
	);
}

void    trap_AddCommand( const char *cmdName ) {
	syscall(
		CG_ADDCOMMAND,
		rtcw::to_vm_arg(cmdName)
	);
}

void    trap_SendClientCommand( const char *s ) {
	syscall(
		CG_SENDCLIENTCOMMAND,
		rtcw::to_vm_arg(s)
	);
}

void    trap_UpdateScreen( void ) {
	syscall(CG_UPDATESCREEN);
}

void    trap_CM_LoadMap( const char *mapname ) {
	syscall(
		CG_CM_LOADMAP,
		rtcw::to_vm_arg(mapname)
	);
}

int     trap_CM_NumInlineModels( void ) {
	return syscall(CG_CM_NUMINLINEMODELS);
}

clipHandle_t trap_CM_InlineModel( int index ) {
	return syscall(
		CG_CM_INLINEMODEL,
		rtcw::to_vm_arg(index)
	);
}

clipHandle_t trap_CM_TempBoxModel( const vec3_t mins, const vec3_t maxs ) {
	return syscall(
		CG_CM_TEMPBOXMODEL,
		rtcw::to_vm_arg(mins),
		rtcw::to_vm_arg(maxs)
	);
}

clipHandle_t trap_CM_TempCapsuleModel( const vec3_t mins, const vec3_t maxs ) {
	return syscall(
		CG_CM_TEMPCAPSULEMODEL,
		rtcw::to_vm_arg(mins),
		rtcw::to_vm_arg(maxs)
	);
}

int     trap_CM_PointContents( const vec3_t p, clipHandle_t model ) {
	return syscall(
		CG_CM_POINTCONTENTS,
		rtcw::to_vm_arg(p),
		rtcw::to_vm_arg(model)
	);
}

int     trap_CM_TransformedPointContents( const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles ) {
	return syscall(
		CG_CM_TRANSFORMEDPOINTCONTENTS,
		rtcw::to_vm_arg(p),
		rtcw::to_vm_arg(model),
		rtcw::to_vm_arg(origin),
		rtcw::to_vm_arg(angles)
	);
}

void    trap_CM_BoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
						  const vec3_t mins, const vec3_t maxs,
						  clipHandle_t model, int brushmask ) {
	syscall(
		CG_CM_BOXTRACE,
		rtcw::to_vm_arg(results),
		rtcw::to_vm_arg(start),
		rtcw::to_vm_arg(end),
		rtcw::to_vm_arg(mins),
		rtcw::to_vm_arg(maxs),
		rtcw::to_vm_arg(model),
		rtcw::to_vm_arg(brushmask)
	);
}

void    trap_CM_TransformedBoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
									 const vec3_t mins, const vec3_t maxs,
									 clipHandle_t model, int brushmask,
									 const vec3_t origin, const vec3_t angles ) {
	syscall(
		CG_CM_TRANSFORMEDBOXTRACE,
		rtcw::to_vm_arg(results),
		rtcw::to_vm_arg(start),
		rtcw::to_vm_arg(end),
		rtcw::to_vm_arg(mins),
		rtcw::to_vm_arg(maxs),
		rtcw::to_vm_arg(model),
		rtcw::to_vm_arg(brushmask),
		rtcw::to_vm_arg(origin),
		rtcw::to_vm_arg(angles)
	);
}

void    trap_CM_CapsuleTrace( trace_t *results, const vec3_t start, const vec3_t end,
							  const vec3_t mins, const vec3_t maxs,
							  clipHandle_t model, int brushmask ) {
	syscall(
		CG_CM_CAPSULETRACE,
		rtcw::to_vm_arg(results),
		rtcw::to_vm_arg(start),
		rtcw::to_vm_arg(end),
		rtcw::to_vm_arg(mins),
		rtcw::to_vm_arg(maxs),
		rtcw::to_vm_arg(model),
		rtcw::to_vm_arg(brushmask)
	);
}

void    trap_CM_TransformedCapsuleTrace( trace_t *results, const vec3_t start, const vec3_t end,
										 const vec3_t mins, const vec3_t maxs,
										 clipHandle_t model, int brushmask,
										 const vec3_t origin, const vec3_t angles ) {
	syscall(
		CG_CM_TRANSFORMEDCAPSULETRACE,
		rtcw::to_vm_arg(results),
		rtcw::to_vm_arg(start),
		rtcw::to_vm_arg(end),
		rtcw::to_vm_arg(mins),
		rtcw::to_vm_arg(maxs),
		rtcw::to_vm_arg(model),
		rtcw::to_vm_arg(brushmask),
		rtcw::to_vm_arg(origin),
		rtcw::to_vm_arg(angles)
	);
}

int     trap_CM_MarkFragments( int numPoints, const vec3_t *points,
							   const vec3_t projection,
							   int maxPoints, vec3_t pointBuffer,
							   int maxFragments, markFragment_t *fragmentBuffer ) {
	return syscall(
		CG_CM_MARKFRAGMENTS,
		rtcw::to_vm_arg(numPoints),
		rtcw::to_vm_arg(points),
		rtcw::to_vm_arg(projection),
		rtcw::to_vm_arg(maxPoints),
		rtcw::to_vm_arg(pointBuffer),
		rtcw::to_vm_arg(maxFragments),
		rtcw::to_vm_arg(fragmentBuffer)
	);
}

void    trap_S_StartSound( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx ) {
	syscall(
		CG_S_STARTSOUND,
		rtcw::to_vm_arg(origin),
		rtcw::to_vm_arg(entityNum),
		rtcw::to_vm_arg(entchannel),
		rtcw::to_vm_arg(sfx)
	);
}

//----(SA)	added
void    trap_S_StartSoundEx( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx, int flags ) {
	syscall(
		CG_S_STARTSOUNDEX,
		rtcw::to_vm_arg(origin),
		rtcw::to_vm_arg(entityNum),
		rtcw::to_vm_arg(entchannel),
		rtcw::to_vm_arg(sfx),
		rtcw::to_vm_arg(flags)
	);
}
//----(SA)	end

void    trap_S_StartLocalSound( sfxHandle_t sfx, int channelNum ) {
	syscall(
		CG_S_STARTLOCALSOUND,
		rtcw::to_vm_arg(sfx),
		rtcw::to_vm_arg(channelNum)
	);
}

void    trap_S_ClearLoopingSounds( qboolean killall ) {
	syscall(
		CG_S_CLEARLOOPINGSOUNDS,
		rtcw::to_vm_arg(killall)
	);
}

void    trap_S_AddLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx, int volume ) {
	syscall(
		CG_S_ADDLOOPINGSOUND,
		rtcw::to_vm_arg(entityNum),
		rtcw::to_vm_arg(origin),
		rtcw::to_vm_arg(velocity),
		rtcw::to_vm_arg(1250),
		rtcw::to_vm_arg(sfx),
		rtcw::to_vm_arg(volume)
	); // volume was previously removed from CG_S_ADDLOOPINGSOUND.  I added 'range'
}

//----(SA)	added
void    trap_S_AddRangedLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx, int range ) {
	syscall(
		CG_S_ADDLOOPINGSOUND,
		rtcw::to_vm_arg(entityNum),
		rtcw::to_vm_arg(origin),
		rtcw::to_vm_arg(velocity),
		rtcw::to_vm_arg(range),
		rtcw::to_vm_arg(sfx),
		rtcw::to_vm_arg(255)
	); // RF, assume full volume, since thats how it worked before
}
//----(SA)	end

void    trap_S_AddRealLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx ) {
	syscall(
		CG_S_ADDREALLOOPINGSOUND,
		rtcw::to_vm_arg(entityNum),
		rtcw::to_vm_arg(origin),
		rtcw::to_vm_arg(velocity),
		rtcw::to_vm_arg(1250),
		rtcw::to_vm_arg(sfx),
		rtcw::to_vm_arg(255)
	);   //----(SA)	modified
}

void    trap_S_StopLoopingSound( int entityNum ) {
	syscall(
		CG_S_STOPLOOPINGSOUND,
		rtcw::to_vm_arg(entityNum)
	);
}

void    trap_S_UpdateEntityPosition( int entityNum, const vec3_t origin ) {
	syscall(
		CG_S_UPDATEENTITYPOSITION,
		rtcw::to_vm_arg(entityNum),
		rtcw::to_vm_arg(origin)
	);
}

// Ridah, talking animations
int     trap_S_GetVoiceAmplitude( int entityNum ) {
	return syscall(
		CG_S_GETVOICEAMPLITUDE,
		rtcw::to_vm_arg(entityNum)
	);
}
// done.

void    trap_S_Respatialize( int entityNum, const vec3_t origin, vec3_t axis[3], int inwater ) {
	syscall(
		CG_S_RESPATIALIZE,
		rtcw::to_vm_arg(entityNum),
		rtcw::to_vm_arg(origin),
		rtcw::to_vm_arg(axis),
		rtcw::to_vm_arg(inwater)
	);
}

sfxHandle_t trap_S_RegisterSound( const char *sample ) {
	CG_DrawInformation();
	return syscall(
		CG_S_REGISTERSOUND,
		rtcw::to_vm_arg(sample)
	);
}

void    trap_S_StartBackgroundTrack( const char *intro, const char *loop ) {
	syscall(
		CG_S_STARTBACKGROUNDTRACK,
		rtcw::to_vm_arg(intro),
		rtcw::to_vm_arg(loop)
	);
}

void    trap_S_StartStreamingSound( const char *intro, const char *loop, int entnum, int channel, int attenuation ) {
	syscall(
		CG_S_STARTSTREAMINGSOUND,
		rtcw::to_vm_arg(intro),
		rtcw::to_vm_arg(loop),
		rtcw::to_vm_arg(entnum),
		rtcw::to_vm_arg(channel),
		rtcw::to_vm_arg(attenuation)
	);
}

void    trap_R_LoadWorldMap( const char *mapname ) {
	syscall(
		CG_R_LOADWORLDMAP,
		rtcw::to_vm_arg(mapname)
	);
}

qhandle_t trap_R_RegisterModel( const char *name ) {
	CG_DrawInformation();
	return syscall(
		CG_R_REGISTERMODEL,
		rtcw::to_vm_arg(name)
	);
}

//----(SA)	added
qboolean trap_R_GetSkinModel( qhandle_t skinid, const char *type, char *name ) {
	return syscall(
		CG_R_GETSKINMODEL,
		rtcw::to_vm_arg(skinid),
		rtcw::to_vm_arg(type),
		rtcw::to_vm_arg(name)
	);
}

qhandle_t trap_R_GetShaderFromModel( qhandle_t modelid, int surfnum, int withlightmap ) {
	return syscall(
		CG_R_GETMODELSHADER,
		rtcw::to_vm_arg(modelid),
		rtcw::to_vm_arg(surfnum),
		rtcw::to_vm_arg(withlightmap)
	);
}
//----(SA)	end

qhandle_t trap_R_RegisterSkin( const char *name ) {
	CG_DrawInformation();
	return syscall(
		CG_R_REGISTERSKIN,
		rtcw::to_vm_arg(name)
	);
}

qhandle_t trap_R_RegisterShader( const char *name ) {
	CG_DrawInformation();
	return syscall(
		CG_R_REGISTERSHADER,
		rtcw::to_vm_arg(name)
	);
}

qhandle_t trap_R_RegisterShaderNoMip( const char *name ) {
	CG_DrawInformation();
	return syscall(
		CG_R_REGISTERSHADERNOMIP,
		rtcw::to_vm_arg(name)
	);
}

void trap_R_RegisterFont( const char *fontName, int pointSize, fontInfo_t *font ) {
	syscall(
		CG_R_REGISTERFONT,
		rtcw::to_vm_arg(fontName),
		rtcw::to_vm_arg(pointSize),
		rtcw::to_vm_arg(font)
	);
}

void    trap_R_ClearScene( void ) {
	syscall(CG_R_CLEARSCENE);
}

void    trap_R_AddRefEntityToScene( const refEntity_t *re ) {
	syscall(
		CG_R_ADDREFENTITYTOSCENE,
		rtcw::to_vm_arg(re)
	);
}

void    trap_R_AddPolyToScene( qhandle_t hShader, int numVerts, const polyVert_t *verts ) {
	syscall(
		CG_R_ADDPOLYTOSCENE,
		rtcw::to_vm_arg(hShader),
		rtcw::to_vm_arg(numVerts),
		rtcw::to_vm_arg(verts)
	);
}

// Ridah
void    trap_R_AddPolysToScene( qhandle_t hShader, int numVerts, const polyVert_t *verts, int numPolys ) {
	syscall(
		CG_R_ADDPOLYSTOSCENE,
		rtcw::to_vm_arg(hShader),
		rtcw::to_vm_arg(numVerts),
		rtcw::to_vm_arg(verts),
		rtcw::to_vm_arg(numPolys)
	);
}
// done.

void    trap_R_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b, int overdraw ) {
	syscall(
		CG_R_ADDLIGHTTOSCENE,
		rtcw::to_vm_arg(org),
		rtcw::to_vm_arg(intensity),
		rtcw::to_vm_arg(r),
		rtcw::to_vm_arg(g),
		rtcw::to_vm_arg(b),
		rtcw::to_vm_arg(overdraw)
	);
}

//----(SA)
void    trap_R_AddCoronaToScene( const vec3_t org, float r, float g, float b, float scale, int id, qboolean visible ) {
	syscall(
		CG_R_ADDCORONATOSCENE,
		rtcw::to_vm_arg(org),
		rtcw::to_vm_arg(r),
		rtcw::to_vm_arg(g),
		rtcw::to_vm_arg(b),
		rtcw::to_vm_arg(scale),
		rtcw::to_vm_arg(id),
		rtcw::to_vm_arg(visible)
	);
}
//----(SA)

//----(SA)
void    trap_R_SetFog( int fogvar, int var1, int var2, float r, float g, float b, float density ) {
	syscall(
		CG_R_SETFOG,
		rtcw::to_vm_arg(fogvar),
		rtcw::to_vm_arg(var1),
		rtcw::to_vm_arg(var2),
		rtcw::to_vm_arg(r),
		rtcw::to_vm_arg(g),
		rtcw::to_vm_arg(b),
		rtcw::to_vm_arg(density)
	);
}
//----(SA)
void    trap_R_RenderScene( const refdef_t *fd ) {
	syscall(
		CG_R_RENDERSCENE,
		rtcw::to_vm_arg(fd)
	);
}

void    trap_R_SetColor( const float *rgba ) {
	syscall(
		CG_R_SETCOLOR,
		rtcw::to_vm_arg(rgba)
	);
}

void    trap_R_DrawStretchPic( float x, float y, float w, float h,
							   float s1, float t1, float s2, float t2, qhandle_t hShader ) {
	syscall(
		CG_R_DRAWSTRETCHPIC,
		rtcw::to_vm_arg(x),
		rtcw::to_vm_arg(y),
		rtcw::to_vm_arg(w),
		rtcw::to_vm_arg(h),
		rtcw::to_vm_arg(s1),
		rtcw::to_vm_arg(t1),
		rtcw::to_vm_arg(s2),
		rtcw::to_vm_arg(t2),
		rtcw::to_vm_arg(hShader)
	);
}

void    trap_R_DrawRotatedPic( float x, float y, float w, float h,
							   float s1, float t1, float s2, float t2, qhandle_t hShader, float angle ) {
	syscall(
		CG_R_DRAWROTATEDPIC,
		rtcw::to_vm_arg(x),
		rtcw::to_vm_arg(y),
		rtcw::to_vm_arg(w),
		rtcw::to_vm_arg(h),
		rtcw::to_vm_arg(s1),
		rtcw::to_vm_arg(t1),
		rtcw::to_vm_arg(s2),
		rtcw::to_vm_arg(t2),
		rtcw::to_vm_arg(hShader),
		rtcw::to_vm_arg(angle)
	);
}

void    trap_R_DrawStretchPicGradient(  float x, float y, float w, float h,
										float s1, float t1, float s2, float t2, qhandle_t hShader,
										const float *gradientColor, int gradientType ) {
	syscall(
		CG_R_DRAWSTRETCHPIC_GRADIENT,
		rtcw::to_vm_arg(x),
		rtcw::to_vm_arg(y),
		rtcw::to_vm_arg(w),
		rtcw::to_vm_arg(h),
		rtcw::to_vm_arg(s1),
		rtcw::to_vm_arg(t1),
		rtcw::to_vm_arg(s2),
		rtcw::to_vm_arg(t2),
		rtcw::to_vm_arg(hShader),
		rtcw::to_vm_arg(gradientColor),
		rtcw::to_vm_arg(gradientType)
	);
}

void    trap_R_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs ) {
	syscall(
		CG_R_MODELBOUNDS,
		rtcw::to_vm_arg(model),
		rtcw::to_vm_arg(mins),
		rtcw::to_vm_arg(maxs)
	);
}

int     trap_R_LerpTag( orientation_t *tag, const refEntity_t *refent, const char *tagName, int startIndex ) {
	return syscall(
		CG_R_LERPTAG,
		rtcw::to_vm_arg(tag),
		rtcw::to_vm_arg(refent),
		rtcw::to_vm_arg(tagName),
		rtcw::to_vm_arg(startIndex)
	);
}

void    trap_R_RemapShader( const char *oldShader, const char *newShader, const char *timeOffset ) {
	syscall(
		CG_R_REMAP_SHADER,
		rtcw::to_vm_arg(oldShader),
		rtcw::to_vm_arg(newShader),
		rtcw::to_vm_arg(timeOffset)
	);
}

void        trap_GetGlconfig( glconfig_t *glconfig ) {
	syscall(
		CG_GETGLCONFIG,
		rtcw::to_vm_arg(glconfig)
	);
}

void        trap_GetGameState( gameState_t *gamestate ) {
	syscall(
		CG_GETGAMESTATE,
		rtcw::to_vm_arg(gamestate)
	);
}

void        trap_GetCurrentSnapshotNumber( int *snapshotNumber, int *serverTime ) {
	syscall(
		CG_GETCURRENTSNAPSHOTNUMBER,
		rtcw::to_vm_arg(snapshotNumber),
		rtcw::to_vm_arg(serverTime)
	);
}

qboolean    trap_GetSnapshot( int snapshotNumber, snapshot_t *snapshot ) {
	return syscall(
		CG_GETSNAPSHOT,
		rtcw::to_vm_arg(snapshotNumber),
		rtcw::to_vm_arg(snapshot)
	);
}

qboolean    trap_GetServerCommand( int serverCommandNumber ) {
	return syscall(
		CG_GETSERVERCOMMAND,
		rtcw::to_vm_arg(serverCommandNumber)
	);
}

int         trap_GetCurrentCmdNumber( void ) {
	return syscall(CG_GETCURRENTCMDNUMBER);
}

qboolean    trap_GetUserCmd( int cmdNumber, usercmd_t *ucmd ) {
	return syscall(
		CG_GETUSERCMD,
		rtcw::to_vm_arg(cmdNumber),
		rtcw::to_vm_arg(ucmd)
	);
}

void        trap_SetUserCmdValue( int stateValue, int holdableValue, float sensitivityScale, int mpSetup, int mpIdentClient ) {
	syscall(
		CG_SETUSERCMDVALUE,
		rtcw::to_vm_arg(stateValue),
		rtcw::to_vm_arg(holdableValue),
		rtcw::to_vm_arg(sensitivityScale),
		rtcw::to_vm_arg(mpSetup),
		rtcw::to_vm_arg(mpIdentClient)
	);
}

void        trap_SetClientLerpOrigin( float x, float y, float z ) {
	syscall(
		CG_SETCLIENTLERPORIGIN,
		rtcw::to_vm_arg(x),
		rtcw::to_vm_arg(y),
		rtcw::to_vm_arg(z)
	);
}

void        testPrintInt( char *string, int i ) {
	syscall(
		CG_TESTPRINTINT,
		rtcw::to_vm_arg(string),
		rtcw::to_vm_arg(i)
	);
}

void        testPrintFloat( char *string, float f ) {
	syscall(
		CG_TESTPRINTFLOAT,
		rtcw::to_vm_arg(string),
		rtcw::to_vm_arg(f)
	);
}

int trap_MemoryRemaining( void ) {
	return syscall(CG_MEMORY_REMAINING);
}

qboolean trap_loadCamera( int camNum, const char *name ) {
	return syscall(
		CG_LOADCAMERA,
		rtcw::to_vm_arg(camNum),
		rtcw::to_vm_arg(name)
	);
}

void trap_startCamera( int camNum, int time ) {
	syscall(
		CG_STARTCAMERA,
		rtcw::to_vm_arg(camNum),
		rtcw::to_vm_arg(time)
	);
}

qboolean trap_getCameraInfo( int camNum, int time, vec3_t *origin, vec3_t *angles, float *fov ) {
	return syscall(
		CG_GETCAMERAINFO,
		rtcw::to_vm_arg(camNum),
		rtcw::to_vm_arg(time),
		rtcw::to_vm_arg(origin),
		rtcw::to_vm_arg(angles),
		rtcw::to_vm_arg(fov)
	);
}


qboolean trap_Key_IsDown( int keynum ) {
	return syscall(
		CG_KEY_ISDOWN,
		rtcw::to_vm_arg(keynum)
	);
}

int trap_Key_GetCatcher( void ) {
	return syscall(CG_KEY_GETCATCHER);
}

void trap_Key_SetCatcher( int catcher ) {
	syscall(
		CG_KEY_SETCATCHER,
		rtcw::to_vm_arg(catcher)
	);
}

int trap_Key_GetKey( const char *binding ) {
	return syscall(
		CG_KEY_GETKEY,
		rtcw::to_vm_arg(binding)
	);
}


int trap_PC_AddGlobalDefine( char *define ) {
	return syscall(
		CG_PC_ADD_GLOBAL_DEFINE,
		rtcw::to_vm_arg(define)
	);
}

int trap_PC_LoadSource( const char *filename ) {
	return syscall(
		CG_PC_LOAD_SOURCE,
		rtcw::to_vm_arg(filename)
	);
}

int trap_PC_FreeSource( int handle ) {
	return syscall(
		CG_PC_FREE_SOURCE,
		rtcw::to_vm_arg(handle)
	);
}

int trap_PC_ReadToken( int handle, pc_token_t *pc_token ) {
	return syscall(
		CG_PC_READ_TOKEN,
		rtcw::to_vm_arg(handle),
		rtcw::to_vm_arg(pc_token)
	);
}


int trap_PC_SourceFileAndLine( int handle, char *filename, int *line ) {
	return syscall(
		CG_PC_SOURCE_FILE_AND_LINE,
		rtcw::to_vm_arg(handle),
		rtcw::to_vm_arg(filename),
		rtcw::to_vm_arg(line)
	);
}

void    trap_S_StopBackgroundTrack( void ) {
	syscall(CG_S_STOPBACKGROUNDTRACK);
}

int trap_RealTime( qtime_t *qtime ) {
	return syscall(
		CG_REAL_TIME,
		rtcw::to_vm_arg(qtime)
	);
}

void trap_SnapVector( float *v ) {
	syscall(
		CG_SNAPVECTOR,
		rtcw::to_vm_arg(v)
	);
}

void trap_SendMoveSpeedsToGame( int entnum, char *movespeeds ) {
	syscall(
		CG_SENDMOVESPEEDSTOGAME,
		rtcw::to_vm_arg(entnum),
		rtcw::to_vm_arg(movespeeds)
	);
}

// this returns a handle.  arg0 is the name in the format "idlogo.roq", set arg1 to NULL, alteredstates to qfalse (do not alter gamestate)
int trap_CIN_PlayCinematic( const char *arg0, int xpos, int ypos, int width, int height, int bits ) {
	return syscall(
		CG_CIN_PLAYCINEMATIC,
		rtcw::to_vm_arg(arg0),
		rtcw::to_vm_arg(xpos),
		rtcw::to_vm_arg(ypos),
		rtcw::to_vm_arg(width),
		rtcw::to_vm_arg(height),
		rtcw::to_vm_arg(bits)
	);
}

// stops playing the cinematic and ends it.  should always return FMV_EOF
// cinematics must be stopped in reverse order of when they are started
e_status trap_CIN_StopCinematic( int handle ) {
	return static_cast<e_status>(syscall(
		CG_CIN_STOPCINEMATIC,
		rtcw::to_vm_arg(handle)
	));
}


// will run a frame of the cinematic but will not draw it.  Will return FMV_EOF if the end of the cinematic has been reached.
e_status trap_CIN_RunCinematic( int handle ) {
	return static_cast<e_status>(syscall(
		CG_CIN_RUNCINEMATIC,
		rtcw::to_vm_arg(handle)
	));
}


// draws the current frame
void trap_CIN_DrawCinematic( int handle ) {
	syscall(
		CG_CIN_DRAWCINEMATIC,
		rtcw::to_vm_arg(handle)
	);
}


// allows you to resize the animation dynamically
void trap_CIN_SetExtents( int handle, int x, int y, int w, int h ) {
	syscall(
		CG_CIN_SETEXTENTS,
		rtcw::to_vm_arg(handle),
		rtcw::to_vm_arg(x),
		rtcw::to_vm_arg(y),
		rtcw::to_vm_arg(w),
		rtcw::to_vm_arg(h)
	);
}

qboolean trap_GetEntityToken( char *buffer, int bufferSize ) {
	return syscall(
		CG_GET_ENTITY_TOKEN,
		rtcw::to_vm_arg(buffer),
		rtcw::to_vm_arg(bufferSize)
	);
}

//----(SA)	added
// bring up a popup menu
extern void Menus_OpenByName( const char *p );

void trap_UI_Popup( const char *arg0 ) {
	syscall(
		CG_INGAME_POPUP,
		rtcw::to_vm_arg(arg0)
	);
}

// NERVE - SMF
void trap_UI_LimboChat( const char *arg0 ) {
	syscall(
		CG_LIMBOCHAT,
		rtcw::to_vm_arg(arg0)
	);
}

void trap_UI_ClosePopup( const char *arg0 ) {
	syscall(
		CG_INGAME_CLOSEPOPUP,
		rtcw::to_vm_arg(arg0)
	);
}

void trap_Key_GetBindingBuf( int keynum, char *buf, int buflen ) {
	syscall(
		CG_KEY_GETBINDINGBUF,
		rtcw::to_vm_arg(keynum),
		rtcw::to_vm_arg(buf),
		rtcw::to_vm_arg(buflen)
	);
}

void trap_Key_SetBinding( int keynum, const char *binding ) {
	syscall(
		CG_KEY_SETBINDING,
		rtcw::to_vm_arg(keynum),
		rtcw::to_vm_arg(binding)
	);
}

void trap_Key_KeynumToStringBuf( int keynum, char *buf, int buflen ) {
	syscall(
		CG_KEY_KEYNUMTOSTRINGBUF,
		rtcw::to_vm_arg(keynum),
		rtcw::to_vm_arg(buf),
		rtcw::to_vm_arg(buflen)
	);
}

#define MAX_VA_STRING       32000

char* trap_TranslateString( const char *string ) {
	static char staticbuf[2][MAX_VA_STRING];
	static int bufcount = 0;
	char *buf;

	buf = staticbuf[bufcount++ % 2];

	syscall(
		CG_TRANSLATE_STRING,
		rtcw::to_vm_arg(string),
		rtcw::to_vm_arg(buf)
	);

	return buf;
}
// -NERVE - SMF

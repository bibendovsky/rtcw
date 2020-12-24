/*
===========================================================================

Wolfenstein: Enemy Territory GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Wolfenstein: Enemy Territory GPL Source Code (Wolf ET Source Code).  

Wolf ET Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Wolf ET Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Wolf ET Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Wolf: ET Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Wolf ET Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/


#include "ui_local.h"

#include "rtcw_vm_args.h"


// this file is only included when building a dll
// syscalls.asm is included instead when building a qvm

static int32_t ( QDECL * syscall )( intptr_t arg, ... ) = ( int32_t ( QDECL * )( intptr_t, ... ) ) - 1;

#if __GNUC__ >= 4
#pragma GCC visibility push(default)
#endif
extern "C" void dllEntry( int32_t ( QDECL *syscallptr )( intptr_t arg,... ) ) {
	syscall = syscallptr;
}
#if __GNUC__ >= 4
#pragma GCC visibility pop
#endif

#if FIXME
int PASSFLOAT( float x ) {
	float floatTemp;
	floatTemp = x;
	return *(int *)&floatTemp;
}
#endif // FIXME

void trap_Print( const char *string ) {
	syscall(
		UI_PRINT,
		rtcw::to_vm_arg(string)
	);
}

void trap_Error( const char *string ) {
	syscall(
		UI_ERROR,
		rtcw::to_vm_arg(string)
	);
}

int trap_Milliseconds( void ) {
	return syscall(UI_MILLISECONDS);
}

void trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags ) {
	syscall(
		UI_CVAR_REGISTER,
		rtcw::to_vm_arg(cvar),
		rtcw::to_vm_arg(var_name),
		rtcw::to_vm_arg(value),
		rtcw::to_vm_arg(flags)
	);
}

void trap_Cvar_Update( vmCvar_t *cvar ) {
	syscall(
		UI_CVAR_UPDATE,
		rtcw::to_vm_arg(cvar)
	);
}

void trap_Cvar_Set( const char *var_name, const char *value ) {
	syscall(
		UI_CVAR_SET,
		rtcw::to_vm_arg(var_name),
		rtcw::to_vm_arg(value)
	);
}

float trap_Cvar_VariableValue( const char *var_name ) {
	int temp;
	temp = syscall(
		UI_CVAR_VARIABLEVALUE,
		rtcw::to_vm_arg(var_name)
	);
	return ( *(float*)&temp );
}

void trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	syscall(
		UI_CVAR_VARIABLESTRINGBUFFER,
		rtcw::to_vm_arg(var_name),
		rtcw::to_vm_arg(buffer),
		rtcw::to_vm_arg(bufsize)
	);
}

void trap_Cvar_LatchedVariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	syscall(
		UI_CVAR_LATCHEDVARIABLESTRINGBUFFER,
		rtcw::to_vm_arg(var_name),
		rtcw::to_vm_arg(buffer),
		rtcw::to_vm_arg(bufsize)
	);
}

void trap_Cvar_SetValue( const char *var_name, float value ) {
	syscall(
		UI_CVAR_SETVALUE,
		rtcw::to_vm_arg(var_name),
		rtcw::to_vm_arg(value)
	);
}

void trap_Cvar_Reset( const char *name ) {
	syscall(
		UI_CVAR_RESET,
		rtcw::to_vm_arg(name)
	);
}

void trap_Cvar_Create( const char *var_name, const char *var_value, int flags ) {
	syscall(
		UI_CVAR_CREATE,
		rtcw::to_vm_arg(var_name),
		rtcw::to_vm_arg(var_value),
		rtcw::to_vm_arg(flags)
	);
}

void trap_Cvar_InfoStringBuffer( int bit, char *buffer, int bufsize ) {
	syscall(
		UI_CVAR_INFOSTRINGBUFFER,
		rtcw::to_vm_arg(bit),
		rtcw::to_vm_arg(buffer),
		rtcw::to_vm_arg(bufsize)
	);
}

int trap_Argc( void ) {
	return syscall(UI_ARGC);
}

void trap_Argv( int n, char *buffer, int bufferLength ) {
	syscall(
		UI_ARGV,
		rtcw::to_vm_arg(n),
		rtcw::to_vm_arg(buffer),
		rtcw::to_vm_arg(bufferLength)
	);
}

void trap_Cmd_ExecuteText( int exec_when, const char *text ) {
	syscall(
		UI_CMD_EXECUTETEXT,
		rtcw::to_vm_arg(exec_when),
		rtcw::to_vm_arg(text)
	);
}

void trap_AddCommand( const char *cmdName ) {
	syscall(
		UI_ADDCOMMAND,
		rtcw::to_vm_arg(cmdName)
	);
}

int trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode ) {
	return syscall(
		UI_FS_FOPENFILE,
		rtcw::to_vm_arg(qpath),
		rtcw::to_vm_arg(f),
		rtcw::to_vm_arg(mode)
	);
}

void trap_FS_Read( void *buffer, int len, fileHandle_t f ) {
	syscall(
		UI_FS_READ,
		rtcw::to_vm_arg(buffer),
		rtcw::to_vm_arg(len),
		rtcw::to_vm_arg(f)
	);
}

void trap_FS_Write( const void *buffer, int len, fileHandle_t f ) {
	syscall(
		UI_FS_WRITE,
		rtcw::to_vm_arg(buffer),
		rtcw::to_vm_arg(len),
		rtcw::to_vm_arg(f)
	);
}

void trap_FS_FCloseFile( fileHandle_t f ) {
	syscall(
		UI_FS_FCLOSEFILE,
		rtcw::to_vm_arg(f)
	);
}

int trap_FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize ) {
	return syscall(
		UI_FS_GETFILELIST,
		rtcw::to_vm_arg(path),
		rtcw::to_vm_arg(extension),
		rtcw::to_vm_arg(listbuf),
		rtcw::to_vm_arg(bufsize)
	);
}

int trap_FS_Delete( const char *filename ) {
	return syscall(
		UI_FS_DELETEFILE,
		rtcw::to_vm_arg(filename)
	);
}

qhandle_t trap_R_RegisterModel( const char *name ) {
	return syscall(
		UI_R_REGISTERMODEL,
		rtcw::to_vm_arg(name)
	);
}

qhandle_t trap_R_RegisterSkin( const char *name ) {
	return syscall(
		UI_R_REGISTERSKIN,
		rtcw::to_vm_arg(name)
	);
}

void trap_R_RegisterFont( const char *fontName, int pointSize, fontInfo_t *font ) {
	syscall(
		UI_R_REGISTERFONT,
		rtcw::to_vm_arg(fontName),
		rtcw::to_vm_arg(pointSize),
		rtcw::to_vm_arg(font)
	);
}

qhandle_t trap_R_RegisterShaderNoMip( const char *name ) {
	return syscall(
		UI_R_REGISTERSHADERNOMIP,
		rtcw::to_vm_arg(name)
	);
}

void trap_R_ClearScene( void ) {
	syscall(UI_R_CLEARSCENE);
}

void trap_R_AddRefEntityToScene( const refEntity_t *re ) {
	syscall(
		UI_R_ADDREFENTITYTOSCENE,
		rtcw::to_vm_arg(re)
	);
}

void trap_R_AddPolyToScene( qhandle_t hShader, int numVerts, const polyVert_t *verts ) {
	syscall(
		UI_R_ADDPOLYTOSCENE,
		rtcw::to_vm_arg(hShader),
		rtcw::to_vm_arg(numVerts),
		rtcw::to_vm_arg(verts)
	);
}

// ydnar: new dlight system
//%	void	trap_R_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b, int overdraw ) {
//%		syscall( UI_R_ADDLIGHTTOSCENE, org, rtcw::to_vm_arg(intensity), rtcw::to_vm_arg(r), rtcw::to_vm_arg(g), rtcw::to_vm_arg(b), overdraw );
//%	}
void    trap_R_AddLightToScene( const vec3_t org, float radius, float intensity, float r, float g, float b, qhandle_t hShader, int flags ) {
	syscall(
		UI_R_ADDLIGHTTOSCENE,
		rtcw::to_vm_arg(org),
		rtcw::to_vm_arg(radius),
		rtcw::to_vm_arg(intensity),
		rtcw::to_vm_arg(r),
		rtcw::to_vm_arg(g),
		rtcw::to_vm_arg(b),
		rtcw::to_vm_arg(hShader),
		rtcw::to_vm_arg(flags)
	);
}

void trap_R_AddCoronaToScene( const vec3_t org, float r, float g, float b, float scale, int id, qboolean visible ) {
	syscall(
		UI_R_ADDCORONATOSCENE,
		rtcw::to_vm_arg(org),
		rtcw::to_vm_arg(r),
		rtcw::to_vm_arg(g),
		rtcw::to_vm_arg(b),
		rtcw::to_vm_arg(scale),
		rtcw::to_vm_arg(id),
		rtcw::to_vm_arg(visible)
	);
}

void trap_R_RenderScene( const refdef_t *fd ) {
	syscall(
		UI_R_RENDERSCENE,
		rtcw::to_vm_arg(fd)
	);
}

void trap_R_SetColor( const float *rgba ) {
	syscall(
		UI_R_SETCOLOR,
		rtcw::to_vm_arg(rgba)
	);
}

void trap_R_Add2dPolys( polyVert_t* verts, int numverts, qhandle_t hShader ) {
	syscall(
		UI_R_DRAW2DPOLYS,
		rtcw::to_vm_arg(verts),
		rtcw::to_vm_arg(numverts),
		rtcw::to_vm_arg(hShader)
	);
}

void trap_R_DrawStretchPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader ) {
	syscall(
		UI_R_DRAWSTRETCHPIC,
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

void trap_R_DrawRotatedPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader, float angle ) {
	syscall(
		UI_R_DRAWROTATEDPIC,
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

void trap_R_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs ) {
	syscall(
		UI_R_MODELBOUNDS,
		rtcw::to_vm_arg(model),
		rtcw::to_vm_arg(mins),
		rtcw::to_vm_arg(maxs)
	);
}

void trap_UpdateScreen( void ) {
	syscall(UI_UPDATESCREEN);
}

int trap_CM_LerpTag( orientation_t *tag, const refEntity_t *refent, const char *tagName, int startIndex ) {
	return syscall(
		UI_CM_LERPTAG,
		rtcw::to_vm_arg(tag),
		rtcw::to_vm_arg(refent),
		rtcw::to_vm_arg(tagName),
		rtcw::to_vm_arg(0)
	); // NEFVE - SMF - fixed
}

void trap_S_StartLocalSound( sfxHandle_t sfx, int channelNum ) {
	syscall(
		UI_S_STARTLOCALSOUND,
		rtcw::to_vm_arg(sfx),
		rtcw::to_vm_arg(channelNum),
		rtcw::to_vm_arg(127) // Gordon: default volume always for the moment
	);
}

sfxHandle_t trap_S_RegisterSound( const char *sample, qboolean compressed ) {
	int i = syscall(
		UI_S_REGISTERSOUND,
		rtcw::to_vm_arg(sample),
		rtcw::to_vm_arg(qfalse) // compressed
	);
#ifdef DEBUG
	if ( i == 0 ) {
		Com_Printf( "^1Warning: Failed to load sound: %s\n", sample );
	}
#endif
	return i;
}

void    trap_S_FadeBackgroundTrack( float targetvol, int time, int num ) {   // yes, i know.  fadebackground coming in, fadestreaming going out.  will have to see where functionality leads...
	syscall(
		UI_S_FADESTREAMINGSOUND,
		rtcw::to_vm_arg(targetvol),
		rtcw::to_vm_arg(time),
		rtcw::to_vm_arg(num)
	); // 'num' is '0' if it's music, '1' if it's "all streaming sounds"
}

void    trap_S_FadeAllSound( float targetvol, int time, qboolean stopsound ) {
	syscall(
		UI_S_FADEALLSOUNDS,
		rtcw::to_vm_arg(targetvol),
		rtcw::to_vm_arg(time),
		rtcw::to_vm_arg(stopsound)
	);
}

void trap_Key_KeynumToStringBuf( int keynum, char *buf, int buflen ) {
	syscall(
		UI_KEY_KEYNUMTOSTRINGBUF,
		rtcw::to_vm_arg(keynum),
		rtcw::to_vm_arg(buf),
		rtcw::to_vm_arg(buflen)
	);
}

void trap_Key_GetBindingBuf( int keynum, char *buf, int buflen ) {
	syscall(
		UI_KEY_GETBINDINGBUF,
		rtcw::to_vm_arg(keynum),
		rtcw::to_vm_arg(buf),
		rtcw::to_vm_arg(buflen)
	);
}

// binding MUST be lower case
void trap_Key_KeysForBinding( const char* binding, int* key1, int* key2 ) {
	syscall(
		UI_KEY_BINDINGTOKEYS,
		rtcw::to_vm_arg(binding),
		rtcw::to_vm_arg(key1),
		rtcw::to_vm_arg(key2)
	);
}

void trap_Key_SetBinding( int keynum, const char *binding ) {
	syscall(
		UI_KEY_SETBINDING,
		rtcw::to_vm_arg(keynum),
		rtcw::to_vm_arg(binding)
	);
}

qboolean trap_Key_IsDown( int keynum ) {
	return syscall(
		UI_KEY_ISDOWN,
		rtcw::to_vm_arg(keynum)
	);
}

qboolean trap_Key_GetOverstrikeMode( void ) {
	return syscall(UI_KEY_GETOVERSTRIKEMODE);
}

void trap_Key_SetOverstrikeMode( qboolean state ) {
	syscall(
		UI_KEY_SETOVERSTRIKEMODE,
		rtcw::to_vm_arg(state)
	);
}

void trap_Key_ClearStates( void ) {
	syscall(UI_KEY_CLEARSTATES);
}

int trap_Key_GetCatcher( void ) {
	return syscall(UI_KEY_GETCATCHER);
}

void trap_Key_SetCatcher( int catcher ) {
	syscall(
		UI_KEY_SETCATCHER,
		rtcw::to_vm_arg(catcher)
	);
}

void trap_GetClipboardData( char *buf, int bufsize ) {
	syscall(
		UI_GETCLIPBOARDDATA,
		rtcw::to_vm_arg(buf),
		rtcw::to_vm_arg(bufsize)
	);
}

void trap_GetClientState( uiClientState_t *state ) {
	syscall(
		UI_GETCLIENTSTATE,
		rtcw::to_vm_arg(state)
	);
}

void trap_GetGlconfig( glconfig_t *glconfig ) {
	syscall(
		UI_GETGLCONFIG,
		rtcw::to_vm_arg(glconfig)
	);
}

int trap_GetConfigString( int index, char* buff, int buffsize ) {
	return syscall(
		UI_GETCONFIGSTRING,
		rtcw::to_vm_arg(index),
		rtcw::to_vm_arg(buff),
		rtcw::to_vm_arg(buffsize)
	);
}

int trap_LAN_GetLocalServerCount( void ) {
	return syscall(UI_LAN_GETLOCALSERVERCOUNT);
}

void trap_LAN_GetLocalServerAddressString( int n, char *buf, int buflen ) {
	syscall(
		UI_LAN_GETLOCALSERVERADDRESSSTRING,
		rtcw::to_vm_arg(n),
		rtcw::to_vm_arg(buf),
		rtcw::to_vm_arg(buflen)
	);
}

int trap_LAN_GetGlobalServerCount( void ) {
	return syscall(UI_LAN_GETGLOBALSERVERCOUNT);
}

void trap_LAN_GetGlobalServerAddressString( int n, char *buf, int buflen ) {
	syscall(
		UI_LAN_GETGLOBALSERVERADDRESSSTRING,
		rtcw::to_vm_arg(n),
		rtcw::to_vm_arg(buf),
		rtcw::to_vm_arg(buflen)
	);
}

int trap_LAN_GetPingQueueCount( void ) {
	return syscall(UI_LAN_GETPINGQUEUECOUNT);
}

void trap_LAN_ClearPing( int n ) {
	syscall(
		UI_LAN_CLEARPING,
		rtcw::to_vm_arg(n)
	);
}

void trap_LAN_GetPing( int n, char *buf, int buflen, int *pingtime ) {
	syscall(
		UI_LAN_GETPING,
		rtcw::to_vm_arg(n),
		rtcw::to_vm_arg(buf),
		rtcw::to_vm_arg(buflen),
		rtcw::to_vm_arg(pingtime)
	);
}

void trap_LAN_GetPingInfo( int n, char *buf, int buflen ) {
	syscall(
		UI_LAN_GETPINGINFO,
		rtcw::to_vm_arg(n),
		rtcw::to_vm_arg(buf),
		rtcw::to_vm_arg(buflen)
	);
}

// NERVE - SMF
qboolean trap_LAN_UpdateVisiblePings( int source ) {
	return syscall(
		UI_LAN_UPDATEVISIBLEPINGS,
		rtcw::to_vm_arg(source)
	);
}

int trap_LAN_GetServerCount( int source ) {
	return syscall(
		UI_LAN_GETSERVERCOUNT,
		rtcw::to_vm_arg(source)
	);
}

int trap_LAN_CompareServers( int source, int sortKey, int sortDir, int s1, int s2 ) {
	return syscall(
		UI_LAN_COMPARESERVERS,
		rtcw::to_vm_arg(source),
		rtcw::to_vm_arg(sortKey),
		rtcw::to_vm_arg(sortDir),
		rtcw::to_vm_arg(s1),
		rtcw::to_vm_arg(s2)
	);
}

void trap_LAN_GetServerAddressString( int source, int n, char *buf, int buflen ) {
	syscall(
		UI_LAN_GETSERVERADDRESSSTRING,
		rtcw::to_vm_arg(source),
		rtcw::to_vm_arg(n),
		rtcw::to_vm_arg(buf),
		rtcw::to_vm_arg(buflen)
	);
}

void trap_LAN_GetServerInfo( int source, int n, char *buf, int buflen ) {
	syscall(
		UI_LAN_GETSERVERINFO,
		rtcw::to_vm_arg(source),
		rtcw::to_vm_arg(n),
		rtcw::to_vm_arg(buf),
		rtcw::to_vm_arg(buflen)
	);
}

int trap_LAN_AddServer( int source, const char *name, const char *addr ) {
	return syscall(
		UI_LAN_ADDSERVER,
		rtcw::to_vm_arg(source),
		rtcw::to_vm_arg(name),
		rtcw::to_vm_arg(addr)
	);
}

void trap_LAN_RemoveServer( int source, const char *addr ) {
	syscall(
		UI_LAN_REMOVESERVER,
		rtcw::to_vm_arg(source),
		rtcw::to_vm_arg(addr)
	);
}

int trap_LAN_GetServerPing( int source, int n ) {
	return syscall(
		UI_LAN_GETSERVERPING,
		rtcw::to_vm_arg(source),
		rtcw::to_vm_arg(n)
	);
}

int trap_LAN_ServerIsVisible( int source, int n ) {
	return syscall(
		UI_LAN_SERVERISVISIBLE,
		rtcw::to_vm_arg(source),
		rtcw::to_vm_arg(n)
	);
}

int trap_LAN_ServerStatus( const char *serverAddress, char *serverStatus, int maxLen ) {
	return syscall(
		UI_LAN_SERVERSTATUS,
		rtcw::to_vm_arg(serverAddress),
		rtcw::to_vm_arg(serverStatus),
		rtcw::to_vm_arg(maxLen)
	);
}

qboolean trap_LAN_ServerIsInFavoriteList( int source, int n  ) {
	return syscall(
		UI_LAN_SERVERISINFAVORITELIST,
		rtcw::to_vm_arg(source),
		rtcw::to_vm_arg(n)
	);
}

void trap_LAN_SaveCachedServers() {
	syscall(UI_LAN_SAVECACHEDSERVERS);
}

void trap_LAN_LoadCachedServers() {
	syscall(UI_LAN_LOADCACHEDSERVERS);
}

void trap_LAN_MarkServerVisible( int source, int n, qboolean visible ) {
	syscall(
		UI_LAN_MARKSERVERVISIBLE,
		rtcw::to_vm_arg(source),
		rtcw::to_vm_arg(n),
		rtcw::to_vm_arg(visible)
	);
}

// DHM - Nerve :: PunkBuster
void trap_SetPbClStatus( int status ) {
	syscall(
		UI_SET_PBCLSTATUS,
		rtcw::to_vm_arg(status)
	);
}
// DHM - Nerve

// TTimo: also for Sv
void trap_SetPbSvStatus( int status ) {
	syscall(
		UI_SET_PBSVSTATUS,
		rtcw::to_vm_arg(status)
	);
}

void trap_LAN_ResetPings( int n ) {
	syscall(
		UI_LAN_RESETPINGS,
		rtcw::to_vm_arg(n)
	);
}
// -NERVE - SMF

int trap_MemoryRemaining( void ) {
	return syscall(UI_MEMORY_REMAINING);
}

void trap_GetCDKey( char *buf, int buflen ) {
	syscall(
		UI_GET_CDKEY,
		rtcw::to_vm_arg(buf),
		rtcw::to_vm_arg(buflen)
	);
}

void trap_SetCDKey( char *buf ) {
	syscall(
		UI_SET_CDKEY,
		rtcw::to_vm_arg(buf)
	);
}

int trap_PC_AddGlobalDefine( const char *define ) {
	return syscall(
		UI_PC_ADD_GLOBAL_DEFINE,
		rtcw::to_vm_arg(define)
	);
}

int trap_PC_RemoveAllGlobalDefines( void ) {
	return syscall(UI_PC_REMOVE_ALL_GLOBAL_DEFINES);
}

int trap_PC_LoadSource( const char *filename ) {
	return syscall(
		UI_PC_LOAD_SOURCE,
		rtcw::to_vm_arg(filename)
	);
}

int trap_PC_FreeSource( int handle ) {
	return syscall(
		UI_PC_FREE_SOURCE,
		rtcw::to_vm_arg(handle)
	);
}

int trap_PC_ReadToken( int handle, pc_token_t *pc_token ) {
	return syscall(
		UI_PC_READ_TOKEN,
		rtcw::to_vm_arg(handle),
		rtcw::to_vm_arg(pc_token)
	);
}

int trap_PC_SourceFileAndLine( int handle, char *filename, int *line ) {
	return syscall(
		UI_PC_SOURCE_FILE_AND_LINE,
		rtcw::to_vm_arg(handle),
		rtcw::to_vm_arg(filename),
		rtcw::to_vm_arg(line)
	);
}

int trap_PC_UnReadToken( int handle ) {
	return syscall(
		UI_PC_UNREAD_TOKEN,
		rtcw::to_vm_arg(handle)
	);
}

void trap_S_StopBackgroundTrack( void ) {
	syscall(UI_S_STOPBACKGROUNDTRACK);
}

void trap_S_StartBackgroundTrack( const char *intro, const char *loop, int fadeupTime ) {
	syscall(
		UI_S_STARTBACKGROUNDTRACK,
		rtcw::to_vm_arg(intro),
		rtcw::to_vm_arg(loop),
		rtcw::to_vm_arg(fadeupTime)
	);
}

int trap_RealTime( qtime_t *qtime ) {
	return syscall(
		UI_REAL_TIME,
		rtcw::to_vm_arg(qtime)
	);
}

// this returns a handle.  arg0 is the name in the format "idlogo.roq", set arg1 to NULL, alteredstates to qfalse (do not alter gamestate)
int trap_CIN_PlayCinematic( const char *arg0, int xpos, int ypos, int width, int height, int bits ) {
	return syscall(
		UI_CIN_PLAYCINEMATIC,
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
		UI_CIN_STOPCINEMATIC,
		rtcw::to_vm_arg(handle)
	));
}


// will run a frame of the cinematic but will not draw it.  Will return FMV_EOF if the end of the cinematic has been reached.
e_status trap_CIN_RunCinematic( int handle ) {
	return static_cast<e_status>(syscall(
		UI_CIN_RUNCINEMATIC,
		rtcw::to_vm_arg(handle)
	));
}


// draws the current frame
void trap_CIN_DrawCinematic( int handle ) {
	syscall(
		UI_CIN_DRAWCINEMATIC,
		rtcw::to_vm_arg(handle)
	);
}


// allows you to resize the animation dynamically
void trap_CIN_SetExtents( int handle, int x, int y, int w, int h ) {
	syscall(
		UI_CIN_SETEXTENTS,
		rtcw::to_vm_arg(handle),
		rtcw::to_vm_arg(x),
		rtcw::to_vm_arg(y),
		rtcw::to_vm_arg(w),
		rtcw::to_vm_arg(h)
	);
}


void    trap_R_RemapShader( const char *oldShader, const char *newShader, const char *timeOffset ) {
	syscall(
		UI_R_REMAP_SHADER,
		rtcw::to_vm_arg(oldShader),
		rtcw::to_vm_arg(newShader),
		rtcw::to_vm_arg(timeOffset)
	);
}

qboolean trap_VerifyCDKey( const char *key, const char *chksum ) {
	return syscall(
		UI_VERIFY_CDKEY,
		rtcw::to_vm_arg(key),
		rtcw::to_vm_arg(chksum)
	);
}

// NERVE - SMF
qboolean trap_GetLimboString( int index, char *buf ) {
	return syscall(
		UI_CL_GETLIMBOSTRING,
		rtcw::to_vm_arg(index),
		rtcw::to_vm_arg(buf)
	);
}

#define MAX_VA_STRING       32000

char* trap_TranslateString( const char *string ) {
	static char staticbuf[2][MAX_VA_STRING];
	static int bufcount = 0;
	char *buf;

	buf = staticbuf[bufcount++ % 2];

#ifdef LOCALIZATION_SUPPORT
	syscall(
		UI_CL_TRANSLATE_STRING,
		rtcw::to_vm_arg(string),
		rtcw::to_vm_arg(buf)
	);
#else
	Q_strncpyz( buf, string, MAX_VA_STRING );
#endif // LOCALIZATION_SUPPORT
	return buf;
}
// -NERVE - SMF

// DHM - Nerve
void trap_CheckAutoUpdate( void ) {
	syscall(UI_CHECKAUTOUPDATE);
}

void trap_GetAutoUpdate( void ) {
	syscall(UI_GET_AUTOUPDATE);
}
// DHM - Nerve

void trap_openURL( const char *s ) {
	syscall(
		UI_OPENURL,
		rtcw::to_vm_arg(s)
	);
}

void trap_GetHunkData( int* hunkused, int* hunkexpected ) {
	syscall(
		UI_GETHUNKDATA,
		rtcw::to_vm_arg(hunkused),
		rtcw::to_vm_arg(hunkexpected)
	);
}

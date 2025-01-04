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


// Former net_wins.c


#include "SDL_net.h"
#include "q_shared.h"
#include "qcommon.h"


namespace {


bool sockInitialized = false;
bool networkingEnabled = false;

cvar_t* net_noudp;
cvar_t* net_noipx;

cvar_t* net_socksEnabled;
cvar_t* net_socksServer;
cvar_t* net_socksPort;
cvar_t* net_socksUsername;
cvar_t* net_socksPassword;

UDPsocket ip_socket;

const int MAX_IPS = 16;
int numIP;
byte localIP[MAX_IPS][4];
UDPpacket udp_packet;


void NetadrToSockadr(
	const netadr_t& a,
	IPaddress& s)
{
	s.host = INADDR_ANY;
	s.port = 0;

	if (a.type == NA_BROADCAST) {
		s.host = INADDR_BROADCAST;
		s.port = a.port;
	} else if (a.type == NA_IP) {
		Uint32 host = 0;

		for (int i = 0; i < 4; ++i) {
			host <<= 8;
			host |= a.ip[3 - i];
		}

		s.host = host;
		s.port = a.port;
	}
}

void SockadrToNetadr(
	const IPaddress& s,
	netadr_t& a)
{
	a.type = NA_IP;
	a.port = s.port;

	Uint32 host = s.host;

	for (int i = 0; i < 4; ++i) {
		a.ip[i] = static_cast<byte>(host & 0xFF);
		host >>= 8;
	}
}

// idnewt
// 192.246.40.70
// 12121212.121212121212
bool Sys_StringToSockaddr(
	const char *s,
	IPaddress& sadr)
{
	int sdl_result = SDLNet_ResolveHost(
		&sadr,
		s,
		0);

	if (sdl_result == 0) {
		return true;
	} else {
		sadr.host = INADDR_ANY;
		sadr.port = 0;

		return false;
	}
}

UDPsocket NET_IPSocket(
	const char* net_interface,
	int port)
{
	if (net_interface) {
		Com_Printf(
			"Opening IP socket: %s:%i\n",
			net_interface, port);
	} else {
		Com_Printf(
			"Opening IP socket: localhost:%i\n",
			port);
	}

	bool is_succeed = true;
	UDPsocket udp_socket = nullptr;

	if (is_succeed) {
		udp_socket = SDLNet_UDP_Open(
			static_cast<Uint16>(port));

		if (!udp_socket) {
			is_succeed = false;

			Com_Printf(
				"WARNING: UDP_OpenSocket: socket: %s\n",
				SDLNet_GetError());
		}
	}

	IPaddress ip_address;

	if (is_succeed) {
		if (!net_interface ||
			net_interface[0] == '\0' ||
			!::Q_stricmp(net_interface, "localhost"))
		{
			ip_address.host = INADDR_ANY;
		} else {
			Sys_StringToSockaddr(
				net_interface,
				ip_address);
		}

		if (port == PORT_ANY) {
			ip_address.port = 0;
		} else {
			SDLNet_Write16(
				static_cast<Uint16>(port),
				&ip_address.port);
		}
	}

	if (is_succeed) {
	} else {
		if (udp_socket) {
			SDLNet_UDP_Close(
				udp_socket);

			udp_socket = nullptr;
		}
	}

	return udp_socket;
}

void NET_GetLocalAddress()
{
	int sdl_result = 0;
	IPaddress current_address;

	sdl_result = SDLNet_ResolveHost(
		&current_address,
		nullptr,
		0);

	const char* host_name = SDLNet_ResolveIP(
		&current_address);

	if (!host_name) {
		host_name = "localhost";
	}

	Com_Printf(
		"Hostname: %s\n",
		host_name);

	IPaddress addresses[MAX_IPS];

	numIP = SDLNet_GetLocalAddresses(
		addresses,
		MAX_IPS);

	for (int i = 0; i < numIP; ++i) {
		const IPaddress& address = addresses[i];

		localIP[i][0] = static_cast<byte>((address.host >>  0) & 0xFF);
		localIP[i][1] = static_cast<byte>((address.host >>  8) & 0xFF);
		localIP[i][2] = static_cast<byte>((address.host >> 16) & 0xFF);
		localIP[i][3] = static_cast<byte>((address.host >> 24) & 0xFF);

		Com_Printf(
			"IP: %i.%i.%i.%i\n",
			localIP[i][0],
			localIP[i][1],
			localIP[i][2],
			localIP[i][3]);
	}
}

void NET_OpenIP()
{
	cvar_t* ip = Cvar_Get(
		"net_ip",
		"localhost",
		CVAR_LATCH);

	int port = Cvar_Get(
		"net_port",
		va("%i", PORT_SERVER),
		CVAR_LATCH)->integer;

	// automatically scan for a valid port, so multiple
	// dedicated servers can be started without requiring
	// a different net_port for each one
	for (int i = 0; i < 10; ++i) {
		ip_socket = NET_IPSocket(
			ip->string,
			port + i);

		if (ip_socket) {
			Cvar_SetValue(
				"net_port",
				static_cast<float>(port + i));

			NET_GetLocalAddress();

			return;
		}
	}

	Com_Printf("WARNING: Couldn't allocate IP port\n");
}

bool NET_GetCvars()
{
	bool modified = false;

	if (net_noudp && net_noudp->modified) {
		modified = true;
	}

	net_noudp = Cvar_Get(
		"net_noudp",
		"0",
		CVAR_LATCH | CVAR_ARCHIVE);

	return modified;
}

void NET_Config(
	bool enableNetworking)
{
	bool stop = false;
	bool start = false;

	// get any latched changes to cvars
	bool modified = NET_GetCvars();

	if (net_noudp->integer != 0) {
		enableNetworking = false;
	}

	// if enable state is the same and no cvars were modified, we have nothing to do
	if (enableNetworking == networkingEnabled && !modified) {
		return;
	}

	if (enableNetworking == networkingEnabled) {
		if (enableNetworking) {
			stop = true;
			start = true;
		} else {
			stop = false;
			start = false;
		}
	} else {
		if (enableNetworking) {
			stop = false;
			start = true;
		} else {
			stop = true;
			start = false;
		}

		networkingEnabled = enableNetworking;
	}

	if (stop) {
		if (ip_socket) {
			SDLNet_UDP_Close(
				ip_socket);

			ip_socket = nullptr;
		}
	}

	if (start) {
		if (net_noudp->integer == 0) {
			NET_OpenIP();
		}
	}
}


} // namespace


void NET_Init()
{
#ifndef RTCW_SP
	int init_result = SDLNet_Init();

	if (init_result != 0) {
		Com_Printf(
			"WARNING: Failed to initialize SDL_net: %s\n",
			SDLNet_GetError());

		return;
	}

	udp_packet.len = 0;
	udp_packet.status = 0;
	udp_packet.maxlen = MAX_MSGLEN;

	sockInitialized = true;

	Com_Printf("SDL_net initialized.\n");

	// this is really just to get the cvars registered
	NET_GetCvars();

	//FIXME testing!
	NET_Config(
		true);
#endif // RTCW_XX
}

void NET_Shutdown()
{
	if (!::sockInitialized) {
		return;
	}

	NET_Config(
		false);

	SDLNet_Quit();

	sockInitialized = false;
}

// sleeps msec or until net socket is ready
void NET_Sleep(
	int msec)
{
	static_cast<void>(msec);
}

void NET_Restart()
{
	NET_Config(
		networkingEnabled);
}

// idnewt
// 192.246.40.70
bool Sys_StringToAdr(
	const char* s,
	netadr_t* a)
{
	IPaddress sadr;

	if (!::Sys_StringToSockaddr(s, sadr)) {
		return false;
	}

	SockadrToNetadr(sadr, *a);

	return true;
}

// Never called by the game logic, just the system event queing
bool Sys_GetPacket(
	netadr_t* net_from,
	msg_t* net_message)
{
	if (!::ip_socket) {
		return false;
	}

	udp_packet.data = net_message->data;

	int sdl_result = SDLNet_UDP_Recv(
		ip_socket,
		&::udp_packet);

	if (sdl_result == 1) {
		SockadrToNetadr(
			udp_packet.address,
			*net_from);

		net_message->readcount = 0;

		if (udp_packet.len > net_message->maxsize) {
			Com_Printf(
				"Oversize packet from %s\n",
				NET_AdrToString(*net_from));

			return false;
		}

		net_message->cursize = udp_packet.len;

		return true;
	} else if (sdl_result == -1) {
		Com_Printf(
			"NET_GetPacket: %s\n",
			SDLNet_GetError());

		return false;
	} else {
		return false;
	}
}

void Sys_SendPacket(
	int length,
	const void* data,
	netadr_t to)
{
	if (to.type != NA_BROADCAST &&
		to.type != NA_BROADCAST_IPX &&
		to.type != NA_IP)
	{
		Com_Error(
			ERR_FATAL,
			"Sys_SendPacket: bad address type");

		return;
	}

	if (!::ip_socket) {
		return;
	}

	IPaddress addr;

	NetadrToSockadr(
		to,
		addr);

	udp_packet.address = addr;
	udp_packet.len = length;
	udp_packet.data = static_cast<Uint8*>(const_cast<void*>(data));

	int sdl_result = SDLNet_UDP_Send(
		ip_socket,
		-1,
		&::udp_packet);

	if (sdl_result != 1) {
		Com_Printf(
			"NET_SendPacket: %s\n",
			SDLNet_GetError());
	}
}

// LAN clients will have their rate var ignored
bool Sys_IsLANAddress(
	netadr_t adr)
{
	if (adr.type == NA_LOOPBACK) {
		return true;
	}

	if (adr.type != NA_IP) {
		return false;
	}


	// choose which comparison to use based on the class of the
	// address being tested any local adresses of a different class than
	// the address being tested will fail based on the first byte

	if (adr.ip[0] == 127 &&
		adr.ip[1] == 0 &&
		adr.ip[2] == 0 &&
		adr.ip[3] == 1)
	{
		return true;
	}

	// Class A
	if ((adr.ip[0] & 0x80) == 0x00) {
		for (int i = 0; i < numIP; ++i) {
			if (adr.ip[0] == localIP[i][0]) {
				return true;
			}
		}
		// the RFC1918 class a block will pass the above test
		return false;
	}

	// Class B
	if ((adr.ip[0] & 0xc0) == 0x80) {
		for (int i = 0; i < numIP; ++i) {
			if (adr.ip[0] == localIP[i][0] &&
				adr.ip[1] == localIP[i][1])
			{
				return true;
			}

			// also check against the RFC1918 class b blocks
			if (adr.ip[0] == 172 &&
				localIP[i][0] == 172 &&
				(adr.ip[1] & 0xF0) == 16 &&
				(localIP[i][1] & 0xF0) == 16)
			{
				return true;
			}
		}

		return false;
	}

	// Class C
	for (int i = 0; i < numIP; ++i) {
		if (adr.ip[0] == localIP[i][0] &&
			adr.ip[1] == localIP[i][1] &&
			adr.ip[2] == localIP[i][2])
		{
			return true;
		}

		// also check against the RFC1918 class c blocks
		if (adr.ip[0] == 192 && 
			localIP[i][0] == 192 &&
			adr.ip[1] == 168 &&
			localIP[i][1] == 168)
		{
			return true;
		}
	}

	return false;
}

void Sys_ShowIP()
{
	for (int i = 0; i < ++numIP; ++i) {
		Com_Printf(
			"IP: %i.%i.%i.%i\n",
			localIP[i][0],
			localIP[i][1],
			localIP[i][2],
			localIP[i][3]);
	}
}

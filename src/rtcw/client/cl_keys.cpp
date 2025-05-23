/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


#include "client.h"

#include "rtcw_vm_args.h"


/*

key up events are sent even if in console mode

*/

field_t historyEditLines[COMMAND_HISTORY];

int nextHistoryLine;                // the last line in the history buffer, not masked
int historyLine;            // the line being displayed from history buffer
							// will be <= nextHistoryLine

field_t g_consoleField;
field_t chatField;
qboolean chat_team;

#if !defined RTCW_ET
qboolean chat_limbo;            // NERVE - SMF

int chat_playerNum;
#endif // RTCW_XX

#if defined RTCW_ET
qboolean chat_buddy;
#endif // RTCW_XX

qboolean key_overstrikeMode;

qboolean anykeydown;
qkey_t keys[MAX_KEYS];


typedef struct {
	const char    *name;
	int keynum;
} keyname_t;

qboolean UI_checkKeyExec( int key );        // NERVE - SMF

#if defined RTCW_ET
qboolean CL_CGameCheckKeyExec( int key );
#endif // RTCW_XX

// names not in this list can either be lowercase ascii, or '0xnn' hex sequences
keyname_t keynames[] =
{
	{"TAB", K_TAB},
	{"ENTER", K_ENTER},
	{"ESCAPE", K_ESCAPE},
	{"SPACE", K_SPACE},
	{"BACKSPACE", K_BACKSPACE},
	{"UPARROW", K_UPARROW},
	{"DOWNARROW", K_DOWNARROW},
	{"LEFTARROW", K_LEFTARROW},
	{"RIGHTARROW", K_RIGHTARROW},

	{"ALT", K_ALT},
	{"CTRL", K_CTRL},
	{"SHIFT", K_SHIFT},

	{"CAPSLOCK", K_CAPSLOCK},


	{"F1", K_F1},
	{"F2", K_F2},
	{"F3", K_F3},
	{"F4", K_F4},
	{"F5", K_F5},
	{"F6", K_F6},
	{"F7", K_F7},
	{"F8", K_F8},
	{"F9", K_F9},
	{"F10", K_F10},
	{"F11", K_F11},
	{"F12", K_F12},

	{"INS", K_INS},
	{"DEL", K_DEL},
	{"PGDN", K_PGDN},
	{"PGUP", K_PGUP},
	{"HOME", K_HOME},
	{"END", K_END},

	{"MOUSE1", K_MOUSE1},
	{"MOUSE2", K_MOUSE2},
	{"MOUSE3", K_MOUSE3},
	{"MOUSE4", K_MOUSE4},
	{"MOUSE5", K_MOUSE5},

	{"MWHEELUP", K_MWHEELUP },
	{"MWHEELDOWN",   K_MWHEELDOWN },

	{"JOY1", K_JOY1},
	{"JOY2", K_JOY2},
	{"JOY3", K_JOY3},
	{"JOY4", K_JOY4},
	{"JOY5", K_JOY5},
	{"JOY6", K_JOY6},
	{"JOY7", K_JOY7},
	{"JOY8", K_JOY8},
	{"JOY9", K_JOY9},
	{"JOY10", K_JOY10},
	{"JOY11", K_JOY11},
	{"JOY12", K_JOY12},
	{"JOY13", K_JOY13},
	{"JOY14", K_JOY14},
	{"JOY15", K_JOY15},
	{"JOY16", K_JOY16},
	{"JOY17", K_JOY17},
	{"JOY18", K_JOY18},
	{"JOY19", K_JOY19},
	{"JOY20", K_JOY20},
	{"JOY21", K_JOY21},
	{"JOY22", K_JOY22},
	{"JOY23", K_JOY23},
	{"JOY24", K_JOY24},
	{"JOY25", K_JOY25},
	{"JOY26", K_JOY26},
	{"JOY27", K_JOY27},
	{"JOY28", K_JOY28},
	{"JOY29", K_JOY29},
	{"JOY30", K_JOY30},
	{"JOY31", K_JOY31},
	{"JOY32", K_JOY32},

	{"AUX1", K_AUX1},
	{"AUX2", K_AUX2},
	{"AUX3", K_AUX3},
	{"AUX4", K_AUX4},
	{"AUX5", K_AUX5},
	{"AUX6", K_AUX6},
	{"AUX7", K_AUX7},
	{"AUX8", K_AUX8},
	{"AUX9", K_AUX9},
	{"AUX10", K_AUX10},
	{"AUX11", K_AUX11},
	{"AUX12", K_AUX12},
	{"AUX13", K_AUX13},
	{"AUX14", K_AUX14},
	{"AUX15", K_AUX15},
	{"AUX16", K_AUX16},

	{"KP_HOME",          K_KP_HOME },
	{"KP_UPARROW",       K_KP_UPARROW },
	{"KP_PGUP",          K_KP_PGUP },
	{"KP_LEFTARROW", K_KP_LEFTARROW },
	{"KP_5",         K_KP_5 },
	{"KP_RIGHTARROW",    K_KP_RIGHTARROW },
	{"KP_END",           K_KP_END },
	{"KP_DOWNARROW", K_KP_DOWNARROW },
	{"KP_PGDN",          K_KP_PGDN },
	{"KP_ENTER",     K_KP_ENTER },
	{"KP_INS",           K_KP_INS },
	{"KP_DEL",           K_KP_DEL },
	{"KP_SLASH",     K_KP_SLASH },
	{"KP_MINUS",     K_KP_MINUS },
	{"KP_PLUS",          K_KP_PLUS },
	{"KP_NUMLOCK",       K_KP_NUMLOCK },
	{"KP_STAR",          K_KP_STAR },
	{"KP_EQUALS",        K_KP_EQUALS },

	{"PAUSE", K_PAUSE},

	{"SEMICOLON", ';'},   // because a raw semicolon seperates commands

	{"COMMAND", K_COMMAND},  //mac

	{NULL,0}
};

keyname_t keynames_d[] =    //deutsch
{
	{"TAB", K_TAB},
	{"EINGABETASTE", K_ENTER},
	{"ESC", K_ESCAPE},
	{"LEERTASTE", K_SPACE},
	{"R" "\xDC" "CKTASTE", K_BACKSPACE}, // "RÜCKTASTE"
	{"PFEILT.AUF", K_UPARROW},
	{"PFEILT.UNTEN", K_DOWNARROW},
	{"PFEILT.LINKS", K_LEFTARROW},
	{"PFEILT.RECHTS", K_RIGHTARROW},

	{"ALT", K_ALT},
	{"STRG", K_CTRL},

#if defined RTCW_SP
	{"UMSCHALT", K_SHIFT},   // (SA) removed one 'L' for laird 11/15/01
#else
	{"UMSCHALLT", K_SHIFT},
#endif // RTCW_XX

	{"FESTSTELLT", K_CAPSLOCK},

	{"F1", K_F1},
	{"F2", K_F2},
	{"F3", K_F3},
	{"F4", K_F4},
	{"F5", K_F5},
	{"F6", K_F6},
	{"F7", K_F7},
	{"F8", K_F8},
	{"F9", K_F9},
	{"F10", K_F10},
	{"F11", K_F11},
	{"F12", K_F12},

	{"EINFG", K_INS},
	{"ENTF", K_DEL},
	{"BILD-AB", K_PGDN},
	{"BILD-AUF", K_PGUP},
	{"POS1", K_HOME},
	{"ENDE", K_END},

	{"MAUS1", K_MOUSE1},
	{"MAUS2", K_MOUSE2},
	{"MAUS3", K_MOUSE3},
	{"MAUS4", K_MOUSE4},
	{"MAUS5", K_MOUSE5},

	{"MRADOBEN", K_MWHEELUP },
	{"MRADUNTEN",    K_MWHEELDOWN },

	{"JOY1", K_JOY1},
	{"JOY2", K_JOY2},
	{"JOY3", K_JOY3},
	{"JOY4", K_JOY4},
	{"JOY5", K_JOY5},
	{"JOY6", K_JOY6},
	{"JOY7", K_JOY7},
	{"JOY8", K_JOY8},
	{"JOY9", K_JOY9},
	{"JOY10", K_JOY10},
	{"JOY11", K_JOY11},
	{"JOY12", K_JOY12},
	{"JOY13", K_JOY13},
	{"JOY14", K_JOY14},
	{"JOY15", K_JOY15},
	{"JOY16", K_JOY16},
	{"JOY17", K_JOY17},
	{"JOY18", K_JOY18},
	{"JOY19", K_JOY19},
	{"JOY20", K_JOY20},
	{"JOY21", K_JOY21},
	{"JOY22", K_JOY22},
	{"JOY23", K_JOY23},
	{"JOY24", K_JOY24},
	{"JOY25", K_JOY25},
	{"JOY26", K_JOY26},
	{"JOY27", K_JOY27},
	{"JOY28", K_JOY28},
	{"JOY29", K_JOY29},
	{"JOY30", K_JOY30},
	{"JOY31", K_JOY31},
	{"JOY32", K_JOY32},

	{"AUX1", K_AUX1},
	{"AUX2", K_AUX2},
	{"AUX3", K_AUX3},
	{"AUX4", K_AUX4},
	{"AUX5", K_AUX5},
	{"AUX6", K_AUX6},
	{"AUX7", K_AUX7},
	{"AUX8", K_AUX8},
	{"AUX9", K_AUX9},
	{"AUX10", K_AUX10},
	{"AUX11", K_AUX11},
	{"AUX12", K_AUX12},
	{"AUX13", K_AUX13},
	{"AUX14", K_AUX14},
	{"AUX15", K_AUX15},
	{"AUX16", K_AUX16},

	{"ZB_POS1",          K_KP_HOME },
	{"ZB_PFEILT.AUF",    K_KP_UPARROW },
	{"ZB_BILD-AUF",      K_KP_PGUP },
	{"ZB_PFEILT.LINKS",  K_KP_LEFTARROW },
	{"ZB_5",         K_KP_5 },
	{"ZB_PFEILT.RECHTS",K_KP_RIGHTARROW },
	{"ZB_ENDE",          K_KP_END },
	{"ZB_PFEILT.UNTEN",  K_KP_DOWNARROW },
	{"ZB_BILD-AB",       K_KP_PGDN },
	{"ZB_ENTER",     K_KP_ENTER },
	{"ZB_EINFG",     K_KP_INS },
	{"ZB_ENTF",          K_KP_DEL },
	{"ZB_SLASH",     K_KP_SLASH },
	{"ZB_MINUS",     K_KP_MINUS },
	{"ZB_PLUS",          K_KP_PLUS },
	{"ZB_NUM",           K_KP_NUMLOCK },
	{"ZB_*",         K_KP_STAR },
	{"ZB_EQUALS",        K_KP_EQUALS },

	{"PAUSE", K_PAUSE},

	{"COMMAND", K_COMMAND},  //mac
	{NULL,0}
};  //end german

keyname_t keynames_f[] =    //french
{
	{"TAB", K_TAB},
	{"ENTREE",   K_ENTER},
	{"ECHAP",    K_ESCAPE},
	{"ESPACE",   K_SPACE},
	{"RETOUR",   K_BACKSPACE},
	{"HAUT", K_UPARROW},
	{"BAS",      K_DOWNARROW},
	{"GAUCHE",   K_LEFTARROW},
	{"DROITE",   K_RIGHTARROW},

	{"ALT",      K_ALT},
	{"CTRL", K_CTRL},
	{"MAJ",      K_SHIFT},

	{"VERRMAJ", K_CAPSLOCK},

	{"F1", K_F1},
	{"F2", K_F2},
	{"F3", K_F3},
	{"F4", K_F4},
	{"F5", K_F5},
	{"F6", K_F6},
	{"F7", K_F7},
	{"F8", K_F8},
	{"F9", K_F9},
	{"F10", K_F10},
	{"F11", K_F11},
	{"F12", K_F12},

	{"INSER", K_INS},
	{"SUPPR", K_DEL},
	{"PGBAS", K_PGDN},
	{"PGHAUT", K_PGUP},
	{"ORIGINE", K_HOME},
	{"FIN", K_END},

	{"SOURIS1", K_MOUSE1},
	{"SOURIS2", K_MOUSE2},
	{"SOURIS3", K_MOUSE3},
	{"SOURIS4", K_MOUSE4},
	{"SOURIS5", K_MOUSE5},

	{"MOLETTEHT.",   K_MWHEELUP },
	{"MOLETTEBAS",   K_MWHEELDOWN },

	{"JOY1", K_JOY1},
	{"JOY2", K_JOY2},
	{"JOY3", K_JOY3},
	{"JOY4", K_JOY4},
	{"JOY5", K_JOY5},
	{"JOY6", K_JOY6},
	{"JOY7", K_JOY7},
	{"JOY8", K_JOY8},
	{"JOY9", K_JOY9},
	{"JOY10", K_JOY10},
	{"JOY11", K_JOY11},
	{"JOY12", K_JOY12},
	{"JOY13", K_JOY13},
	{"JOY14", K_JOY14},
	{"JOY15", K_JOY15},
	{"JOY16", K_JOY16},
	{"JOY17", K_JOY17},
	{"JOY18", K_JOY18},
	{"JOY19", K_JOY19},
	{"JOY20", K_JOY20},
	{"JOY21", K_JOY21},
	{"JOY22", K_JOY22},
	{"JOY23", K_JOY23},
	{"JOY24", K_JOY24},
	{"JOY25", K_JOY25},
	{"JOY26", K_JOY26},
	{"JOY27", K_JOY27},
	{"JOY28", K_JOY28},
	{"JOY29", K_JOY29},
	{"JOY30", K_JOY30},
	{"JOY31", K_JOY31},
	{"JOY32", K_JOY32},

	{"AUX1", K_AUX1},
	{"AUX2", K_AUX2},
	{"AUX3", K_AUX3},
	{"AUX4", K_AUX4},
	{"AUX5", K_AUX5},
	{"AUX6", K_AUX6},
	{"AUX7", K_AUX7},
	{"AUX8", K_AUX8},
	{"AUX9", K_AUX9},
	{"AUX10", K_AUX10},
	{"AUX11", K_AUX11},
	{"AUX12", K_AUX12},
	{"AUX13", K_AUX13},
	{"AUX14", K_AUX14},
	{"AUX15", K_AUX15},
	{"AUX16", K_AUX16},

	{"PN_ORIGINE",       K_KP_HOME },
	{"PN_HAUT",          K_KP_UPARROW },
	{"PN_PGBAS",     K_KP_PGUP },
	{"PN_GAUCHE",        K_KP_LEFTARROW },
	{"PN_5",         K_KP_5 },
	{"PN_DROITE",        K_KP_RIGHTARROW },
	{"PN_FIN",           K_KP_END },
	{"PN_BAS",           K_KP_DOWNARROW },
	{"PN_PGBAS",     K_KP_PGDN },
	{"PN_ENTR",          K_KP_ENTER },
	{"PN_INSER",     K_KP_INS },
	{"PN_SUPPR",     K_KP_DEL },
	{"PN_SLASH",     K_KP_SLASH },
	{"PN_MOINS",     K_KP_MINUS },
	{"PN_PLUS",          K_KP_PLUS },
	{"PN_VERRNUM",       K_KP_NUMLOCK },
	{"PN_*",         K_KP_STAR },
	{"PN_EQUALS",        K_KP_EQUALS },

	{"PAUSE", K_PAUSE},

	{"COMMAND", K_COMMAND},  //mac

	{NULL,0}
};  //end french

#if !defined RTCW_ET
keyname_t keynames_s[] =  //Spanish - Updated 11/5
#else
keyname_t keynames_s[] =  //Spanish
#endif // RTCW_XX

{
	{"TABULADOR", K_TAB},
	{"INTRO", K_ENTER},
	{"ESC", K_ESCAPE},
	{"BARRA_ESPACIAD", K_SPACE},
	{"RETROCESO", K_BACKSPACE},
	{"CURSOR_ARRIBA", K_UPARROW},
	{"CURSOR_ABAJO", K_DOWNARROW},
	{"CURSOR_IZQDA", K_LEFTARROW},
	{"CURSOR_DERECHA", K_RIGHTARROW},

	{"ALT", K_ALT},
	{"CTRL", K_CTRL},
	{"MAY" "\xDA" "S", K_SHIFT}, // "MAYÚS"

	{"BLOQ_MAY" "\xDA" "S", K_CAPSLOCK}, // "BLOQ_MAYÚS"

	{"F1", K_F1},
	{"F2", K_F2},
	{"F3", K_F3},
	{"F4", K_F4},
	{"F5", K_F5},
	{"F6", K_F6},
	{"F7", K_F7},
	{"F8", K_F8},
	{"F9", K_F9},
	{"F10", K_F10},
	{"F11", K_F11},
	{"F12", K_F12},

	{"INSERT", K_INS},
	{"SUPR", K_DEL},
	{"AV_P" "\xC1" "G", K_PGDN}, // "AV_PÁG"
	{"RE_P" "\xC1" "G", K_PGUP}, // "RE_PÁG"
	{"INICIO", K_HOME},
	{"FIN", K_END},

	{"RAT" "\xD3" "N1", K_MOUSE1}, // "RATÓN1"
	{"RAT" "\xD3" "N2", K_MOUSE2}, // "RATÓN2"
	{"RAT" "\xD3" "N3", K_MOUSE3}, // "RATÓN3"
	{"RAT" "\xD3" "N4", K_MOUSE4}, // "RATÓN4"
	{"RAT" "\xD3" "N5", K_MOUSE5}, // "RATÓN5"

	{"RUEDA_HACIA_ARRIBA",   K_MWHEELUP },
	{"RUEDA_HACIA_ABAJO",    K_MWHEELDOWN },

	{"JOY1", K_JOY1},
	{"JOY2", K_JOY2},
	{"JOY3", K_JOY3},
	{"JOY4", K_JOY4},
	{"JOY5", K_JOY5},
	{"JOY6", K_JOY6},
	{"JOY7", K_JOY7},
	{"JOY8", K_JOY8},
	{"JOY9", K_JOY9},
	{"JOY10", K_JOY10},
	{"JOY11", K_JOY11},
	{"JOY12", K_JOY12},
	{"JOY13", K_JOY13},
	{"JOY14", K_JOY14},
	{"JOY15", K_JOY15},
	{"JOY16", K_JOY16},
	{"JOY17", K_JOY17},
	{"JOY18", K_JOY18},
	{"JOY19", K_JOY19},
	{"JOY20", K_JOY20},
	{"JOY21", K_JOY21},
	{"JOY22", K_JOY22},
	{"JOY23", K_JOY23},
	{"JOY24", K_JOY24},
	{"JOY25", K_JOY25},
	{"JOY26", K_JOY26},
	{"JOY27", K_JOY27},
	{"JOY28", K_JOY28},
	{"JOY29", K_JOY29},
	{"JOY30", K_JOY30},
	{"JOY31", K_JOY31},
	{"JOY32", K_JOY32},

	{"AUX1", K_AUX1},
	{"AUX2", K_AUX2},
	{"AUX3", K_AUX3},
	{"AUX4", K_AUX4},
	{"AUX5", K_AUX5},
	{"AUX6", K_AUX6},
	{"AUX7", K_AUX7},
	{"AUX8", K_AUX8},
	{"AUX9", K_AUX9},
	{"AUX10", K_AUX10},
	{"AUX11", K_AUX11},
	{"AUX12", K_AUX12},
	{"AUX13", K_AUX13},
	{"AUX14", K_AUX14},
	{"AUX15", K_AUX15},
	{"AUX16", K_AUX16},

	{"INICIO(NUM)",          K_KP_HOME },
	{"ARRIBA(NUM)",      K_KP_UPARROW },
	{"RE_P" "\xC1" "G(NUM)",          K_KP_PGUP }, // "RE_PÁG(NUM)"
	{"IZQUIERDA(NUM)",   K_KP_LEFTARROW },
	{"5(NUM)",           K_KP_5 },
	{"DERECHA(NUM)", K_KP_RIGHTARROW },
	{"FIN(NUM)",         K_KP_END },
	{"ABAJO(NUM)",   K_KP_DOWNARROW },
	{"AV_P" "\xC1" "G(NUM)",          K_KP_PGDN }, // "AV_PÁG(NUM)"
	{"INTRO(NUM)",       K_KP_ENTER },
	{"INS(NUM)",         K_KP_INS },
	{"SUPR(NUM)",            K_KP_DEL },
	{"/(NUM)",       K_KP_SLASH },
	{"-(NUM)",       K_KP_MINUS },
	{"+(NUM)",           K_KP_PLUS },
	{"BLOQ_NUM",     K_KP_NUMLOCK },
	{"*(NUM)",           K_KP_STAR },
	{"INTRO(NUM)",       K_KP_EQUALS },

	{"PAUSA", K_PAUSE},

	{"PUNTO_Y_COMA", ';'},    // because a raw semicolon seperates commands

	{"COMANDO", K_COMMAND},  //mac

	{NULL,0}
};

keyname_t keynames_i[] =  //Italian
{
	{"TAB", K_TAB},
	{"INVIO", K_ENTER},
	{"ESC", K_ESCAPE},
	{"SPAZIO", K_SPACE},
	{"BACKSPACE", K_BACKSPACE},
	{"FRECCIASU", K_UPARROW},
	{"FRECCIAGI" "\xD9", K_DOWNARROW}, // "FRECCIAGIÙ"
	{"FRECCIASX", K_LEFTARROW},
	{"FRECCIADX", K_RIGHTARROW},

	{"ALT", K_ALT},
	{"CTRL", K_CTRL},
	{"MAIUSC", K_SHIFT},

	{"BLOCMAIUSC", K_CAPSLOCK},

	{"F1", K_F1},
	{"F2", K_F2},
	{"F3", K_F3},
	{"F4", K_F4},
	{"F5", K_F5},
	{"F6", K_F6},
	{"F7", K_F7},
	{"F8", K_F8},
	{"F9", K_F9},
	{"F10", K_F10},
	{"F11", K_F11},
	{"F12", K_F12},

	{"INS", K_INS},
	{"CANC", K_DEL},
	{"PAGGI" "\xD9", K_PGDN}, // "PAGGIÙ"
	{"PAGGSU", K_PGUP},
	{"HOME", K_HOME},
	{"FINE", K_END},

	{"MOUSE1", K_MOUSE1},
	{"MOUSE2", K_MOUSE2},
	{"MOUSE3", K_MOUSE3},
	{"MOUSE4", K_MOUSE4},
	{"MOUSE5", K_MOUSE5},

	{"ROTELLASU",    K_MWHEELUP },
	{"ROTELLAGI" "\xD9",   K_MWHEELDOWN }, // "ROTELLAGIÙ"

	{"JOY1", K_JOY1},
	{"JOY2", K_JOY2},
	{"JOY3", K_JOY3},
	{"JOY4", K_JOY4},
	{"JOY5", K_JOY5},
	{"JOY6", K_JOY6},
	{"JOY7", K_JOY7},
	{"JOY8", K_JOY8},
	{"JOY9", K_JOY9},
	{"JOY10", K_JOY10},
	{"JOY11", K_JOY11},
	{"JOY12", K_JOY12},
	{"JOY13", K_JOY13},
	{"JOY14", K_JOY14},
	{"JOY15", K_JOY15},
	{"JOY16", K_JOY16},
	{"JOY17", K_JOY17},
	{"JOY18", K_JOY18},
	{"JOY19", K_JOY19},
	{"JOY20", K_JOY20},
	{"JOY21", K_JOY21},
	{"JOY22", K_JOY22},
	{"JOY23", K_JOY23},
	{"JOY24", K_JOY24},
	{"JOY25", K_JOY25},
	{"JOY26", K_JOY26},
	{"JOY27", K_JOY27},
	{"JOY28", K_JOY28},
	{"JOY29", K_JOY29},
	{"JOY30", K_JOY30},
	{"JOY31", K_JOY31},
	{"JOY32", K_JOY32},

	{"AUX1", K_AUX1},
	{"AUX2", K_AUX2},
	{"AUX3", K_AUX3},
	{"AUX4", K_AUX4},
	{"AUX5", K_AUX5},
	{"AUX6", K_AUX6},
	{"AUX7", K_AUX7},
	{"AUX8", K_AUX8},
	{"AUX9", K_AUX9},
	{"AUX10", K_AUX10},
	{"AUX11", K_AUX11},
	{"AUX12", K_AUX12},
	{"AUX13", K_AUX13},
	{"AUX14", K_AUX14},
	{"AUX15", K_AUX15},
	{"AUX16", K_AUX16},

	{"TN_HOME",          K_KP_HOME },
	{"TN_FRECCIASU",     K_KP_UPARROW },
	{"TN_PAGGSU",            K_KP_PGUP },
	{"TN_FRECCIASX", K_KP_LEFTARROW },
	{"TN_5",         K_KP_5 },
	{"TN_FRECCIA_DX",    K_KP_RIGHTARROW },
	{"TN_FINE",          K_KP_END },
	{"TN_FRECCIAGI" "\xD9",    K_KP_DOWNARROW }, // "TN_FRECCIAGIÙ"
	{"TN_PAGGI" "\xD9",            K_KP_PGDN }, // "TN_PAGGIÙ"
	{"TN_INVIO",     K_KP_ENTER },
	{"TN_INS",           K_KP_INS },
	{"TN_CANC",          K_KP_DEL },
	{"TN_/",     K_KP_SLASH },
	{"TN_-",     K_KP_MINUS },
	{"TN_+",         K_KP_PLUS },
	{"TN_BLOCNUM",       K_KP_NUMLOCK },
	{"TN_*",         K_KP_STAR },
	{"TN_=",     K_KP_EQUALS },

	{"PAUSA", K_PAUSE},

	{"\xF2", ';'},   // "ò" because a raw semicolon seperates commands

	{"COMMAND", K_COMMAND},  //mac

	{NULL,0}
};

/*
=============================================================================

EDIT FIELDS

=============================================================================
*/


/*
===================
Field_Draw

Handles horizontal scrolling and cursor blinking
x, y, amd width are in pixels
===================
*/
void Field_VariableSizeDraw( field_t *edit, int x, int y, int width, int size, qboolean showCursor ) {
	int len;
	int drawLen;
	int prestep;
	int cursorChar;
	char str[MAX_STRING_CHARS];
	int i;

	drawLen = edit->widthInChars;
	len = strlen( edit->buffer ) + 1;

	// guarantee that cursor will be visible
	if ( len <= drawLen ) {
		prestep = 0;
	} else {
		if ( edit->scroll + drawLen > len ) {
			edit->scroll = len - drawLen;
			if ( edit->scroll < 0 ) {
				edit->scroll = 0;
			}
		}
		prestep = edit->scroll;

/*
		if ( edit->cursor < len - drawLen ) {
			prestep = edit->cursor;	// cursor at start
		} else {
			prestep = len - drawLen;
		}
*/
	}

	if ( prestep + drawLen > len ) {
		drawLen = len - prestep;
	}

	// extract <drawLen> characters from the field at <prestep>
	if ( drawLen >= MAX_STRING_CHARS ) {
		Com_Error( ERR_DROP, "drawLen >= MAX_STRING_CHARS" );
	}

	memcpy( str, edit->buffer + prestep, drawLen );
	str[ drawLen ] = 0;

	// draw it
	if ( size == SMALLCHAR_WIDTH ) {
		float color[4];

		color[0] = color[1] = color[2] = color[3] = 1.0;
		SCR_DrawSmallStringExt( x, y, str, color, qfalse );
	} else {
		// draw big string with drop shadow
		SCR_DrawBigString( x, y, str, 1.0 );
	}

	// draw the cursor
	if ( !showCursor ) {
		return;
	}

	if ( (int)( cls.realtime >> 8 ) & 1 ) {
		return;     // off blink
	}

	if ( key_overstrikeMode ) {
		cursorChar = 11;
	} else {
		cursorChar = 10;
	}

	i = drawLen - ( Q_PrintStrlen( str ) + 1 );

	if ( size == SMALLCHAR_WIDTH ) {
		SCR_DrawSmallChar( x + ( edit->cursor - prestep - i ) * size, y, cursorChar );
	} else {
		str[0] = cursorChar;
		str[1] = 0;
		SCR_DrawBigString( x + ( edit->cursor - prestep - i ) * size, y, str, 1.0 );

	}
}

void Field_Draw( field_t *edit, int x, int y, int width, qboolean showCursor ) {
	Field_VariableSizeDraw( edit, x, y, width, SMALLCHAR_WIDTH, showCursor );
}

void Field_BigDraw( field_t *edit, int x, int y, int width, qboolean showCursor ) {
	Field_VariableSizeDraw( edit, x, y, width, BIGCHAR_WIDTH, showCursor );
}

/*
================
Field_Paste
================
*/
void Field_Paste( field_t *edit ) {
	char    *cbd;
	int pasteLen, i;

	cbd = Sys_GetClipboardData();

	if ( !cbd ) {
		return;
	}

	// send as if typed, so insert / overstrike works properly
	pasteLen = strlen( cbd );
	for ( i = 0 ; i < pasteLen ; i++ ) {
		Field_CharEvent( edit, cbd[i] );
	}

	Sys_FreeClipboardData(cbd);
}

/*
=================
Field_KeyDownEvent

Performs the basic line editing functions for the console,
in-game talk, and menu fields

Key events are used for non-printable characters, others are gotten from char events.
=================
*/
void Field_KeyDownEvent( field_t *edit, int key ) {
	int len;

	// shift-insert is paste
	if ( ( ( key == K_INS ) || ( key == K_KP_INS ) ) && keys[K_SHIFT].down ) {
		Field_Paste( edit );
		return;
	}

	len = strlen( edit->buffer );

#if !defined RTCW_ET
	if ( key == K_DEL ) {
#else
	if ( key == K_DEL || key == K_KP_DEL ) {
#endif // RTCW_XX

		if ( edit->cursor < len ) {
			memmove( edit->buffer + edit->cursor,
					 edit->buffer + edit->cursor + 1, len - edit->cursor );
		}
		return;
	}

#if !defined RTCW_ET
	if ( key == K_RIGHTARROW ) {
#else
	if ( key == K_RIGHTARROW || key == K_KP_RIGHTARROW ) {
#endif // RTCW_XX

		if ( edit->cursor < len ) {
			edit->cursor++;
		}

		if ( edit->cursor >= edit->scroll + edit->widthInChars && edit->cursor <= len ) {
			edit->scroll++;
		}
		return;
	}

#if !defined RTCW_ET
	if ( key == K_LEFTARROW ) {
#else
	if ( key == K_LEFTARROW || key == K_KP_LEFTARROW ) {
#endif // RTCW_XX

		if ( edit->cursor > 0 ) {
			edit->cursor--;
		}
		if ( edit->cursor < edit->scroll ) {
			edit->scroll--;
		}
		return;
	}

#if !defined RTCW_ET
	if ( key == K_HOME || ( tolower( key ) == 'a' && keys[K_CTRL].down ) ) {
#else
	if ( key == K_HOME || key == K_KP_HOME || ( tolower( key ) == 'a' && keys[K_CTRL].down ) ) {
#endif // RTCW_XX

		edit->cursor = 0;
		return;
	}

#if !defined RTCW_ET
	if ( key == K_END || ( tolower( key ) == 'e' && keys[K_CTRL].down ) ) {
#else
	if ( key == K_END || key == K_KP_END || ( tolower( key ) == 'e' && keys[K_CTRL].down ) ) {
#endif // RTCW_XX

		edit->cursor = len;
		return;
	}

#if !defined RTCW_ET
	if ( key == K_INS ) {
#else
	if ( key == K_INS || key == K_KP_INS ) {
#endif // RTCW_XX

		key_overstrikeMode = !key_overstrikeMode;
		return;
	}
}

/*
==================
Field_CharEvent
==================
*/
void Field_CharEvent( field_t *edit, int ch ) {
	int len;

	if ( ch == 'v' - 'a' + 1 ) {  // ctrl-v is paste
		Field_Paste( edit );
		return;
	}

	if ( ch == 'c' - 'a' + 1 ) {  // ctrl-c clears the field
		Field_Clear( edit );
		return;
	}

	len = strlen( edit->buffer );

	if ( ch == 'h' - 'a' + 1 ) {      // ctrl-h is backspace
		if ( edit->cursor > 0 ) {
			memmove( edit->buffer + edit->cursor - 1,
					 edit->buffer + edit->cursor, len + 1 - edit->cursor );
			edit->cursor--;
			if ( edit->cursor < edit->scroll ) {
				edit->scroll--;
			}
		}
		return;
	}

	if ( ch == 'a' - 'a' + 1 ) {  // ctrl-a is home
		edit->cursor = 0;
		edit->scroll = 0;
		return;
	}

	if ( ch == 'e' - 'a' + 1 ) {  // ctrl-e is end
		edit->cursor = len;
		edit->scroll = edit->cursor - edit->widthInChars;
		return;
	}

	//
	// ignore any other non printable chars
	//
	if ( ch < 32 ) {
		return;
	}

	if ( key_overstrikeMode ) {
		if ( edit->cursor == MAX_EDIT_LINE - 1 ) {
			return;
		}
		edit->buffer[edit->cursor] = ch;
		edit->cursor++;
	} else {    // insert mode
		if ( len == MAX_EDIT_LINE - 1 ) {
			return; // all full
		}
		memmove( edit->buffer + edit->cursor + 1,
				 edit->buffer + edit->cursor, len + 1 - edit->cursor );
		edit->buffer[edit->cursor] = ch;
		edit->cursor++;
	}


	if ( edit->cursor >= edit->widthInChars ) {
		edit->scroll++;
	}

	if ( edit->cursor == len + 1 ) {
		edit->buffer[edit->cursor] = 0;
	}
}

/*
=============================================================================

CONSOLE LINE EDITING

==============================================================================
*/

#if !defined RTCW_ET
static const char *completionString;
static char shortestMatch[MAX_TOKEN_CHARS];
static int matchCount;
#else
static char completionString[MAX_TOKEN_CHARS];
static char currentMatch[MAX_TOKEN_CHARS];
static int matchCount;
static int matchIndex;
#endif // RTCW_XX

/*
===============
FindMatches

===============
*/
static void FindMatches( const char *s ) {
	int i;

	if ( Q_stricmpn( s, completionString, strlen( completionString ) ) ) {
		return;
	}
	matchCount++;
	if ( matchCount == 1 ) {

#if !defined RTCW_ET
		Q_strncpyz( shortestMatch, s, sizeof( shortestMatch ) );
		return;
	}

	// cut shortestMatch to the amount common with s
	for ( i = 0 ; s[i] ; i++ ) {
		if ( tolower( shortestMatch[i] ) != tolower( s[i] ) ) {
			shortestMatch[i] = 0;
		}
	}
#else
		Q_strncpyz( currentMatch, s, sizeof( currentMatch ) );
		return;
	}

	// cut currentMatch to the amount common with s
	for ( i = 0 ; s[i] ; i++ ) {
		if ( tolower( currentMatch[i] ) != tolower( s[i] ) ) {
			currentMatch[i] = 0;
		}
	}
	currentMatch[i] = 0;
#endif // RTCW_XX

}

#if defined RTCW_ET
/*
===============
FindIndexMatch

===============
*/
static int findMatchIndex;
static void FindIndexMatch( const char *s ) {

	if ( Q_stricmpn( s, completionString, strlen( completionString ) ) ) {
		return;
	}

	if ( findMatchIndex == matchIndex ) {
		Q_strncpyz( currentMatch, s, sizeof( currentMatch ) );
	}

	findMatchIndex++;
}
#endif // RTCW_XX

/*
===============
PrintMatches

===============
*/
static void PrintMatches( const char *s ) {

#if !defined RTCW_ET
	if ( !Q_stricmpn( s, shortestMatch, strlen( shortestMatch ) ) ) {
		Com_Printf( "    %s\n", s );
#else
	if ( !Q_stricmpn( s, currentMatch, strlen( currentMatch ) ) ) {
		Com_Printf( "  ^9%s^0\n", s );
#endif // RTCW_XX

	}
}

#if defined RTCW_ET
// ydnar: to display cvar values
static void PrintCvarMatches( const char *s ) {
	if ( !Q_stricmpn( s, currentMatch, strlen( currentMatch ) ) ) {
		Com_Printf( "  ^9%s = ^5%s^0\n", s, Cvar_VariableString( s ) );
	}
}
#endif // RTCW_XX

static void keyConcatArgs( void ) {
	int i;
	const char    *arg;

	for ( i = 1 ; i < Cmd_Argc() ; i++ ) {
		Q_strcat( g_consoleField.buffer, sizeof( g_consoleField.buffer ), " " );
		arg = Cmd_Argv( i );
		while ( *arg ) {
			if ( *arg == ' ' ) {
				Q_strcat( g_consoleField.buffer, sizeof( g_consoleField.buffer ),  "\"" );
				break;
			}
			arg++;
		}
		Q_strcat( g_consoleField.buffer, sizeof( g_consoleField.buffer ),  Cmd_Argv( i ) );
		if ( *arg == ' ' ) {
			Q_strcat( g_consoleField.buffer, sizeof( g_consoleField.buffer ),  "\"" );
		}
	}
}

static void ConcatRemaining( const char *src, const char *start ) {
	char *str;

	str = const_cast<char*> (strstr( src, start ));
	if ( !str ) {
		keyConcatArgs();
		return;
	}

	str += strlen( start );
	Q_strcat( g_consoleField.buffer, sizeof( g_consoleField.buffer ), str );
}


/*
===============
CompleteCommand

Tab expansion
===============
*/
static void CompleteCommand( void ) {
	field_t *edit;
	field_t temp;

	edit = &g_consoleField;

#if !defined RTCW_ET
	// only look at the first token for completion purposes
	Cmd_TokenizeString( edit->buffer );

	completionString = Cmd_Argv( 0 );
	if ( completionString[0] == '\\' || completionString[0] == '/' ) {
		completionString++;
	}

	matchCount = 0;
	shortestMatch[0] = 0;

	if ( strlen( completionString ) == 0 ) {
		return;
	}

	Cmd_CommandCompletion( FindMatches );
	Cvar_CommandCompletion( FindMatches );

	if ( matchCount == 0 ) {
		return; // no matches
	}

	Com_Memcpy( &temp, edit, sizeof( field_t ) );

	if ( matchCount == 1 ) {
		Com_sprintf( edit->buffer, sizeof( edit->buffer ), "\\%s", shortestMatch );
		if ( Cmd_Argc() == 1 ) {
			Q_strcat( g_consoleField.buffer, sizeof( g_consoleField.buffer ), " " );
		} else {
			ConcatRemaining( temp.buffer, completionString );
		}
		edit->cursor = strlen( edit->buffer );
		return;
	}

	// multiple matches, complete to shortest
	Com_sprintf( edit->buffer, sizeof( edit->buffer ), "\\%s", shortestMatch );
	edit->cursor = strlen( edit->buffer );
	ConcatRemaining( temp.buffer, completionString );

	Com_Printf( "]%s\n", edit->buffer );

	// run through again, printing matches
	Cmd_CommandCompletion( PrintMatches );
	Cvar_CommandCompletion( PrintMatches );
#else
	if ( !con.acLength ) {
		// only look at the first token for completion purposes
		Cmd_TokenizeString( edit->buffer );

		Q_strncpyz( completionString, Cmd_Argv( 0 ), sizeof( completionString ) );
		if ( completionString[0] == '\\' || completionString[0] == '/' ) {
			// rain - in strcpy, src and dest cannot overlap
			//Q_strncpyz( completionString, completionString+1, sizeof(completionString) );
			memmove( completionString, completionString + 1, sizeof( completionString ) - 1 );
		}

		matchCount = 0;
		matchIndex = 0;
		currentMatch[0] = 0;

		if ( strlen( completionString ) == 0 ) {
			return;
		}

		Cmd_CommandCompletion( FindMatches );
		Cvar_CommandCompletion( FindMatches );

		if ( matchCount == 0 ) {
			return; // no matches
		}

		Com_Memcpy( &temp, edit, sizeof( field_t ) );

		if ( matchCount == 1 ) {
			Com_sprintf( edit->buffer, sizeof( edit->buffer ), "\\%s", currentMatch );
			if ( Cmd_Argc() == 1 ) {
				Q_strcat( g_consoleField.buffer, sizeof( g_consoleField.buffer ), " " );
			} else {
				ConcatRemaining( temp.buffer, completionString );
			}
			edit->cursor = strlen( edit->buffer );
			return;
		}

		// multiple matches, complete to shortest
		Com_sprintf( edit->buffer, sizeof( edit->buffer ), "\\%s", currentMatch );
		con.acLength = edit->cursor = strlen( edit->buffer );
		ConcatRemaining( temp.buffer, completionString );

		Com_Printf( "]%s\n", edit->buffer );

		// run through again, printing matches
		Cmd_CommandCompletion( PrintMatches );
		Cvar_CommandCompletion( PrintCvarMatches );
	} else {
		if ( matchCount != 1 ) {
			// get the next match and show instead
			char lastMatch[MAX_TOKEN_CHARS];

			Q_strncpyz( lastMatch, currentMatch, sizeof( lastMatch ) );

			matchIndex++;
			if ( matchIndex == matchCount ) {
				matchIndex = 0;
			}
			findMatchIndex = 0;
			Cmd_CommandCompletion( FindIndexMatch );
			Cvar_CommandCompletion( FindIndexMatch );

			Com_Memcpy( &temp, edit, sizeof( field_t ) );

			// and print it
			Com_sprintf( edit->buffer, sizeof( edit->buffer ), "\\%s", currentMatch );
			edit->cursor = strlen( edit->buffer );
			ConcatRemaining( temp.buffer, lastMatch );
		}
	}
#endif // RTCW_XX

}


/*
====================
Console_Key

Handles history and console scrollback
====================
*/
void Console_Key( int key ) {
	// ctrl-L clears screen
	if ( key == 'l' && keys[K_CTRL].down ) {
		Cbuf_AddText( "clear\n" );
		return;
	}

	// enter finishes the line
	if ( key == K_ENTER || key == K_KP_ENTER ) {

#if defined RTCW_ET
		con.acLength = 0;
#endif // RTCW_XX

		// if not in the game explicitly prepent a slash if needed
		if ( cls.state != CA_ACTIVE && g_consoleField.buffer[0] != '\\'
			 && g_consoleField.buffer[0] != '/' ) {
			char temp[MAX_STRING_CHARS];

			Q_strncpyz( temp, g_consoleField.buffer, sizeof( temp ) );
			Com_sprintf( g_consoleField.buffer, sizeof( g_consoleField.buffer ), "\\%s", temp );
			g_consoleField.cursor++;
		}

		Com_Printf( "]%s\n", g_consoleField.buffer );

		// leading slash is an explicit command
		if ( g_consoleField.buffer[0] == '\\' || g_consoleField.buffer[0] == '/' ) {
			Cbuf_AddText( g_consoleField.buffer + 1 );    // valid command
			Cbuf_AddText( "\n" );
		} else {
			// other text will be chat messages
			if ( !g_consoleField.buffer[0] ) {
				return; // empty lines just scroll the console without adding to history
			} else {
				Cbuf_AddText( "cmd say " );
				Cbuf_AddText( g_consoleField.buffer );
				Cbuf_AddText( "\n" );
			}
		}

		// copy line to history buffer
		historyEditLines[nextHistoryLine % COMMAND_HISTORY] = g_consoleField;
		nextHistoryLine++;
		historyLine = nextHistoryLine;

		Field_Clear( &g_consoleField );

		g_consoleField.widthInChars = g_console_field_width;

		if ( cls.state == CA_DISCONNECTED ) {
			SCR_UpdateScreen();     // force an update, because the command
		}                           // may take some time
		return;
	}

	// command completion

	if ( key == K_TAB ) {
		CompleteCommand();
		return;
	}

#if defined RTCW_ET
	// clear autocompletion buffer on normal key input
	if ( ( key >= K_SPACE && key <= K_BACKSPACE ) || ( key == K_LEFTARROW ) || ( key == K_RIGHTARROW ) ||
		 ( key >= K_KP_LEFTARROW && key <= K_KP_RIGHTARROW ) ||
		 ( key >= K_KP_SLASH && key <= K_KP_PLUS ) || ( key >= K_KP_STAR && key <= K_KP_EQUALS ) ) {
		con.acLength = 0;
	}
#endif // RTCW_XX

	// command history (ctrl-p ctrl-n for unix style)

	//----(SA)	added some mousewheel functionality to the console
	if ( ( key == K_MWHEELUP && keys[K_SHIFT].down ) || ( key == K_UPARROW ) || ( key == K_KP_UPARROW ) ||
		 ( ( tolower( key ) == 'p' ) && keys[K_CTRL].down ) ) {
		if ( nextHistoryLine - historyLine < COMMAND_HISTORY
			 && historyLine > 0 ) {
			historyLine--;
		}
		g_consoleField = historyEditLines[ historyLine % COMMAND_HISTORY ];

#if defined RTCW_ET
		con.acLength = 0;
#endif // RTCW_XX

		return;
	}

	//----(SA)	added some mousewheel functionality to the console
	if ( ( key == K_MWHEELDOWN && keys[K_SHIFT].down ) || ( key == K_DOWNARROW ) || ( key == K_KP_DOWNARROW ) ||
		 ( ( tolower( key ) == 'n' ) && keys[K_CTRL].down ) ) {
		if ( historyLine == nextHistoryLine ) {
			return;
		}
		historyLine++;
		g_consoleField = historyEditLines[ historyLine % COMMAND_HISTORY ];
#if defined RTCW_ET
		con.acLength = 0;
#endif // RTCW_XX

		return;
	}

	// console scrolling

#if !defined RTCW_ET
	if ( key == K_PGUP ) {
#else
	if ( key == K_PGUP || key == K_KP_PGUP ) {
#endif // RTCW_XX

		Con_PageUp();
		return;
	}

#if !defined RTCW_ET
	if ( key == K_PGDN ) {
#else
	if ( key == K_PGDN || key == K_KP_PGDN ) {
#endif // RTCW_XX

		Con_PageDown();
		return;
	}

	if ( key == K_MWHEELUP ) {   //----(SA)	added some mousewheel functionality to the console
		Con_PageUp();
		if ( keys[K_CTRL].down ) { // hold <ctrl> to accelerate scrolling
			Con_PageUp();
			Con_PageUp();
		}
		return;
	}

	if ( key == K_MWHEELDOWN ) { //----(SA)	added some mousewheel functionality to the console
		Con_PageDown();
		if ( keys[K_CTRL].down ) { // hold <ctrl> to accelerate scrolling
			Con_PageDown();
			Con_PageDown();
		}
		return;
	}

	// ctrl-home = top of console

#if !defined RTCW_ET
	if ( key == K_HOME && keys[K_CTRL].down ) {
#else
	if ( ( key == K_HOME || key == K_KP_HOME ) && keys[K_CTRL].down ) {
#endif // RTCW_XX

		Con_Top();
		return;
	}

	// ctrl-end = bottom of console

#if !defined RTCW_ET
	if ( key == K_END && keys[K_CTRL].down ) {
#else
	if ( ( key == K_END || key == K_KP_END ) && keys[K_CTRL].down ) {
#endif // RTCW_XX

		Con_Bottom();
		return;
	}

	// pass to the normal editline routine
	Field_KeyDownEvent( &g_consoleField, key );
}

//============================================================================


/*
================
Message_Key

In game talk message
================
*/
void Message_Key( int key ) {

	char buffer[MAX_STRING_CHARS];


	if ( key == K_ESCAPE ) {
		cls.keyCatchers &= ~KEYCATCH_MESSAGE;
		Field_Clear( &chatField );
		return;
	}

	if ( key == K_ENTER || key == K_KP_ENTER ) {
		if ( chatField.buffer[0] && cls.state == CA_ACTIVE ) {

#if !defined RTCW_ET
			if ( chat_playerNum != -1 ) {

				Com_sprintf( buffer, sizeof( buffer ), "tell %i \"%s\"\n", chat_playerNum, chatField.buffer );
			} else if ( chat_team ) {

				Com_sprintf( buffer, sizeof( buffer ), "say_team \"%s\"\n", chatField.buffer );
			}
			// NERVE - SMF
			else if ( chat_limbo ) {

				Com_sprintf( buffer, sizeof( buffer ), "say_limbo \"%s\"\n", chatField.buffer );
			}
			// -NERVE - SMF
			else {
#else
			if ( chat_team ) {
				Com_sprintf( buffer, sizeof( buffer ), "say_team \"%s\"\n", chatField.buffer );
			} else if ( chat_buddy ) {
				Com_sprintf( buffer, sizeof( buffer ), "say_buddy \"%s\"\n", chatField.buffer );
			} else {
#endif // RTCW_XX

				Com_sprintf( buffer, sizeof( buffer ), "say \"%s\"\n", chatField.buffer );
			}



			CL_AddReliableCommand( buffer );
		}
		cls.keyCatchers &= ~KEYCATCH_MESSAGE;
		Field_Clear( &chatField );
		return;
	}

	Field_KeyDownEvent( &chatField, key );
}

//============================================================================


qboolean Key_GetOverstrikeMode( void ) {
	return key_overstrikeMode;
}


void Key_SetOverstrikeMode( qboolean state ) {
	key_overstrikeMode = state;
}


/*
===================
Key_IsDown
===================
*/
qboolean Key_IsDown( int keynum ) {
	if ( keynum == -1 ) {
		return qfalse;
	}

	return keys[keynum].down;
}


/*
===================
Key_StringToKeynum

Returns a key number to be used to index keys[] by looking at
the given string.  Single ascii characters return themselves, while
the K_* names are matched up.

0x11 will be interpreted as raw hex, which will allow new controlers

to be configured even if they don't have defined names.
===================
*/
int Key_StringToKeynum( const char *str ) {
	keyname_t   *kn;

	if ( !str || !str[0] ) {
		return -1;
	}
	if ( !str[1] ) {
		// BBi
		char result[2] = { str[0], '\0' };

#if defined RTCW_ET
		// Always lowercase
// BBi
#if 0
		Q_strlwr( str );
#else
		Q_strlwr(result);
#endif // 0
#endif // RTCW_XX

// BBi
#if 0
		return str[0];
#else
		return result[0];
#endif // 0
	}

	// check for hex code
	if ( str[0] == '0' && str[1] == 'x' && strlen( str ) == 4 ) {
		int n1, n2;

		n1 = str[2];

#if defined RTCW_SP
		if ( Q_isnumeric( n1 ) ) {
#else
		if ( n1 >= '0' && n1 <= '9' ) {
#endif // RTCW_XX

			n1 -= '0';
		} else if ( n1 >= 'a' && n1 <= 'f' ) {
			n1 = n1 - 'a' + 10;
		} else {
			n1 = 0;
		}

		n2 = str[3];

#if defined RTCW_SP
		if ( Q_isnumeric( n2 ) ) {
#else
		if ( n2 >= '0' && n2 <= '9' ) {
#endif // RTCW_XX

			n2 -= '0';
		} else if ( n2 >= 'a' && n2 <= 'f' ) {
			n2 = n2 - 'a' + 10;
		} else {
			n2 = 0;
		}

		return n1 * 16 + n2;
	}

	// scan for a text match
	for ( kn = keynames ; kn->name ; kn++ ) {
		if ( !Q_stricmp( str,kn->name ) ) {
			return kn->keynum;
		}
	}

	return -1;
}

/*
===================
Key_KeynumToString

Returns a string (either a single ascii char, a K_* name, or a 0x11 hex string) for the
given keynum.
===================
*/
const char *Key_KeynumToString( int keynum, qboolean bTranslate ) {
	keyname_t   *kn;
	static char tinystr[5];
	int i, j;

	if ( keynum == -1 ) {
		return "<KEY NOT FOUND>";
	}

	if ( keynum < 0 || keynum > 255 ) {
		return "<OUT OF RANGE>";
	}

	// check for printable ascii (don't use quote)
	if ( keynum > 32 && keynum < 127 && keynum != '"' ) {
		tinystr[0] = keynum;
		tinystr[1] = 0;
		if ( keynum == ';' && !bTranslate ) {
			//fall through and use keyname table
		} else {
			return tinystr;
		}
	}


	kn = keynames;    //init to english

#if defined RTCW_SP
	if ( bTranslate ) {
		if ( cl_language->integer - 1 == LANGUAGE_FRENCH ) {
			kn = keynames_f;  //use french
		} else if ( cl_language->integer - 1 == LANGUAGE_GERMAN ) {
			kn = keynames_d;  //use german
		} else if ( cl_language->integer - 1 == LANGUAGE_ITALIAN ) {
			kn = keynames_i;  //use italian
		} else if ( cl_language->integer - 1 == LANGUAGE_SPANISH ) {
			kn = keynames_s;  //use spanish
		}
	}
#else
	if ( bTranslate ) {
		if ( cl_language->integer - 1 == LANGUAGE_FRENCH ) {
			kn = keynames_f;  //use french
		} else if ( cl_language->integer - 1 == LANGUAGE_GERMAN ) {
			kn = keynames_d;  //use german
		} else if ( cl_language->integer - 1 == LANGUAGE_ITALIAN ) {
			kn = keynames_i;  //use italian
		} else if ( cl_language->integer - 1 == LANGUAGE_SPANISH ) {
			kn = keynames_s;  //use spanish
		}
	}
#endif // RTCW_XX

	// check for a key string
	for ( ; kn->name ; kn++ ) {
		if ( keynum == kn->keynum ) {
			return kn->name;
		}
	}

	// make a hex string
	i = keynum >> 4;
	j = keynum & 15;

	tinystr[0] = '0';
	tinystr[1] = 'x';
	tinystr[2] = i > 9 ? i - 10 + 'a' : i + '0';
	tinystr[3] = j > 9 ? j - 10 + 'a' : j + '0';
	tinystr[4] = 0;

	return tinystr;
}

#if defined RTCW_ET
#define BIND_HASH_SIZE 1024

static int32_t generateHashValue( const char *fname ) {
	int i;
	int32_t hash;

	if ( !fname ) {
		return 0;
	}

	hash = 0;
	i = 0;
	while ( fname[i] != '\0' ) {
		hash += (int32_t)( fname[i] ) * ( i + 119 );
		i++;
	}
	hash &= ( BIND_HASH_SIZE - 1 );
	return hash;
}
#endif // RTCW_XX

/*
===================
Key_SetBinding
===================
*/
void Key_SetBinding( int keynum, const char *binding ) {

#if defined RTCW_ET
	char *lcbinding;    // fretn - make a copy of our binding lowercase
						// so name toggle scripts work again: bind x name BzZIfretn?
						// resulted into bzzifretn?
#endif // RTCW_XX

	if ( keynum == -1 ) {
		return;
	}

	// free old bindings
	if ( keys[ keynum ].binding ) {
		Z_Free( keys[ keynum ].binding );
	}

	// allocate memory for new binding
	keys[keynum].binding = CopyString( binding );

#if defined RTCW_ET
	lcbinding = CopyString( binding );
	Q_strlwr( lcbinding ); // saves doing it on all the generateHashValues in Key_GetBindingByString

	keys[keynum].hash = generateHashValue( lcbinding );
#endif // RTCW_XX


	// consider this like modifying an archived cvar, so the
	// file write will be triggered at the next oportunity
	cvar_modifiedFlags |= CVAR_ARCHIVE;
}


/*
===================
Key_GetBinding
===================
*/
const char *Key_GetBinding( int keynum ) {
	if ( keynum == -1 ) {
		return "";
	}

	return keys[ keynum ].binding;
}

#if defined RTCW_ET
// binding MUST be lower case
void Key_GetBindingByString( const char* binding, int* key1, int* key2 ) {
	int i;
	int hash = generateHashValue( binding );

	*key1 = -1;
	*key2 = -1;

	for ( i = 0; i < MAX_KEYS; i++ ) {
		if ( keys[i].hash == hash && !Q_stricmp( binding, keys[i].binding ) ) {
			if ( *key1 == -1 ) {
				*key1 = i;
			} else if ( *key2 == -1 ) {
				*key2 = i;
				return;
			}
		}
	}
}
#endif // RTCW_XX

/*
===================
Key_GetKey
===================
*/

int Key_GetKey( const char *binding ) {
	int i;

	if ( binding ) {
		for ( i = 0 ; i < 256 ; i++ ) {
			if ( keys[i].binding && Q_stricmp( binding, keys[i].binding ) == 0 ) {
				return i;
			}
		}
	}
	return -1;
}

/*
===================
Key_Unbind_f
===================
*/
void Key_Unbind_f( void ) {
	int b;

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "unbind <key> : remove commands from a key\n" );
		return;
	}

	b = Key_StringToKeynum( Cmd_Argv( 1 ) );
	if ( b == -1 ) {
		Com_Printf( "\"%s\" isn't a valid key\n", Cmd_Argv( 1 ) );
		return;
	}

	Key_SetBinding( b, "" );
}

/*
===================
Key_Unbindall_f
===================
*/
void Key_Unbindall_f( void ) {
	int i;

	for ( i = 0 ; i < 256 ; i++ )
		if ( keys[i].binding ) {
			Key_SetBinding( i, "" );
		}
}


/*
===================
Key_Bind_f
===================
*/
void Key_Bind_f( void ) {
	int i, c, b;
	char cmd[1024];

	c = Cmd_Argc();

	if ( c < 2 ) {
		Com_Printf( "bind <key> [command] : attach a command to a key\n" );
		return;
	}
	b = Key_StringToKeynum( Cmd_Argv( 1 ) );
	if ( b == -1 ) {
		Com_Printf( "\"%s\" isn't a valid key\n", Cmd_Argv( 1 ) );
		return;
	}

	if ( c == 2 ) {
		if ( keys[b].binding ) {
			Com_Printf( "\"%s\" = \"%s\"\n", Cmd_Argv( 1 ), keys[b].binding );
		} else {
			Com_Printf( "\"%s\" is not bound\n", Cmd_Argv( 1 ) );
		}
		return;
	}

// copy the rest of the command line
	cmd[0] = 0;     // start out with a null string
	for ( i = 2 ; i < c ; i++ )
	{
		strcat( cmd, Cmd_Argv( i ) );
		if ( i != ( c - 1 ) ) {
			strcat( cmd, " " );
		}
	}

	Key_SetBinding( b, cmd );
}

/*
============
Key_WriteBindings

Writes lines containing "bind key value"
============
*/
void Key_WriteBindings( fileHandle_t f ) {
	int i;

	FS_Printf( f, "unbindall\n" );

	for ( i = 0 ; i < 256 ; i++ ) {
		if ( keys[i].binding && keys[i].binding[0] ) {
			FS_Printf( f, "bind %s \"%s\"\n", Key_KeynumToString( i, qfalse ), keys[i].binding );

		}

	}
}


/*
============
Key_Bindlist_f

============
*/
void Key_Bindlist_f( void ) {
	int i;

	for ( i = 0 ; i < 256 ; i++ ) {
		if ( keys[i].binding && keys[i].binding[0] ) {
			Com_Printf( "%s \"%s\"\n", Key_KeynumToString( i, qfalse ), keys[i].binding );
		}
	}
}


/*
===================
CL_InitKeyCommands
===================
*/
void CL_InitKeyCommands( void ) {
	// register our functions
	Cmd_AddCommand( "bind",Key_Bind_f );
	Cmd_AddCommand( "unbind",Key_Unbind_f );
	Cmd_AddCommand( "unbindall",Key_Unbindall_f );
	Cmd_AddCommand( "bindlist",Key_Bindlist_f );
}

/*
===================
CL_KeyEvent

Called by the system for both key up and key down events
===================
*/

#if defined RTCW_SP
//static int consoleCount = 0; // TTimo: unused
#else
//static consoleCount = 0;
#endif // RTCW_XX

#if defined RTCW_ET
qboolean consoleButtonWasPressed = qfalse;
#endif // RTCW_XX

void CL_KeyEvent( int key, qboolean down, unsigned time ) {
	char    *kb;
	char cmd[1024];

#if defined RTCW_SP
	int activeMenu = 0;
#else
	qboolean bypassMenu = qfalse;       // NERVE - SMF
#endif // RTCW_XX

#if defined RTCW_ET
	qboolean onlybinds = qfalse;

	if ( !key ) {
		return;
	}

	switch ( key ) {
	case K_KP_PGUP:
	case K_KP_EQUALS:
	case K_KP_5:
	case K_KP_LEFTARROW:
	case K_KP_UPARROW:
	case K_KP_RIGHTARROW:
	case K_KP_DOWNARROW:
	case K_KP_END:
	case K_KP_PGDN:
	case K_KP_INS:
	case K_KP_DEL:
	case K_KP_HOME:
		if ( Sys_IsNumLockDown() ) {
			onlybinds = qtrue;
		}
		break;
	}
#endif // RTCW_XX


	// update auto-repeat status and BUTTON_ANY status
	keys[key].down = down;

	if ( down ) {
		keys[key].repeats++;
		if ( keys[key].repeats == 1 ) {
			anykeydown++;
		}
	} else {
		keys[key].repeats = 0;
		anykeydown--;
		if ( anykeydown < 0 ) {
			anykeydown = 0;
		}
	}

#if defined RTCW_MP
	// are we waiting to clear stats and move to briefing screen
	if ( down && cl_waitForFire && cl_waitForFire->integer ) {    //DAJ BUG in dedicated cl_waitForFire don't exist
		if ( cls.keyCatchers & KEYCATCH_CONSOLE ) {   // get rid of the consol
			Con_ToggleConsole_f();
		}
		// clear all input controls
		CL_ClearKeys();
		// allow only attack command input
		kb = keys[key].binding;
		if ( !Q_stricmp( kb, "+attack" ) ) {
			// clear the stats out, ignore the keypress
			Cvar_Set( "g_missionStats", "xx" );       // just remove the stats, but still wait until we're done loading, and player has hit fire to begin playing
			Cvar_Set( "cl_waitForFire", "0" );
		}
		return;     // no buttons while waiting
	}

	// are we waiting to begin the level
	if ( down && cl_missionStats && cl_missionStats->string[0] && cl_missionStats->string[1] ) {  //DAJ BUG in dedicated cl_missionStats don't exist
		if ( cls.keyCatchers & KEYCATCH_CONSOLE ) {   // get rid of the consol
			Con_ToggleConsole_f();
		}
		// clear all input controls
		CL_ClearKeys();
		// allow only attack command input
		kb = keys[key].binding;
		if ( !Q_stricmp( kb, "+attack" ) ) {
			// clear the stats out, ignore the keypress
			Cvar_Set( "com_expectedhunkusage", "-1" );
			Cvar_Set( "g_missionStats", "0" );
		}
		return;     // no buttons while waiting
	}
#endif // RTCW_XX

	// console key is hardcoded, so the user can never unbind it
	if ( key == '`' || key == '~' ) {
		if ( !down ) {
			return;

		}
		Con_ToggleConsole_f();

#if defined RTCW_ET
		// the console key should never be used as a char
		consoleButtonWasPressed = qtrue;
#endif // RTCW_XX

		return;

#if defined RTCW_ET
	} else {
		consoleButtonWasPressed = qfalse;
#endif // RTCW_XX

	}

#if !defined RTCW_MP
//----(SA)	added
	if ( cl.cameraMode ) {
		if ( !( cls.keyCatchers & ( KEYCATCH_UI | KEYCATCH_CONSOLE ) ) ) {    // let menu/console handle keys if necessary

			// in cutscenes we need to handle keys specially (pausing not allowed in camera mode)
			if ( (  key == K_ESCAPE ||
					key == K_SPACE ||
					key == K_ENTER ) && down ) {

#if !defined RTCW_ET
				if ( down ) {
					CL_AddReliableCommand( "cameraInterrupt" );
				}
#else
				CL_AddReliableCommand( "cameraInterrupt" );
#endif // RTCW_XX

				return;
			}

			// eat all keys
			if ( down ) {
				return;
			}
		}

		if ( ( cls.keyCatchers & KEYCATCH_CONSOLE ) && key == K_ESCAPE ) {
			// don't allow menu starting when console is down and camera running
			return;
		}
	}
//----(SA)	end
#endif // RTCW_XX


	// most keys during demo playback will bring up the menu, but non-ascii

	// keys can still be used for bound actions
	if ( down && ( key < 128 || key == K_MOUSE1 )
		 && ( clc.demoplaying || cls.state == CA_CINEMATIC ) && !cls.keyCatchers ) {

		Cvar_Set( "nextdemo","" );
		key = K_ESCAPE;
	}

#if defined RTCW_SP
//----(SA)	get the active menu if in ui mode
	if ( cls.keyCatchers & KEYCATCH_UI ) {
		activeMenu = VM_Call(uivm, rtcw::to_vm_arg(UI_GET_ACTIVE_MENU));
	}
#endif // RTCW_XX

	// escape is always handled special
	if ( key == K_ESCAPE && down ) {
		if ( cls.keyCatchers & KEYCATCH_MESSAGE ) {
			// clear message mode
			Message_Key( key );
			return;
		}

		// escape always gets out of CGAME stuff
		if ( cls.keyCatchers & KEYCATCH_CGAME ) {
			cls.keyCatchers &= ~KEYCATCH_CGAME;
			VM_Call(cgvm, CG_EVENT_HANDLING, rtcw::to_vm_arg(static_cast<int>(CGAME_EVENT_NONE)));
			return;
		}

		if ( !( cls.keyCatchers & KEYCATCH_UI ) ) {
			if ( cls.state == CA_ACTIVE && !clc.demoplaying ) {

#if !defined RTCW_ET
				VM_Call(uivm, UI_SET_ACTIVE_MENU, rtcw::to_vm_arg(static_cast<int>(UIMENU_INGAME)));
#else
				// Arnout: on request
				if ( cls.keyCatchers & KEYCATCH_CONSOLE ) {  // get rid of the console
					Con_ToggleConsole_f();
				} else {
					VM_Call(uivm, UI_SET_ACTIVE_MENU, rtcw::to_vm_arg(UIMENU_INGAME));
				}
#endif // RTCW_XX

			} else {
				CL_Disconnect_f();
				S_StopAllSounds();
				VM_Call(uivm, UI_SET_ACTIVE_MENU, rtcw::to_vm_arg(static_cast<int>(UIMENU_MAIN)));
			}
			return;
		}

#if defined RTCW_SP
		if ( activeMenu == UIMENU_PREGAME ) {  // eat escape too at this point
			return;
		}
#endif // RTCW_XX

		VM_Call(uivm, UI_KEY_EVENT, rtcw::to_vm_arg(key), rtcw::to_vm_arg(down));
		return;
	}

	//
	// key up events only perform actions if the game key binding is
	// a button command (leading + sign).  These will be processed even in
	// console mode and menu mode, to keep the character from continuing
	// an action started before a mode switch.
	//
	if ( !down ) {
		kb = keys[key].binding;
		if ( kb && kb[0] == '+' ) {
			// button commands add keynum and time as parms so that multiple
			// sources can be discriminated and subframe corrected
			Com_sprintf( cmd, sizeof( cmd ), "-%s %i %i\n", kb + 1, key, time );
			Cbuf_AddText( cmd );
		}

		if ( cls.keyCatchers & KEYCATCH_UI && uivm ) {

#if !defined RTCW_ET
			VM_Call(uivm, UI_KEY_EVENT, rtcw::to_vm_arg(key), rtcw::to_vm_arg(down));
		} else if ( cls.keyCatchers & KEYCATCH_CGAME && cgvm ) {
			VM_Call(cgvm, CG_KEY_EVENT, rtcw::to_vm_arg(key), rtcw::to_vm_arg(down));
		}
#else
			if ( !onlybinds || VM_Call(uivm, UI_WANTSBINDKEYS) ) {
				VM_Call(uivm, UI_KEY_EVENT, rtcw::to_vm_arg(key), rtcw::to_vm_arg(down));
			}
		} else if ( cls.keyCatchers & KEYCATCH_CGAME && cgvm ) {
			if ( !onlybinds || VM_Call(cgvm, CG_WANTSBINDKEYS) ) {
				VM_Call(cgvm, CG_KEY_EVENT, rtcw::to_vm_arg(key), rtcw::to_vm_arg(down));
			}
		}
#endif // RTCW_XX


		return;
	}

#if defined RTCW_MP
	// NERVE - SMF - if we just want to pass it along to game
	if ( cl_bypassMouseInput && cl_bypassMouseInput->integer ) {    //DAJ BUG in dedicated cl_missionStats don't exist
		if ( ( key == K_MOUSE1 || key == K_MOUSE2 || key == K_MOUSE3 ) ) {
			if ( cl_bypassMouseInput->integer == 1 ) {
				bypassMenu = qtrue;
			}
		} else if ( !UI_checkKeyExec( key ) ) {
			bypassMenu = qtrue;
		}
	}
#elif defined RTCW_ET
	// NERVE - SMF - if we just want to pass it along to game
	if ( cl_bypassMouseInput && cl_bypassMouseInput->integer ) {
		if ( ( key == K_MOUSE1 || key == K_MOUSE2 || key == K_MOUSE3 || key == K_MOUSE4 || key == K_MOUSE5 ) ) {
			if ( cl_bypassMouseInput->integer == 1 ) {
				bypassMenu = qtrue;
			}
		} else if ( ( cls.keyCatchers & KEYCATCH_UI && !UI_checkKeyExec( key ) ) || ( cls.keyCatchers & KEYCATCH_CGAME && !CL_CGameCheckKeyExec( key ) ) ) {
			bypassMenu = qtrue;
		}
	}
#endif // RTCW_XX

	// distribute the key down event to the apropriate handler
	if ( cls.keyCatchers & KEYCATCH_CONSOLE ) {

#if !defined RTCW_ET
		Console_Key( key );

#if defined RTCW_SP
	} else if ( cls.keyCatchers & KEYCATCH_UI ) {
#elif defined RTCW_MP
	} else if ( cls.keyCatchers & KEYCATCH_UI && !bypassMenu ) {
#endif // RTCW_XX

		kb = keys[key].binding;

#if defined RTCW_SP
		if ( activeMenu == UIMENU_CLIPBOARD ) {
#elif defined RTCW_MP
		if ( VM_Call(uivm, UI_GET_ACTIVE_MENU) == UIMENU_CLIPBOARD ) {
#endif // RTCW_XX

			// any key gets out of clipboard
			key = K_ESCAPE;

#if defined RTCW_SP
		} else if ( activeMenu == UIMENU_PREGAME ) {
			if ( key != K_MOUSE1 ) {
				return; // eat all keys except mouse click
			}
#endif // RTCW_XX

		} else {

			// when in the notebook, check for the key bound to "notebook" and allow that as an escape key

			if ( kb ) {
				if ( !Q_stricmp( "notebook", kb ) ) {
					if ( VM_Call(uivm, UI_GET_ACTIVE_MENU) == UIMENU_NOTEBOOK ) {
						key = K_ESCAPE;
					}
				}

#if defined RTCW_SP
//				if(!Q_stricmp("help", kb)) {
//					if(VM_Call( uivm, UI_GET_ACTIVE_MENU) == UIMENU_HELP)
//						key = K_ESCAPE;
///				}
#elif defined RTCW_MP
				if ( !Q_stricmp( "help", kb ) ) {
					if ( VM_Call(uivm, UI_GET_ACTIVE_MENU) == UIMENU_HELP ) {
						key = K_ESCAPE;
					}
				}
#endif // RTCW_XX

			}
		}

#if defined RTCW_SP
		if ( uivm ) {
			VM_Call(uivm, UI_KEY_EVENT, rtcw::to_vm_arg(key), rtcw::to_vm_arg(down));
		}
#elif defined RTCW_MP
		VM_Call(uivm, UI_KEY_EVENT, rtcw::to_vm_arg(key), rtcw::to_vm_arg(down));
#endif // RTCW_XX

	} else if ( cls.keyCatchers & KEYCATCH_CGAME ) {
		if ( cgvm ) {
			VM_Call(cgvm, CG_KEY_EVENT, rtcw::to_vm_arg(key), rtcw::to_vm_arg(down));
		}
	} else if ( cls.keyCatchers & KEYCATCH_MESSAGE ) {
		Message_Key( key );
	} else if ( cls.state == CA_DISCONNECTED ) {

		Console_Key( key );
#else
		if ( !onlybinds ) {
			Console_Key( key );
		}
	} else if ( cls.keyCatchers & KEYCATCH_UI && !bypassMenu ) {
		if ( !onlybinds || VM_Call(uivm, UI_WANTSBINDKEYS) ) {
			VM_Call(uivm, UI_KEY_EVENT, rtcw::to_vm_arg(key), rtcw::to_vm_arg(down));
		}
	} else if ( cls.keyCatchers & KEYCATCH_CGAME && !bypassMenu ) {
		if ( cgvm ) {
			if ( !onlybinds || VM_Call(cgvm, CG_WANTSBINDKEYS) ) {
				VM_Call(cgvm, CG_KEY_EVENT, rtcw::to_vm_arg(key), rtcw::to_vm_arg(down));
			}
		}
	} else if ( cls.keyCatchers & KEYCATCH_MESSAGE ) {
		if ( !onlybinds ) {
			Message_Key( key );
		}
	} else if ( cls.state == CA_DISCONNECTED ) {

		if ( !onlybinds ) {
			Console_Key( key );
		}

#endif // RTCW_XX

	} else {
		// send the bound action
		kb = keys[key].binding;
		if ( !kb ) {
			if ( key >= 200 ) {
				Com_Printf( "%s is unbound, use controls menu to set.\n"
							, Key_KeynumToString( key, qfalse ) );
			}
		} else if ( kb[0] == '+' ) {
			// button commands add keynum and time as parms so that multiple
			// sources can be discriminated and subframe corrected
			Com_sprintf( cmd, sizeof( cmd ), "%s %i %i\n", kb, key, time );
			Cbuf_AddText( cmd );
		} else {
			// down-only command
			Cbuf_AddText( kb );
			Cbuf_AddText( "\n" );
		}
	}
}


/*
===================
CL_CharEvent

Normal keyboard characters, already shifted / capslocked / etc
===================
*/
void CL_CharEvent( int key ) {
	// the console key should never be used as a char

#if !defined RTCW_ET
	if ( key == '`' || key == '~' ) {
#else
	// ydnar: added uk equivalent of shift+`
	// the RIGHT way to do this would be to have certain keys disable the equivalent SE_CHAR event

	// fretn - this should be fixed in Com_EventLoop
	// but I can't be arsed to leave this as is

	if ( key == (unsigned char) '`' || key == (unsigned char) '~' || key == (unsigned char) '\xAC' ) {
#endif // RTCW_XX

		return;
	}

	// distribute the key down event to the apropriate handler
	if ( cls.keyCatchers & KEYCATCH_CONSOLE ) {
		Field_CharEvent( &g_consoleField, key );
	} else if ( cls.keyCatchers & KEYCATCH_UI )   {
		VM_Call(uivm, UI_KEY_EVENT, rtcw::to_vm_arg(key | K_CHAR_FLAG), rtcw::to_vm_arg(qtrue));

#if defined RTCW_ET
	} else if ( cls.keyCatchers & KEYCATCH_CGAME )   {
		VM_Call(cgvm, CG_KEY_EVENT, rtcw::to_vm_arg(key | K_CHAR_FLAG), rtcw::to_vm_arg(qtrue));
#endif // RTCW_XX

	} else if ( cls.keyCatchers & KEYCATCH_MESSAGE )   {
		Field_CharEvent( &chatField, key );
	} else if ( cls.state == CA_DISCONNECTED )   {
		Field_CharEvent( &g_consoleField, key );
	}
}


/*
===================
Key_ClearStates
===================
*/
void Key_ClearStates( void ) {
	int i;

	anykeydown = qfalse;

	for ( i = 0 ; i < MAX_KEYS ; i++ ) {
		if ( keys[i].down ) {
			CL_KeyEvent( i, qfalse, 0 );

		}
		keys[i].down = 0;
		keys[i].repeats = 0;
	}
}


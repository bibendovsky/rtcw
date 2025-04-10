/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		l_struct.h
 *
 * desc:		structure reading/writing
 *
 *
 *****************************************************************************/


#define MAX_STRINGFIELD             80
//field types
#define FT_CHAR                     1           // char
#define FT_INT                          2           // int
#define FT_FLOAT                        3           // float
#define FT_STRING                       4           // char [MAX_STRINGFIELD]
#define FT_STRUCT                       6           // struct (sub structure)
//type only mask
#define FT_TYPE                     0x00FF  // only type, clear subtype
//sub types
#define FT_ARRAY                        0x0100  // array of type
#define FT_BOUNDED                  0x0200  // bounded value
#define FT_UNSIGNED                 0x0400

//structure field definition
typedef struct fielddef_s
{
	const char *name;                                     //name of the field
	int offset;                                     //offset in the structure
	int type;                                       //type of the field
	//type specific fields
	int maxarray;                                   //maximum array size
	float floatmin, floatmax;                   //float min and max
	struct structdef_s *substruct;          //sub structure
} fielddef_t;

//structure definition
typedef struct structdef_s
{
	int size;
	fielddef_t *fields;
} structdef_t;

//read a structure from a script
int ReadStructure( source_t *source, structdef_t *def, char *structure );
//write a structure to a file
int WriteStructure( FILE *fp, structdef_t *def, char *structure );
//writes indents
int WriteIndent( FILE *fp, int indent );
//writes a float without traling zeros
int WriteFloat( FILE *fp, float value );



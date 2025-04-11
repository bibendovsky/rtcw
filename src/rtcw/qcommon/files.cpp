/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/


/*****************************************************************************
 * name:		files.c
 *
 * desc:		handle based filesystem for Quake III Arena
 *
 *
 *****************************************************************************/

#if defined RTCW_ET
#define MP_LEGACY_PAK 0x7776DC09
#endif // RTCW_XX

#include <limits>

#include "miniz.h"

#include "q_shared.h"
#include "qcommon.h"
#include "rtcw_endian.h"
#include "rtcw_string.h"
#include "rtcw_unique_ptr.h"


/*
=============================================================================

QUAKE3 FILESYSTEM

All of Quake's data access is through a hierarchical file system, but the contents of
the file system can be transparently merged from several sources.

A "qpath" is a reference to game file data.  MAX_ZPATH is 256 characters, which must include
a terminating zero. "..", "\\", and ":" are explicitly illegal in qpaths to prevent any
references outside the quake directory system.

The "base path" is the path to the directory holding all the game directories and usually
the executable.  It defaults to ".", but can be overridden with a "+set fs_basepath c:\quake3"
command line to allow code debugging in a different directory.  Basepath cannot
be modified at all after startup.  Any files that are created (demos, screenshots,
etc) will be created reletive to the base path, so base path should usually be writable.

The "cd path" is the path to an alternate hierarchy that will be searched if a file
is not located in the base path.  A user can do a partial install that copies some
data to a base path created on their hard drive and leave the rest on the cd.  Files
are never writen to the cd path.  It defaults to a value set by the installer, like
"e:\quake3", but it can be overridden with "+set ds_cdpath g:\quake3".

If a user runs the game directly from a CD, the base path would be on the CD.  This
should still function correctly, but all file writes will fail (harmlessly).

The "home path" is the path used for all write access. On win32 systems we have "base path"
== "home path", but on *nix systems the base installation is usually readonly, and
"home path" points to ~/.q3a or similar

The user can also install custom mods and content in "home path", so it should be searched
along with "home path" and "cd path" for game content.


The "base game" is the directory under the paths where data comes from by default, and
can be either "baseq3" or "demoq3".

The "current game" may be the same as the base game, or it may be the name of another
directory under the paths that should be searched for files before looking in the base game.
This is the basis for addons.

Clients automatically set the game directory after receiving a gamestate from a server,
so only servers need to worry about +set fs_game.

No other directories outside of the base game and current game will ever be referenced by
filesystem functions.

To save disk space and speed loading, directory trees can be collapsed into zip files.
The files use a ".pk3" extension to prevent users from unzipping them accidentally, but
otherwise the are simply normal uncompressed zip files.  A game directory can have multiple
zip files of the form "pak0.pk3", "pak1.pk3", etc.  Zip files are searched in decending order
from the highest number to the lowest, and will always take precedence over the filesystem.
This allows a pk3 distributed as a patch to override all existing data.

Because we will have updated executables freely available online, there is no point to
trying to restrict demo / oem versions of the game with code changes.  Demo / oem versions
should be exactly the same executables as release versions, but with different data that
automatically restricts where game media can come from to prevent add-ons from working.

After the paths are initialized, quake will look for the product.txt file.  If not
found and verified, the game will run in restricted mode.  In restricted mode, only
files contained in demoq3/pak0.pk3 will be available for loading, and only if the zip header is
verified to not have been modified.  A single exception is made for q3config.cfg.  Files
can still be written out in restricted mode, so screenshots and demos are allowed.
Restricted mode can be tested by setting "+set fs_restrict 1" on the command line, even
if there is a valid product.txt under the basepath or cdpath.

If not running in restricted mode, and a file is not found in any local filesystem,
an attempt will be made to download it and save it under the base path.

If the "fs_copyfiles" cvar is set to 1, then every time a file is sourced from the cd
path, it will be copied over to the base path.  This is a development aid to help build
test releases and to copy working sets over slow network links.

File search order: when FS_FOpenFileRead gets called it will go through the fs_searchpaths
structure and stop on the first successful hit. fs_searchpaths is built with successive
calls to FS_AddGameDirectory

Additionaly, we search in several subdirectories:
current game is the current mode
base game is a variable to allow mods based on other mods
(such as baseq3 + missionpack content combination in a mod for instance)
BASEGAME is the hardcoded base game ("baseq3")

e.g. the qpath "sound/newstuff/test.wav" would be searched for in the following places:

home path + current game's zip files
home path + current game's directory
base path + current game's zip files
base path + current game's directory
cd path + current game's zip files
cd path + current game's directory

home path + base game's zip file
home path + base game's directory
base path + base game's zip file
base path + base game's directory
cd path + base game's zip file
cd path + base game's directory

home path + BASEGAME's zip file
home path + BASEGAME's directory
base path + BASEGAME's zip file
base path + BASEGAME's directory
cd path + BASEGAME's zip file
cd path + BASEGAME's directory

server download, to be written to home path + current game's directory


The filesystem can be safely shutdown and reinitialized with different
basedir / cddir / game combinations, but all other subsystems that rely on it
(sound, video) must also be forced to restart.

Because the same files are loaded by both the clip model (CM_) and renderer (TR_)
subsystems, a simple single-file caching scheme is used.  The CM_ subsystems will
load the file with a request to cache.  Only one file will be kept cached at a time,
so any models that are going to be referenced by both subsystems should alternate
between the CM_ load function and the ref load function.

TODO: A qpath that starts with a leading slash will always refer to the base game, even if another
game is currently active.  This allows character models, skins, and sounds to be downloaded
to a common directory no matter which game is active.

How to prevent downloading zip files?
Pass pk3 file names in systeminfo, and download before FS_Restart()?

Aborting a download disconnects the client from the server.

How to mark files as downloadable?  Commercial add-ons won't be downloadable.

Non-commercial downloads will want to download the entire zip file.
the game would have to be reset to actually read the zip in

Auto-update information

Path separators

Casing

  separate server gamedir and client gamedir, so if the user starts
  a local game after having connected to a network game, it won't stick
  with the network game.

  allow menu options for game selection?

Read / write config to floppy option.

Different version coexistance?

When building a pak file, make sure a wolfconfig.cfg isn't present in it,
or configs will never get loaded from disk!

  todo:

  downloading (outside fs?)
  game directory passing and restarting

=============================================================================

*/


namespace
{


class MinizIo
{
public:
	MinizIo();
	~MinizIo();

	bool open(const char* file_name);
	void close();

	bool is_open() const;
	int64_t get_file_size() const;

	size_t read(int64_t position, void* buffer, size_t count);

private:
	FILE* file_;
	int64_t file_size_;

private:
	MinizIo(const MinizIo& that);
	MinizIo& operator=(const MinizIo&);
};

// --------------------------------------------------------------------------

MinizIo::MinizIo()
	:
	file_(),
	file_size_()
{}

MinizIo::~MinizIo()
{
	close();
}

bool MinizIo::open(const char* file_name)
{
	close();

	file_ = fopen(file_name, "rb");

	if (file_ == NULL)
	{
		return false;
	}

	if (fseek(file_, 0, SEEK_END) != 0)
	{
		close();
		return false;
	}

	file_size_ = ftell(file_);

	if (file_size_ < 0)
	{
		close();
		return false;
	}

	return true;
}

void MinizIo::close()
{
	if (file_ != NULL)
	{
		fclose(file_);
		file_ = NULL;
	}

	file_size_ = 0;
}

bool MinizIo::is_open() const
{
	return file_ != NULL;
}

int64_t MinizIo::get_file_size() const
{
	return file_size_;
}

size_t MinizIo::read(int64_t position, void* buffer, const size_t count)
{
	if (!is_open() || position > LONG_MAX)
	{
		return 0;
	}

	if (fseek(file_, static_cast<long>(position), SEEK_SET) != 0)
	{
		return 0;
	}

	return fread(buffer, 1, count, file_);
}

// ==========================================================================

class MinizZip
{
public:
	struct FileStat
	{
		rtcw::String file_name_;
		uint32_t crc_;
		int compressed_size_;
		int uncompressed_size_;
		bool is_directory_;
		bool is_encrypted_;
		bool is_supported_;

		FileStat()
			:
			file_name_(),
			crc_(),
			compressed_size_(),
			uncompressed_size_(),
			is_directory_(),
			is_encrypted_(),
			is_supported_()
		{}
	}; // FileStat


	class File
	{
	public:
		~File()
		{
			close();
		}

		bool is_open() const
		{
			return miniz_file_state_ != NULL;
		}

		int read(
			void* buffer_ptr,
			const int count)
		{
			if (!is_open() || !buffer_ptr || count <= 0)
			{
				return 0;
			}

			const int read_result = static_cast<int>(mz_zip_reader_extract_iter_read(miniz_file_state_, buffer_ptr, count));

			position_ += read_result;

			return static_cast<int>(read_result);
		}

		int get_position() const
		{
			return position_;
		}


	private:
		friend class MinizZip;


		mz_zip_reader_extract_iter_state* miniz_file_state_;
		int position_;

		File(const File&);
		File& operator=(const File&);


		explicit File(
			mz_zip_reader_extract_iter_state* miniz_file_state)
			:
			miniz_file_state_(miniz_file_state),
			position_()
		{}

		void close()
		{
			if (miniz_file_state_)
			{
				static_cast<void>(mz_zip_reader_extract_iter_free(miniz_file_state_));
				miniz_file_state_ = NULL;
			}
		}
	}; // File


	MinizZip()
		:
		is_open_(),
		miniz_zip_(),
		io_(),
		file_count_()
	{}

	~MinizZip()
	{
		close();
	}


	bool open(
		const char* const file_name)
	{
		close();

		if (!io_.open(file_name))
		{
			return false;
		}

		miniz_zip_.m_pIO_opaque = &io_;
		miniz_zip_.m_pRead = miniz_file_read_func;

		if (!::mz_zip_reader_init(&miniz_zip_, io_.get_file_size(), MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY))
		{
			close();
			return false;
		}

		const mz_uint miniz_file_count = mz_zip_reader_get_num_files(&miniz_zip_);

		if (miniz_file_count > static_cast<mz_uint>(std::numeric_limits<int>::max()))
		{
			close();
			return false;
		}

		is_open_ = true;
		file_count_ = static_cast<int>(miniz_file_count);

		return true;
	}

	void close()
	{
		is_open_ = false;
		mz_zip_reader_end(&miniz_zip_);
		io_.close();
		file_count_ = 0;

		memset(&miniz_zip_, 0, sizeof(mz_zip_archive));
	}

	bool is_open() const
	{
		return is_open_;
	}

	File* open_file(
		const int file_index)
	{
		if (!is_open() || file_index < 0 || file_index >= file_count_)
		{
			return NULL;
		}

		mz_zip_reader_extract_iter_state* miniz_file_state =
			mz_zip_reader_extract_iter_new(&miniz_zip_, static_cast<mz_uint>(file_index), 0);

		if (!miniz_file_state)
		{
			return NULL;
		}

		return new File(miniz_file_state);
	}

	int get_file_count() const
	{
		return file_count_;
	}

	int calculate_file_names_size()
	{
		if (!is_open_)
		{
			return 0;
		}

		int size = 0;

		for (int i = 0; i < file_count_; ++i)
		{
			const int file_name_size = static_cast<int>(
				mz_zip_reader_get_filename(&miniz_zip_, static_cast<mz_uint>(i), NULL, 0));

			if (file_name_size == 0)
			{
				return 0;
			}

			size += file_name_size;
		}

		return size;
	}

	FileStat get_file_stat(
		const int file_index)
	{
		if (!is_open_ || file_index < 0 || file_index >= file_count_)
		{
			return FileStat();
		}

		mz_zip_archive_file_stat miniz_file_stat;

		if (!::mz_zip_reader_file_stat(&miniz_zip_, static_cast<mz_uint>(file_index), &miniz_file_stat))
		{
			return FileStat();
		}

		FileStat file_stat;

		file_stat.file_name_ = miniz_file_stat.m_filename;
		file_stat.crc_ = miniz_file_stat.m_crc32;
		file_stat.compressed_size_ = static_cast<int>(miniz_file_stat.m_comp_size);
		file_stat.uncompressed_size_ = static_cast<int>(miniz_file_stat.m_uncomp_size);
		file_stat.is_directory_ = (miniz_file_stat.m_is_directory != MZ_FALSE);
		file_stat.is_encrypted_ = (miniz_file_stat.m_is_encrypted != MZ_FALSE);
		file_stat.is_supported_ = (miniz_file_stat.m_is_supported != MZ_FALSE);

		return file_stat;
	}


private:
	bool is_open_;
	mz_zip_archive miniz_zip_;
	MinizIo io_;
	int file_count_;

	MinizZip(const MinizZip&);
	MinizZip& operator=(const MinizZip&);

	mz_zip_reader_extract_iter_state* open_file_internal(
		const int file_index)
	{
		if (!is_open_ || file_index < 0 || file_index >= file_count_)
		{
			return NULL;
		}

		return mz_zip_reader_extract_iter_new(&miniz_zip_, static_cast<mz_uint>(file_index), 0);
	}

	static size_t miniz_file_read_func(
		void* opaque,
		mz_uint64 position,
		void* buffer_ptr,
		size_t count)
	{
		if (!opaque)
		{
			return 0;
		}

		MinizIo& miniz_io = *static_cast<MinizIo*>(opaque);

		return miniz_io.read(static_cast<int64_t>(position), buffer_ptr, count);
	}
}; // MinizZip


} // namespace


#if !defined RTCW_ET
// TTimo: moved to qcommon.h
// NOTE: could really do with a cvar
//#define	BASEGAME			"main"
#define DEMOGAME            "demomain"

// every time a new demo pk3 file is built, this checksum must be updated.
// the easiest way to get it is to just run the game and see what it spits out

#if defined RTCW_MP
//DHM - Nerve :: Wolf Multiplayer demo checksum
#endif // RTCW_XX

// NOTE TTimo: always needs the 'u' for unsigned int (gcc)

#if defined RTCW_SP
#define DEMO_PAK_CHECKSUM   2985661941u

// if this is defined, the executable positively won't work with any paks other
// than the demo pak, even if productid is present.  This is only used for our
// last demo release to prevent the mac and linux users from using the demo
// executable with the production windows pak before the mac/linux products
// hit the shelves a little later
// NOW defined in build files
//#define PRE_RELEASE_TADEMO
#elif defined RTCW_MP
#define DEMO_PAK_CHECKSUM   2031778175u
#endif // RTCW_XX

#endif // RTCW_XX

#define MAX_ZPATH           256
#define MAX_SEARCH_PATHS    4096
#define MAX_FILEHASH_SIZE   1024

typedef struct fileInPack_s {
	char                    *name;      // name of the file
	int miniz_file_index_; // file info position in zip
	struct  fileInPack_s*   next;       // next file in the hash
} fileInPack_t;

typedef struct {
	char pakFilename[MAX_OSPATH];               // c:\quake3\baseq3\pak0.pk3
	char pakBasename[MAX_OSPATH];               // pak0
	char pakGamename[MAX_OSPATH];               // baseq3
	MinizZip* miniz_zip_ptr_;					// handle to zip file
	int checksum;                               // regular checksum
	int pure_checksum;                          // checksum for pure
	int numfiles;                               // number of files in pk3
	int referenced;                             // referenced file flags
	int hashSize;                               // hash table size (power of 2)
	fileInPack_t*   *hashTable;                 // hash table
	fileInPack_t*   buildBuffer;                // buffer with the filenames etc.
} pack_t;

typedef struct {
	char path[MAX_OSPATH];              // c:\quake3
	char gamedir[MAX_OSPATH];           // baseq3
} directory_t;

typedef struct searchpath_s {
	struct searchpath_s *next;

	pack_t      *pack;      // only one of pack / dir will be non NULL
	directory_t *dir;
} searchpath_t;

#if !defined RTCW_ET
static char fs_gamedir[MAX_OSPATH];         // this will be a single file name with no separators
#else
//bani - made fs_gamedir non-static
char fs_gamedir[MAX_OSPATH];        // this will be a single file name with no separators
#endif // RTCW_XX

static cvar_t      *fs_debug;
static cvar_t      *fs_homepath;
static cvar_t      *fs_basepath;

#if defined RTCW_ET
static cvar_t      *fs_buildpath;
static cvar_t      *fs_buildgame;
#endif // RTCW_XX

static cvar_t      *fs_basegame;
static cvar_t      *fs_cdpath;
static cvar_t      *fs_copyfiles;
static cvar_t      *fs_gamedirvar;
static cvar_t      *fs_restrict;
static searchpath_t    *fs_searchpaths;
static int fs_readCount;                    // total bytes read
static int fs_loadCount;                    // total files read
static int fs_loadStack;                    // total files in memory
static int fs_packFiles;                    // total number of files in packs

static int fs_fakeChkSum;
static int fs_checksumFeed;

typedef struct qfile_gus {
	FILE* o;
	MinizZip* miniz_zip_ptr_;
	MinizZip::File* miniz_file_ptr_;
} qfile_gut;

typedef struct qfile_us {
	qfile_gut file;
	qboolean unique;
} qfile_ut;

typedef struct {
	qfile_ut handleFiles;
	qboolean handleSync;
	int fileSize;
	int zipFilePos;
	qboolean zipFile;
	qboolean streamed;
	char name[MAX_ZPATH];
} fileHandleData_t;

static fileHandleData_t fsh[MAX_FILE_HANDLES];

#if !defined RTCW_SP
// TTimo - show_bug.cgi?id=540
// wether we did a reorder on the current search path when joining the server
static qboolean fs_reordered;
#endif // RTCW_XX

// never load anything from pk3 files that are not present at the server when pure

#if !defined RTCW_SP
// ex: when fs_numServerPaks != 0, FS_FOpenFileRead won't load anything outside of pk3 except .cfg .menu .game .dat
#endif // RTCW_XX

static int fs_numServerPaks;
static int fs_serverPaks[MAX_SEARCH_PATHS];                     // checksums
static char     *fs_serverPakNames[MAX_SEARCH_PATHS];           // pk3 names

// only used for autodownload, to make sure the client has at least
// all the pk3 files that are referenced at the server side
static int fs_numServerReferencedPaks;
static int fs_serverReferencedPaks[MAX_SEARCH_PATHS];               // checksums
static char     *fs_serverReferencedPakNames[MAX_SEARCH_PATHS];     // pk3 names

// last valid game folder used
char lastValidBase[MAX_OSPATH];
char lastValidGame[MAX_OSPATH];

#ifdef FS_MISSING
FILE*       missingFiles = NULL;
#endif

#if defined RTCW_ET
qboolean legacy_mp_bin = qfalse;
#endif // RTCW_XX


/*
==============
FS_Initialized
==============
*/

qboolean FS_Initialized() {
	return ( fs_searchpaths != NULL );
}

/*
=================
FS_PakIsPure
=================
*/
qboolean FS_PakIsPure( pack_t *pack ) {
	int i;

	if ( fs_numServerPaks ) {

#if defined RTCW_SP
		// NOTE TTimo we are matching checksums without checking the pak names
		//   this means you can have the same pk3 as the server under a different name, you will still get through sv_pure validation
		//   (what happens when two pk3's have the same checkums? is it a likely situation?)
		//   also, if there's a wrong checksumed pk3 and autodownload is enabled, the checksum will be appended to the downloaded pk3 name
#endif // RTCW_XX

		for ( i = 0 ; i < fs_numServerPaks ; i++ ) {
			// FIXME: also use hashed file names

#if !defined RTCW_SP
			// NOTE TTimo: a pk3 with same checksum but different name would be validated too
			//   I don't see this as allowing for any exploit, it would only happen if the client does manips of it's file names 'not a bug'
#endif // RTCW_XX

			if ( pack->checksum == fs_serverPaks[i] ) {
				return qtrue;       // on the approved list
			}
		}
		return qfalse;  // not on the pure server pak list
	}
	return qtrue;
}


/*
=================
FS_LoadStack
return load stack
=================
*/
int FS_LoadStack() {
	return fs_loadStack;
}

/*
================
return a hash value for the filename
================
*/
static int32_t FS_HashFileName( const char *fname, int hashSize ) {
	int i;
	int32_t hash;
	char letter;

	hash = 0;
	i = 0;
	while ( fname[i] != '\0' ) {
		letter = tolower( fname[i] );
		if ( letter == '.' ) {
			break;                          // don't include extension
		}
		if ( letter == '\\' ) {
			letter = '/';                   // damn path names
		}
		if ( letter == PATH_SEP ) {
			letter = '/';                           // damn path names
		}
		hash += (int32_t)( letter ) * ( i + 119 );
		i++;
	}
	hash = ( hash ^ ( hash >> 10 ) ^ ( hash >> 20 ) );
	hash &= ( hashSize - 1 );
	return hash;
}

static fileHandle_t FS_HandleForFile( void ) {
	int i;

	for ( i = 1 ; i < MAX_FILE_HANDLES ; i++ ) {
		const qfile_gut& file = fsh[i].handleFiles.file;

		if (!(file.o || file.miniz_file_ptr_ || file.miniz_zip_ptr_)) {
			return i;
		}
	}

	Com_Error( ERR_DROP, "FS_HandleForFile: none free" );
	return 0;
}

static FILE *FS_FileForHandle( fileHandle_t f ) {
	if ( f < 0 || f > MAX_FILE_HANDLES ) {

#if !defined RTCW_ET
		Com_Error( ERR_DROP, "FS_FileForHandle: out of reange" );
#else
		Com_Error( ERR_DROP, "FS_FileForHandle: %d out of range", f );
#endif // RTCW_XX

	}
	if ( fsh[f].zipFile == qtrue ) {
		Com_Error( ERR_DROP, "FS_FileForHandle: can't get FILE on zip file" );
	}
	if ( !fsh[f].handleFiles.file.o ) {
		Com_Error( ERR_DROP, "FS_FileForHandle: NULL" );
	}

	return fsh[f].handleFiles.file.o;
}

void    FS_ForceFlush( fileHandle_t f ) {
	FILE *file;

	file = FS_FileForHandle( f );
	setvbuf( file, NULL, _IONBF, 0 );
}

/*
================
FS_filelength

If this is called on a non-unique FILE (from a pak file),
it will return the size of the pak file, not the expected
size of the file.
================
*/
int FS_filelength( fileHandle_t f ) {
	int pos;
	int end;
	FILE*   h;

	h = FS_FileForHandle( f );
	pos = ftell( h );
	fseek( h, 0, SEEK_END );
	end = ftell( h );
	fseek( h, pos, SEEK_SET );

	return end;
}

/*
====================
FS_ReplaceSeparators

Fix things up differently for win/unix/mac
====================
*/
static void FS_ReplaceSeparators( char *path ) {
	char    *s;

	for ( s = path ; *s ; s++ ) {
		if ( *s == '/' || *s == '\\' ) {
			*s = PATH_SEP;
		}
	}
}

/*
===================
FS_BuildOSPath

Qpath may have either forward or backwards slashes
===================
*/
char *FS_BuildOSPath( const char *base, const char *game, const char *qpath ) {
	char temp[MAX_OSPATH];
	static char ospath[2][MAX_OSPATH];
	static int toggle;

	toggle ^= 1;        // flip-flop to allow two returns without clash

	if ( !game || !game[0] ) {
		game = fs_gamedir;
	}

	Com_sprintf( temp, sizeof( temp ), "/%s/%s", game, qpath );
	FS_ReplaceSeparators( temp );
	Com_sprintf( ospath[toggle], sizeof( ospath[0] ), "%s%s", base, temp );

	return ospath[toggle];
}


/*
============
FS_CreatePath

Creates any directories needed to store the given filename
============
*/

#if !defined RTCW_ET
static qboolean FS_CreatePath( char *OSPath ) {
	char    *ofs;
#else
int FS_CreatePath( const char *OSPath_ ) {
	// use va() to have a clean const char* prototype
	char *OSPath = va( "%s", OSPath_ );
	char *ofs;
#endif // RTCW_XX

	// make absolutely sure that it can't back up the path
	// FIXME: is c: allowed???
	if ( strstr( OSPath, ".." ) || strstr( OSPath, "::" ) ) {
		Com_Printf( "WARNING: refusing to create relative path \"%s\"\n", OSPath );
		return qtrue;
	}

	for ( ofs = OSPath + 1 ; *ofs ; ofs++ ) {
		if ( *ofs == PATH_SEP ) {
			// create the directory
			*ofs = 0;
			Sys_Mkdir( OSPath );
			*ofs = PATH_SEP;
		}
	}
	return qfalse;
}

/*
=================
FS_CopyFile

Copy a fully specified file from one place to another
=================
*/

#if defined RTCW_SP
static void FS_CopyFile( char *fromOSPath, char *toOSPath ) {
#else
void FS_CopyFile( char *fromOSPath, char *toOSPath ) {
#endif // RTCW_XX

	FILE    *f;
	int len;
	byte    *buf;

#if defined RTCW_SP
	//Com_Printf( "copy %s to %s\n", fromOSPath, toOSPath );
#else
	Com_Printf( "copy %s to %s\n", fromOSPath, toOSPath );
#endif // RTCW_XX

	if ( strstr( fromOSPath, "journal.dat" ) || strstr( fromOSPath, "journaldata.dat" ) ) {
		Com_Printf( "Ignoring journal files\n" );
		return;
	}

	f = fopen( fromOSPath, "rb" );
	if ( !f ) {
		return;
	}
	fseek( f, 0, SEEK_END );
	len = ftell( f );
	fseek( f, 0, SEEK_SET );

#if !defined RTCW_ET
	// we are using direct malloc instead of Z_Malloc here, so it
	// probably won't work on a mac... Its only for developers anyway...
#endif // RTCW_XX

	// BBi
	//buf = static_cast<byte*> (malloc( len ));
	buf = new byte[len];
	// BBi

	if ( fread( buf, 1, len, f ) != len ) {
		Com_Error( ERR_FATAL, "Short read in FS_Copyfiles()\n" );
	}
	fclose( f );

	if ( FS_CreatePath( toOSPath ) ) {
		return;
	}

	f = fopen( toOSPath, "wb" );
	if ( !f ) {

#if !defined RTCW_SP
		// BBi
		//free( buf );    //DAJ free as well
		delete [] buf; //DAJ free as well
		// BBi
#endif // RTCW_XX

		return;
	}
	if ( fwrite( buf, 1, len, f ) != len ) {
		Com_Error( ERR_FATAL, "Short write in FS_Copyfiles()\n" );
	}
	fclose( f );

	// BBi
	//free( buf );
	delete [] buf;
	// BBi
}

#if defined RTCW_SP
void FS_CopyFileOS( char *from, char *to ) {
	FILE    *f;
	int len;
	byte    *buf;
	char *fromOSPath, *toOSPath;

	fromOSPath = FS_BuildOSPath( fs_homepath->string, fs_gamedir, from );
	toOSPath = FS_BuildOSPath( fs_homepath->string, fs_gamedir, to );

	//Com_Printf( "copy %s to %s\n", fromOSPath, toOSPath );

	if ( strstr( fromOSPath, "journal.dat" ) || strstr( fromOSPath, "journaldata.dat" ) ) {
		Com_Printf( "Ignoring journal files\n" );
		return;
	}

	f = fopen( fromOSPath, "rb" );
	if ( !f ) {
		return;
	}
	fseek( f, 0, SEEK_END );
	len = ftell( f );
	fseek( f, 0, SEEK_SET );

	// we are using direct malloc instead of Z_Malloc here, so it
	// probably won't work on a mac... Its only for developers anyway...

	// BBi
	//buf = static_cast<byte*> (malloc( len ));
	buf = new byte[len];
	// BBi

	if ( fread( buf, 1, len, f ) != len ) {
		Com_Error( ERR_FATAL, "Short read in FS_Copyfiles()\n" );
	}
	fclose( f );

	if ( FS_CreatePath( toOSPath ) ) {
		return;
	}

	f = fopen( toOSPath, "wb" );
	if ( !f ) {
		return;
	}
	if ( fwrite( buf, 1, len, f ) != len ) {
		Com_Error( ERR_FATAL, "Short write in FS_Copyfiles()\n" );
	}
	fclose( f );

	// BBi
	//free( buf );
	delete [] buf;
	// BBi
}
#endif // RTCW_XX

extern bool FS_Remove(const char* path);

/*
================
FS_FileExists

Tests if the file exists in the current gamedir, this DOES NOT
search the paths.  This is to determine if opening a file to write
(which always goes into the current gamedir) will cause any overwrites.
NOTE TTimo: this goes with FS_FOpenFileWrite for opening the file afterwards
================
*/
qboolean FS_FileExists( const char *file ) {
	FILE *f;
	char *testpath;

	testpath = FS_BuildOSPath( fs_homepath->string, fs_gamedir, file );

	f = fopen( testpath, "rb" );
	if ( f ) {
		fclose( f );
		return qtrue;
	}
	return qfalse;
}

/*
================
FS_SV_FileExists

Tests if the file exists
================
*/
qboolean FS_SV_FileExists( const char *file ) {
	FILE *f;
	char *testpath;

	testpath = FS_BuildOSPath( fs_homepath->string, file, "" );
	testpath[strlen( testpath ) - 1] = '\0';

	f = fopen( testpath, "rb" );
	if ( f ) {
		fclose( f );
		return qtrue;
	}
	return qfalse;
}

#if defined RTCW_ET
qboolean FS_OS_FileExists( const char *file ) {
	FILE *f;
	f = fopen( file, "rb" );
	if ( f ) {
		fclose( f );
		return qtrue;
	}
	return qfalse;
}
#endif // RTCW_XX

/*
===========
FS_SV_FOpenFileWrite

===========
*/
fileHandle_t FS_SV_FOpenFileWrite( const char *filename ) {
	char *ospath;
	fileHandle_t f;

	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization\n" );
	}

	ospath = FS_BuildOSPath( fs_homepath->string, filename, "" );
	ospath[strlen( ospath ) - 1] = '\0';

	f = FS_HandleForFile();
	fsh[f].zipFile = qfalse;

	if ( fs_debug->integer ) {
		Com_Printf( "FS_SV_FOpenFileWrite: %s\n", ospath );
	}

	if ( FS_CreatePath( ospath ) ) {
		return 0;
	}

	Com_DPrintf( "writing to: %s\n", ospath );
	fsh[f].handleFiles.file.o = fopen( ospath, "wb" );

	Q_strncpyz( fsh[f].name, filename, sizeof( fsh[f].name ) );

	fsh[f].handleSync = qfalse;
	if ( !fsh[f].handleFiles.file.o ) {
		f = 0;
	}
	return f;
}

/*
===========
FS_SV_FOpenFileRead
search for a file somewhere below the home path, base path or cd path
we search in that order, matching FS_SV_FOpenFileRead order
===========
*/
int FS_SV_FOpenFileRead( const char *filename, fileHandle_t *fp ) {
	char *ospath;
	fileHandle_t f = 0;

	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization\n" );
	}

	f = FS_HandleForFile();
	fsh[f].zipFile = qfalse;

	Q_strncpyz( fsh[f].name, filename, sizeof( fsh[f].name ) );

	// don't let sound stutter

#if !defined RTCW_MP
//	S_ClearSoundBuffer();
#else
	S_ClearSoundBuffer();
#endif // RTCW_XX

	// search homepath
	ospath = FS_BuildOSPath( fs_homepath->string, filename, "" );
	// remove trailing slash
	ospath[strlen( ospath ) - 1] = '\0';

	if ( fs_debug->integer ) {
		Com_Printf( "FS_SV_FOpenFileRead (fs_homepath): %s\n", ospath );
	}

	fsh[f].handleFiles.file.o = fopen( ospath, "rb" );
	fsh[f].handleSync = qfalse;
	if ( !fsh[f].handleFiles.file.o ) {
		// NOTE TTimo on non *nix systems, fs_homepath == fs_basepath, might want to avoid
		if ( Q_stricmp( fs_homepath->string,fs_basepath->string ) ) {
			// search basepath
			ospath = FS_BuildOSPath( fs_basepath->string, filename, "" );
			ospath[strlen( ospath ) - 1] = '\0';

			if ( fs_debug->integer ) {
				Com_Printf( "FS_SV_FOpenFileRead (fs_basepath): %s\n", ospath );
			}

			fsh[f].handleFiles.file.o = fopen( ospath, "rb" );
			fsh[f].handleSync = qfalse;

			if ( !fsh[f].handleFiles.file.o ) {
				f = 0;
			}
		}
	}

	if ( !fsh[f].handleFiles.file.o ) {
		// search cd path
		ospath = FS_BuildOSPath( fs_cdpath->string, filename, "" );
		ospath[strlen( ospath ) - 1] = '\0';

		if ( fs_debug->integer ) {
			Com_Printf( "FS_SV_FOpenFileRead (fs_cdpath) : %s\n", ospath );
		}

		fsh[f].handleFiles.file.o = fopen( ospath, "rb" );
		fsh[f].handleSync = qfalse;

		if ( !fsh[f].handleFiles.file.o ) {
			f = 0;
		}
	}

	*fp = f;
	if ( f ) {
		return FS_filelength( f );
	}
	return 0;
}

extern bool sys_fs_rename(const char* old_path, const char* new_path);

/*
===========
FS_SV_Rename

===========
*/
void FS_SV_Rename( const char *from, const char *to ) {
	char            *from_ospath, *to_ospath;

	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization\n" );
	}

	// don't let sound stutter

#if !defined RTCW_MP
//	S_ClearSoundBuffer();
#else
	S_ClearSoundBuffer();
#endif // RTCW_XX

	from_ospath = FS_BuildOSPath( fs_homepath->string, from, "" );
	to_ospath = FS_BuildOSPath( fs_homepath->string, to, "" );
	from_ospath[strlen( from_ospath ) - 1] = '\0';
	to_ospath[strlen( to_ospath ) - 1] = '\0';

	if ( fs_debug->integer ) {
		Com_Printf( "FS_SV_Rename: %s --> %s\n", from_ospath, to_ospath );
	}

	if (!sys_fs_rename(from_ospath, to_ospath))
	{
		// Failed, try copying it and deleting the original
		FS_CopyFile( from_ospath, to_ospath );
		FS_Remove( from_ospath );
	}
}



/*
===========
FS_Rename

===========
*/
void FS_Rename( const char *from, const char *to ) {
	char            *from_ospath, *to_ospath;

	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization\n" );
	}

	// don't let sound stutter

#if !defined RTCW_MP
//	S_ClearSoundBuffer();
#else
	S_ClearSoundBuffer();
#endif // RTCW_XX

	from_ospath = FS_BuildOSPath( fs_homepath->string, fs_gamedir, from );
	to_ospath = FS_BuildOSPath( fs_homepath->string, fs_gamedir, to );

	if ( fs_debug->integer ) {
		Com_Printf( "FS_Rename: %s --> %s\n", from_ospath, to_ospath );
	}

	if (!sys_fs_rename(from_ospath, to_ospath))
	{
#if defined RTCW_SP
		// Failed first attempt, try deleting destination, and renaming again
		FS_Remove( to_ospath );
		if (!sys_fs_rename(from_ospath, to_ospath))
		{
#endif // RTCW_XX

			// Failed, try copying it and deleting the original
			FS_CopyFile( from_ospath, to_ospath );
			FS_Remove( from_ospath );

#if defined RTCW_SP
		}
#endif // RTCW_XX

	}
}

/*
==============
FS_FCloseFile

If the FILE pointer is an open pak file, leave it open.

For some reason, other dll's can't just cal fclose()
on files returned by FS_FOpenFile...
==============
*/
void FS_FCloseFile( fileHandle_t f ) {
	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization\n" );
	}

	if (fsh[f].zipFile == qtrue)
	{
		qfile_gut& file = fsh[f].handleFiles.file;

		delete file.miniz_file_ptr_;
		file.miniz_file_ptr_ = NULL;

		if (fsh[f].handleFiles.unique)
		{
			delete file.miniz_zip_ptr_;
			file.miniz_zip_ptr_ = NULL;
		}

		Com_Memset(&fsh[f], 0, sizeof(fsh[f]));
		return;
	}

	// we didn't find it as a pak, so close it as a unique file
	if ( fsh[f].handleFiles.file.o ) {
		fclose( fsh[f].handleFiles.file.o );
	}
	Com_Memset( &fsh[f], 0, sizeof( fsh[f] ) );
}

/*
===========
FS_FOpenFileWrite

===========
*/
fileHandle_t FS_FOpenFileWrite( const char *filename ) {
	char            *ospath;
	fileHandle_t f;

	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization\n" );
	}

	f = FS_HandleForFile();
	fsh[f].zipFile = qfalse;

	ospath = FS_BuildOSPath( fs_homepath->string, fs_gamedir, filename );

	if ( fs_debug->integer ) {
		Com_Printf( "FS_FOpenFileWrite: %s\n", ospath );
	}

	if ( FS_CreatePath( ospath ) ) {
		return 0;
	}

	// enabling the following line causes a recursive function call loop
	// when running with +set logfile 1 +set developer 1
	//Com_DPrintf( "writing to: %s\n", ospath );
	fsh[f].handleFiles.file.o = fopen( ospath, "wb" );

	Q_strncpyz( fsh[f].name, filename, sizeof( fsh[f].name ) );

	fsh[f].handleSync = qfalse;
	if ( !fsh[f].handleFiles.file.o ) {
		f = 0;
	}
	return f;
}

/*
===========
FS_FOpenFileAppend

===========
*/
fileHandle_t FS_FOpenFileAppend( const char *filename ) {
	char            *ospath;
	fileHandle_t f;

	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization\n" );
	}

	f = FS_HandleForFile();
	fsh[f].zipFile = qfalse;

	Q_strncpyz( fsh[f].name, filename, sizeof( fsh[f].name ) );

	// don't let sound stutter

#if !defined RTCW_MP
//	S_ClearSoundBuffer();
#else
	S_ClearSoundBuffer();
#endif // RTCW_XX

	ospath = FS_BuildOSPath( fs_homepath->string, fs_gamedir, filename );

	if ( fs_debug->integer ) {
		Com_Printf( "FS_FOpenFileAppend: %s\n", ospath );
	}

	if ( FS_CreatePath( ospath ) ) {
		return 0;
	}

	fsh[f].handleFiles.file.o = fopen( ospath, "ab" );
	fsh[f].handleSync = qfalse;
	if ( !fsh[f].handleFiles.file.o ) {
		f = 0;
	}
	return f;
}

/*
===========
FS_FilenameCompare

Ignore case and seprator char distinctions
===========
*/
qboolean FS_FilenameCompare( const char *s1, const char *s2 ) {
	int c1, c2;

	do {
		c1 = *s1++;
		c2 = *s2++;

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

		if ( c1 == '\\' || c1 == ':' ) {
			c1 = '/';
		}
		if ( c2 == '\\' || c2 == ':' ) {
			c2 = '/';
		}

		if ( c1 != c2 ) {
			return -1;      // strings not equal
		}
	} while ( c1 );

	return 0;       // strings are equal
}

#if defined RTCW_SP
/*
===========
FS_FileCompare

Do a binary check of the two files, return qfalse if they are different, otherwise qtrue
===========
*/
qboolean FS_FileCompare( const char *s1, const char *s2 ) {
	FILE    *f1, *f2;
	int len1, len2, pos;
	byte    *b1, *b2, *p1, *p2;

	f1 = fopen( s1, "rb" );
	if ( !f1 ) {
		Com_Error( ERR_FATAL, "FS_FileCompare: %s does not exist\n", s1 );
	}

	f2 = fopen( s2, "rb" );
	if ( !f2 ) {  // this file is allowed to not be there, since it might not exist in the previous build
		fclose( f1 );
		return qfalse;
		//Com_Error( ERR_FATAL, "FS_FileCompare: %s does not exist\n", s2 );
	}

	// first do a length test
	pos = ftell( f1 );
	fseek( f1, 0, SEEK_END );
	len1 = ftell( f1 );
	fseek( f1, pos, SEEK_SET );

	pos = ftell( f2 );
	fseek( f2, 0, SEEK_END );
	len2 = ftell( f2 );
	fseek( f2, pos, SEEK_SET );

	if ( len1 != len2 ) {
		fclose( f1 );
		fclose( f2 );
		return qfalse;
	}

	// now do a binary compare

	// BBi
	//b1 = static_cast<byte*> (malloc( len1 ));
	b1 = new byte[len1];
	// BBi

	if ( fread( b1, 1, len1, f1 ) != len1 ) {
		Com_Error( ERR_FATAL, "Short read in FS_FileCompare()\n" );
	}
	fclose( f1 );

	// BBi
	//b2 = static_cast<byte*> (malloc( len2 ));
	b2 = new byte[len2];
	// BBi

	if ( fread( b2, 1, len2, f2 ) != len2 ) {
		Com_Error( ERR_FATAL, "Short read in FS_FileCompare()\n" );
	}
	fclose( f2 );

	//if (!memcmp(b1, b2, (int)min(len1,len2) )) {
	p1 = b1;
	p2 = b2;
	for ( pos = 0; pos < len1; pos++, p1++, p2++ )
	{
		if ( *p1 != *p2 ) {

			// BBi
			//free( b1 );
			//free( b2 );

			delete [] b1;
			delete [] b2;
			// BBi

			return qfalse;
		}
	}
	//}

	// they are identical

	// BBi
	//free( b1 );
	//free( b2 );

	delete [] b1;
	delete [] b2;
	// BBi

	return qtrue;
}
#endif // RTCW_XX

/*
===========
FS_ShiftedStrStr
===========
*/
char *FS_ShiftedStrStr( const char *string, const char *substring, int shift ) {
	char buf[MAX_STRING_TOKENS];
	int i;

	for ( i = 0; substring[i]; i++ ) {
		buf[i] = substring[i] + shift;
	}
	buf[i] = '\0';
	return const_cast<char*> (strstr( string, buf ));
}

#if !defined RTCW_SP
/*
==========
FS_ShiftStr
perform simple string shifting to avoid scanning from the exe
==========
*/
char *FS_ShiftStr( const char *string, int shift ) {
	static char buf[MAX_STRING_CHARS];
	int i,l;

	l = strlen( string );
	for ( i = 0; i < l; i++ ) {
		buf[i] = string[i] + shift;
	}
	buf[i] = '\0';
	return buf;
}
#endif // RTCW_XX

/*
===========
FS_FOpenFileRead

Finds the file in the search path.
Returns filesize and an open FILE pointer.
Used for streaming data out of either a
separate file or a ZIP file.
===========
*/
extern qboolean com_fullyInitialized;

#if !defined RTCW_SP
// see FS_FOpenFileRead_Filtered
static int fs_filter_flag = 0;
#endif // RTCW_XX

int FS_FOpenFileRead( const char *filename, fileHandle_t *file, qboolean uniqueFILE ) {
	searchpath_t    *search;
	char            *netpath;
	pack_t          *pak;
	fileInPack_t    *pakFile;
	directory_t     *dir;
	int32_t hash;

	FILE            *temp;
	int l;
	char demoExt[16];

	hash = 0;

	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization\n" );
	}

#if !defined RTCW_SP
	// TTimo - NOTE
	// when checking for file existence, it's probably safer to use FS_FileExists, as I'm not
	// sure this chunk of code is really up to date with everything
#endif // RTCW_XX

	if ( file == NULL ) {
		// just wants to see if file is there
		for ( search = fs_searchpaths ; search ; search = search->next ) {
			//
			if ( search->pack ) {
				hash = FS_HashFileName( filename, search->pack->hashSize );
			}
			// is the element a pak file?
			if ( search->pack && search->pack->hashTable[hash] ) {

#if !defined RTCW_SP
				if ( fs_filter_flag & FS_EXCLUDE_PK3 ) {
					continue;
				}
#endif // RTCW_XX

				// look through all the pak file elements
				pak = search->pack;
				pakFile = pak->hashTable[hash];
				do {
					// case and separator insensitive comparisons
					if ( !FS_FilenameCompare( pakFile->name, filename ) ) {
						// found it!
						return qtrue;
					}
					pakFile = pakFile->next;
				} while ( pakFile != NULL );
			} else if ( search->dir ) {

#if !defined RTCW_SP
				if ( fs_filter_flag & FS_EXCLUDE_DIR ) {
					continue;
				}
#endif // RTCW_XX

				dir = search->dir;

				netpath = FS_BuildOSPath( dir->path, dir->gamedir, filename );
				temp = fopen( netpath, "rb" );
				if ( !temp ) {
					continue;
				}
				fclose( temp );
				return qtrue;
			}
		}
		return qfalse;
	}

	if ( !filename ) {
		Com_Error( ERR_FATAL, "FS_FOpenFileRead: NULL 'filename' parameter passed\n" );
	}

	Com_sprintf( demoExt, sizeof( demoExt ), ".dm_%d",PROTOCOL_VERSION );
	// qpaths are not supposed to have a leading slash
	if ( filename[0] == '/' || filename[0] == '\\' ) {
		filename++;
	}

	// make absolutely sure that it can't back up the path.
	// The searchpaths do guarantee that something will always
	// be prepended, so we don't need to worry about "c:" or "//limbo"
	if ( strstr( filename, ".." ) || strstr( filename, "::" ) ) {
		*file = 0;
		return -1;
	}

	// make sure the q3key file is only readable by the quake3.exe at initialization
	// any other time the key should only be accessed in memory using the provided functions

#if !defined RTCW_ET
	if ( com_fullyInitialized && strstr( filename, "rtcwkey" ) ) {
#else
	if ( com_fullyInitialized && strstr( filename, "etkey" ) ) {
#endif // RTCW_XX

		*file = 0;
		return -1;
	}

	//
	// search through the path, one element at a time
	//

	*file = FS_HandleForFile();
	fsh[*file].handleFiles.unique = uniqueFILE;

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		//
		if ( search->pack ) {
			hash = FS_HashFileName( filename, search->pack->hashSize );
		}
		// is the element a pak file?
		if ( search->pack && search->pack->hashTable[hash] ) {

#if !defined RTCW_SP
			if ( fs_filter_flag & FS_EXCLUDE_PK3 ) {
				continue;
			}
#endif // RTCW_XX

			// disregard if it doesn't match one of the allowed pure pak files
			if ( !FS_PakIsPure( search->pack ) ) {
				continue;
			}

			// look through all the pak file elements
			pak = search->pack;
			pakFile = pak->hashTable[hash];
			do {
				// case and separator insensitive comparisons
				if ( !FS_FilenameCompare( pakFile->name, filename ) ) {
					// found it!

					// mark the pak as having been referenced and mark specifics on cgame and ui
					// shaders, txt, arena files  by themselves do not count as a reference as
					// these are loaded from all pk3s
					// from every pk3 file..
					l = strlen( filename );
					if ( !( pak->referenced & FS_GENERAL_REF ) ) {
						if ( Q_stricmp( filename + l - 7, ".shader" ) != 0 &&
							 Q_stricmp( filename + l - 4, ".txt" ) != 0 &&
							 Q_stricmp( filename + l - 4, ".cfg" ) != 0 &&
							 Q_stricmp( filename + l - 7, ".config" ) != 0 &&
							 strstr( filename, "levelshots" ) == NULL &&
							 Q_stricmp( filename + l - 4, ".bot" ) != 0 &&
							 Q_stricmp( filename + l - 6, ".arena" ) != 0 &&
							 Q_stricmp( filename + l - 5, ".menu" ) != 0 ) {
							pak->referenced |= FS_GENERAL_REF;
						}
					}

#if defined RTCW_SP
					// qagame.qvm	- 13
					// dTZT`X!di`
					if ( !( pak->referenced & FS_QAGAME_REF ) && FS_ShiftedStrStr( filename, "dTZT`X!di`", 13 ) ) {
						pak->referenced |= FS_QAGAME_REF;
					}
					// cgame.qvm	- 7
					// \`Zf^'jof
					if ( !( pak->referenced & FS_CGAME_REF ) && FS_ShiftedStrStr( filename, "\\`Zf^'jof", 7 ) ) {
						pak->referenced |= FS_CGAME_REF;
					}
					// ui.qvm		- 5
					// pd)lqh
					if ( !( pak->referenced & FS_UI_REF ) && FS_ShiftedStrStr( filename, "pd)lqh", 5 ) ) {
						pak->referenced |= FS_UI_REF;
					}
#else
					// for OS client/server interoperability, we expect binaries for .so and .dll to be in the same pk3
					// so that when we reference the DLL files on any platform, this covers everyone else

#if 0 // TTimo: use that stuff for shifted strings
					Com_Printf( "SYS_DLLNAME_QAGAME + %d: '%s'\n", SYS_DLLNAME_QAGAME_SHIFT, FS_ShiftStr( "qagame_mp_x86.dll" /*"qagame.mp.i386.so"*/, SYS_DLLNAME_QAGAME_SHIFT ) );
					Com_Printf( "SYS_DLLNAME_CGAME + %d: '%s'\n", SYS_DLLNAME_CGAME_SHIFT, FS_ShiftStr( "cgame_mp_x86.dll" /*"cgame.mp.i386.so"*/, SYS_DLLNAME_CGAME_SHIFT ) );
					Com_Printf( "SYS_DLLNAME_UI + %d: '%s'\n", SYS_DLLNAME_UI_SHIFT, FS_ShiftStr( "ui_mp_x86.dll" /*"ui.mp.i386.so"*/, SYS_DLLNAME_UI_SHIFT ) );
#endif
					// qagame dll
					if ( !( pak->referenced & FS_QAGAME_REF ) && FS_ShiftedStrStr( filename, SYS_DLLNAME_QAGAME, -SYS_DLLNAME_QAGAME_SHIFT ) ) {
						pak->referenced |= FS_QAGAME_REF;
					}
					// cgame dll
					if ( !( pak->referenced & FS_CGAME_REF ) && FS_ShiftedStrStr( filename, SYS_DLLNAME_CGAME, -SYS_DLLNAME_CGAME_SHIFT ) ) {
						pak->referenced |= FS_CGAME_REF;
					}
					// ui dll
					if ( !( pak->referenced & FS_UI_REF ) && FS_ShiftedStrStr( filename, SYS_DLLNAME_UI, -SYS_DLLNAME_UI_SHIFT ) ) {
						pak->referenced |= FS_UI_REF;
					}

#if !defined RTCW_ET && !defined PRE_RELEASE_DEMO && !defined DO_LIGHT_DEDICATED
					// DHM -- Nerve :: Don't allow maps to be loaded from pak0 (singleplayer)
					if ( Q_stricmp( filename + l - 4, ".bsp" ) == 0 &&
						 Q_stricmp( pak->pakBasename, "pak0" ) == 0 ) {

						*file = 0;
						return -1;
					}
#elif defined RTCW_ET
//#if !defined(PRE_RELEASE_DEMO) && !defined(DO_LIGHT_DEDICATED)
//					// DHM -- Nerve :: Don't allow maps to be loaded from pak0 (singleplayer)
//					if ( Q_stricmp(filename + l - 4, ".bsp") == 0 &&
//						Q_stricmp( pak->pakBasename, "pak0" ) == 0 ) {
//
//						*file = 0;
//						return -1;
//					}
//#endif
#endif // RTCW_XX

#endif // RTCW_XX

					if (uniqueFILE)
					{
						// open a new file on the pakfile
						rtcw::UniquePtr<MinizZip> miniz_zip_uptr(new MinizZip());

						if (!miniz_zip_uptr->open(pak->pakFilename))
						{
							Com_Error(ERR_FATAL, "Couldn't reopen %s", pak->pakFilename);
						}

						fsh[*file].handleFiles.file.miniz_zip_ptr_ = miniz_zip_uptr.release();
					}
					else
					{
						fsh[*file].handleFiles.file.miniz_zip_ptr_ = pak->miniz_zip_ptr_;
					}

					Q_strncpyz(fsh[*file].name, filename, sizeof(fsh[*file].name));
					fsh[*file].zipFile = qtrue;

					MinizZip* miniz_zip_ptr = fsh[*file].handleFiles.file.miniz_zip_ptr_;
					MinizZip::File* miniz_file_ptr = miniz_zip_ptr->open_file(pakFile->miniz_file_index_);

					fsh[*file].handleFiles.file.miniz_file_ptr_ = miniz_file_ptr;
					fsh[*file].zipFilePos = pakFile->miniz_file_index_;

					// set the file position in the zip file (also sets the current file info)

					if (fs_debug->integer)
					{
						Com_Printf("FS_FOpenFileRead: %s (found in '%s')\n",
							filename, pak->pakFilename);
					}

					const MinizZip::FileStat file_info = miniz_zip_ptr->get_file_stat(pakFile->miniz_file_index_);

					return file_info.uncompressed_size_;
				}
				pakFile = pakFile->next;
			} while ( pakFile != NULL );
		} else if ( search->dir ) {

#if !defined RTCW_SP
			if ( fs_filter_flag & FS_EXCLUDE_DIR ) {
				continue;
			}
#endif // RTCW_XX

			// check a file in the directory tree

#if defined RTCW_SP
			// if we are running restricted, the only files we
			// will allow to come from the directory are .cfg files
#else
			// if we are running restricted, or if the filesystem is configured for pure (fs_numServerPaks)
			// the only files we will allow to come from the directory are .cfg files
#endif // RTCW_XX

			l = strlen( filename );

#if defined RTCW_SP
			// FIXME TTimo I'm not sure about the fs_numServerPaks test
			// if you are using FS_ReadFile to find out if a file exists,
			//   this test can make the search fail although the file is in the directory
			// I had the problem on show_bug.cgi?id=8
			// turned out I used FS_FileExists instead
#endif // RTCW_XX

			if ( fs_restrict->integer || fs_numServerPaks ) {

				if ( Q_stricmp( filename + l - 4, ".cfg" )       // for config files

#if defined RTCW_SP
//					&& Q_stricmp( filename + l - 5, ".menu" )	// menu files
					 && Q_stricmp( filename + l - 4, ".svg" ) // savegames
#else
					 && Q_stricmp( filename + l - 5, ".menu" )  // menu files
#endif // RTCW_XX

					 && Q_stricmp( filename + l - 5, ".game" )  // menu files
					 && Q_stricmp( filename + l - strlen( demoExt ), demoExt ) // menu files

#if !defined RTCW_ET
					 && Q_stricmp( filename + l - 4, ".dat" ) ) { // for journal files
#else
					 && Q_stricmp( filename + l - 4, ".dat" ) // for journal files
					 && Q_stricmp( filename + l - 8, "bots.txt" )
					 && Q_stricmp( filename + l - 8, ".botents" )
					 ) {
#endif // RTCW_XX

					continue;
				}
			}

			dir = search->dir;

			netpath = FS_BuildOSPath( dir->path, dir->gamedir, filename );
			fsh[*file].handleFiles.file.o = fopen( netpath, "rb" );
			if ( !fsh[*file].handleFiles.file.o ) {
				continue;
			}

			if ( Q_stricmp( filename + l - 4, ".cfg" )       // for config files
				 && Q_stricmp( filename + l - 5, ".menu" )  // menu files
				 && Q_stricmp( filename + l - 5, ".game" )  // menu files
				 && Q_stricmp( filename + l - strlen( demoExt ), demoExt ) // menu files

#if !defined RTCW_ET
				 && Q_stricmp( filename + l - 4, ".dat" ) ) { // for journal files
#else
				 && Q_stricmp( filename + l - 4, ".dat" )
				 && Q_stricmp( filename + l - 8, ".botents" )
				 /*&& !strstr( filename, "botfiles" )*/ ) { // RF, need this for dev
#endif // RTCW_XX

				fs_fakeChkSum = random();
			}

			Q_strncpyz( fsh[*file].name, filename, sizeof( fsh[*file].name ) );
			fsh[*file].zipFile = qfalse;
			if ( fs_debug->integer ) {
				Com_Printf( "FS_FOpenFileRead: %s (found in '%s/%s')\n", filename,
							dir->path, dir->gamedir );
			}

			// if we are getting it from the cdpath, optionally copy it
			//  to the basepath
			if ( fs_copyfiles->integer && !Q_stricmp( dir->path, fs_cdpath->string ) ) {
				char    *copypath;

				copypath = FS_BuildOSPath( fs_basepath->string, dir->gamedir, filename );
				FS_CopyFile( netpath, copypath );

#if defined RTCW_ET
			} else if ( fs_copyfiles->integer && fs_buildpath->string[0] && Q_stricmp( dir->path, fs_buildpath->string ) ) {
				char    *copypath;

				copypath = FS_BuildOSPath( fs_buildpath->string, fs_buildgame->string, filename );
				FS_CopyFile( netpath, copypath );
#endif // RTCW_XX

			}

			return FS_filelength( *file );
		}
	}

	Com_DPrintf( "Can't find %s\n", filename );
#ifdef FS_MISSING
	if ( missingFiles ) {
		fprintf( missingFiles, "%s\n", filename );
	}
#endif
	*file = 0;
	return -1;
}

#if !defined RTCW_SP
int FS_FOpenFileRead_Filtered( const char *qpath, fileHandle_t *file, qboolean uniqueFILE, int filter_flag ) {
	int ret;

	fs_filter_flag = filter_flag;
	ret = FS_FOpenFileRead( qpath, file, uniqueFILE );
	fs_filter_flag = 0;

	return ret;
}

// TTimo
// relevant to client only
#if !defined( DEDICATED )
/*
==================
FS_CL_ExtractFromPakFile

NERVE - SMF - Extracts the latest file from a pak file.

Compares packed file against extracted file. If no differences, does not copy.
This is necessary for exe/dlls which may or may not be locked.

NOTE TTimo:
  fullpath gives the full OS path to the dll that will potentially be loaded
	on win32 it's always in fs_basepath/<fs_game>/
	on linux it can be in fs_homepath/<fs_game>/ or fs_basepath/<fs_game>/
  the dll is extracted to fs_homepath (== fs_basepath on win32) if needed

  the return value doesn't tell wether file was extracted or not, it just says wether it's ok to continue
  (i.e. either the right file was extracted successfully, or it was already present)

  cvar_lastVersion is the optional name of a CVAR_ARCHIVE used to store the wolf version for the last extracted .so
  show_bug.cgi?id=463

==================
*/
qboolean FS_CL_ExtractFromPakFile( const char *fullpath, const char *gamedir, const char *filename, const char *cvar_lastVersion ) {
	int srcLength;
	int destLength;
	unsigned char   *srcData;
	unsigned char   *destData;
	qboolean needToCopy;
	FILE            *destHandle;

	needToCopy = qtrue;

	// read in compressed file
	srcLength = FS_ReadFile( filename, (void **)&srcData );

	// if its not in the pak, we bail
	if ( srcLength == -1 ) {
		return qfalse;
	}

	// read in local file
	destHandle = fopen( fullpath, "rb" );

	// if we have a local file, we need to compare the two
	if ( destHandle ) {
		fseek( destHandle, 0, SEEK_END );
		destLength = ftell( destHandle );
		fseek( destHandle, 0, SEEK_SET );

		if ( destLength > 0 ) {
			destData = (unsigned char*)Z_Malloc( destLength );

#if !defined RTCW_ET
			fread( destData, 1, destLength, destHandle );
#else
//			fread( destData, 1, destLength, destHandle );
			fread( destData, destLength, 1, destHandle );
#endif // RTCW_XX


			// compare files
			if ( destLength == srcLength ) {
				int i;

				for ( i = 0; i < destLength; i++ ) {
					if ( destData[i] != srcData[i] ) {
						break;
					}
				}

				if ( i == destLength ) {
					needToCopy = qfalse;
				}
			}

			Z_Free( destData ); // TTimo
		}

		fclose( destHandle );
	}

	// write file
	if ( needToCopy ) {
		fileHandle_t f;

		// Com_DPrintf("FS_ExtractFromPakFile: FS_FOpenFileWrite '%s'\n", filename);
		f = FS_FOpenFileWrite( filename );
		if ( !f ) {
			Com_Printf( "Failed to open %s\n", filename );
			return qfalse;
		}

		FS_Write( srcData, srcLength, f );

		FS_FCloseFile( f );
	}

	FS_FreeFile( srcData );
	return qtrue;
}
#endif
#endif // RTCW_XX

#if defined RTCW_ET
/*
==============
FS_AllowDeletion
==============
*/
qboolean FS_AllowDeletion( const char *filename ) {
	// for safety, only allow deletion from the save, profiles and demo directory
	if ( Q_strncmp( filename, "save/", 5 ) != 0 &&
		 Q_strncmp( filename, "profiles/", 9 ) != 0 &&
		 Q_strncmp( filename, "demos/", 6 ) != 0 ) {
		return qfalse;
	}

	return qtrue;
}


/*
==============
FS_DeleteDir
==============
*/
int FS_DeleteDir( const char *dirname, qboolean nonEmpty, qboolean recursive ) {
	char *ospath;
	char **pFiles = NULL;
	int i, nFiles = 0;

	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization\n" );
	}

	if ( !dirname || dirname[0] == 0 ) {
		return 0;
	}

	if ( !FS_AllowDeletion( dirname ) ) {
		return 0;
	}

	if ( recursive ) {
		ospath = FS_BuildOSPath( fs_homepath->string, fs_gamedir, dirname );
		pFiles = Sys_ListFiles( ospath, "/", NULL, &nFiles, qfalse );
		for ( i = 0; i < nFiles; i++ ) {
			char temp[MAX_OSPATH];

			if ( !Q_stricmp( pFiles[i], ".." ) || !Q_stricmp( pFiles[i], "." ) ) {
				continue;
			}

			Com_sprintf( temp, sizeof( temp ), "%s/%s", dirname, pFiles[i] );

			if ( !FS_DeleteDir( temp, nonEmpty, recursive ) ) {
				return 0;
			}
		}
		Sys_FreeFileList( pFiles );
	}

	if ( nonEmpty ) {
		ospath = FS_BuildOSPath( fs_homepath->string, fs_gamedir, dirname );
		pFiles = Sys_ListFiles( ospath, NULL, NULL, &nFiles, qfalse );
		for ( i = 0; i < nFiles; i++ ) {
			ospath = FS_BuildOSPath( fs_homepath->string, fs_gamedir, va( "%s/%s", dirname, pFiles[i] ) );

#if FIXME
			if ( remove( ospath ) == -1 ) {  // failure
#else
			if (!FS_Remove(ospath))
			{
#endif // FIXME
				return 0;
			}
		}
		Sys_FreeFileList( pFiles );
	}

	ospath = FS_BuildOSPath( fs_homepath->string, fs_gamedir, dirname );

#if FIXME
	if ( Q_rmdir( ospath ) == 0 ) {
#else
	if (FS_Remove(ospath))
	{
#endif // FIXME
		return 1;
	}

	return 0;
}

/*
==============
FS_OSStatFile
Test an file given OS path:
returns -1 if not found
returns 1 if directory
returns 0 otherwise
==============
*/
extern int FS_OSStatFile(const char* path);

#endif // RTCW_XX


/*
==============
FS_Delete
TTimo - this was not in the 1.30 filesystem code
using fs_homepath for the file to remove
==============
*/
int FS_Delete( const char *filename ) {
	char *ospath;

#if defined RTCW_ET
	int stat;
#endif // RTCW_XX

	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization\n" );
	}

	if ( !filename || filename[0] == 0 ) {
		return 0;
	}

#if !defined RTCW_ET
	// for safety, only allow deletion from the save directory
	if ( Q_strncmp( filename, "save/", 5 ) != 0 ) {
#else
	if ( !FS_AllowDeletion( filename ) ) {
#endif // RTCW_XX

		return 0;
	}

	ospath = FS_BuildOSPath( fs_homepath->string, fs_gamedir, filename );

#if !defined RTCW_ET
#if FIXME
	if ( remove( ospath ) != -1 ) {  // success
#else
	if (FS_Remove(ospath))
	{
#endif // FIXME
		return 1;
	}
#else
	stat = FS_OSStatFile( ospath );
	if ( stat == -1 ) {
		return 0;
	}

	if ( stat == 1 ) {
		return( FS_DeleteDir( filename, qtrue, qtrue ) );
	} else {
#if FIXME
		if ( remove( ospath ) != -1 ) {  // success
#else
		if (FS_Remove(ospath))
		{
#endif // FIXME
			return 1;
		}
	}
#endif // RTCW_XX

	return 0;
}


/*
=================
FS_Read

Properly handles partial reads
=================
*/
int FS_Read2( void *buffer, int len, fileHandle_t f ) {
	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization\n" );
	}

	if ( !f ) {
		return 0;
	}
	if ( fsh[f].streamed ) {
		int r;
		fsh[f].streamed = qfalse;

		r = FS_Read(buffer, len, f);

		fsh[f].streamed = qtrue;
		return r;
	} else {
		return FS_Read( buffer, len, f );
	}
}

int FS_Read( void *buffer, int len, fileHandle_t f ) {
	int block, remaining;
	int read;
	byte    *buf;
	int tries;

	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization\n" );
	}

	if ( !f ) {
		return 0;
	}

	buf = (byte *)buffer;
	fs_readCount += len;

	if ( fsh[f].zipFile == qfalse ) {
		remaining = len;
		tries = 0;
		while ( remaining ) {
			block = remaining;

#if defined RTCW_ET
//			read = fread (buf, block, 1, fsh[f].handleFiles.file.o);
#endif // RTCW_XX

			read = fread( buf, 1, block, fsh[f].handleFiles.file.o );
			if ( read == 0 ) {
				// we might have been trying to read from a CD, which
				// sometimes returns a 0 read on windows
				if ( !tries ) {
					tries = 1;
				} else {
					return len - remaining;   //Com_Error (ERR_FATAL, "FS_Read: 0 bytes read");
				}
			}

			if ( read == -1 ) {
				Com_Error( ERR_FATAL, "FS_Read: -1 bytes read" );
			}

			remaining -= read;
			buf += read;
		}
		return len;
	} else {
		MinizZip::File* miniz_file_ptr = fsh[f].handleFiles.file.miniz_file_ptr_;

		if (!miniz_file_ptr)
		{
			return 0;
		}

		return miniz_file_ptr->read(buffer, len);
	}
}

/*
=================
FS_Write

Properly handles partial writes
=================
*/
int FS_Write( const void *buffer, int len, fileHandle_t h ) {
	int block, remaining;
	int written;
	byte    *buf;
	int tries;
	FILE    *f;

	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization\n" );
	}

	if ( !h ) {
		return 0;
	}

	f = FS_FileForHandle( h );
	buf = (byte *)buffer;

	remaining = len;
	tries = 0;
	while ( remaining ) {
		block = remaining;
		written = fwrite( buf, 1, block, f );
		if ( written == 0 ) {
			if ( !tries ) {
				tries = 1;
			} else {

#if !defined RTCW_ET
				Com_Printf( "FS_Write: 0 bytes written\n" );
#else
				Com_Printf( "FS_Write: 0 bytes written (%d attempted)\n", block );
#endif // RTCW_XX

				return 0;
			}
		}

#if !defined RTCW_ET
		if ( written == -1 ) {
			Com_Printf( "FS_Write: -1 bytes written\n" );
#else
		if ( written < 0 ) {
			Com_Printf( "FS_Write: %d bytes written (%d attempted)\n", written, block );
#endif // RTCW_XX

			return 0;
		}

		remaining -= written;
		buf += written;
	}
	if ( fsh[h].handleSync ) {
		fflush( f );
	}
	return len;
}

#if defined RTCW_SP
#define MAXPRINTMSG 4096
#endif // RTCW_XX

void QDECL FS_Printf( fileHandle_t h, const char *fmt, ... ) {
	va_list argptr;
	char msg[MAXPRINTMSG];

	va_start( argptr,fmt );

#if defined RTCW_SP
	vsprintf( msg,fmt,argptr );
#else
	Q_vsnprintf( msg, sizeof( msg ), fmt, argptr );
#endif // RTCW_XX

	va_end( argptr );

	FS_Write( msg, strlen( msg ), h );
}

/*
=================
FS_Seek

=================
*/
int FS_Seek( fileHandle_t f, int32_t offset, int origin ) {
	int _origin;
	char foo[65536];

	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization\n" );
		return -1;
	}

	if ( fsh[f].streamed ) {
		fsh[f].streamed = qfalse;

		FS_Seek(f, offset, origin);

		fsh[f].streamed = qtrue;
	}

	if ( fsh[f].zipFile == qtrue ) {
		if (offset > 65536 || origin != FS_SEEK_SET)
		{
			Com_Error(ERR_FATAL, "Full Zip file seeking not supported.\n" );
			return -1;
		}

		qfile_gut& file = fsh[f].handleFiles.file;

		const int current_position = file.miniz_file_ptr_->get_position();

		// Best case.
		//
		if (current_position == offset)
		{
			return 0;
		}

		// Average case.
		//
		if (current_position < offset)
		{
			const int skip_count = offset - current_position;
			const int skip_result = file.miniz_file_ptr_->read(foo, skip_count);

			if (skip_result != skip_count)
			{
				Com_Error(ERR_FATAL, "Zip file seek error.\n" );
				return -1;
			}

			return 0;
		}

		// Worst case: reopen zip file stream and possibly skip some bytes.
		//
		delete file.miniz_file_ptr_;
		file.miniz_file_ptr_ = file.miniz_zip_ptr_->open_file(fsh[f].zipFilePos);

		if (!file.miniz_file_ptr_)
		{
			Com_Error(ERR_FATAL, "Failed to reopen Zip file stream.\n" );
			return -1;
		}

		if (offset == 0)
		{
			return 0;
		}

		const int skip_result = file.miniz_file_ptr_->read(foo, offset);

		if (skip_result != offset)
		{
			Com_Error(ERR_FATAL, "Zip file seek error.\n" );
			return -1;
		}

		return 0;
	} else {
		FILE *file;
		file = FS_FileForHandle( f );
		switch ( origin ) {
		case FS_SEEK_CUR:
			_origin = SEEK_CUR;
			break;
		case FS_SEEK_END:
			_origin = SEEK_END;
			break;
		case FS_SEEK_SET:
			_origin = SEEK_SET;
			break;
		default:
			_origin = SEEK_CUR;
			Com_Error( ERR_FATAL, "Bad origin in FS_Seek\n" );
			break;
		}

		return fseek( file, offset, _origin );
	}
}


/*
======================================================================================

CONVENIENCE FUNCTIONS FOR ENTIRE FILES

======================================================================================
*/

int FS_FileIsInPAK( const char *filename, int *pChecksum ) {
	searchpath_t    *search;
	pack_t          *pak;
	fileInPack_t    *pakFile;
	int32_t hash = 0;

	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization\n" );
	}

	if ( !filename ) {
		Com_Error( ERR_FATAL, "FS_FOpenFileRead: NULL 'filename' parameter passed\n" );
	}

	// qpaths are not supposed to have a leading slash
	if ( filename[0] == '/' || filename[0] == '\\' ) {
		filename++;
	}

	// make absolutely sure that it can't back up the path.
	// The searchpaths do guarantee that something will always
	// be prepended, so we don't need to worry about "c:" or "//limbo"
	if ( strstr( filename, ".." ) || strstr( filename, "::" ) ) {
		return -1;
	}

	//
	// search through the path, one element at a time
	//

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		//
		if ( search->pack ) {
			hash = FS_HashFileName( filename, search->pack->hashSize );
		}
		// is the element a pak file?
		if ( search->pack && search->pack->hashTable[hash] ) {
			// disregard if it doesn't match one of the allowed pure pak files
			if ( !FS_PakIsPure( search->pack ) ) {
				continue;
			}

			// look through all the pak file elements
			pak = search->pack;
			pakFile = pak->hashTable[hash];
			do {
				// case and separator insensitive comparisons
				if ( !FS_FilenameCompare( pakFile->name, filename ) ) {
					if ( pChecksum ) {
						*pChecksum = pak->pure_checksum;
					}

#if defined RTCW_ET
					// Mac hack
					if ( pak->checksum == MP_LEGACY_PAK ) {
						legacy_mp_bin = qtrue;
					} else {
						legacy_mp_bin = qfalse;
					}
#endif // RTCW_XX

					return 1;
				}
				pakFile = pakFile->next;
			} while ( pakFile != NULL );
		}
	}
	return -1;
}

/*
============
FS_ReadFile

Filename are relative to the quake search path
a null buffer will just return the file length without loading
============
*/
int FS_ReadFile( const char *qpath, void **buffer ) {
	fileHandle_t h;
	byte*           buf;
	qboolean isConfig;
	int len;

	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization\n" );
	}

	if ( !qpath || !qpath[0] ) {
		Com_Error( ERR_FATAL, "FS_ReadFile with empty name\n" );
	}

	buf = NULL; // quiet compiler warning

	// if this is a .cfg file and we are playing back a journal, read
	// it from the journal file
	if ( strstr( qpath, ".cfg" ) ) {
		isConfig = qtrue;
		if ( com_journal && com_journal->integer == 2 ) {
			int r;

			Com_DPrintf( "Loading %s from journal file.\n", qpath );
			r = FS_Read( &len, sizeof( len ), com_journalDataFile );
			if ( r != sizeof( len ) ) {
				if ( buffer != NULL ) {
					*buffer = NULL;
				}
				return -1;
			}
			// if the file didn't exist when the journal was created
			if ( !len ) {
				if ( buffer == NULL ) {
					return 1;           // hack for old journal files
				}
				*buffer = NULL;
				return -1;
			}
			if ( buffer == NULL ) {
				return len;
			}

			buf = static_cast<byte*> (Hunk_AllocateTempMemory( len + 1 ));
			*buffer = buf;

			r = FS_Read( buf, len, com_journalDataFile );
			if ( r != len ) {
				Com_Error( ERR_FATAL, "Read from journalDataFile failed" );
			}

			fs_loadCount++;
			fs_loadStack++;

			// guarantee that it will have a trailing 0 for string operations
			buf[len] = 0;

			return len;
		}
	} else {
		isConfig = qfalse;
	}

	// look for it in the filesystem or pack files
	len = FS_FOpenFileRead( qpath, &h, qfalse );
	if ( h == 0 ) {
		if ( buffer ) {
			*buffer = NULL;
		}
		// if we are journalling and it is a config file, write a zero to the journal file
		if ( isConfig && com_journal && com_journal->integer == 1 ) {
			Com_DPrintf( "Writing zero for %s to journal file.\n", qpath );
			len = 0;
			FS_Write( &len, sizeof( len ), com_journalDataFile );
			FS_Flush( com_journalDataFile );
		}
		return -1;
	}

	if ( !buffer ) {
		if ( isConfig && com_journal && com_journal->integer == 1 ) {
			Com_DPrintf( "Writing len for %s to journal file.\n", qpath );
			FS_Write( &len, sizeof( len ), com_journalDataFile );
			FS_Flush( com_journalDataFile );
		}
		FS_FCloseFile( h );
		return len;
	}

	fs_loadCount++;
	fs_loadStack++;

	buf = static_cast<byte*> (Hunk_AllocateTempMemory( len + 1 ));
	*buffer = buf;

	FS_Read( buf, len, h );

	// guarantee that it will have a trailing 0 for string operations
	buf[len] = 0;
	FS_FCloseFile( h );

	// if we are journalling and it is a config file, write it to the journal file
	if ( isConfig && com_journal && com_journal->integer == 1 ) {
		Com_DPrintf( "Writing %s to journal file.\n", qpath );
		FS_Write( &len, sizeof( len ), com_journalDataFile );
		FS_Write( buf, len, com_journalDataFile );
		FS_Flush( com_journalDataFile );
	}
	return len;
}

/*
=============
FS_FreeFile
=============
*/
void FS_FreeFile( void *buffer ) {
	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization\n" );
	}
	if ( !buffer ) {
		Com_Error( ERR_FATAL, "FS_FreeFile( NULL )" );
	}
	fs_loadStack--;

	Hunk_FreeTempMemory( buffer );

	// if all of our temp files are free, clear all of our space
	if ( fs_loadStack == 0 ) {
		Hunk_ClearTempMemory();
	}
}

/*
============
FS_WriteFile

Filename are reletive to the quake search path
============
*/
void FS_WriteFile( const char *qpath, const void *buffer, int size ) {
	fileHandle_t f;

	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization\n" );
	}

	if ( !qpath || !buffer ) {
		Com_Error( ERR_FATAL, "FS_WriteFile: NULL parameter" );
	}

	f = FS_FOpenFileWrite( qpath );
	if ( !f ) {
		Com_Printf( "Failed to open %s\n", qpath );
		return;
	}

	FS_Write( buffer, size, f );

	FS_FCloseFile( f );
}



/*
==========================================================================

ZIP FILE LOADING

==========================================================================
*/

/*
=================
FS_LoadZipFile

Creates a new pak_t in the search chain for the contents
of a zip file.
=================
*/
static pack_t* FS_LoadZipFile(
	char* zipfile,
	const char* basename)
{
	fileInPack_t    *buildBuffer;
	pack_t          *pack;
	rtcw::String filename_inzip;
	int i, len;
	int32_t hash;
	int fs_numHeaderLongs;
	int             *fs_headerLongs;
	char            *namePtr;

	fs_numHeaderLongs = 0;

	rtcw::UniquePtr<MinizZip> miniz_zip_uptr(new MinizZip());

	if (!miniz_zip_uptr->open(zipfile))
	{
		return NULL;
	}

	const int file_count = miniz_zip_uptr->get_file_count();

	fs_packFiles += file_count;
	len = miniz_zip_uptr->calculate_file_names_size();

	buildBuffer = static_cast<fileInPack_t*> (Z_Malloc((file_count * sizeof(fileInPack_t)) + len));
	namePtr = ((char *)buildBuffer) + file_count * sizeof(fileInPack_t);
	fs_headerLongs = static_cast<int*> (Z_Malloc(file_count * sizeof(int)));

	// get the hash table size from the number of files in the zip
	// because lots of custom pk3 files have less than 32 or 64 files
	for (i = 1; i <= MAX_FILEHASH_SIZE; i <<= 1)
	{
		if (i > file_count)
		{
			break;
		}
	}

	pack = static_cast<pack_t*> (Z_Malloc(sizeof(pack_t) + i * sizeof(fileInPack_t *)));
	pack->hashSize = i;
	pack->hashTable = (fileInPack_t **)(((char *)pack) + sizeof(pack_t));
	for (i = 0; i < pack->hashSize; i++)
	{
		pack->hashTable[i] = NULL;
	}

	Q_strncpyz(pack->pakFilename, zipfile, sizeof(pack->pakFilename));
	Q_strncpyz(pack->pakBasename, basename, sizeof(pack->pakBasename));

	// strip .pk3 if needed
	if (strlen(pack->pakBasename) > 4 && !Q_stricmp(pack->pakBasename + strlen(pack->pakBasename) - 4, ".pk3"))
	{
		pack->pakBasename[strlen(pack->pakBasename) - 4] = 0;
	}

	pack->miniz_zip_ptr_ = miniz_zip_uptr.release();
	pack->numfiles = file_count;

	for (i = 0; i < file_count; ++i)
	{
		const MinizZip::FileStat file_info = pack->miniz_zip_ptr_->get_file_stat(i);

		if (file_info.uncompressed_size_ > 0)
		{
			fs_headerLongs[fs_numHeaderLongs++] = static_cast<int>(file_info.crc_);
		}

		filename_inzip = file_info.file_name_;
		Q_strlwr(&filename_inzip[0]);
		hash = FS_HashFileName(filename_inzip.c_str(), pack->hashSize);
		buildBuffer[i].name = namePtr;
		strcpy(buildBuffer[i].name, filename_inzip.c_str());
		namePtr += filename_inzip.length() + 1;
		// store the file position in the zip

		// BBi
		//unzGetCurrentFileInfoPosition( uf, &buildBuffer[i].pos );
		buildBuffer[i].miniz_file_index_ = i;
		// BBi

		//
		buildBuffer[i].next = pack->hashTable[hash];
		pack->hashTable[hash] = &buildBuffer[i];
	}

	pack->checksum = Com_BlockChecksum(fs_headerLongs, 4 * fs_numHeaderLongs);
	pack->pure_checksum = Com_BlockChecksumKey(fs_headerLongs, 4 * fs_numHeaderLongs, rtcw::Endian::le(fs_checksumFeed));

#if defined RTCW_MP
	// TTimo: DO_LIGHT_DEDICATED
	// curious about the size of those
	//Com_DPrintf("Com_BlockChecksumKey: %s %u\n", pack->pakBasename, 4 * fs_numHeaderLongs);
	// cumulated for light dedicated: 21558 bytes
#endif // RTCW_XX

	rtcw::Endian::lei(pack->checksum);
	rtcw::Endian::lei(pack->pure_checksum);

	Z_Free(fs_headerLongs);

	pack->buildBuffer = buildBuffer;
	return pack;
}

/*
=================================================================================

DIRECTORY SCANNING FUNCTIONS

=================================================================================
*/

#define MAX_FOUND_FILES 0x1000

static int FS_ReturnPath( const char *zname, char *zpath, int *depth ) {
	int len, at, newdep;

	newdep = 0;
	zpath[0] = 0;
	len = 0;
	at = 0;

	while ( zname[at] != 0 )
	{
		if ( zname[at] == '/' || zname[at] == '\\' ) {
			len = at;
			newdep++;
		}
		at++;
	}
	strcpy( zpath, zname );
	zpath[len] = 0;
	*depth = newdep;

	return len;
}

/*
==================
FS_AddFileToList
==================
*/
static int FS_AddFileToList( char *name, char *list[MAX_FOUND_FILES], int nfiles ) {
	int i;

	if ( nfiles == MAX_FOUND_FILES - 1 ) {
		return nfiles;
	}
	for ( i = 0 ; i < nfiles ; i++ ) {
		if ( !Q_stricmp( name, list[i] ) ) {
			return nfiles;      // allready in list
		}
	}
	list[nfiles] = CopyString( name );
	nfiles++;

	return nfiles;
}

/*
===============
FS_ListFilteredFiles

Returns a uniqued list of files that match the given criteria
from all search paths
===============
*/
char **FS_ListFilteredFiles( const char *path, const char *extension, const char *filter, int *numfiles ) {
	int nfiles;
	char            **listCopy;
	char            *list[MAX_FOUND_FILES];
	searchpath_t    *search;
	int i;
	int pathLength;
	int extensionLength;
	int length, pathDepth, temp;
	pack_t          *pak;
	fileInPack_t    *buildBuffer;
	char zpath[MAX_ZPATH];

	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization\n" );
	}

	if ( !path ) {
		*numfiles = 0;
		return NULL;
	}
	if ( !extension ) {
		extension = "";
	}

	pathLength = strlen( path );
	if ( path[pathLength - 1] == '\\' || path[pathLength - 1] == '/' ) {
		pathLength--;
	}
	extensionLength = strlen( extension );
	nfiles = 0;
	FS_ReturnPath( path, zpath, &pathDepth );

	//
	// search through the path, one element at a time, adding to list
	//
	for ( search = fs_searchpaths ; search ; search = search->next ) {
		// is the element a pak file?
		if ( search->pack ) {

			//ZOID:  If we are pure, don't search for files on paks that
			// aren't on the pure list
			if ( !FS_PakIsPure( search->pack ) ) {
				continue;
			}

			// look through all the pak file elements
			pak = search->pack;
			buildBuffer = pak->buildBuffer;
			for ( i = 0; i < pak->numfiles; i++ ) {
				char    *name;
				int zpathLen, depth;

				// check for directory match
				name = buildBuffer[i].name;
				//
				if ( filter ) {
					// case insensitive
					if ( !Com_FilterPath( filter, name, qfalse ) ) {
						continue;
					}
					// unique the match
					nfiles = FS_AddFileToList( name, list, nfiles );
				} else {

					zpathLen = FS_ReturnPath( name, zpath, &depth );

					if ( ( depth - pathDepth ) > 2 || pathLength > zpathLen || Q_stricmpn( name, path, pathLength ) ) {
						continue;
					}

					// check for extension match
					length = strlen( name );
					if ( length < extensionLength ) {
						continue;
					}

					if ( Q_stricmp( name + length - extensionLength, extension ) ) {
						continue;
					}
					// unique the match

					temp = pathLength;
					if ( pathLength ) {
						temp++;     // include the '/'
					}
					nfiles = FS_AddFileToList( name + temp, list, nfiles );
				}
			}
		} else if ( search->dir ) { // scan for files in the filesystem
			char    *netpath;
			int numSysFiles;
			char    **sysFiles;
			char    *name;

			// don't scan directories for files if we are pure or restricted

#if defined RTCW_SP
			// allow listing of savegames for the demo menus
			if ( ( fs_restrict->integer || fs_numServerPaks ) && Q_stricmp( extension, "svg" ) ) {
#elif defined RTCW_MP
			if ( fs_restrict->integer || fs_numServerPaks ) {
#else
			if ( fs_numServerPaks ) {
				continue;
			} else if ( fs_restrict->integer &&
						( !com_gameInfo.usesProfiles || ( com_gameInfo.usesProfiles && Q_stricmpn( path, "profiles", 8 ) ) ) &&
						Q_stricmpn( path, "demos", 5 ) ) {
#endif // RTCW_XX

				continue;
			} else {
				netpath = FS_BuildOSPath( search->dir->path, search->dir->gamedir, path );
				sysFiles = Sys_ListFiles( netpath, extension, filter, &numSysFiles, qfalse );
				for ( i = 0 ; i < numSysFiles ; i++ ) {
					// unique the match
					name = sysFiles[i];
					nfiles = FS_AddFileToList( name, list, nfiles );
				}
				Sys_FreeFileList( sysFiles );
			}
		}
	}

	// return a copy of the list
	*numfiles = nfiles;

	if ( !nfiles ) {
		return NULL;
	}

	listCopy = static_cast<char**> (Z_Malloc( ( nfiles + 1 ) * sizeof( *listCopy ) ));
	for ( i = 0 ; i < nfiles ; i++ ) {
		listCopy[i] = list[i];
	}
	listCopy[i] = NULL;

	return listCopy;
}

/*
=================
FS_ListFiles
=================
*/
char **FS_ListFiles( const char *path, const char *extension, int *numfiles ) {
	return FS_ListFilteredFiles( path, extension, NULL, numfiles );
}

/*
=================
FS_FreeFileList
=================
*/
void FS_FreeFileList( char **list ) {
	int i;

	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization\n" );
	}

	if ( !list ) {
		return;
	}

	for ( i = 0 ; list[i] ; i++ ) {
		Z_Free( list[i] );
	}

	Z_Free( list );
}


/*
================
FS_GetFileList
================
*/
int FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize ) {
	int nFiles, i, nTotal, nLen;
	char **pFiles = NULL;

	*listbuf = 0;
	nFiles = 0;
	nTotal = 0;

	if ( Q_stricmp( path, "$modlist" ) == 0 ) {
		return FS_GetModList( listbuf, bufsize );
	}

	pFiles = FS_ListFiles( path, extension, &nFiles );

	for ( i = 0; i < nFiles; i++ ) {
		nLen = strlen( pFiles[i] ) + 1;
		if ( nTotal + nLen + 1 < bufsize ) {
			strcpy( listbuf, pFiles[i] );
			listbuf += nLen;
			nTotal += nLen;
		} else {
			nFiles = i;
			break;
		}
	}

	FS_FreeFileList( pFiles );

	return nFiles;
}

/*
=======================
Sys_ConcatenateFileLists

mkv: Naive implementation. Concatenates three lists into a
	 new list, and frees the old lists from the heap.
bk001129 - from cvs1.17 (mkv)

FIXME TTimo those two should move to common.c next to Sys_ListFiles
=======================
 */
static unsigned int Sys_CountFileList( char **list ) {
	int i = 0;

	if ( list ) {
		while ( *list )
		{
			list++;
			i++;
		}
	}
	return i;
}

static char** Sys_ConcatenateFileLists( char **list0, char **list1, char **list2 ) {
	int totalLength = 0;
	char** cat = NULL, **dst, **src;

	totalLength += Sys_CountFileList( list0 );
	totalLength += Sys_CountFileList( list1 );
	totalLength += Sys_CountFileList( list2 );

	/* Create new list. */
	dst = cat = static_cast<char**> (Z_Malloc( ( totalLength + 1 ) * sizeof( char* ) ));

	/* Copy over lists. */
	if ( list0 ) {
		for ( src = list0; *src; src++, dst++ )
			*dst = *src;
	}
	if ( list1 ) {
		for ( src = list1; *src; src++, dst++ )
			*dst = *src;
	}
	if ( list2 ) {
		for ( src = list2; *src; src++, dst++ )
			*dst = *src;
	}

	// Terminate the list
	*dst = NULL;

	// Free our old lists.
	// NOTE: not freeing their content, it's been merged in dst and still being used
	if ( list0 ) {
		Z_Free( list0 );
	}
	if ( list1 ) {
		Z_Free( list1 );
	}
	if ( list2 ) {
		Z_Free( list2 );
	}

	return cat;
}

/*
================
FS_GetModList

Returns a list of mod directory names
A mod directory is a peer to baseq3 with a pk3 in it
The directories are searched in base path, cd path and home path
================
*/
int FS_GetModList( char *listbuf, int bufsize ) {
	int nMods, i, j, nTotal, nLen, nPaks, nPotential, nDescLen;
	char **pFiles = NULL;
	char **pPaks = NULL;
	char *name, *path;
	char descPath[MAX_OSPATH];
	fileHandle_t descHandle;

	int dummy;
	char **pFiles0 = NULL;
	char **pFiles1 = NULL;
	char **pFiles2 = NULL;
	qboolean bDrop = qfalse;

	*listbuf = 0;
	nMods = nPotential = nTotal = 0;

	pFiles0 = Sys_ListFiles( fs_homepath->string, NULL, NULL, &dummy, qtrue );
	pFiles1 = Sys_ListFiles( fs_basepath->string, NULL, NULL, &dummy, qtrue );

#if defined RTCW_SP
	if ( fs_cdpath->string && strlen( fs_cdpath->string ) ) {
#else
	// DHM - Nerve :: Don't add blank paths (root)
	if ( fs_cdpath->string[0] ) {
#endif // RTCW_XX

		pFiles2 = Sys_ListFiles( fs_cdpath->string, NULL, NULL, &dummy, qtrue );
	}

	// we searched for mods in the three paths
	// it is likely that we have duplicate names now, which we will cleanup below
	pFiles = Sys_ConcatenateFileLists( pFiles0, pFiles1, pFiles2 );
	nPotential = Sys_CountFileList( pFiles );

	for ( i = 0 ; i < nPotential ; i++ ) {
		name = pFiles[i];
		// NOTE: cleaner would involve more changes
		// ignore duplicate mod directories
		if ( i != 0 ) {
			bDrop = qfalse;
			for ( j = 0; j < i; j++ )
			{
				if ( Q_stricmp( pFiles[j],name ) == 0 ) {
					// this one can be dropped
					bDrop = qtrue;
					break;
				}
			}
		}
		if ( bDrop ) {
			continue;
		}

#if defined RTCW_SP
		// we drop the basegame, "." and ".."
		if ( Q_stricmp( name, BASEGAME ) && Q_stricmpn( name, ".", 1 ) ) {
#elif defined RTCW_MP
		// we drop "baseq3" "." and ".."
		if ( Q_stricmp( name, "main" ) && Q_stricmpn( name, ".", 1 ) ) {
#else
		// we drop "baseq3" "." and ".."
		if ( Q_stricmp( name, BASEGAME ) && Q_stricmpn( name, ".", 1 ) ) {
#endif // RTCW_XX

			// now we need to find some .pk3 files to validate the mod
			// NOTE TTimo: (actually I'm not sure why .. what if it's a mod under developement with no .pk3?)
			// we didn't keep the information when we merged the directory names, as to what OS Path it was found under
			//   so it could be in base path, cd path or home path
			//   we will try each three of them here (yes, it's a bit messy)

#if defined RTCW_ET
			// NOTE Arnout: what about dropping the current loaded mod as well?
#endif // RTCW_XX

			path = FS_BuildOSPath( fs_basepath->string, name, "" );
			nPaks = 0;
			pPaks = Sys_ListFiles( path, ".pk3", NULL, &nPaks, qfalse );
			Sys_FreeFileList( pPaks ); // we only use Sys_ListFiles to check wether .pk3 files are present

			/* Try on cd path */
			if ( nPaks <= 0 ) {
				path = FS_BuildOSPath( fs_cdpath->string, name, "" );
				nPaks = 0;
				pPaks = Sys_ListFiles( path, ".pk3", NULL, &nPaks, qfalse );
				Sys_FreeFileList( pPaks );
			}

			/* try on home path */
			if ( nPaks <= 0 ) {
				path = FS_BuildOSPath( fs_homepath->string, name, "" );
				nPaks = 0;
				pPaks = Sys_ListFiles( path, ".pk3", NULL, &nPaks, qfalse );
				Sys_FreeFileList( pPaks );
			}

			if ( nPaks > 0 ) {
				nLen = strlen( name ) + 1;
				// nLen is the length of the mod path
				// we need to see if there is a description available
				descPath[0] = '\0';
				strcpy( descPath, name );
				strcat( descPath, "/description.txt" );
				nDescLen = FS_SV_FOpenFileRead( descPath, &descHandle );
				if ( nDescLen > 0 && descHandle ) {
					FILE *file;
					file = FS_FileForHandle( descHandle );
					Com_Memset( descPath, 0, sizeof( descPath ) );
					nDescLen = fread( descPath, 1, 48, file );
					if ( nDescLen >= 0 ) {
						descPath[nDescLen] = '\0';
					}
					FS_FCloseFile( descHandle );
				} else {
					strcpy( descPath, name );
				}
				nDescLen = strlen( descPath ) + 1;

				if ( nTotal + nLen + 1 + nDescLen + 1 < bufsize ) {
					strcpy( listbuf, name );
					listbuf += nLen;
					strcpy( listbuf, descPath );
					listbuf += nDescLen;
					nTotal += nLen + nDescLen;
					nMods++;
				} else {
					break;
				}
			}
		}
	}
	Sys_FreeFileList( pFiles );

	return nMods;
}




//============================================================================

/*
================
FS_Dir_f
================
*/
void FS_Dir_f( void ) {
	const char    *path;
	const char    *extension;
	char    **dirnames;
	int ndirs;
	int i;

	if ( Cmd_Argc() < 2 || Cmd_Argc() > 3 ) {
		Com_Printf( "usage: dir <directory> [extension]\n" );
		return;
	}

	if ( Cmd_Argc() == 2 ) {
		path = Cmd_Argv( 1 );
		extension = "";
	} else {
		path = Cmd_Argv( 1 );
		extension = Cmd_Argv( 2 );
	}

	Com_Printf( "Directory of %s %s\n", path, extension );
	Com_Printf( "---------------\n" );

	dirnames = FS_ListFiles( path, extension, &ndirs );

	for ( i = 0; i < ndirs; i++ ) {
		Com_Printf( "%s\n", dirnames[i] );
	}
	FS_FreeFileList( dirnames );
}

/*
===========
FS_ConvertPath
===========
*/
void FS_ConvertPath( char *s ) {
	while ( *s ) {
		if ( *s == '\\' || *s == ':' ) {
			*s = '/';
		}
		s++;
	}
}

/*
===========
FS_PathCmp

Ignore case and seprator char distinctions
===========
*/
int FS_PathCmp( const char *s1, const char *s2 ) {
	int c1, c2;

	do {
		c1 = *s1++;
		c2 = *s2++;

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

		if ( c1 == '\\' || c1 == ':' ) {
			c1 = '/';
		}
		if ( c2 == '\\' || c2 == ':' ) {
			c2 = '/';
		}

		if ( c1 < c2 ) {
			return -1;      // strings not equal
		}
		if ( c1 > c2 ) {
			return 1;
		}
	} while ( c1 );

	return 0;       // strings are equal
}

/*
================
FS_SortFileList
================
*/
void FS_SortFileList( char **filelist, int numfiles ) {
	int i, j, k, numsortedfiles;
	char **sortedlist;

	sortedlist = static_cast<char**> (Z_Malloc( ( numfiles + 1 ) * sizeof( *sortedlist ) ));
	sortedlist[0] = NULL;
	numsortedfiles = 0;
	for ( i = 0; i < numfiles; i++ ) {
		for ( j = 0; j < numsortedfiles; j++ ) {
			if ( FS_PathCmp( filelist[i], sortedlist[j] ) < 0 ) {
				break;
			}
		}
		for ( k = numsortedfiles; k > j; k-- ) {
			sortedlist[k] = sortedlist[k - 1];
		}
		sortedlist[j] = filelist[i];
		numsortedfiles++;
	}
	Com_Memcpy( filelist, sortedlist, numfiles * sizeof( *filelist ) );
	Z_Free( sortedlist );
}

/*
================
FS_NewDir_f
================
*/
void FS_NewDir_f( void ) {
	const char    *filter;
	char    **dirnames;
	int ndirs;
	int i;

	if ( Cmd_Argc() < 2 ) {
		Com_Printf( "usage: fdir <filter>\n" );
		Com_Printf( "example: fdir *q3dm*.bsp\n" );
		return;
	}

	filter = Cmd_Argv( 1 );

	Com_Printf( "---------------\n" );

	dirnames = FS_ListFilteredFiles( "", "", filter, &ndirs );

	FS_SortFileList( dirnames, ndirs );

	for ( i = 0; i < ndirs; i++ ) {
		FS_ConvertPath( dirnames[i] );
		Com_Printf( "%s\n", dirnames[i] );
	}
	Com_Printf( "%d files listed\n", ndirs );
	FS_FreeFileList( dirnames );
}

/*
============
FS_Path_f

============
*/
void FS_Path_f( void ) {
	searchpath_t    *s;
	int i;

	Com_Printf( "Current search path:\n" );
	for ( s = fs_searchpaths; s; s = s->next ) {
		if ( s->pack ) {

#if defined RTCW_ET
			//			Com_Printf( "%s %X (%i files)\n", s->pack->pakFilename, s->pack->checksum, s->pack->numfiles );
#endif // RTCW_XX

			Com_Printf( "%s (%i files)\n", s->pack->pakFilename, s->pack->numfiles );
			if ( fs_numServerPaks ) {
				if ( !FS_PakIsPure( s->pack ) ) {
					Com_Printf( "    not on the pure list\n" );
				} else {
					Com_Printf( "    on the pure list\n" );
				}
			}
		} else {
			Com_Printf( "%s/%s\n", s->dir->path, s->dir->gamedir );
		}
	}

	Com_Printf( "\n" );
	for ( i = 1 ; i < MAX_FILE_HANDLES ; i++ ) {
		if ( fsh[i].handleFiles.file.o ) {
			Com_Printf( "handle %i: %s\n", i, fsh[i].name );
		}
	}
}

/*
============
FS_TouchFile_f

The only purpose of this function is to allow game script files to copy
arbitrary files furing an "fs_copyfiles 1" run.
============
*/
void FS_TouchFile_f( void ) {
	fileHandle_t f;

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "Usage: touchFile <file>\n" );
		return;
	}

	FS_FOpenFileRead( Cmd_Argv( 1 ), &f, qfalse );
	if ( f ) {
		FS_FCloseFile( f );
	}
}

//===========================================================================


static int paksort( const void *a, const void *b ) {
	char    *aa, *bb;

	aa = *(char **)a;
	bb = *(char **)b;

	return FS_PathCmp( aa, bb );
}

/*
================
FS_AddGameDirectory

Sets fs_gamedir, adds the directory to the head of the path,
then loads the zip headers
================
*/
#define MAX_PAKFILES    1024
static void FS_AddGameDirectory( const char *path, const char *dir ) {
	searchpath_t    *sp;
	int i;
	searchpath_t    *search;
	pack_t          *pak;
	char            *pakfile;
	int numfiles;
	char            **pakfiles;
	char            *sorted[MAX_PAKFILES];

#if defined RTCW_MP
// JPW NERVE
	char mpsppakfilestring[4];

	sprintf( mpsppakfilestring,"msp" );
// jpw
#elif defined RTCW_ET
// JPW NERVE
	/*char			mpsppakfilestring[4];

	sprintf( mpsppakfilestring, "msp" );*/
// jpw
#endif // RTCW_XX

	// this fixes the case where fs_basepath is the same as fs_cdpath
	// which happens on full installs
	for ( sp = fs_searchpaths ; sp ; sp = sp->next ) {
		if ( sp->dir && !Q_stricmp( sp->dir->path, path ) && !Q_stricmp( sp->dir->gamedir, dir ) ) {
			return;         // we've already got this one
		}
	}

	Q_strncpyz( fs_gamedir, dir, sizeof( fs_gamedir ) );

	//
	// add the directory to the search path
	//
	search = static_cast<searchpath_t*> (Z_Malloc( sizeof( searchpath_t ) ));
	search->dir = static_cast<directory_t*> (Z_Malloc( sizeof( *search->dir ) ));

	Q_strncpyz( search->dir->path, path, sizeof( search->dir->path ) );
	Q_strncpyz( search->dir->gamedir, dir, sizeof( search->dir->gamedir ) );
	search->next = fs_searchpaths;
	fs_searchpaths = search;

	// find all pak files in this directory
	pakfile = FS_BuildOSPath( path, dir, "" );
	pakfile[ strlen( pakfile ) - 1 ] = 0; // strip the trailing slash

	pakfiles = Sys_ListFiles( pakfile, ".pk3", NULL, &numfiles, qfalse );

	// sort them so that later alphabetic matches override
	// earlier ones.  This makes pak1.pk3 override pak0.pk3
	if ( numfiles > MAX_PAKFILES ) {
		numfiles = MAX_PAKFILES;
	}
	for ( i = 0 ; i < numfiles ; i++ ) {
		sorted[i] = pakfiles[i];

#if defined RTCW_SP
// JPW NERVE sp_* to _p_* so "sp_pak*" gets alphabetically sorted before "pak*"
//----(SA)	SP mod

		// (SA) sort order to be further clarified later (10/8/01)
		if ( !Q_strncmp( sorted[i],"sp_",3 ) ) { //	sort sp first
#elif defined RTCW_MP
// JPW NERVE KLUDGE: sorry, temp mod mp_* to _p_* so "mp_pak*" gets alphabetically sorted before "pak*"

		if ( !Q_strncmp( sorted[i],"mp_",3 ) ) {
#else
/*		if (!Q_strncmp(sorted[i],"mp_",3))
			memcpy(sorted[i],"zz",2);	*/
#endif // RTCW_XX

#if !defined RTCW_ET
			memcpy( sorted[i],"zz",2 );
		}
#endif // RTCW_XX

#if !defined RTCW_SP
// jpw
#endif // RTCW_XX

	}

	qsort( sorted, numfiles, sizeof (size_t), paksort );

	for ( i = 0 ; i < numfiles ; i++ ) {

#if !defined RTCW_ET
#if defined RTCW_SP
		if ( Q_strncmp( sorted[i],"mp_",3 ) ) { // (SA) SP mod -- exclude mp_*

// JPW NERVE KLUDGE: fix filenames broken in mp/sp/pak sort above
//----(SA)	mod for SP
			if ( !Q_strncmp( sorted[i],"zz_",3 ) ) {
				memcpy( sorted[i],"sp",2 );
#elif defined RTCW_MP
		if ( Q_strncmp( sorted[i],"sp_",3 ) ) { // JPW NERVE -- exclude sp_*
// JPW NERVE KLUDGE: fix filenames broken in mp/sp/pak sort above

			if ( !Q_strncmp( sorted[i],"zz_",3 ) ) {
				memcpy( sorted[i],"mp",2 );
#endif // RTCW_XX

			}

// jpw
#else
/*		if (Q_strncmp(sorted[i],"sp_",3)) { // JPW NERVE -- exclude sp_*
// JPW NERVE KLUDGE: fix filenames broken in mp/sp/pak sort above

			if (!Q_strncmp(sorted[i],"zz_",3))
				memcpy(sorted[i],"mp",2);

// jpw
*/
#endif // RTCW_XX

			pakfile = FS_BuildOSPath( path, dir, sorted[i] );
			if ( ( pak = FS_LoadZipFile( pakfile, sorted[i] ) ) == 0 ) {
				continue;
			}
			// store the game name for downloading
			strcpy( pak->pakGamename, dir );

			search = static_cast<searchpath_t*> (Z_Malloc( sizeof( searchpath_t ) ));
			search->pack = pak;
			search->next = fs_searchpaths;
			fs_searchpaths = search;

#if !defined RTCW_ET
		}
#else
//		}
#endif // RTCW_XX

	}

	// done
	Sys_FreeFileList( pakfiles );
}

/*
================
FS_idPak
================
*/
qboolean FS_idPak( const char *pak, const char *base ) {
	int i;

#if !defined RTCW_SP
	if ( !FS_FilenameCompare( pak, va( "%s/mp_bin", base ) ) ) {
		return qtrue;
	}
#endif // RTCW_XX

	for ( i = 0; i < NUM_ID_PAKS; i++ ) {
		if ( !FS_FilenameCompare( pak, va( "%s/pak%d", base, i ) ) ) {
			break;
		}

#if !defined RTCW_ET
// JPW NERVE -- this fn prevents external sources from downloading/overwriting official files, so exclude both SP and MP files from this list as well
		if ( !FS_FilenameCompare( pak, va( "%s/mp_pak%d",base,i ) ) ) {
			break;
		}
		if ( !FS_FilenameCompare( pak, va( "%s/sp_pak%d",base,i ) ) ) {
			break;
		}
// jpw
#else
/*
// JPW NERVE -- this fn prevents external sources from downloading/overwriting official files, so exclude both SP and MP files from this list as well
		if ( !FS_FilenameCompare(pak, va("%s/mp_pak%d",base,i)) ) {
			break;
		}
		if ( !FS_FilenameCompare(pak, va("%s/sp_pak%d",base,i)) ) {
			break;
		}
// jpw
*/
#endif // RTCW_XX

	}
	if ( i < NUM_ID_PAKS ) {
		return qtrue;
	}
	return qfalse;
}

#if defined RTCW_ET
typedef struct {
	char pakname[MAX_QPATH];
	qboolean ok;
} officialpak_t;

/*
================
FS_VerifyOfficialPaks
================
*/
qboolean FS_VerifyOfficialPaks( void ) {
	int i, j;
	searchpath_t    *sp;
	int numOfficialPaksOnServer = 0;
	int numOfficialPaksLocal = 0;
	officialpak_t officialpaks[64];

	if ( !fs_numServerPaks ) {
		return qtrue;
	}

	for ( i = 0; i < fs_numServerPaks; i++ ) {
		if ( FS_idPak( fs_serverPakNames[i], BASEGAME ) ) {
			Q_strncpyz( officialpaks[numOfficialPaksOnServer].pakname, fs_serverPakNames[i], sizeof( officialpaks[0].pakname ) );
			officialpaks[numOfficialPaksOnServer].ok = qfalse;
			numOfficialPaksOnServer++;
		}
	}

	for ( i = 0; i < fs_numServerPaks; i++ ) {
		for ( sp = fs_searchpaths ; sp ; sp = sp->next ) {
			if ( sp->pack && sp->pack->checksum == fs_serverPaks[i] ) {
				char packPath[MAX_QPATH];

				Com_sprintf( packPath, sizeof( packPath ), "%s/%s", sp->pack->pakGamename, sp->pack->pakBasename );

				if ( FS_idPak( packPath, BASEGAME ) ) {
					for ( j = 0; j < numOfficialPaksOnServer; j++ ) {
						if ( !Q_stricmp( packPath, officialpaks[j].pakname ) ) {
							officialpaks[j].ok = qtrue;
						}
					}
					numOfficialPaksLocal++;
				}
				break;
			}
		}
	}

	if ( numOfficialPaksOnServer != numOfficialPaksLocal ) {
		for ( i = 0; i < numOfficialPaksOnServer; i++ ) {
			if ( officialpaks[i].ok != qtrue ) {
				Com_Printf( "ERROR: Missing/corrupt official pak file %s\n", officialpaks[i].pakname );
			}
		}
		return qfalse;
	} else {
		return qtrue;
	}
}
#endif // RTCW_XX

/*
================
FS_ComparePaks

----------------
dlstring == qtrue

Returns a list of pak files that we should download from the server. They all get stored
in the current gamedir and an FS_Restart will be fired up after we download them all.

The string is the format:

@remotename@localname [repeat]

static int		fs_numServerReferencedPaks;
static int		fs_serverReferencedPaks[MAX_SEARCH_PATHS];
static char		*fs_serverReferencedPakNames[MAX_SEARCH_PATHS];

----------------
dlstring == qfalse

we are not interested in a download string format, we want something human-readable
(this is used for diagnostics while connecting to a pure server)

================
*/

#if defined RTCW_ET
qboolean CL_WWWBadChecksum( const char *pakname );
#endif // RTCW_XX

qboolean FS_ComparePaks( char *neededpaks, int len, qboolean dlstring ) {
	searchpath_t    *sp;
	qboolean havepak, badchecksum;
	int i;

	if ( !fs_numServerReferencedPaks ) {
		return qfalse; // Server didn't send any pack information along
	}

	*neededpaks = 0;

	for ( i = 0 ; i < fs_numServerReferencedPaks ; i++ ) {
		// Ok, see if we have this pak file
		badchecksum = qfalse;
		havepak = qfalse;

		// never autodownload any of the id paks

#if defined RTCW_SP
		if ( FS_idPak( fs_serverReferencedPakNames[i], "baseq3" ) || FS_idPak( fs_serverReferencedPakNames[i], "missionpack" ) ) {
#elif defined RTCW_MP
		if ( FS_idPak( fs_serverReferencedPakNames[i], "main" ) ) {
#else
		if ( FS_idPak( fs_serverReferencedPakNames[i], BASEGAME ) ) {
#endif // RTCW_XX

			continue;
		}

		for ( sp = fs_searchpaths ; sp ; sp = sp->next ) {
			if ( sp->pack && sp->pack->checksum == fs_serverReferencedPaks[i] ) {
				havepak = qtrue; // This is it!
				break;
			}
		}

		if ( !havepak && fs_serverReferencedPakNames[i] && *fs_serverReferencedPakNames[i] ) {
			// Don't got it

			if ( dlstring ) {
				// Remote name
				Q_strcat( neededpaks, len, "@" );
				Q_strcat( neededpaks, len, fs_serverReferencedPakNames[i] );
				Q_strcat( neededpaks, len, ".pk3" );

				// Local name
				Q_strcat( neededpaks, len, "@" );
				// Do we have one with the same name?
				if ( FS_SV_FileExists( va( "%s.pk3", fs_serverReferencedPakNames[i] ) ) ) {
					char st[MAX_ZPATH];
					// We already have one called this, we need to download it to another name
					// Make something up with the checksum in it
					Com_sprintf( st, sizeof( st ), "%s.%08x.pk3", fs_serverReferencedPakNames[i], fs_serverReferencedPaks[i] );
					Q_strcat( neededpaks, len, st );
				} else
				{
					Q_strcat( neededpaks, len, fs_serverReferencedPakNames[i] );
					Q_strcat( neededpaks, len, ".pk3" );
				}
			} else
			{
				Q_strcat( neededpaks, len, fs_serverReferencedPakNames[i] );
				Q_strcat( neededpaks, len, ".pk3" );
				// Do we have one with the same name?
				if ( FS_SV_FileExists( va( "%s.pk3", fs_serverReferencedPakNames[i] ) ) ) {
					Q_strcat( neededpaks, len, " (local file exists with wrong checksum)" );

#if defined RTCW_ET
					#ifndef DEDICATED
					// let the client subsystem track bad download redirects (dl file with wrong checksums)
					// this is a bit ugly but the only other solution would have been callback passing..
					if ( CL_WWWBadChecksum( va( "%s.pk3", fs_serverReferencedPakNames[i] ) ) ) {
						// remove a potentially malicious download file
						// (this is also intended to avoid expansion of the pk3 into a file with different checksum .. messes up wwwdl chkfail)
						char *rmv = FS_BuildOSPath( fs_homepath->string, va( "%s.pk3", fs_serverReferencedPakNames[i] ), "" );
						rmv[strlen( rmv ) - 1] = '\0';
						FS_Remove( rmv );
					}
					#endif
#endif // RTCW_XX

				}
				Q_strcat( neededpaks, len, "\n" );
			}
		}
	}

	if ( *neededpaks ) {
		Com_Printf( "Need paks: %s\n", neededpaks );
		return qtrue;
	}

	return qfalse; // We have them all
}

/*
================
FS_Shutdown

Frees all resources and closes all files
================
*/
void FS_Shutdown( qboolean closemfp ) {
	searchpath_t    *p, *next;
	int i;

	for ( i = 0; i < MAX_FILE_HANDLES; i++ ) {
		if ( fsh[i].fileSize ) {
			FS_FCloseFile( i );
		}
	}

	// free everything
	for ( p = fs_searchpaths ; p ; p = next ) {
		next = p->next;

		if ( p->pack ) {
			delete p->pack->miniz_zip_ptr_;

			Z_Free( p->pack->buildBuffer );
			Z_Free( p->pack );
		}
		if ( p->dir ) {
			Z_Free( p->dir );
		}
		Z_Free( p );
	}

	// any FS_ calls will now be an error until reinitialized
	fs_searchpaths = NULL;

	Cmd_RemoveCommand( "path" );
	Cmd_RemoveCommand( "dir" );
	Cmd_RemoveCommand( "fdir" );
	Cmd_RemoveCommand( "touchFile" );

#ifdef FS_MISSING
	if ( closemfp ) {
		fclose( missingFiles );
	}
#endif
}

#if defined RTCW_SP
void Com_AppendCDKey( const char *filename );
void Com_ReadCDKey( const char *filename );
#else
/*
================
FS_ReorderPurePaks
NOTE TTimo: the reordering that happens here is not reflected in the cvars (\cvarlist *pak*)
  this can lead to misleading situations, see show_bug.cgi?id=540
================
*/
static void FS_ReorderPurePaks() {
	searchpath_t *s;
	int i;
	searchpath_t **p_insert_index, // for linked list reordering
	**p_previous;     // when doing the scan

	// only relevant when connected to pure server
	if ( !fs_numServerPaks ) {
		return;
	}

	fs_reordered = qfalse;

	p_insert_index = &fs_searchpaths; // we insert in order at the beginning of the list
	for ( i = 0 ; i < fs_numServerPaks ; i++ ) {
		p_previous = p_insert_index; // track the pointer-to-current-item
		for ( s = *p_insert_index; s; s = s->next ) { // the part of the list before p_insert_index has been sorted already
			if ( s->pack && fs_serverPaks[i] == s->pack->checksum ) {
				fs_reordered = qtrue;
				// move this element to the insert list
				*p_previous = s->next;
				s->next = *p_insert_index;
				*p_insert_index = s;
				// increment insert list
				p_insert_index = &s->next;
				break; // iterate to next server pack
			}
			p_previous = &s->next;
		}
	}

}
#endif // RTCW_XX

/*
================
FS_Startup
================
*/
static void FS_Startup( const char *gameName ) {
	const char *homePath;
	cvar_t  *fs;

	Com_Printf( "----- FS_Startup -----\n" );

	fs_debug = Cvar_Get( "fs_debug", "0", 0 );
	fs_copyfiles = Cvar_Get( "fs_copyfiles", "0", CVAR_INIT );
	fs_cdpath = Cvar_Get( "fs_cdpath", Sys_DefaultCDPath(), CVAR_INIT );
	fs_basepath = Cvar_Get( "fs_basepath", Sys_DefaultInstallPath(), CVAR_INIT );

#if defined RTCW_ET
	fs_buildpath = Cvar_Get( "fs_buildpath", "", CVAR_INIT );
	fs_buildgame = Cvar_Get( "fs_buildgame", BASEGAME, CVAR_INIT );
#endif // RTCW_XX

	fs_basegame = Cvar_Get( "fs_basegame", "", CVAR_INIT );
	homePath = Sys_DefaultHomePath();
	if ( !homePath || !homePath[0] ) {
		homePath = fs_basepath->string;
	}
	fs_homepath = Cvar_Get( "fs_homepath", homePath, CVAR_INIT );
	fs_gamedirvar = Cvar_Get( "fs_game", "", CVAR_INIT | CVAR_SYSTEMINFO );
	fs_restrict = Cvar_Get( "fs_restrict", "", CVAR_INIT );

	// BBi
	FS_AddGameDirectory (fs_basepath->string, "rtcw");
	// BBi

	// add search path elements in reverse priority order
	if ( fs_cdpath->string[0] ) {
		FS_AddGameDirectory( fs_cdpath->string, gameName );
	}
	if ( fs_basepath->string[0] ) {
		FS_AddGameDirectory( fs_basepath->string, gameName );
	}
	// fs_homepath is somewhat particular to *nix systems, only add if relevant
	// NOTE: same filtering below for mods and basegame
	if ( fs_basepath->string[0] && Q_stricmp( fs_homepath->string,fs_basepath->string ) ) {
		FS_AddGameDirectory( fs_homepath->string, gameName );
	}

#if !defined RTCW_ET || (defined RTCW_ET && !defined PRE_RELEASE_DEMO)
	// check for additional base game so mods can be based upon other mods
	if ( fs_basegame->string[0] && !Q_stricmp( gameName, BASEGAME ) && Q_stricmp( fs_basegame->string, gameName ) ) {
		if ( fs_cdpath->string[0] ) {
			FS_AddGameDirectory( fs_cdpath->string, fs_basegame->string );
		}
		if ( fs_basepath->string[0] ) {
			FS_AddGameDirectory( fs_basepath->string, fs_basegame->string );
		}
		if ( fs_homepath->string[0] && Q_stricmp( fs_homepath->string,fs_basepath->string ) ) {
			FS_AddGameDirectory( fs_homepath->string, fs_basegame->string );
		}
	}

	// check for additional game folder for mods
	if ( fs_gamedirvar->string[0] && !Q_stricmp( gameName, BASEGAME ) && Q_stricmp( fs_gamedirvar->string, gameName ) ) {
		if ( fs_cdpath->string[0] ) {
			FS_AddGameDirectory( fs_cdpath->string, fs_gamedirvar->string );
		}
		if ( fs_basepath->string[0] ) {
			FS_AddGameDirectory( fs_basepath->string, fs_gamedirvar->string );
		}
		if ( fs_homepath->string[0] && Q_stricmp( fs_homepath->string,fs_basepath->string ) ) {
			FS_AddGameDirectory( fs_homepath->string, fs_gamedirvar->string );
		}
	}
#endif // RTCW_XX

	Com_ReadCDKey( BASEGAME );
	fs = Cvar_Get( "fs_game", "", CVAR_INIT | CVAR_SYSTEMINFO );
	if ( fs && fs->string[0] != 0 ) {
		Com_AppendCDKey( fs->string );
	}

	// add our commands
	Cmd_AddCommand( "path", FS_Path_f );
	Cmd_AddCommand( "dir", FS_Dir_f );
	Cmd_AddCommand( "fdir", FS_NewDir_f );
	Cmd_AddCommand( "touchFile", FS_TouchFile_f );

#if !defined RTCW_SP
	// show_bug.cgi?id=506
	// reorder the pure pk3 files according to server order
	FS_ReorderPurePaks();
#endif // RTCW_XX

	// print the current search paths
	FS_Path_f();

	fs_gamedirvar->modified = qfalse; // We just loaded, it's not modified

	Com_Printf( "----------------------\n" );

#ifdef FS_MISSING
	if ( missingFiles == NULL ) {
		missingFiles = fopen( "\\missing.txt", "ab" );
	}
#endif
	Com_Printf( "%d files in pk3 files\n", fs_packFiles );
}


/*
===================
FS_SetRestrictions

Looks for product keys and restricts media add on ability
if the full version is not found
===================
*/
static void FS_SetRestrictions( void ) {

#if defined RTCW_SP
//	searchpath_t	*path;
#else
	searchpath_t	*path;
#endif // RTCW_XX

#if defined RTCW_SP
#ifndef WOLF_SP_DEMO
	// if fs_restrict is set, don't even look for the id file,
	// which allows the demo release to be tested even if
	// the full game is present
	if ( !fs_restrict->integer ) {
		return; // no restrictions
	}
#endif
#else
#ifndef PRE_RELEASE_DEMO
	// if fs_restrict is set, don't even look for the id file,
	// which allows the demo release to be tested even if
	// the full game is present
	if ( !fs_restrict->integer ) {
		// look for the full game id

		// NO RESTRICTIONS IN RETAIL GAME
		return;
	}
#endif
#endif // RTCW_XX

	Cvar_Set( "fs_restrict", "1" );

	Com_Printf( "\nRunning in restricted demo mode.\n\n" );

	// restart the filesystem with just the demo directory
	FS_Shutdown( qfalse );

#if defined RTCW_SP
#ifdef WOLF_SP_DEMO
	FS_Startup( DEMOGAME );
#else
	FS_Startup( BASEGAME );
#endif
#elif defined RTCW_MP
#ifdef PRE_RELEASE_DEMO
	FS_Startup( DEMOGAME );
#else
	FS_Startup( BASEGAME );
#endif
#else
	FS_Startup( BASEGAME );
#endif // RTCW_XX

#if !defined RTCW_SP
	// make sure that the pak file has the header checksum we expect
	for ( path = fs_searchpaths ; path ; path = path->next ) {
		if ( path->pack ) {
#endif // RTCW_XX

#if defined RTCW_MP
			// a tiny attempt to keep the checksum from being scannable from the exe
			if ( ( path->pack->checksum ^ 0x02261994u )
				 != ( DEMO_PAK_CHECKSUM ^ 0x02261994u ) ) {
				Com_Error( ERR_FATAL, "Corrupted pak0.pk3: %u", path->pack->checksum );
#elif defined RTCW_ET
// every time a new demo pk3 file is built, this checksum must be updated.
// the easiest way to get it is to just run the game and see what it spits out
//DHM - Nerve :: Wolf Multiplayer demo checksum
// NOTE TTimo: always needs the 'u' for unsigned int (gcc)
#define DEMO_MPBIN_CHECKSUM 2217494506u
#define DEMO_PAK0_CHECKSUM  846032800u

#define SYS_PAKNAME_MPBIN_SHIFT 3
#define SYS_PAKNAME_MPBIN "psbelq"
#define SYS_PAKNAME_PAK0_SHIFT 7
#define SYS_PAKNAME_PAK0 "whr7"

#if 0 // use that stuff for shifted strings
			Com_Printf( "SYS_PAKNAME_MPBIN + %d: '%s'\n", SYS_PAKNAME_MPBIN_SHIFT, FS_ShiftStr( "mp_bin", SYS_PAKNAME_MPBIN_SHIFT ) );
			Com_Printf( "SYS_PAKNAME_PAK0 + %d: '%s'\n", SYS_PAKNAME_PAK0_SHIFT, FS_ShiftStr( "pak0", SYS_PAKNAME_PAK0_SHIFT ) );
#endif
			if ( FS_ShiftedStrStr( path->pack->pakBasename, SYS_PAKNAME_MPBIN, -SYS_PAKNAME_MPBIN_SHIFT ) ) {
				// a tiny attempt to keep the checksum from being scannable from the exe
				if ( ( path->pack->checksum ^ 0x01042000u )
					 != ( DEMO_MPBIN_CHECKSUM ^ 0x01042000u ) ) {
					Com_Error( ERR_FATAL, "Corrupted pakfile: %u", path->pack->checksum );
				}
			} else if ( FS_ShiftedStrStr( path->pack->pakBasename, SYS_PAKNAME_PAK0, -SYS_PAKNAME_PAK0_SHIFT ) ) {
				// a tiny attempt to keep the checksum from being scannable from the exe
				if ( ( path->pack->checksum ^ 0x04062001u )
					 != ( DEMO_PAK0_CHECKSUM ^ 0x04062001u ) ) {
					Com_Error( ERR_FATAL, "Corrupted pakfile: %u", path->pack->checksum );
				}
			} else {
				Com_Error( ERR_FATAL, "Corrupted pakfile: %u", path->pack->checksum );
#endif // RTCW_XX

#if !defined RTCW_SP
			}
		}
	}
#endif // RTCW_XX

}

#if defined RTCW_SP
/*
=====================
FS_GamePureChecksum

Returns the checksum of the pk3 from which the server loaded the qagame.qvm
=====================
*/
#else
/*
=====================
FS_GamePureChecksum
Returns the checksum of the pk3 from which the server loaded the qagame.qvm
NOTE TTimo: this is not used in RTCW so far
=====================
*/
#endif // RTCW_XX

const char *FS_GamePureChecksum( void ) {
	static char info[MAX_STRING_TOKENS];
	searchpath_t *search;

	info[0] = 0;

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		// is the element a pak file?
		if ( search->pack ) {
			if ( search->pack->referenced & FS_QAGAME_REF ) {
				Com_sprintf( info, sizeof( info ), "%d", search->pack->checksum );
			}
		}
	}

	return info;
}

#if defined RTCW_SP || (!defined RTCW_SP && !defined DO_LIGHT_DEDICATED)
/*
=====================
FS_LoadedPakChecksums

Returns a space separated string containing the checksums of all loaded pk3 files.
Servers with sv_pure set will get this string and pass it to clients.
=====================
*/
const char *FS_LoadedPakChecksums( void ) {
	static char info[BIG_INFO_STRING];
	searchpath_t    *search;

	info[0] = 0;

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		// is the element a pak file?
		if ( !search->pack ) {
			continue;
		}

		Q_strcat( info, sizeof( info ), va( "%i ", search->pack->checksum ) );
	}

	return info;
}

/*
=====================
FS_LoadedPakNames

Returns a space separated string containing the names of all loaded pk3 files.
Servers with sv_pure set will get this string and pass it to clients.
=====================
*/
const char *FS_LoadedPakNames( void ) {
	static char info[BIG_INFO_STRING];
	searchpath_t    *search;

	info[0] = 0;

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		// is the element a pak file?
		if ( !search->pack ) {
			continue;
		}

		if ( *info ) {
			Q_strcat( info, sizeof( info ), " " );
		}

#if defined RTCW_ET
		// Arnout: changed to have the full path
		//Q_strcat( info, sizeof( info ), search->pack->pakBasename );
		Q_strcat( info, sizeof( info ), search->pack->pakGamename );
		Q_strcat( info, sizeof( info ), "/" );
#endif // RTCW_XX

		Q_strcat( info, sizeof( info ), search->pack->pakBasename );
	}

	return info;
}

/*
=====================
FS_LoadedPakPureChecksums

Returns a space separated string containing the pure checksums of all loaded pk3 files.
Servers with sv_pure use these checksums to compare with the checksums the clients send
back to the server.
=====================
*/
const char *FS_LoadedPakPureChecksums( void ) {
	static char info[BIG_INFO_STRING];
	searchpath_t    *search;

	info[0] = 0;

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		// is the element a pak file?
		if ( !search->pack ) {
			continue;
		}

		Q_strcat( info, sizeof( info ), va( "%i ", search->pack->pure_checksum ) );
	}

#if !defined RTCW_SP
	// DO_LIGHT_DEDICATED
	// only comment out when you need a new pure checksums string
	//Com_DPrintf("FS_LoadPakPureChecksums: %s\n", info);
#endif // RTCW_XX

	return info;
}

/*
=====================
FS_ReferencedPakChecksums

Returns a space separated string containing the checksums of all referenced pk3 files.
The server will send this to the clients so they can check which files should be auto-downloaded.
=====================
*/
const char *FS_ReferencedPakChecksums( void ) {
	static char info[BIG_INFO_STRING];
	searchpath_t *search;

	info[0] = 0;

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		// is the element a pak file?
		if ( search->pack ) {
			if ( search->pack->referenced || Q_stricmpn( search->pack->pakGamename, BASEGAME, strlen( BASEGAME ) ) ) {
				Q_strcat( info, sizeof( info ), va( "%i ", search->pack->checksum ) );
			}
		}
	}

	return info;
}

#if !defined RTCW_SP
/*
=====================
FS_ReferencedPakNames

Returns a space separated string containing the names of all referenced pk3 files.
The server will send this to the clients so they can check which files should be auto-downloaded.
=====================
*/
const char *FS_ReferencedPakNames( void ) {
	static char info[BIG_INFO_STRING];
	searchpath_t    *search;

	info[0] = 0;

	// we want to return ALL pk3's from the fs_game path
	// and referenced one's from baseq3
	for ( search = fs_searchpaths ; search ; search = search->next ) {
		// is the element a pak file?
		if ( search->pack ) {
			if ( *info ) {
				Q_strcat( info, sizeof( info ), " " );
			}
			if ( search->pack->referenced || Q_stricmpn( search->pack->pakGamename, BASEGAME, strlen( BASEGAME ) ) ) {
				Q_strcat( info, sizeof( info ), search->pack->pakGamename );
				Q_strcat( info, sizeof( info ), "/" );
				Q_strcat( info, sizeof( info ), search->pack->pakBasename );
			}
		}
	}

	return info;
}
#endif // RTCW_XX

#if defined RTCW_SP
/*
=====================
FS_ReferencedPakPureChecksums

Returns a space separated string containing the pure checksums of all referenced pk3 files.
Servers with sv_pure set will get this string back from clients for pure validation

The string has a specific order, "cgame ui @ ref1 ref2 ref3 ..."

NOTE TTimo: this code is taken from Wolf MP source
pure checksums code is not relevant to SP binary anyway
=====================
*/
#else
/*
=====================
FS_ReferencedPakPureChecksums

Returns a space separated string containing the pure checksums of all referenced pk3 files.
Servers with sv_pure set will get this string back from clients for pure validation

The string has a specific order, "cgame ui @ ref1 ref2 ref3 ..."

NOTE TTimo - DO_LIGHT_DEDICATED
this function is only used by the client to build the string sent back to server
we don't have any need of overriding it for light, but it's useless in dedicated
=====================
*/
#endif // RTCW_XX

const char *FS_ReferencedPakPureChecksums( void ) {
	static char info[BIG_INFO_STRING];
	searchpath_t    *search;
	int nFlags, numPaks, checksum;

	info[0] = 0;
	checksum = fs_checksumFeed;

	numPaks = 0;

#if defined RTCW_SP
	for ( nFlags = FS_GENERAL_REF; nFlags; nFlags = nFlags >> 1 ) {
#else
	for ( nFlags = FS_CGAME_REF; nFlags; nFlags = nFlags >> 1 ) {
		if ( nFlags & FS_GENERAL_REF ) {
			// add a delimter between must haves and general refs
			//Q_strcat(info, sizeof(info), "@ ");
			info[strlen( info ) + 1] = '\0';
			info[strlen( info ) + 2] = '\0';
			info[strlen( info )] = '@';
			info[strlen( info )] = ' ';
		}
#endif // RTCW_XX

		for ( search = fs_searchpaths ; search ; search = search->next ) {
			// is the element a pak file and has it been referenced based on flag?
			if ( search->pack && ( search->pack->referenced & nFlags ) ) {
				Q_strcat( info, sizeof( info ), va( "%i ", search->pack->pure_checksum ) );

#if !defined RTCW_SP
				if ( nFlags & ( FS_CGAME_REF | FS_UI_REF ) ) {
					break;
				}
#endif // RTCW_XX

				checksum ^= search->pack->pure_checksum;
				numPaks++;
			}
		}
		if ( fs_fakeChkSum != 0 ) {
			// only added if a non-pure file is referenced
			Q_strcat( info, sizeof( info ), va( "%i ", fs_fakeChkSum ) );
		}
	}
	// last checksum is the encoded number of referenced pk3s
	checksum ^= numPaks;
	Q_strcat( info, sizeof( info ), va( "%i ", checksum ) );

	return info;
}
#else

/*
=========================================================================================
DO_LIGHT_DEDICATED, general notes
we are going to fake the checksums sent to the clients
that only matters to the pk3 we have replaced by their lighter version, currently:

Cvar_Set2: sv_pakNames mp_pakmaps0 mp_pak2 mp_pak1 mp_pak0 pak0
Cvar_Set2: sv_paks -1153491798 125907563 -1023558518 764840216 1886207346

all the files above have their 'server required' content collapsed into a single pak0.pk3

the other .pk3 files should be handled as usual

more details are in unix/dedicated-only.txt

=========================================================================================
*/

// our target faked checksums
// those don't need to be encrypted or anything, that's what you see in the +set developer 1
static const char* pak_checksums = "-137448799 131270674 125907563 -1023558518 764840216 1886207346";
static const char* pak_names = "mp_pak4 mp_pak3 mp_pak2 mp_pak1 mp_pak0 pak0";

/*
this is the pure checksum string for a constant value of fs_checksumFeed we have choosen (see SV_SpawnServer)
to obtain the new string for a different fs_checksumFeed value, run a regular server and enable the relevant
verbosity code in SV_SpawnServer and FS_LoadedPakPureChecksums (the full server version of course)

NOTE: if you have an mp_bin in the middle, you need to take out it's checksum
  (we keep mp_bin out of the faked stuff because we don't want to have to update those feeds too often heh)

once you have the clear versions, you can shift them by commenting out the code chunk in FS_RandChecksumFeed
you need to use the right line in FS_LoadedPakPureChecksums wether you are running on clear strings, or shifted ones
*/

/*
// clear checksums, rebuild those from a regular server and you will shift them next
static const int feeds[5] = {

#if !defined RTCW_ET
	0xd6009839, 0x636bb1d5, 0x198df4c9, 0x7ffa631b, 0x8f89a69e
#else
  0x14d48835, 0xc44ed670, 0xd1c8da0d, 0x98df0626, 0xb4e51e7a
#endif // RTCW_XX

};

static const char* pak_purechecksums[5] = {

#if !defined RTCW_ET
 "943814896 694898104 1407923890 242847633 1117823230 -1543700213",
 "-111135514 976363775 -1066586315 -509503305 226888806 623380740",
 "465689568 1394972621 1593048073 488347192 -238809598 -396332776",
 "-2000534548 253432450 -1505880367 682854303 -1183636432 -1745648892",
 "-588301396 -637070806 -49124646 831116909 -666702847 1152718748"
#else
 "-631058236 1439191868 -1758535722 -1109639830 -756342425 -26055934",
 "420891163 -2077045804 -1212476885 273103692 1907819222 -1162012968",
 "724865970 393950398 1987220301 679766798 -966287476 -1045306141",
 "468836794 -690412926 -481399336 1089964294 -1538547350 394664641",
 "-1484520489 -1891368444 -510451918 -919424191 -1623567814 889557862"
#endif // RTCW_XX

};
*/

static const unsigned int feeds[5] = {

#if !defined RTCW_ET
	0xd6009839, 0x636bb1d5, 0x198df4c9, 0x7ffa631b, 0x8f89a69e
#else
	0x14d48835, 0xc44ed670, 0xd1c8da0d, 0x98df0626, 0xb4e51e7a
#endif // RTCW_XX

};

// shifted strings, so that it's not directly scannable from exe
// see FS_RandChecksumFeed to generate them
static const char* pak_purechecksums[5] = {

#if !defined RTCW_ET
	"FA@E>AEFC-CFAEFE>=A->A=DF?@EF=-?A?EADC@@->>>DE?@?@=-:>BA@D==?>@",
	";????ACC?B.GEDADAEEC.;?>DDCFDA?C.;C>GC>AA>C.@@DFFFF>D.D@AAF>EB>",
	"CEDEGHDEG/@BHCHFAEA@/@DHB?CG?FB/CGGBCF@HA/<ABGG?HDHG/<BHEBBAFFE",
	"=B@@@ECDEDH0BECDCBDE@0=AE@EHH@CFG0FHBHEDC@C0=AAHCFCFDCB0=AGDEFDHHIB",
	">FIIDABDJG1>GDHAHAIAG1>EJBCEGEG1IDBBBGJAJ1>GGGHACIEH1BBFCHBIHEI"
#else
	// rain - escaped ?s to prevent parsing as trigraph
	":C@>=BE?@C->A@F>F>ECE-:>DBEB@BD\?\?-:>>=FC@FE@=-:DBC@A?A?B-:?C=BBF@A",
	"B@>FG??DA.;@>EE>BCF>B.;?@?@BEDFFC.@EA?>ADG@.?G>EF?G@@@.;??D@>?@GDF",
	"FACGEDHF?/BHBHD?BHG/@HGFAA?B?@/EFHFEEFHG/<HEEAGFCFE/<@?CDB?E@C@",
	"DFHHCFGID0=FI@DABIBF0=DHACIICCF0A@HIIFDBID0=AECHEDGCE@0CIDFFDFDA",
	">BEIEFCAEIJ1>BIJBDGIEEE1>FBAEFBJBI1>JBJECEBJB1>BGCDFGHIBE1IIJFFHIGC"
#endif // RTCW_XX

};

// counter to walk through the randomized list
static int feed_index = -1;

static int lookup_randomized[5] = { 0, 1, 2, 3, 4 };

/*
=====================
randomize the order of the 5 checksums we rely on
5 random swaps of the table
=====================
*/
void FS_InitRandomFeed() {
	int i, swap, aux;
	for ( i = 0; i < 5; i++ )
	{
		swap = (int)( 5.0 * rand() / ( RAND_MAX + 1.0 ) );
		aux = lookup_randomized[i]; lookup_randomized[i] = lookup_randomized[swap]; lookup_randomized[swap] = aux;
	}
}

/*
=====================
FS_RandChecksumFeed

Return a random checksum feed among our list
we keep the seed and use it when requested for the pure checksum
=====================
*/
int FS_RandChecksumFeed() {
	/*
	// use this to dump shifted versions of the pure checksum strings
	int i;
	for(i=0;i<5;i++)
	{
	  Com_Printf("FS_RandChecksumFeed: %s\n", FS_ShiftStr(pak_purechecksums[i], 13+i));
	}
	*/
	if ( feed_index == -1 ) {
		FS_InitRandomFeed();
	}
	feed_index = ( feed_index + 1 ) % 5;
	return static_cast<int>(feeds[lookup_randomized[feed_index]]);
}

/*
=====================
FS_LoadedPakChecksums

Returns a space separated string containing the checksums of all loaded pk3 files.
Servers with sv_pure set will get this string and pass it to clients.

DO_LIGHT_DEDICATED:
drop lightweight pak0 checksum, put the faked pk3s checksums instead
=====================
*/
const char *FS_LoadedPakChecksums( void ) {
	static char info[BIG_INFO_STRING];
	searchpath_t    *search;

	info[0] = 0;

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		// is the element a pak file?
		if ( !search->pack ) {
			continue;
		}

		if ( strcmp( search->pack->pakBasename,"pak0" ) ) {
			// this is a regular pk3
			Q_strcat( info, sizeof( info ), va( "%i ", search->pack->checksum ) );
		} else
		{
			// this is the light pk3
			Q_strcat( info, sizeof( info ), va( "%s ", pak_checksums ) );
		}
	}

	return info;
}

/*
=====================
FS_LoadedPakNames

Returns a space separated string containing the names of all loaded pk3 files.
Servers with sv_pure set will get this string and pass it to clients.

DO_LIGHT_DEDICATED:
drop lightweight pak0 name, put the faked pk3s names instead
=====================
*/
const char *FS_LoadedPakNames( void ) {
	static char info[BIG_INFO_STRING];
	searchpath_t    *search;

	info[0] = 0;

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		// is the element a pak file?
		if ( !search->pack ) {
			continue;
		}

		if ( *info ) {
			Q_strcat( info, sizeof( info ), " " );
		}
		if ( strcmp( search->pack->pakBasename,"pak0" ) ) {
			// regular pk3
			Q_strcat( info, sizeof( info ), search->pack->pakBasename );
		} else
		{
			// light pk3
			Q_strcat( info, sizeof( info ), pak_names );
		}
	}

	return info;
}

/*
=====================
FS_LoadedPakPureChecksums

Returns a space separated string containing the pure checksums of all loaded pk3 files.
Servers with sv_pure use these checksums to compare with the checksums the clients send
back to the server.

DO_LIGHT_DEDICATED:
FS_LoadPakChecksums to send the pak string to the client
FS_LoadPakPureChecksums is used locally to compare against what the client sends back

the pure_checksums are computed by Com_MemoryBlockChecksum with a random key (fs_checksumFeed)
since we can't do this on restricted server, we always use the same fs_checksumFeed value

drop lightweight pak0 checksum, put the faked pk3s pure checksums instead

=====================
*/
const char *FS_LoadedPakPureChecksums( void ) {
	static char info[BIG_INFO_STRING];
	searchpath_t    *search;

	info[0] = 0;

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		// is the element a pak file?
		if ( !search->pack ) {
			continue;
		}

		if ( strcmp( search->pack->pakBasename,"pak0" ) ) {
			// this is a regular pk3
			Q_strcat( info, sizeof( info ), va( "%i ", search->pack->pure_checksum ) );
		} else
		{
			// this is the light pk3
			// use this if you are running on shifted strings
			Q_strcat( info, sizeof( info ), va( "%s ", FS_ShiftStr( pak_purechecksums[lookup_randomized[feed_index]], -13 - lookup_randomized[feed_index] ) ) );
			// use this if you are running on clear checksum strings instead of shifted ones
			//Q_strcat( info, sizeof( info ), va("%s ", pak_purechecksums[lookup_randomized[feed_index]] ) );
		}
	}

	return info;
}

/*
=====================
FS_ReferencedPakChecksums

Returns a space separated string containing the checksums of all referenced pk3 files.
The server will send this to the clients so they can check which files should be auto-downloaded.

DO_LIGHT_DEDICATED:
don't send the checksum of pak0 (even if it's referenced)

NOTE:
do we need to fake referenced paks too?
those are Id paks, so you can't download them
mp_pakmaps0 would be a worthy candidate for download though, but we don't have it anyway
the only thing if we omit sending of some referenced stuff, you don't get the console message that says "you're missing this"
=====================
*/
const char *FS_ReferencedPakChecksums( void ) {
	static char info[BIG_INFO_STRING];
	searchpath_t *search;

	info[0] = 0;

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		// is the element a pak file?
		if ( search->pack ) {
			if ( search->pack->referenced ) {
				if ( strcmp( search->pack->pakBasename, "pak0" ) ) {
					// this is not the light pk3
					Q_strcat( info, sizeof( info ), va( "%i ", search->pack->checksum ) );
				}
			}
		}
	}

	return info;
}

/*
=====================
FS_ReferencedPakNames

Returns a space separated string containing the names of all referenced pk3 files.
The server will send this to the clients so they can check which files should be auto-downloaded.

DO_LIGHT_DEDICATED:
don't send pak0 see above for details
=====================
*/
const char *FS_ReferencedPakNames( void ) {
	static char info[BIG_INFO_STRING];
	searchpath_t    *search;

	info[0] = 0;

	// we want to return ALL pk3's from the fs_game path
	// and referenced one's from baseq3
	for ( search = fs_searchpaths ; search ; search = search->next ) {
		// is the element a pak file?
		if ( search->pack ) {
			if ( *info ) {
				Q_strcat( info, sizeof( info ), " " );
			}
			if ( search->pack->referenced ) {
				if ( strcmp( search->pack->pakBasename, "pak0" ) ) {
					// this is not the light pk3
					Q_strcat( info, sizeof( info ), search->pack->pakGamename );
					Q_strcat( info, sizeof( info ), "/" );
					Q_strcat( info, sizeof( info ), search->pack->pakBasename );
				}
			}
		}
	}

	return info;
}
#endif // RTCW_XX


#if defined RTCW_SP
/*
=====================
FS_ReferencedPakNames

Returns a space separated string containing the names of all referenced pk3 files.
The server will send this to the clients so they can check which files should be auto-downloaded.
=====================
*/
const char *FS_ReferencedPakNames( void ) {
	static char info[BIG_INFO_STRING];
	searchpath_t    *search;

	info[0] = 0;

	// we want to return ALL pk3's from the fs_game path
	// and referenced one's from baseq3
	for ( search = fs_searchpaths ; search ; search = search->next ) {
		// is the element a pak file?
		if ( search->pack ) {
			if ( *info ) {
				Q_strcat( info, sizeof( info ), " " );
			}
			if ( search->pack->referenced || Q_stricmpn( search->pack->pakGamename, BASEGAME, strlen( BASEGAME ) ) ) {
				Q_strcat( info, sizeof( info ), search->pack->pakGamename );
				Q_strcat( info, sizeof( info ), "/" );
				Q_strcat( info, sizeof( info ), search->pack->pakBasename );
			}
		}
	}

	return info;
}
#endif // RTCW_XX

/*
=====================
FS_ClearPakReferences
=====================
*/
void FS_ClearPakReferences( int flags ) {
	searchpath_t *search;

	if ( !flags ) {
		flags = -1;
	}
	for ( search = fs_searchpaths; search; search = search->next ) {
		// is the element a pak file and has it been referenced?
		if ( search->pack ) {
			search->pack->referenced &= ~flags;
		}
	}
}


/*
=====================
FS_PureServerSetLoadedPaks

If the string is empty, all data sources will be allowed.
If not empty, only pk3 files that match one of the space
separated checksums will be checked for files, with the
exception of .cfg and .dat files.
=====================
*/
void FS_PureServerSetLoadedPaks( const char *pakSums, const char *pakNames ) {
	int i, c, d;

	Cmd_TokenizeString( pakSums );

	c = Cmd_Argc();
	if ( c > MAX_SEARCH_PATHS ) {
		c = MAX_SEARCH_PATHS;
	}

	fs_numServerPaks = c;

	for ( i = 0 ; i < c ; i++ ) {
		fs_serverPaks[i] = atoi( Cmd_Argv( i ) );
	}

	if ( fs_numServerPaks ) {
		Com_DPrintf( "Connected to a pure server.\n" );

#if !defined RTCW_SP
	} else
	{
		if ( fs_reordered ) {
			// show_bug.cgi?id=540
			// force a restart to make sure the search order will be correct
			Com_DPrintf( "FS search reorder is required\n" );
			FS_Restart( fs_checksumFeed );
			return;
		}
#endif // RTCW_XX

	}

	for ( i = 0 ; i < c ; i++ ) {
		if ( fs_serverPakNames[i] ) {
			Z_Free( fs_serverPakNames[i] );
		}
		fs_serverPakNames[i] = NULL;
	}
	if ( pakNames && *pakNames ) {
		Cmd_TokenizeString( pakNames );

		d = Cmd_Argc();
		if ( d > MAX_SEARCH_PATHS ) {
			d = MAX_SEARCH_PATHS;
		}

		for ( i = 0 ; i < d ; i++ ) {
			fs_serverPakNames[i] = CopyString( Cmd_Argv( i ) );
		}
	}
}

/*
=====================
FS_PureServerSetReferencedPaks

The checksums and names of the pk3 files referenced at the server
are sent to the client and stored here. The client will use these
checksums to see if any pk3 files need to be auto-downloaded.
=====================
*/
void FS_PureServerSetReferencedPaks( const char *pakSums, const char *pakNames ) {
	int i, c, d;

	Cmd_TokenizeString( pakSums );

	c = Cmd_Argc();
	if ( c > MAX_SEARCH_PATHS ) {
		c = MAX_SEARCH_PATHS;
	}

	fs_numServerReferencedPaks = c;

	for ( i = 0 ; i < c ; i++ ) {
		fs_serverReferencedPaks[i] = atoi( Cmd_Argv( i ) );
	}

	for ( i = 0 ; i < c ; i++ ) {
		if ( fs_serverReferencedPakNames[i] ) {
			Z_Free( fs_serverReferencedPakNames[i] );
		}
		fs_serverReferencedPakNames[i] = NULL;
	}
	if ( pakNames && *pakNames ) {
		Cmd_TokenizeString( pakNames );

		d = Cmd_Argc();
		if ( d > MAX_SEARCH_PATHS ) {
			d = MAX_SEARCH_PATHS;
		}

		for ( i = 0 ; i < d ; i++ ) {
			fs_serverReferencedPakNames[i] = CopyString( Cmd_Argv( i ) );
		}
	}
}

/*
================
FS_InitFilesystem

Called only at inital startup, not when the filesystem
is resetting due to a game change
================
*/
void FS_InitFilesystem( void ) {
	// allow command line parms to override our defaults
	// we have to specially handle this, because normal command
	// line variable sets don't happen until after the filesystem
	// has already been initialized
	Com_StartupVariable( "fs_cdpath" );
	Com_StartupVariable( "fs_basepath" );

#if defined RTCW_ET
	Com_StartupVariable( "fs_buildpath" );
	Com_StartupVariable( "fs_buildgame" );
#endif // RTCW_XX

	Com_StartupVariable( "fs_homepath" );
	Com_StartupVariable( "fs_game" );
	Com_StartupVariable( "fs_copyfiles" );
	Com_StartupVariable( "fs_restrict" );

	// try to start up normally
	FS_Startup( BASEGAME );

	// see if we are going to allow add-ons
	FS_SetRestrictions();

#if defined RTCW_SP
	// if we can't find default.cfg, assume that the paths are
	// busted and error out now, rather than getting an unreadable
	// graphics screen when the font fails to load
	if ( FS_ReadFile( "default.cfg", NULL ) <= 0 ) {
		Com_Error( ERR_FATAL, "Couldn't load default.cfg" );
	}
#elif defined RTCW_MP
#ifndef UPDATE_SERVER
	// if we can't find default.cfg, assume that the paths are
	// busted and error out now, rather than getting an unreadable
	// graphics screen when the font fails to load
	if ( FS_ReadFile( "default.cfg", NULL ) <= 0 ) {
		// TTimo - added some verbosity, 'couldn't load default.cfg' confuses the hell out of users
		Com_Error( ERR_FATAL, "Couldn't load default.cfg - I am missing essential files - verify your installation?" );
	}
#endif
#else
	// if we can't find default.cfg, assume that the paths are
	// busted and error out now, rather than getting an unreadable
	// graphics screen when the font fails to load
	// Arnout: we want the nice error message here as well
	if ( FS_ReadFile( "default.cfg", NULL ) <= 0 ) {
		Com_Error( ERR_FATAL, "Couldn't load default.cfg - I am missing essential files - verify your installation?" );
	}
#endif // RTCW_XX

	Q_strncpyz( lastValidBase, fs_basepath->string, sizeof( lastValidBase ) );
	Q_strncpyz( lastValidGame, fs_gamedirvar->string, sizeof( lastValidGame ) );
}


/*
================
FS_Restart
================
*/

#if defined RTCW_ET
//void CL_PurgeCache( void );
#endif // RTCW_XX

void FS_Restart( int checksumFeed ) {

#if defined RTCW_ET
#ifndef DEDICATED
	// Arnout: big hack to clear the image cache on a FS_Restart
//	CL_PurgeCache();
#endif
#endif // RTCW_XX

	// free anything we currently have loaded
	FS_Shutdown( qfalse );

	// set the checksum feed
	fs_checksumFeed = checksumFeed;

	// clear pak references
	FS_ClearPakReferences( 0 );

	// try to start up normally
	FS_Startup( BASEGAME );

	// see if we are going to allow add-ons
	FS_SetRestrictions();

	// if we can't find default.cfg, assume that the paths are
	// busted and error out now, rather than getting an unreadable
	// graphics screen when the font fails to load
	if ( FS_ReadFile( "default.cfg", NULL ) <= 0 ) {
		// this might happen when connecting to a pure server not using BASEGAME/pak0.pk3
		// (for instance a TA demo server)
		if ( lastValidBase[0] ) {
			FS_PureServerSetLoadedPaks( "", "" );
			Cvar_Set( "fs_basepath", lastValidBase );
			Cvar_Set( "fs_gamedirvar", lastValidGame );
			lastValidBase[0] = '\0';
			lastValidGame[0] = '\0';
			Cvar_Set( "fs_restrict", "0" );
			FS_Restart( checksumFeed );
			Com_Error( ERR_DROP, "Invalid game folder\n" );
			return;
		}

#if !defined RTCW_ET
		Com_Error( ERR_FATAL, "Couldn't load default.cfg" );
#else
		// TTimo - added some verbosity, 'couldn't load default.cfg' confuses the hell out of users
		Com_Error( ERR_FATAL, "Couldn't load default.cfg - I am missing essential files - verify your installation?" );
#endif // RTCW_XX

	}

	// bk010116 - new check before safeMode
	if ( Q_stricmp( fs_gamedirvar->string, lastValidGame ) ) {
		// skip the wolfconfig.cfg if "safe" is on the command line
		if ( !Com_SafeMode() ) {

#if !defined RTCW_ET
#if defined RTCW_SP
			Cbuf_AddText( "exec wolfconfig.cfg\n" );
#elif defined RTCW_MP
			Cbuf_AddText( "exec wolfconfig_mp.cfg\n" );
#endif // RTCW_XX
#else
			const char *cl_profileStr = Cvar_VariableString( "cl_profile" );

			if ( com_gameInfo.usesProfiles && cl_profileStr[0] ) {
				// bani - check existing pid file and make sure it's ok
				if ( !Com_CheckProfile( va( "profiles/%s/profile.pid", cl_profileStr ) ) ) {
#ifndef _DEBUG
					Com_Printf( "^3WARNING: profile.pid found for profile '%s' - system settings will revert to defaults\n", cl_profileStr );
					// ydnar: set crashed state
					Cbuf_AddText( "set com_crashed 1\n" );
#endif
				}

				// bani - write a new one
				if ( !Com_WriteProfile( va( "profiles/%s/profile.pid", cl_profileStr ) ) ) {
					Com_Printf( "^3WARNING: couldn't write profiles/%s/profile.pid\n", cl_profileStr );
				}

				// exec the config
				Cbuf_AddText( va( "exec profiles/%s/%s\n", cl_profileStr, CONFIG_NAME ) );

			} else {
				Cbuf_AddText( va( "exec %s\n", CONFIG_NAME ) );
			}
#endif // RTCW_XX

		}
	}

	Q_strncpyz( lastValidBase, fs_basepath->string, sizeof( lastValidBase ) );
	Q_strncpyz( lastValidGame, fs_gamedirvar->string, sizeof( lastValidGame ) );

}

#if defined RTCW_SP
/*
=================
FS_ConditionalRestart
restart if necessary
=================
*/
#else
/*
=================
FS_ConditionalRestart
restart if necessary

 FIXME TTimo
this doesn't catch all cases where an FS_Restart is necessary
see show_bug.cgi?id=478
=================
*/
#endif // RTCW_XX

qboolean FS_ConditionalRestart( int checksumFeed ) {
	if ( fs_gamedirvar->modified || checksumFeed != fs_checksumFeed ) {
		FS_Restart( checksumFeed );
		return qtrue;
	}
	return qfalse;
}

/*
========================================================================================

Handle based file calls for virtual machines

========================================================================================
*/

int     FS_FOpenFileByMode( const char *qpath, fileHandle_t *f, fsMode_t mode ) {
	int r;
	qboolean sync;

	sync = qfalse;

	switch ( mode ) {
	case FS_READ:
		r = FS_FOpenFileRead( qpath, f, qtrue );
		break;
	case FS_WRITE:
		*f = FS_FOpenFileWrite( qpath );
		r = 0;
		if ( *f == 0 ) {
			r = -1;
		}
		break;
	case FS_APPEND_SYNC:
		sync = qtrue;
	case FS_APPEND:
		*f = FS_FOpenFileAppend( qpath );
		r = 0;
		if ( *f == 0 ) {
			r = -1;
		}
		break;
	default:
		Com_Error( ERR_FATAL, "FSH_FOpenFile: bad mode" );
		return -1;
	}

	if ( !f ) {
		return r;
	}

	if ( *f ) {
		fsh[*f].fileSize = r;
		fsh[*f].streamed = qfalse;

		// uncommenting this makes fs_reads
		// use the background threads --
		// MAY be faster for loading levels depending on the use of file io
		// q3a not faster
		// wolf not faster

//		if (mode == FS_READ) {
//			Sys_BeginStreamedFile( *f, 0x4000 );
//			fsh[*f].streamed = qtrue;
//		}
	}
	fsh[*f].handleSync = sync;

	return r;
}

int     FS_FTell( fileHandle_t f ) {
	int pos;
	if ( fsh[f].zipFile == qtrue ) {
		MinizZip::File* miniz_file_ptr = fsh[f].handleFiles.file.miniz_file_ptr_;

		if (!miniz_file_ptr)
		{
			return 0;
		}

		pos = miniz_file_ptr->get_position();
	} else {
		pos = ftell( fsh[f].handleFiles.file.o );
	}
	return pos;
}

void    FS_Flush( fileHandle_t f ) {
	fflush( fsh[f].handleFiles.file.o );
}

#if !defined RTCW_SP
// CVE-2006-2082
// compared requested pak against the names as we built them in FS_ReferencedPakNames
qboolean FS_VerifyPak( const char *pak ) {
	char teststring[ BIG_INFO_STRING ];
	searchpath_t    *search;

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		if ( search->pack ) {
			Q_strncpyz( teststring, search->pack->pakGamename, sizeof( teststring ) );
			Q_strcat( teststring, sizeof( teststring ), "/" );
			Q_strcat( teststring, sizeof( teststring ), search->pack->pakBasename );
			Q_strcat( teststring, sizeof( teststring ), ".pk3" );
			if ( !Q_stricmp( teststring, pak ) ) {
				return qtrue;
			}
		}
	}
	return qfalse;
}
#endif // RTCW_XX

#if defined RTCW_ET
qboolean FS_IsPure( void ) {
	return fs_numServerPaks != 0;
}

unsigned int FS_ChecksumOSPath( char *OSPath ) {
	FILE    *f;
	int len;
	byte    *buf;
	unsigned int checksum;

	f = fopen( OSPath, "rb" );
	if ( !f ) {
		return (unsigned int)-1;
	}
	fseek( f, 0, SEEK_END );
	len = ftell( f );
	fseek( f, 0, SEEK_SET );

	// BBi
	//buf = static_cast<byte*> (malloc( len ));
	buf = new byte[len];
	// BBi

	if ( fread( buf, 1, len, f ) != len ) {
		Com_Error( ERR_FATAL, "short read in FS_ChecksumOSPath\n" );
	}
	fclose( f );

	// Com_BlockChecksum returns an indian-dependent value
	// (better fix would have to be doing the LittleLong inside that function..)
	checksum = rtcw::Endian::le( Com_BlockChecksum( buf, len ) );

	// BBi
	//free( buf );
	delete [] buf;
	// BBi

	return checksum;
}
#endif // RTCW_XX


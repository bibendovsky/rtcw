#ifndef RTCW_ZLIB_INCLUDED
#define RTCW_ZLIB_INCLUDED


#ifndef MINIZ_HEADER_FILE_ONLY
#define MINIZ_HEADER_FILE_ONLY
#endif // MINIZ_HEADER_FILE_ONLY


#include "miniz.h"


#ifndef ZEXPORT
#define ZEXPORT
#endif

#ifndef OF
#define OF(args) args
#endif

typedef long z_off_t;


#endif // RTCW_ZLIB_INCLUDED

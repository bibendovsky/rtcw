/*
RTCW: Unofficial source port of Return to Castle Wolfenstein and Wolfenstein: Enemy Territory
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
Copyright (c) 2012-2025 Boris I. Bendovsky bibendovsky@hotmail.com and Contributors
SPDX-License-Identifier: GPL-3.0
*/

/*
 * name:		tr_image.c
 *
 * desc:
 *
*/

#include "tr_local.h"
#include "rtcw_endian.h"

/*
 * Include file for users of JPEG library.
 * You will need to have included system headers that define at least
 * the typedefs FILE and size_t before you can include jpeglib.h.
 * (stdio.h is sufficient on ANSI-conforming systems.)
 * You may also wish to include "jerror.h".
 */

// BBi
//#define JPEG_INTERNALS
//
//#include "jpeglib.h"

#include "rtcw_jpeg_reader.h"
#include "rtcw_jpeg_writer.h"


namespace {


rtcw::JpegReader g_jpeg_reader;
rtcw::JpegWriter g_jpeg_writer;


} // namespace
// BBi


static void LoadBMP( const char *name, byte **pic, int *width, int *height );
static void LoadTGA( const char *name, byte **pic, int *width, int *height );
static void LoadJPG( const char *name, byte **pic, int *width, int *height );

static byte s_intensitytable[256];
static unsigned char s_gammatable[256];

int gl_filter_min = GL_LINEAR_MIPMAP_NEAREST;
int gl_filter_max = GL_LINEAR;

// BBi
//#if defined RTCW_ET
// BBi

float gl_anisotropy = 1.0F;

// BBi
//#endif // RTCW_XX
// BBi

#define FILE_HASH_SIZE      4096
static image_t*        hashTable[FILE_HASH_SIZE];

// Ridah, in order to prevent zone fragmentation, all images will
// be read into this buffer. In order to keep things as fast as possible,
// we'll give it a starting value, which will account for the majority of
// images, but allow it to grow if the buffer isn't big enough
#define R_IMAGE_BUFFER_SIZE     ( 512 * 512 * 4 )     // 512 x 512 x 32bit

typedef enum {
	BUFFER_IMAGE,
	BUFFER_SCALED,
	BUFFER_RESAMPLED,
	BUFFER_MAX_TYPES
} bufferMemType_t;

int imageBufferSize[BUFFER_MAX_TYPES] = {0,0,0};
void        *imageBufferPtr[BUFFER_MAX_TYPES] = {NULL,NULL,NULL};

void *R_GetImageBuffer( int size, bufferMemType_t bufferType ) {
	if ( imageBufferSize[bufferType] < R_IMAGE_BUFFER_SIZE && size <= imageBufferSize[bufferType] ) {
		imageBufferSize[bufferType] = R_IMAGE_BUFFER_SIZE;
		imageBufferPtr[bufferType] = malloc( imageBufferSize[bufferType] );
#if !defined RTCW_SP
//DAJ TEST		imageBufferPtr[bufferType] = Z_Malloc( imageBufferSize[bufferType] );
#endif // RTCW_XX
	}
	if ( size > imageBufferSize[bufferType] ) {   // it needs to grow
		if ( imageBufferPtr[bufferType] ) {
			free( imageBufferPtr[bufferType] );
		}

#if !defined RTCW_SP
//DAJ TEST		Z_Free( imageBufferPtr[bufferType] );
#endif // RTCW_XX
		imageBufferSize[bufferType] = size;
		imageBufferPtr[bufferType] = malloc( imageBufferSize[bufferType] );
#if !defined RTCW_SP
//DAJ TEST		imageBufferPtr[bufferType] = Z_Malloc( imageBufferSize[bufferType] );
#endif // RTCW_XX
	}

	return imageBufferPtr[bufferType];
}

void R_FreeImageBuffer( void ) {
	int bufferType;
	for ( bufferType = 0; bufferType < BUFFER_MAX_TYPES; bufferType++ ) {
		if ( !imageBufferPtr[bufferType] ) {
			return;
		}

		free( imageBufferPtr[bufferType] );

#if !defined RTCW_SP
//DAJ TEST		Z_Free( imageBufferPtr[bufferType] );
#endif // RTCW_XX

		imageBufferSize[bufferType] = 0;
		imageBufferPtr[bufferType] = NULL;
	}
}

/*
** R_GammaCorrect
*/
void R_GammaCorrect( byte *buffer, int bufSize ) {
#ifndef RTCW_VANILLA
	if (!glConfigEx.is_path_ogl_1_x())
	{
		return;
	}
#endif // RTCW_VANILLA

	int i;

	for ( i = 0; i < bufSize; i++ ) {
		buffer[i] = s_gammatable[buffer[i]];
	}
}

typedef struct {
	const char *name;
	int minimize, maximize;
} textureMode_t;

textureMode_t modes[] = {
	{"GL_NEAREST", GL_NEAREST, GL_NEAREST},
	{"GL_LINEAR", GL_LINEAR, GL_LINEAR},
	{"GL_NEAREST_MIPMAP_NEAREST", GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST},
	{"GL_LINEAR_MIPMAP_NEAREST", GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR},
	{"GL_NEAREST_MIPMAP_LINEAR", GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST},
	{"GL_LINEAR_MIPMAP_LINEAR", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR}
};

/*
================
return a hash value for the filename
================
*/
static int32_t generateHashValue( const char *fname ) {
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
		hash += (int32_t)( letter ) * ( i + 119 );
		i++;
	}
	hash &= ( FILE_HASH_SIZE - 1 );
	return hash;
}

/*
===============
GL_TextureMode
===============
*/
void GL_TextureMode( const char *string ) {
	int i;
	image_t *glt;

	for ( i = 0 ; i < 6 ; i++ ) {
		if ( !Q_stricmp( modes[i].name, string ) ) {
			break;
		}
	}

	// BBi
	//// hack to prevent trilinear from being set on voodoo,
	//// because their driver freaks...
	//if ( i == 5 && glConfig.hardwareType == GLHW_3DFX_2D3D ) {
	//	ri.Printf( PRINT_ALL, "Refusing to set trilinear on a voodoo.\n" );
	//	i = 3;
	//}
	// BBi


	if ( i == 6 ) {
		ri.Printf( PRINT_ALL, "bad filter name\n" );
		return;
	}

	gl_filter_min = modes[i].minimize;
	gl_filter_max = modes[i].maximize;

	// change all the existing mipmap texture objects
	for ( i = 0 ; i < tr.numImages ; i++ ) {
		glt = tr.images[ i ];

#if defined RTCW_ET
		GL_Bind( glt );
		// ydnar: for allowing lightmap debugging
#endif // RTCW_XX

		if ( glt->mipmap ) {

// BBi
//#if !defined RTCW_ET
//			GL_Bind( glt );
//			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min );
//			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max );
//#else
//			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min );
//			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max );
//		} else
//		{
//			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
//			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max );
//#endif // RTCW_XX

#if !defined RTCW_ET
			GL_Bind (glt);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
#else
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
		} else
		{
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
#endif // RTCW_XX
// BBi

		}
	}
}

// BBi
//#if defined RTCW_ET
// BBi

/*
===============
GL_TextureAnisotropy
===============
*/
void GL_TextureAnisotropy(float anisotropy)
{
	if (r_ext_texture_filter_anisotropic->integer == 1 &&
		glConfig.anisotropicAvailable)
	{
		if (anisotropy < 1.0F)
		{
			gl_anisotropy = glConfig.maxAnisotropy;
		}
		else
		{
			gl_anisotropy = anisotropy;

			if (gl_anisotropy < 1)
			{
				gl_anisotropy = 1;
			}
			else if (gl_anisotropy > glConfig.maxAnisotropy)
			{
				gl_anisotropy = glConfig.maxAnisotropy;
			}
		}
	}
	else
	{
		gl_anisotropy = 1.0F;
	}

	if (!glConfig.anisotropicAvailable)
	{
		return;
	}

	// change all the existing texture objects
	for (int i = 0; i < tr.numImages; ++i)
	{
		image_t* glt = tr.images[i];
		GL_Bind(glt);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, gl_anisotropy);
	}
}

// BBi
//#endif // RTCW_XX
// BBi

/*
===============
R_SumOfUsedImages
===============
*/
int R_SumOfUsedImages( void ) {
	int total;

#if defined RTCW_SP
	int i;
#else
	int i, fc = ( tr.frameCount - 1 );
#endif // RTCW_XX

	total = 0;
	for ( i = 0; i < tr.numImages; i++ ) {

#if defined RTCW_SP
		if ( tr.images[i]->frameUsed == tr.frameCount ) {
#else
		if ( tr.images[i]->frameUsed == fc ) {
#endif // RTCW_XX

			total += tr.images[i]->uploadWidth * tr.images[i]->uploadHeight;
		}
	}

	return total;
}

/*
===============
R_ImageList_f
===============
*/
void R_ImageList_f( void ) {
	int i;
	image_t *image;
	int texels;
	const char *yesno[] = {
		"no ", "yes"
	};

#if !defined RTCW_ET
	ri.Printf( PRINT_ALL, "\n      -w-- -h-- -mm- -TMU- -if-- wrap --name-------\n" );
#else
	ri.Printf( PRINT_ALL, "\n      -w-- -h-- -mm- -TMU- GLname -if-- wrap --name-------\n" );
#endif // RTCW_XX

	texels = 0;

	for ( i = 0 ; i < tr.numImages ; i++ ) {
		image = tr.images[ i ];

		texels += image->uploadWidth * image->uploadHeight;

#if !defined RTCW_ET
		ri.Printf( PRINT_ALL,  "%4i: %4i %4i  %s   %d   ",
				   i, image->uploadWidth, image->uploadHeight, yesno[image->mipmap], image->TMU );
#else
		ri.Printf( PRINT_ALL,  "%4i: %4i %4i  %s   %d   %5d ",
				   i, image->uploadWidth, image->uploadHeight, yesno[image->mipmap], image->TMU, image->texnum );
#endif // RTCW_XX

		switch ( image->internalFormat ) {
		case 1:
			ri.Printf( PRINT_ALL, "I    " );
			break;
		case 2:
			ri.Printf( PRINT_ALL, "IA   " );
			break;
		case 3:
			ri.Printf( PRINT_ALL, "RGB  " );
			break;
		case 4:
			ri.Printf( PRINT_ALL, "RGBA " );
			break;
		case GL_RGBA8:
			ri.Printf( PRINT_ALL, "RGBA8" );
			break;
		case GL_RGB8:

#if !defined RTCW_ET
			ri.Printf( PRINT_ALL, "RGB8" );
#else
			ri.Printf( PRINT_ALL, "RGB8 " );
#endif // RTCW_XX

			break;

#if defined RTCW_ET
		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
			ri.Printf( PRINT_ALL, "DXT3 " );
			break;
#endif // RTCW_XX

		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
			ri.Printf( PRINT_ALL, "DXT5 " );
			break;
		case GL_RGB4_S3TC:
			ri.Printf( PRINT_ALL, "S3TC4" );
			break;
		case GL_RGBA4:
			ri.Printf( PRINT_ALL, "RGBA4" );
			break;
		case GL_RGB5:
			ri.Printf( PRINT_ALL, "RGB5 " );
			break;
		default:
			ri.Printf( PRINT_ALL, "???? " );
		}

		switch ( image->wrapClampMode ) {
		case GL_REPEAT:
			ri.Printf( PRINT_ALL, "rept " );
			break;
		case GL_CLAMP:
			ri.Printf( PRINT_ALL, "clmp " );
			break;

		// BBi
		case GL_CLAMP_TO_EDGE:
			ri.Printf( PRINT_ALL, "clmpe" );
			break;
		// BBi

		default:
			ri.Printf( PRINT_ALL, "%4i ", image->wrapClampMode );
			break;
		}

		ri.Printf( PRINT_ALL, " %s\n", image->imgName );
	}
	ri.Printf( PRINT_ALL, " ---------\n" );
	ri.Printf( PRINT_ALL, " %i total texels (not including mipmaps)\n", texels );
	ri.Printf( PRINT_ALL, " %i total images\n\n", tr.numImages );
}

//=======================================================================

/*
================
ResampleTexture

Used to resample images in a more general than quartering fashion.

This will only be filtered properly if the resampled size
is greater than half the original size.

If a larger shrinking is needed, use the mipmap function
before or after.
================
*/
static void ResampleTexture( unsigned *in, int inwidth, int inheight, unsigned *out,
							 int outwidth, int outheight ) {
	int i, j;
	unsigned    *inrow, *inrow2;
	unsigned frac, fracstep;

#if defined RTCW_SP
	unsigned p1[1024], p2[1024];
#else
	unsigned p1[2048], p2[2048];
#endif // RTCW_XX

	byte        *pix1, *pix2, *pix3, *pix4;

#if !defined RTCW_SP
	if ( outwidth > 2048 ) {
		ri.Error( ERR_DROP, "ResampleTexture: max width" );
	}
#endif // RTCW_XX

	fracstep = inwidth * 0x10000 / outwidth;

	frac = fracstep >> 2;
	for ( i = 0 ; i < outwidth ; i++ ) {
		p1[i] = 4 * ( frac >> 16 );
		frac += fracstep;
	}
	frac = 3 * ( fracstep >> 2 );
	for ( i = 0 ; i < outwidth ; i++ ) {
		p2[i] = 4 * ( frac >> 16 );
		frac += fracstep;
	}

	for ( i = 0 ; i < outheight ; i++, out += outwidth ) {
		inrow = in + inwidth * (int)( ( i + 0.25 ) * inheight / outheight );
		inrow2 = in + inwidth * (int)( ( i + 0.75 ) * inheight / outheight );
		frac = fracstep >> 1;
		for ( j = 0 ; j < outwidth ; j++ ) {
			pix1 = (byte *)inrow + p1[j];
			pix2 = (byte *)inrow + p2[j];
			pix3 = (byte *)inrow2 + p1[j];
			pix4 = (byte *)inrow2 + p2[j];
			( ( byte * )( out + j ) )[0] = ( pix1[0] + pix2[0] + pix3[0] + pix4[0] ) >> 2;
			( ( byte * )( out + j ) )[1] = ( pix1[1] + pix2[1] + pix3[1] + pix4[1] ) >> 2;
			( ( byte * )( out + j ) )[2] = ( pix1[2] + pix2[2] + pix3[2] + pix4[2] ) >> 2;
			( ( byte * )( out + j ) )[3] = ( pix1[3] + pix2[3] + pix3[3] + pix4[3] ) >> 2;
		}
	}
}

/*
================
R_LightScaleTexture

Scale up the pixel values in a texture to increase the
lighting range
================
*/
void R_LightScaleTexture( unsigned *in, int inwidth, int inheight, qboolean only_gamma ) {
#ifndef RTCW_VANILLA
	if (!glConfigEx.is_path_ogl_1_x())
	{
		return;
	}
#endif // RTCW_VANILLA

	if ( only_gamma ) {
		if ( !glConfig.deviceSupportsGamma ) {
			int i, c;
			byte    *p;

			p = (byte *)in;

			c = inwidth * inheight;
			for ( i = 0 ; i < c ; i++, p += 4 )
			{
				p[0] = s_gammatable[p[0]];
				p[1] = s_gammatable[p[1]];
				p[2] = s_gammatable[p[2]];
			}
		}
	} else
	{
		int i, c;
		byte    *p;

		p = (byte *)in;

		c = inwidth * inheight;

		if ( glConfig.deviceSupportsGamma ) {
			for ( i = 0 ; i < c ; i++, p += 4 )
			{
				p[0] = s_intensitytable[p[0]];
				p[1] = s_intensitytable[p[1]];
				p[2] = s_intensitytable[p[2]];
			}
		} else
		{
			for ( i = 0 ; i < c ; i++, p += 4 )
			{
				p[0] = s_gammatable[s_intensitytable[p[0]]];
				p[1] = s_gammatable[s_intensitytable[p[1]]];
				p[2] = s_gammatable[s_intensitytable[p[2]]];
			}
		}
	}
}


/*
================
R_MipMap2

Operates in place, quartering the size of the texture
Proper linear filter
================
*/
static void R_MipMap2( unsigned *in, int inWidth, int inHeight ) {
	int i, j, k;
	byte        *outpix;
	int inWidthMask, inHeightMask;
	int total;
	int outWidth, outHeight;
	unsigned    *temp;

	outWidth = inWidth >> 1;
	outHeight = inHeight >> 1;
	temp = static_cast<unsigned*> (ri.Hunk_AllocateTempMemory( outWidth * outHeight * 4 ));

	inWidthMask = inWidth - 1;
	inHeightMask = inHeight - 1;

	for ( i = 0 ; i < outHeight ; i++ ) {
		for ( j = 0 ; j < outWidth ; j++ ) {
			outpix = ( byte * )( temp + i * outWidth + j );
			for ( k = 0 ; k < 4 ; k++ ) {
				total =
					1 * ( (byte *)&in[ ( ( i * 2 - 1 ) & inHeightMask ) * inWidth + ( ( j * 2 - 1 ) & inWidthMask ) ] )[k] +
					2 * ( (byte *)&in[ ( ( i * 2 - 1 ) & inHeightMask ) * inWidth + ( ( j * 2 ) & inWidthMask ) ] )[k] +
					2 * ( (byte *)&in[ ( ( i * 2 - 1 ) & inHeightMask ) * inWidth + ( ( j * 2 + 1 ) & inWidthMask ) ] )[k] +
					1 * ( (byte *)&in[ ( ( i * 2 - 1 ) & inHeightMask ) * inWidth + ( ( j * 2 + 2 ) & inWidthMask ) ] )[k] +

					2 * ( (byte *)&in[ ( ( i * 2 ) & inHeightMask ) * inWidth + ( ( j * 2 - 1 ) & inWidthMask ) ] )[k] +
					4 * ( (byte *)&in[ ( ( i * 2 ) & inHeightMask ) * inWidth + ( ( j * 2 ) & inWidthMask ) ] )[k] +
					4 * ( (byte *)&in[ ( ( i * 2 ) & inHeightMask ) * inWidth + ( ( j * 2 + 1 ) & inWidthMask ) ] )[k] +
					2 * ( (byte *)&in[ ( ( i * 2 ) & inHeightMask ) * inWidth + ( ( j * 2 + 2 ) & inWidthMask ) ] )[k] +

					2 * ( (byte *)&in[ ( ( i * 2 + 1 ) & inHeightMask ) * inWidth + ( ( j * 2 - 1 ) & inWidthMask ) ] )[k] +
					4 * ( (byte *)&in[ ( ( i * 2 + 1 ) & inHeightMask ) * inWidth + ( ( j * 2 ) & inWidthMask ) ] )[k] +
					4 * ( (byte *)&in[ ( ( i * 2 + 1 ) & inHeightMask ) * inWidth + ( ( j * 2 + 1 ) & inWidthMask ) ] )[k] +
					2 * ( (byte *)&in[ ( ( i * 2 + 1 ) & inHeightMask ) * inWidth + ( ( j * 2 + 2 ) & inWidthMask ) ] )[k] +

					1 * ( (byte *)&in[ ( ( i * 2 + 2 ) & inHeightMask ) * inWidth + ( ( j * 2 - 1 ) & inWidthMask ) ] )[k] +
					2 * ( (byte *)&in[ ( ( i * 2 + 2 ) & inHeightMask ) * inWidth + ( ( j * 2 ) & inWidthMask ) ] )[k] +
					2 * ( (byte *)&in[ ( ( i * 2 + 2 ) & inHeightMask ) * inWidth + ( ( j * 2 + 1 ) & inWidthMask ) ] )[k] +
					1 * ( (byte *)&in[ ( ( i * 2 + 2 ) & inHeightMask ) * inWidth + ( ( j * 2 + 2 ) & inWidthMask ) ] )[k];
				outpix[k] = total / 36;
			}
		}
	}

	memcpy( in, temp, outWidth * outHeight * 4 );
	ri.Hunk_FreeTempMemory( temp );
}

/*
================
R_MipMap

Operates in place, quartering the size of the texture
================
*/
static void R_MipMap( byte *in, int width, int height ) {
	int i, j;
	byte    *out;
	int row;

	if ( !r_simpleMipMaps->integer ) {
		R_MipMap2( (unsigned *)in, width, height );
		return;
	}

	if ( width == 1 && height == 1 ) {
		return;
	}

	row = width * 4;
	out = in;
	width >>= 1;
	height >>= 1;

	if ( width == 0 || height == 0 ) {
		width += height;    // get largest
		for ( i = 0 ; i < width ; i++, out += 4, in += 8 ) {
			out[0] = ( in[0] + in[4] ) >> 1;
			out[1] = ( in[1] + in[5] ) >> 1;
			out[2] = ( in[2] + in[6] ) >> 1;
			out[3] = ( in[3] + in[7] ) >> 1;
		}
		return;
	}

	for ( i = 0 ; i < height ; i++, in += row ) {
		for ( j = 0 ; j < width ; j++, out += 4, in += 8 ) {
			out[0] = ( in[0] + in[4] + in[row + 0] + in[row + 4] ) >> 2;
			out[1] = ( in[1] + in[5] + in[row + 1] + in[row + 5] ) >> 2;
			out[2] = ( in[2] + in[6] + in[row + 2] + in[row + 6] ) >> 2;
			out[3] = ( in[3] + in[7] + in[row + 3] + in[row + 7] ) >> 2;
		}
	}
}

/*
================
R_MipMap

Operates in place, quartering the size of the texture
================
*/

#if !defined RTCW_ET
static float R_RMSE( byte *in, int width, int height ) {
	int i, j;
	float out, rmse, rtemp;
	int row;

	rmse = 0.0f;

	if ( width <= 32 || height <= 32 ) {
		return 9999.0f;
	}

	row = width * 4;

	width >>= 1;
	height >>= 1;

	for ( i = 0 ; i < height ; i++, in += row ) {
		for ( j = 0 ; j < width ; j++, out += 4, in += 8 ) {
			out = ( in[0] + in[4] + in[row + 0] + in[row + 4] ) >> 2;
			rtemp = ( ( c::fabs( out - in[0] ) + c::fabs( out - in[4] ) + c::fabs( out - in[row + 0] ) + c::fabs( out - in[row + 4] ) ) );
			rtemp = rtemp * rtemp;
			rmse += rtemp;
			out = ( in[1] + in[5] + in[row + 1] + in[row + 5] ) >> 2;
			rtemp = ( ( c::fabs( out - in[1] ) + c::fabs( out - in[5] ) + c::fabs( out - in[row + 1] ) + c::fabs( out - in[row + 5] ) ) );
			rtemp = rtemp * rtemp;
			rmse += rtemp;
			out = ( in[2] + in[6] + in[row + 2] + in[row + 6] ) >> 2;
			rtemp = ( ( c::fabs( out - in[2] ) + c::fabs( out - in[6] ) + c::fabs( out - in[row + 2] ) + c::fabs( out - in[row + 6] ) ) );
			rtemp = rtemp * rtemp;
			rmse += rtemp;
			out = ( in[3] + in[7] + in[row + 3] + in[row + 7] ) >> 2;
			rtemp = ( ( c::fabs( out - in[3] ) + c::fabs( out - in[7] ) + c::fabs( out - in[row + 3] ) + c::fabs( out - in[row + 7] ) ) );
			rtemp = rtemp * rtemp;
			rmse += rtemp;
		}
	}
	rmse = c::sqrt( rmse / ( height * width * 4 ) );
	return rmse;
}
#else
#if 0 // rain - unused
static float R_RMSE( byte *in, int width, int height ) {
	int i, j;
	float out, rmse, rtemp;
	int row;

	rmse = 0.0f;

	if ( width <= 32 || height <= 32 ) {
		return 9999.0f;
	}

	row = width * 4;

	width >>= 1;
	height >>= 1;

	for ( i = 0 ; i < height ; i++, in += row ) {
		for ( j = 0 ; j < width ; j++, out += 4, in += 8 ) {
			out = ( in[0] + in[4] + in[row + 0] + in[row + 4] ) >> 2;
			rtemp = ( ( Q_fabs( out - in[0] ) + Q_fabs( out - in[4] ) + Q_fabs( out - in[row + 0] ) + Q_fabs( out - in[row + 4] ) ) );
			rtemp = rtemp * rtemp;
			rmse += rtemp;
			out = ( in[1] + in[5] + in[row + 1] + in[row + 5] ) >> 2;
			rtemp = ( ( Q_fabs( out - in[1] ) + Q_fabs( out - in[5] ) + Q_fabs( out - in[row + 1] ) + Q_fabs( out - in[row + 5] ) ) );
			rtemp = rtemp * rtemp;
			rmse += rtemp;
			out = ( in[2] + in[6] + in[row + 2] + in[row + 6] ) >> 2;
			rtemp = ( ( Q_fabs( out - in[2] ) + Q_fabs( out - in[6] ) + Q_fabs( out - in[row + 2] ) + Q_fabs( out - in[row + 6] ) ) );
			rtemp = rtemp * rtemp;
			rmse += rtemp;
			out = ( in[3] + in[7] + in[row + 3] + in[row + 7] ) >> 2;
			rtemp = ( ( Q_fabs( out - in[3] ) + Q_fabs( out - in[7] ) + Q_fabs( out - in[row + 3] ) + Q_fabs( out - in[row + 7] ) ) );
			rtemp = rtemp * rtemp;
			rmse += rtemp;
		}
	}
	rmse = c::sqrt( rmse / ( height * width * 4 ) );
	return rmse;
}
#endif
#endif // RTCW_XX

/*
==================
R_BlendOverTexture

Apply a color blend over a set of pixels
==================
*/
static void R_BlendOverTexture( byte *data, int pixelCount, byte blend[4] ) {
	int i;
	int inverseAlpha;
	int premult[3];

	inverseAlpha = 255 - blend[3];
	premult[0] = blend[0] * blend[3];
	premult[1] = blend[1] * blend[3];
	premult[2] = blend[2] * blend[3];

	for ( i = 0 ; i < pixelCount ; i++, data += 4 ) {
		data[0] = ( data[0] * inverseAlpha + premult[0] ) >> 9;
		data[1] = ( data[1] * inverseAlpha + premult[1] ) >> 9;
		data[2] = ( data[2] * inverseAlpha + premult[2] ) >> 9;
	}
}

byte mipBlendColors[16][4] = {
	{0,0,0,0},
	{255,0,0,128},
	{0,255,0,128},
	{0,0,255,128},
	{255,0,0,128},
	{0,255,0,128},
	{0,0,255,128},
	{255,0,0,128},
	{0,255,0,128},
	{0,0,255,128},
	{255,0,0,128},
	{0,255,0,128},
	{0,0,255,128},
	{255,0,0,128},
	{0,255,0,128},
	{0,0,255,128},
};


/*
===============
Upload32

===============
*/
static void Upload32(   unsigned *data,
						int width, int height,
						qboolean mipmap,
						qboolean picmip,

#if defined RTCW_SP
						qboolean characterMip,  //----(SA)	added
#endif // RTCW_XX

						qboolean lightMap,
						int *format,
						int *pUploadWidth, int *pUploadHeight,
						qboolean noCompress ) {
	int samples;
	int scaled_width, scaled_height;
	unsigned    *scaledBuffer = NULL;
	unsigned    *resampledBuffer = NULL;
	int i, c;
	byte        *scan;
	GLenum internalFormat = GL_RGB;

// BBi
//#if !defined RTCW_ET
//	float rMax = 0, gMax = 0, bMax = 0;
//	static int rmse_saved = 0;
//#else
//// rain - unused
////	static		int rmse_saved = 0;
//#endif // RTCW_XX
//
//#if defined RTCW_SP
//	float rmse;
//#endif // RTCW_XX
//
//	// do the root mean square error stuff first
//
//#if !defined RTCW_ET
//	if ( r_rmse->value ) {
//		while ( R_RMSE( (byte *)data, width, height ) < r_rmse->value ) {
//			rmse_saved += ( height * width * 4 ) - ( ( width >> 1 ) * ( height >> 1 ) * 4 );
//			resampledBuffer = static_cast<unsigned*> (R_GetImageBuffer( ( width >> 1 ) * ( height >> 1 ) * 4, BUFFER_RESAMPLED ));
//			ResampleTexture( data, width, height, resampledBuffer, width >> 1, height >> 1 );
//			data = resampledBuffer;
//			width = width >> 1;
//			height = height >> 1;
//			ri.Printf( PRINT_ALL, "r_rmse of %f has saved %dkb\n", r_rmse->value, ( rmse_saved / 1024 ) );
//		}
//#else
///*	if (r_rmse->value) {
//		while (R_RMSE((byte *)data, width, height) < r_rmse->value) {
//			rmse_saved += (height*width*4)-((width>>1)*(height>>1)*4);
//			resampledBuffer = R_GetImageBuffer( (width>>1) * (height>>1) * 4, BUFFER_RESAMPLED );
//			ResampleTexture (data, width, height, resampledBuffer, width>>1, height>>1);
//			data = resampledBuffer;
//			width = width>>1;
//			height = height>>1;
//			ri.Printf (PRINT_ALL, "r_rmse of %f has saved %dkb\n", r_rmse->value, (rmse_saved/1024));
//		}
//	}*/
//#endif // RTCW_XX
//
//#if defined RTCW_SP
//	} else {
//		// just do the RMSE of 1 (reduce perfect)
//		while ( R_RMSE( (byte *)data, width, height ) < 1.0 ) {
//			rmse_saved += ( height * width * 4 ) - ( ( width >> 1 ) * ( height >> 1 ) * 4 );
//			resampledBuffer = static_cast<unsigned*> (R_GetImageBuffer( ( width >> 1 ) * ( height >> 1 ) * 4, BUFFER_RESAMPLED ));
//			ResampleTexture( data, width, height, resampledBuffer, width >> 1, height >> 1 );
//			data = resampledBuffer;
//			width = width >> 1;
//			height = height >> 1;
//			ri.Printf( PRINT_ALL, "r_rmse of %f has saved %dkb\n", r_rmse->value, ( rmse_saved / 1024 ) );
//		}
//#endif // RTCW_XX
//
//#if !defined RTCW_ET
//	}
//#endif // RTCW_XX
// BBi

	// BBi
	bool canUseNpotTexture = (glConfigEx.use_arb_texture_non_power_of_two_ && (!picmip));

	if (canUseNpotTexture) {
		scaled_width = width;
		scaled_height = height;
	} else {
	// BBi

	//
	// convert to exact power of 2 sizes
	//
	for ( scaled_width = 1 ; scaled_width < width ; scaled_width <<= 1 )
		;
	for ( scaled_height = 1 ; scaled_height < height ; scaled_height <<= 1 )
		;
	if ( r_roundImagesDown->integer && scaled_width > width ) {
		scaled_width >>= 1;
	}
	if ( r_roundImagesDown->integer && scaled_height > height ) {
		scaled_height >>= 1;
	}

	if ( scaled_width != width || scaled_height != height ) {
		//resampledBuffer = ri.Hunk_AllocateTempMemory( scaled_width * scaled_height * 4 );
		resampledBuffer = static_cast<unsigned*> (R_GetImageBuffer( scaled_width * scaled_height * 4, BUFFER_RESAMPLED ));
		ResampleTexture( data, width, height, resampledBuffer, scaled_width, scaled_height );
		data = resampledBuffer;
		width = scaled_width;
		height = scaled_height;
	}

	//
	// perform optional picmip operation
	//
	if ( picmip ) {

#if defined RTCW_SP
		if ( characterMip ) {
			scaled_width >>= r_picmip2->integer;
			scaled_height >>= r_picmip2->integer;
		} else {
			scaled_width >>= r_picmip->integer;
			scaled_height >>= r_picmip->integer;
		}
#else
		scaled_width >>= r_picmip->integer;
		scaled_height >>= r_picmip->integer;
	}

	//
	// clamp to minimum size
	//
	if ( scaled_width < 1 ) {
		scaled_width = 1;
	}
	if ( scaled_height < 1 ) {
		scaled_height = 1;
#endif // RTCW_XX

	}

// BBi
	}
// BBi

	//
	// clamp to the current upper OpenGL limit
	// scale both axis down equally so we don't have to
	// deal with a half mip resampling
	//
	while ( scaled_width > glConfig.maxTextureSize
			|| scaled_height > glConfig.maxTextureSize ) {
		scaled_width >>= 1;
		scaled_height >>= 1;
	}

// BBi
//#if defined RTCW_SP
//	rmse = R_RMSE( (byte *)data, width, height );
//
//	if ( r_lowMemTextureSize->integer && ( scaled_width > r_lowMemTextureSize->integer || scaled_height > r_lowMemTextureSize->integer ) && rmse < r_lowMemTextureThreshold->value ) {
//		int scale;
//
//		for ( scale = 1 ; scale < r_lowMemTextureSize->integer; scale <<= 1 ) {
//			;
//		}
//
//		while ( scaled_width > scale || scaled_height > scale ) {
//			scaled_width >>= 1;
//			scaled_height >>= 1;
//		}
//
//		ri.Printf( PRINT_ALL, "r_lowMemTextureSize forcing reduction from %i x %i to %i x %i\n", width, height, scaled_width, scaled_height );
//
//		resampledBuffer = static_cast<unsigned*> (R_GetImageBuffer( scaled_width * scaled_height * 4, BUFFER_RESAMPLED ));
//		ResampleTexture( data, width, height, resampledBuffer, scaled_width, scaled_height );
//		data = resampledBuffer;
//		width = scaled_width;
//		height = scaled_height;
//
//	}
//
//
//	//
//	// clamp to minimum size
//	//
//	if ( scaled_width < 1 ) {
//		scaled_width = 1;
//	}
//	if ( scaled_height < 1 ) {
//		scaled_height = 1;
//	}
//#endif // RTCW_XX
// BBi

	//scaledBuffer = ri.Hunk_AllocateTempMemory( sizeof( unsigned ) * scaled_width * scaled_height );
	scaledBuffer = static_cast<unsigned*> (R_GetImageBuffer( sizeof( unsigned ) * scaled_width * scaled_height, BUFFER_SCALED ));

	//
	// scan the texture for each channel's max values
	// and verify if the alpha channel is being used or not
	//
	c = width * height;
	scan = ( (byte *)data );
	samples = 3;
	if ( !lightMap ) {
		for ( i = 0; i < c; i++ )
		{

// BBi
//#if !defined RTCW_ET
//			if ( scan[i * 4 + 0] > rMax ) {
//				rMax = scan[i * 4 + 0];
//			}
//			if ( scan[i * 4 + 1] > gMax ) {
//				gMax = scan[i * 4 + 1];
//			}
//			if ( scan[i * 4 + 2] > bMax ) {
//				bMax = scan[i * 4 + 2];
//			}
//#endif // RTCW_XX
// BBi

			if ( scan[i * 4 + 3] != 255 ) {
				samples = 4;
				break;
			}
		}
		// select proper internal format
		if ( samples == 3 ) {
// BBi
			if ((!noCompress) && (glConfig.textureCompression == TC_ARB))
				internalFormat = GL_COMPRESSED_RGB;
			else
// BBi
			if ( !noCompress && glConfig.textureCompression == TC_EXT_COMP_S3TC ) {
				// TODO: which format is best for which textures?

#if defined RTCW_ET
				//internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
#endif // RTCW_XX

				internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			} else if ( !noCompress && glConfig.textureCompression == TC_S3TC )   {
				// BBi
				//internalFormat = GL_RGB4_S3TC;
				internalFormat = GL_RGB_S3TC;
				// BBi
			} else if ( r_texturebits->integer == 16 )   {
				internalFormat = GL_RGB5;
			} else if ( r_texturebits->integer == 32 )   {
				internalFormat = GL_RGB8;
			} else
			{
				// BBi Numeric value not valid anymore
				//internalFormat = 3;
				internalFormat = GL_RGB;
				// BBi
			}
		} else if ( samples == 4 )   {

// BBi
			if ((!noCompress) && (glConfig.textureCompression == TC_ARB))
				internalFormat = GL_COMPRESSED_RGBA;
			else
// BBi

			if ( !noCompress && glConfig.textureCompression == TC_EXT_COMP_S3TC ) {
				// TODO: which format is best for which textures?

#if defined RTCW_ET
				//internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
#endif // RTCW_XX

				internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;

			// BBi
			} else if ((!noCompress) && (glConfig.textureCompression == TC_S3TC)) {
				internalFormat = GL_RGBA_DXT5_S3TC;
			// BBi

			} else if ( r_texturebits->integer == 16 )   {
				internalFormat = GL_RGBA4;
			} else if ( r_texturebits->integer == 32 )   {
				internalFormat = GL_RGBA8;
			} else
			{
				// BBi Numeric value not valid anymore
				//internalFormat = 4;
				internalFormat = GL_RGBA;
				// BBi
			}
		}
	} else {
		// BBi Numeric value not valid anymore
		//internalFormat = 3;
		internalFormat = GL_RGB;
		// BBi
	}
	// copy or resample data as appropriate for first MIP level
	if ( ( scaled_width == width ) &&
		 ( scaled_height == height ) ) {
		if ( !mipmap ) {
			glTexImage2D( GL_TEXTURE_2D, 0, internalFormat, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
			*pUploadWidth = scaled_width;
			*pUploadHeight = scaled_height;
			*format = internalFormat;

			goto done;
		}
		memcpy( scaledBuffer, data, width * height * 4 );
	} else
	{
		// use the normal mip-mapping function to go down from here
		while ( width > scaled_width || height > scaled_height ) {
			R_MipMap( (byte *)data, width, height );
			width >>= 1;
			height >>= 1;
			if ( width < 1 ) {
				width = 1;
			}
			if ( height < 1 ) {
				height = 1;
			}
		}
		memcpy( scaledBuffer, data, width * height * 4 );
	}

	R_LightScaleTexture( scaledBuffer, scaled_width, scaled_height, !mipmap );

	*pUploadWidth = scaled_width;
	*pUploadHeight = scaled_height;
	*format = internalFormat;

	glTexImage2D( GL_TEXTURE_2D, 0, internalFormat, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaledBuffer );

	// BBi
	//if ( mipmap ) {
	if ((mipmap) && (!glConfigEx.use_arb_framebuffer_object_)) {
	// BBi
		int miplevel;

		miplevel = 0;
		while ( scaled_width > 1 || scaled_height > 1 )
		{
			R_MipMap( (byte *)scaledBuffer, scaled_width, scaled_height );
			scaled_width >>= 1;
			scaled_height >>= 1;
			if ( scaled_width < 1 ) {
				scaled_width = 1;
			}
			if ( scaled_height < 1 ) {
				scaled_height = 1;
			}
			miplevel++;

			if ( r_colorMipLevels->integer ) {
				R_BlendOverTexture( (byte *)scaledBuffer, scaled_width * scaled_height, mipBlendColors[miplevel] );
			}

			glTexImage2D( GL_TEXTURE_2D, miplevel, internalFormat, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaledBuffer );
		}
	}
done:

	if ( mipmap ) {
		// BBi
		if (glConfigEx.use_arb_framebuffer_object_)
			glGenerateMipmap (GL_TEXTURE_2D);
		// BBi

		// BBi
		//::glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min );
		//::glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max );
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
		// BBi
	} else
	{

#if defined RTCW_ET
		// ydnar: for allowing lightmap debugging
#endif // RTCW_XX

// BBi
//		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
//
//#if !defined RTCW_ET
//		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
//#else
//		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max );
//#endif // RTCW_XX

		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

#if !defined RTCW_ET
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#else
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
#endif // RTCW_XX
// BBi

	}

// BBi
//#if defined RTCW_ET
// BBi

	if ( glConfig.anisotropicAvailable ) {
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, gl_anisotropy );
	}

// BBi
//#endif // RTCW_XX
// BBi

	GL_CheckErrors();

	//if ( scaledBuffer != 0 )
	//	ri.Hunk_FreeTempMemory( scaledBuffer );
	//if ( resampledBuffer != 0 )
	//	ri.Hunk_FreeTempMemory( resampledBuffer );
}


#if defined RTCW_SP
//----(SA)	modified
#endif // RTCW_XX

/*
================
R_CreateImage

This is the only way any image_t are created
================
*/
#if defined RTCW_SP
image_t *R_CreateImageExt( const char *name, const byte *pic, int width, int height,
						   qboolean mipmap, qboolean allowPicmip, qboolean characterMip, int glWrapClampMode ) {
#else
image_t *R_CreateImage( const char *name, const byte *pic, int width, int height,
						qboolean mipmap, qboolean allowPicmip, int glWrapClampMode ) {
#endif // RTCW_XX

	image_t     *image;
	qboolean isLightmap = qfalse;
	int32_t hash;
	qboolean noCompress = qfalse;

	if ( strlen( name ) >= MAX_QPATH ) {
		ri.Error( ERR_DROP, "R_CreateImage: \"%s\" is too long\n", name );
	}
	if ( !strncmp( name, "*lightmap", 9 ) ) {
		isLightmap = qtrue;
		noCompress = qtrue;
	}
	if ( !noCompress && strstr( name, "skies" ) ) {
		noCompress = qtrue;
	}
	if ( !noCompress && strstr( name, "weapons" ) ) {    // don't compress view weapon skins
		noCompress = qtrue;
	}
	// RF, if the shader hasn't specifically asked for it, don't allow compression
	if ( r_ext_compressed_textures->integer == 2 && ( tr.allowCompress != qtrue ) ) {
		noCompress = qtrue;
	} else if ( r_ext_compressed_textures->integer == 1 && ( tr.allowCompress < 0 ) )     {
		noCompress = qtrue;
	}

#if defined RTCW_ET
	// ydnar: don't compress textures smaller or equal to 128x128 pixels
	else if ( ( width * height ) <= ( 128 * 128 ) ) {
		noCompress = qtrue;
	}
#endif // RTCW_XX

	if ( tr.numImages == MAX_DRAWIMAGES ) {
		ri.Error( ERR_DROP, "R_CreateImage: MAX_DRAWIMAGES hit\n" );
	}

	// Ridah
	image = tr.images[tr.numImages] = static_cast<image_t*> (R_CacheImageAlloc( sizeof( image_t ) ));

	glGenTextures( 1, &image->texnum );

	tr.numImages++;

	image->mipmap = mipmap;
	image->allowPicmip = allowPicmip;

	strcpy( image->imgName, name );

	image->width = width;
	image->height = height;

	image->wrapClampMode = glWrapClampMode;

	// lightmaps are always allocated on TMU 1
	if ( glConfigEx.use_arb_multitexture_ && isLightmap ) {
		image->TMU = 1;
	} else {
		image->TMU = 0;
	}

	if ( glConfigEx.use_arb_multitexture_ ) {
		GL_SelectTexture( image->TMU );
	}

	GL_Bind( image );

	Upload32( (unsigned *)pic,
			  image->width, image->height,
			  image->mipmap,
			  allowPicmip,

#if defined RTCW_SP
			  characterMip,                     //----(SA)	added
#endif // RTCW_XX

			  isLightmap,
			  &image->internalFormat,
			  &image->uploadWidth,
			  &image->uploadHeight,
			  noCompress );

// BBi
//#if defined RTCW_ET
//	// ydnar: opengl 1.2 GL_CLAMP_TO_EDGE SUPPORT
//	// only 1.1 headers, joy
//	#define GL_CLAMP_TO_EDGE    0x812F
//	if ( r_clampToEdge->integer && glWrapClampMode == GL_CLAMP ) {
//		glWrapClampMode = GL_CLAMP_TO_EDGE;
//	}
//#endif // RTCW_XX
// BBi

	// BBi
	//::glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glWrapClampMode );
	//::glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glWrapClampMode );
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glWrapClampMode);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glWrapClampMode);
	// BBi

	glBindTexture( GL_TEXTURE_2D, 0 );

	if ( image->TMU == 1 ) {
		GL_SelectTexture( 0 );
	}

	hash = generateHashValue( name );
	image->next = hashTable[hash];
	hashTable[hash] = image;

	// Ridah
	image->hash = hash;

	return image;
}

#if defined RTCW_SP
image_t *R_CreateImage( const char *name, const byte *pic, int width, int height,
						qboolean mipmap, qboolean allowPicmip, int glWrapClampMode ) {
	return R_CreateImageExt( name, pic, width, height, mipmap, allowPicmip, qfalse, glWrapClampMode );
}

//----(SA)	end
#endif // RTCW_XX

/*
=========================================================

BMP LOADING

=========================================================
*/

// BBi
//typedef struct
//{
//	char id[2];
//	unsigned long fileSize;
//	unsigned long reserved0;
//	unsigned long bitmapDataOffset;
//	unsigned long bitmapHeaderSize;
//	unsigned long width;
//	unsigned long height;
//	unsigned short planes;
//	unsigned short bitsPerPixel;
//	unsigned long compression;
//	unsigned long bitmapDataSize;
//	unsigned long hRes;
//	unsigned long vRes;
//	unsigned long colors;
//	unsigned long importantColors;
//	unsigned char palette[256][4];
//} BMPHeader_t;

struct BMPHeader_t {
	int8_t id[2];
	uint32_t fileSize;
	uint32_t reserved0;
	uint32_t bitmapDataOffset;
	uint32_t bitmapHeaderSize;
	uint32_t width;
	uint32_t height;
	uint16_t planes;
	uint16_t bitsPerPixel;
	uint32_t compression;
	uint32_t bitmapDataSize;
	uint32_t hRes;
	uint32_t vRes;
	uint32_t colors;
	uint32_t importantColors;
	uint8_t palette[256][4];
}; // struct BMPHeader_t
// BBi

static void LoadBMP( const char *name, byte **pic, int *width, int *height ) {
	int columns, rows, numPixels;
	byte    *pixbuf;
	int row, column;
	byte    *buf_p;
	byte    *buffer;
	int length;
	BMPHeader_t bmpHeader;
	byte        *bmpRGBA;

	*pic = NULL;

	//
	// load the file
	//
	length = ri.FS_ReadFile( ( char * ) name, (void **)&buffer );
	if ( !buffer ) {
		return;
	}

	buf_p = buffer;

	bmpHeader.id[0] = *buf_p++;
	bmpHeader.id[1] = *buf_p++;
	bmpHeader.fileSize = rtcw::Endian::le( *( int32_t * ) buf_p );
	buf_p += 4;
	bmpHeader.reserved0 = rtcw::Endian::le( *( int32_t * ) buf_p );
	buf_p += 4;
	bmpHeader.bitmapDataOffset = rtcw::Endian::le( *( int32_t * ) buf_p );
	buf_p += 4;
	bmpHeader.bitmapHeaderSize = rtcw::Endian::le( *( int32_t * ) buf_p );
	buf_p += 4;
	bmpHeader.width = rtcw::Endian::le( *( int32_t * ) buf_p );
	buf_p += 4;
	bmpHeader.height = rtcw::Endian::le( *( int32_t * ) buf_p );
	buf_p += 4;
	bmpHeader.planes = rtcw::Endian::le( *( short * ) buf_p );
	buf_p += 2;
	bmpHeader.bitsPerPixel = rtcw::Endian::le( *( short * ) buf_p );
	buf_p += 2;
	bmpHeader.compression = rtcw::Endian::le( *( int32_t * ) buf_p );
	buf_p += 4;
	bmpHeader.bitmapDataSize = rtcw::Endian::le( *( int32_t * ) buf_p );
	buf_p += 4;
	bmpHeader.hRes = rtcw::Endian::le( *( int32_t * ) buf_p );
	buf_p += 4;
	bmpHeader.vRes = rtcw::Endian::le( *( int32_t * ) buf_p );
	buf_p += 4;
	bmpHeader.colors = rtcw::Endian::le( *( int32_t * ) buf_p );
	buf_p += 4;
	bmpHeader.importantColors = rtcw::Endian::le( *( int32_t * ) buf_p );
	buf_p += 4;

	memcpy( bmpHeader.palette, buf_p, sizeof( bmpHeader.palette ) );

	if ( bmpHeader.bitsPerPixel == 8 ) {
		buf_p += 1024;
	}

	if ( bmpHeader.id[0] != 'B' && bmpHeader.id[1] != 'M' ) {
		ri.Error( ERR_DROP, "LoadBMP: only Windows-style BMP files supported (%s)\n", name );
	}
	if ( bmpHeader.fileSize != length ) {
		ri.Error( ERR_DROP, "LoadBMP: header size does not match file size (%d vs. %d) (%s)\n", bmpHeader.fileSize, length, name );
	}
	if ( bmpHeader.compression != 0 ) {
		ri.Error( ERR_DROP, "LoadBMP: only uncompressed BMP files supported (%s)\n", name );
	}
	if ( bmpHeader.bitsPerPixel < 8 ) {
		ri.Error( ERR_DROP, "LoadBMP: monochrome and 4-bit BMP files not supported (%s)\n", name );
	}

	columns = bmpHeader.width;
	rows = bmpHeader.height;
	if ( rows < 0 ) {
		rows = -rows;
	}
	numPixels = columns * rows;

	if ( width ) {
		*width = columns;
	}
	if ( height ) {
		*height = rows;
	}

	bmpRGBA = static_cast<byte*> (R_GetImageBuffer( numPixels * 4, BUFFER_IMAGE ));

	*pic = bmpRGBA;


	for ( row = rows - 1; row >= 0; row-- )
	{
		pixbuf = bmpRGBA + row * columns * 4;

		for ( column = 0; column < columns; column++ )
		{
			unsigned char red, green, blue, alpha;
			int palIndex;
			unsigned short shortPixel;

			switch ( bmpHeader.bitsPerPixel )
			{
			case 8:
				palIndex = *buf_p++;
				*pixbuf++ = bmpHeader.palette[palIndex][2];
				*pixbuf++ = bmpHeader.palette[palIndex][1];
				*pixbuf++ = bmpHeader.palette[palIndex][0];
				*pixbuf++ = 0xff;
				break;
			case 16:
				shortPixel = *( unsigned short * ) pixbuf;
				pixbuf += 2;
				*pixbuf++ = ( shortPixel & ( 31 << 10 ) ) >> 7;
				*pixbuf++ = ( shortPixel & ( 31 << 5 ) ) >> 2;
				*pixbuf++ = ( shortPixel & ( 31 ) ) << 3;
				*pixbuf++ = 0xff;
				break;

			case 24:
				blue = *buf_p++;
				green = *buf_p++;
				red = *buf_p++;
				*pixbuf++ = red;
				*pixbuf++ = green;
				*pixbuf++ = blue;
				*pixbuf++ = 255;
				break;
			case 32:
				blue = *buf_p++;
				green = *buf_p++;
				red = *buf_p++;
				alpha = *buf_p++;
				*pixbuf++ = red;
				*pixbuf++ = green;
				*pixbuf++ = blue;
				*pixbuf++ = alpha;
				break;
			default:
				ri.Error( ERR_DROP, "LoadBMP: illegal pixel_size '%d' in file '%s'\n", bmpHeader.bitsPerPixel, name );
				break;
			}
		}
	}

	ri.FS_FreeFile( buffer );

}


/*
=================================================================

PCX LOADING

=================================================================
*/


/*
==============
LoadPCX
==============
*/

#if defined RTCW_ET
#define DECODEPCX( b, d, r ) d = *b++; if ( ( d & 0xC0 ) == 0xC0 ) {r = d & 0x3F; d = *b++;} else {r = 1;}
#endif // RTCW_XX

// BBi
//static void LoadPCX( const char *filename, byte **pic, byte **palette, int *width, int *height ) {
//	byte    *raw;
//	pcx_t   *pcx;
//
//#if !defined RTCW_ET
//	int x, y;
//#else
//	int x, y, lsize;
//#endif // RTCW_XX
//
//	int len;
//	int dataByte, runLength;
//	byte    *out, *pix;
//	int xmax, ymax;
//
//	*pic = NULL;
//	*palette = NULL;
//
//#if defined RTCW_ET
//	runLength = 0;
//#endif // RTCW_XX
//
//	//
//	// load the file
//	//
//	len = ri.FS_ReadFile( ( char * ) filename, (void **)&raw );
//	if ( !raw ) {
//		return;
//	}
//
//	//
//	// parse the PCX file
//	//
//	pcx = (pcx_t *)raw;
//	raw = &pcx->data;
//
//	xmax = LittleShort( pcx->xmax );
//	ymax = LittleShort( pcx->ymax );
//
//	if ( pcx->manufacturer != 0x0a
//		 || pcx->version != 5
//		 || pcx->encoding != 1
//		 || pcx->bits_per_pixel != 8
//		 || xmax >= 1024
//		 || ymax >= 1024 ) {
//		ri.Printf( PRINT_ALL, "Bad pcx file %s (%i x %i) (%i x %i)\n", filename, xmax + 1, ymax + 1, pcx->xmax, pcx->ymax );
//		return;
//	}
//
//	out = static_cast<byte*> (R_GetImageBuffer( ( ymax + 1 ) * ( xmax + 1 ), BUFFER_IMAGE ));
//
//	*pic = out;
//
//	pix = out;
//
//	if ( palette ) {
//
//#if defined RTCW_SP
//		*palette = static_cast<byte*> (malloc( 768 ));
//#else
//		*palette = static_cast<byte*> (ri.Z_Malloc( 768 ));
//#endif // RTCW_XX
//
//		memcpy( *palette, (byte *)pcx + len - 768, 768 );
//	}
//
//	if ( width ) {
//		*width = xmax + 1;
//	}
//	if ( height ) {
//		*height = ymax + 1;
//	}
//// FIXME: use bytes_per_line here?
//
//#if !defined RTCW_ET
//	for ( y = 0 ; y <= ymax ; y++, pix += xmax + 1 )
//	{
//		for ( x = 0 ; x <= xmax ; )
//		{
//			dataByte = *raw++;
//
//			if ( ( dataByte & 0xC0 ) == 0xC0 ) {
//				runLength = dataByte & 0x3F;
//				dataByte = *raw++;
//			} else {
//				runLength = 1;
//			}
//
//			while ( runLength-- > 0 )
//				pix[x++] = dataByte;
//		}
//
//	}
//#else
//	// Arnout: this doesn't work for all pcx files
//	/*for (y=0 ; y<=ymax ; y++, pix += xmax+1)
//	{
//		for (x=0 ; x<=xmax ; )
//		{
//			dataByte = *raw++;
//
//			if((dataByte & 0xC0) == 0xC0)
//			{
//				runLength = dataByte & 0x3F;
//				dataByte = *raw++;
//			}
//			else
//				runLength = 1;
//
//			while(runLength-- > 0)
//				pix[x++] = dataByte;
//		}
//
//	}*/
//
//	lsize = pcx->color_planes * pcx->bytes_per_line;
//
//	// go scanline by scanline
//	for ( y = 0; y <= pcx->ymax; y++, pix += pcx->xmax + 1 )
//	{
//		// do a scanline
//		for ( x = 0; x <= pcx->xmax; )
//		{
//			DECODEPCX( raw, dataByte, runLength );
//			while ( runLength-- > 0 )
//				pix[ x++ ] = dataByte;
//		}
//
//		// discard any other data
//		while ( x < lsize )
//		{
//			DECODEPCX( raw, dataByte, runLength );
//			x++;
//		}
//		while ( runLength-- > 0 )
//			x++;
//	}
//#endif // RTCW_XX
//
//	if ( raw - (byte *)pcx > len ) {
//		ri.Printf( PRINT_DEVELOPER, "PCX file %s was malformed", filename );
//
//#if defined RTCW_SP
//		free( *pic );
//#else
//		ri.Free( *pic );
//#endif // RTCW_XX
//
//		*pic = NULL;
//	}
//
//	ri.FS_FreeFile( pcx );
//}

static void LoadPCX (
	const char* filename,
	uint8_t** pic,
	uint8_t** palette,
	int* width,
	int* height)
{
	uint8_t* raw;
	pcx_t* pcx;

#if !defined RTCW_ET
	int x, y;
#else
	int x, y, lsize;
#endif // RTCW_XX

	int len;
	int dataByte, runLength;
	byte    *out, *pix;
	int xmax, ymax;

	*pic = NULL;
	*palette = NULL;

#if defined RTCW_ET
	runLength = 0;
#endif // RTCW_XX

	//
	// load the file
	//
	len = ri.FS_ReadFile( ( char * ) filename, (void **)&raw );
	if ( !raw ) {
		return;
	}

	//
	// parse the PCX file
	//
	pcx = (pcx_t *)raw;
	raw = &pcx->data;

	xmax = rtcw::Endian::le( pcx->xmax );
	ymax = rtcw::Endian::le( pcx->ymax );

	if ( pcx->manufacturer != 0x0a
		|| pcx->version != 5
		|| pcx->encoding != 1
		|| pcx->bits_per_pixel != 8
		|| xmax >= 1024
		|| ymax >= 1024 ) {
			ri.Printf( PRINT_ALL, "Bad pcx file %s (%i x %i) (%i x %i)\n", filename, xmax + 1, ymax + 1, pcx->xmax, pcx->ymax );
			return;
	}

	out = static_cast<byte*> (R_GetImageBuffer( ( ymax + 1 ) * ( xmax + 1 ), BUFFER_IMAGE ));

	*pic = out;

	pix = out;

	if ( palette ) {

#if defined RTCW_SP
		*palette = static_cast<byte*> (malloc( 768 ));
#else
		*palette = static_cast<byte*> (ri.Z_Malloc( 768 ));
#endif // RTCW_XX

		memcpy( *palette, (byte *)pcx + len - 768, 768 );
	}

	if ( width ) {
		*width = xmax + 1;
	}
	if ( height ) {
		*height = ymax + 1;
	}
	// FIXME: use bytes_per_line here?

#if !defined RTCW_ET
	for ( y = 0 ; y <= ymax ; y++, pix += xmax + 1 )
	{
		for ( x = 0 ; x <= xmax ; )
		{
			dataByte = *raw++;

			if ( ( dataByte & 0xC0 ) == 0xC0 ) {
				runLength = dataByte & 0x3F;
				dataByte = *raw++;
			} else {
				runLength = 1;
			}

			while ( runLength-- > 0 )
				pix[x++] = dataByte;
		}

	}
#else
	// Arnout: this doesn't work for all pcx files
	/*for (y=0 ; y<=ymax ; y++, pix += xmax+1)
	{
	for (x=0 ; x<=xmax ; )
	{
	dataByte = *raw++;

	if((dataByte & 0xC0) == 0xC0)
	{
	runLength = dataByte & 0x3F;
	dataByte = *raw++;
	}
	else
	runLength = 1;

	while(runLength-- > 0)
	pix[x++] = dataByte;
	}

	}*/

	lsize = pcx->color_planes * pcx->bytes_per_line;

	// go scanline by scanline
	for ( y = 0; y <= pcx->ymax; y++, pix += pcx->xmax + 1 )
	{
		// do a scanline
		for ( x = 0; x <= pcx->xmax; )
		{
			DECODEPCX( raw, dataByte, runLength );
			while ( runLength-- > 0 )
				pix[ x++ ] = dataByte;
		}

		// discard any other data
		while ( x < lsize )
		{
			DECODEPCX( raw, dataByte, runLength );
			x++;
		}
		while ( runLength-- > 0 )
			x++;
	}
#endif // RTCW_XX

	if ( raw - (byte *)pcx > len ) {
		ri.Printf( PRINT_DEVELOPER, "PCX file %s was malformed", filename );

#if defined RTCW_SP
		free( *pic );
#else
		ri.Free( *pic );
#endif // RTCW_XX

		*pic = NULL;
	}

	ri.FS_FreeFile( pcx );
}
// BBi

/*
==============
LoadPCX32
==============
*/
static void LoadPCX32( const char *filename, byte **pic, int *width, int *height ) {
	byte    *palette;
	byte    *pic8;
	int i, c, p;
	byte    *pic32;

	LoadPCX( filename, &pic8, &palette, width, height );
	if ( !pic8 ) {
		*pic = NULL;
		return;
	}

	c = ( *width ) * ( *height );
	pic32 = *pic = static_cast<byte*> (R_GetImageBuffer( 4 * c, BUFFER_IMAGE ));
	for ( i = 0 ; i < c ; i++ ) {
		p = pic8[i];
		pic32[0] = palette[p * 3];
		pic32[1] = palette[p * 3 + 1];
		pic32[2] = palette[p * 3 + 2];
		pic32[3] = 255;
		pic32 += 4;
	}

#if defined RTCW_SP
	// BBi
	free( pic8 );
	free( palette );
	// BBi
#else
	ri.Free( pic8 );
	ri.Free( palette );
#endif // RTCW_XX

}

/*
=========================================================

TARGA LOADING

=========================================================
*/

/*
=============
LoadTGA
=============
*/
void LoadTGA( const char *name, byte **pic, int *width, int *height ) {
	int columns, rows, numPixels;
	byte    *pixbuf;
	int row, column;
	byte    *buf_p;
	byte    *buffer;
	TargaHeader targa_header;
	byte        *targa_rgba;

	*pic = NULL;

	//
	// load the file
	//
	ri.FS_ReadFile( ( char * ) name, (void **)&buffer );
	if ( !buffer ) {
		return;
	}

	buf_p = buffer;

	targa_header.id_length = *buf_p++;
	targa_header.colormap_type = *buf_p++;
	targa_header.image_type = *buf_p++;

	targa_header.colormap_index = rtcw::Endian::le( *(short *)buf_p );
	buf_p += 2;
	targa_header.colormap_length = rtcw::Endian::le( *(short *)buf_p );
	buf_p += 2;
	targa_header.colormap_size = *buf_p++;
	targa_header.x_origin = rtcw::Endian::le( *(short *)buf_p );
	buf_p += 2;
	targa_header.y_origin = rtcw::Endian::le( *(short *)buf_p );
	buf_p += 2;
	targa_header.width = rtcw::Endian::le( *(short *)buf_p );
	buf_p += 2;
	targa_header.height = rtcw::Endian::le( *(short *)buf_p );
	buf_p += 2;
	targa_header.pixel_size = *buf_p++;
	targa_header.attributes = *buf_p++;

	if ( targa_header.image_type != 2
		 && targa_header.image_type != 10
		 && targa_header.image_type != 3 ) {
		ri.Error( ERR_DROP, "LoadTGA: Only type 2 (RGB), 3 (gray), and 10 (RGB) TGA images supported\n" );
	}

	if ( targa_header.colormap_type != 0 ) {
		ri.Error( ERR_DROP, "LoadTGA: colormaps not supported\n" );
	}

	if ( ( targa_header.pixel_size != 32 && targa_header.pixel_size != 24 ) && targa_header.image_type != 3 ) {
		ri.Error( ERR_DROP, "LoadTGA: Only 32 or 24 bit images supported (no colormaps)\n" );
	}

	columns = targa_header.width;
	rows = targa_header.height;
	numPixels = columns * rows;

	if ( width ) {
		*width = columns;
	}
	if ( height ) {
		*height = rows;
	}

	targa_rgba = static_cast<byte*> (R_GetImageBuffer( numPixels * 4, BUFFER_IMAGE ));
	*pic = targa_rgba;

	if ( targa_header.id_length != 0 ) {
		buf_p += targa_header.id_length;  // skip TARGA image comment

	}
	if ( targa_header.image_type == 2 || targa_header.image_type == 3 ) {
		// Uncompressed RGB or gray scale image
		for ( row = rows - 1; row >= 0; row-- )
		{
			pixbuf = targa_rgba + row * columns * 4;
			for ( column = 0; column < columns; column++ )
			{
				unsigned char red,green,blue,alphabyte;
				switch ( targa_header.pixel_size )
				{

				case 8:
					blue = *buf_p++;
					green = blue;
					red = blue;
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = 255;
					break;

				case 24:
					blue = *buf_p++;
					green = *buf_p++;
					red = *buf_p++;
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = 255;
					break;
				case 32:
					blue = *buf_p++;
					green = *buf_p++;
					red = *buf_p++;
					alphabyte = *buf_p++;
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = alphabyte;
					break;
				default:
					ri.Error( ERR_DROP, "LoadTGA: illegal pixel_size '%d' in file '%s'\n", targa_header.pixel_size, name );
					break;
				}
			}
		}
	} else if ( targa_header.image_type == 10 )       { // Runlength encoded RGB images
		unsigned char red,green,blue,alphabyte,packetHeader,packetSize,j;

		red = 0;
		green = 0;
		blue = 0;
		alphabyte = 0xff;

		for ( row = rows - 1; row >= 0; row-- ) {
			pixbuf = targa_rgba + row * columns * 4;
			for ( column = 0; column < columns; ) {
				packetHeader = *buf_p++;
				packetSize = 1 + ( packetHeader & 0x7f );
				if ( packetHeader & 0x80 ) {        // run-length packet
					switch ( targa_header.pixel_size ) {
					case 24:
						blue = *buf_p++;
						green = *buf_p++;
						red = *buf_p++;
						alphabyte = 255;
						break;
					case 32:
						blue = *buf_p++;
						green = *buf_p++;
						red = *buf_p++;
						alphabyte = *buf_p++;
						break;
					default:
						ri.Error( ERR_DROP, "LoadTGA: illegal pixel_size '%d' in file '%s'\n", targa_header.pixel_size, name );
						break;
					}

					for ( j = 0; j < packetSize; j++ ) {
						*pixbuf++ = red;
						*pixbuf++ = green;
						*pixbuf++ = blue;
						*pixbuf++ = alphabyte;
						column++;
						if ( column == columns ) { // run spans across rows
							column = 0;
							if ( row > 0 ) {
								row--;
							} else {
								goto breakOut;
							}
							pixbuf = targa_rgba + row * columns * 4;
						}
					}
				} else {                            // non run-length packet
					for ( j = 0; j < packetSize; j++ ) {
						switch ( targa_header.pixel_size ) {
						case 24:
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = 255;
							break;
						case 32:
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							alphabyte = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = alphabyte;
							break;
						default:
							ri.Error( ERR_DROP, "LoadTGA: illegal pixel_size '%d' in file '%s'\n", targa_header.pixel_size, name );
							break;
						}
						column++;
						if ( column == columns ) { // pixel packet run spans across rows
							column = 0;
							if ( row > 0 ) {
								row--;
							} else {
								goto breakOut;
							}
							pixbuf = targa_rgba + row * columns * 4;
						}
					}
				}
			}
breakOut:;
		}
	}

	ri.FS_FreeFile( buffer );
}

static void LoadJPG(
	const char* filename,
	uint8_t** pic,
	int* width,
	int* height)
{
	*pic = NULL;

	void* src_data = NULL;

	int src_size = ri.FS_ReadFile(filename, &src_data);

	if (src_data == NULL)
		return;

	bool jpeg_result;

	jpeg_result = g_jpeg_reader.open(src_data, src_size, *width, *height);

	if (!jpeg_result) {
		ri.Error(ERR_FATAL, "JPEG: %s\n",
			g_jpeg_reader.get_error_message().c_str());
	}

	void* dst_data = R_GetImageBuffer(4 * (*width) * (*height), BUFFER_IMAGE);

	jpeg_result = g_jpeg_reader.decode(dst_data);

	ri.FS_FreeFile(src_data);

	if (!jpeg_result) {
		ri.Error(ERR_FATAL, "JPEG: %s\n",
			g_jpeg_reader.get_error_message().c_str());
	}

	*pic = static_cast<byte*>(dst_data);
}

void SaveJPG(
	const char* file_name,
	int quality,
	int width,
	int height,
	uint8_t* src_data)
{
	int dst_size = 0;
	int dst_max_size = g_jpeg_writer.estimate_dst_size(width, height);
	void* dst_data = ri.Hunk_AllocateTempMemory(dst_max_size);
	bool jpeg_result;

	jpeg_result = g_jpeg_writer.encode(
		quality, src_data, width, height, dst_data, dst_size);

	if (jpeg_result)
		ri.FS_WriteFile(file_name, dst_data, dst_size);
	else {
		ri.Printf(PRINT_ALL, S_COLOR_RED "JPEG: %s\n",
			g_jpeg_writer.get_error_message().c_str());
	}

	ri.Hunk_FreeTempMemory(dst_data);
}
// BBi

//===================================================================

/*
=================
R_LoadImage

Loads any of the supported image types into a cannonical
32 bit format.
=================
*/
void R_LoadImage( const char *name, byte **pic, int *width, int *height ) {
	int len;

	*pic = NULL;
	*width = 0;
	*height = 0;

	len = strlen( name );
	if ( len < 5 ) {
		return;
	}

	if ( !Q_stricmp( name + len - 4, ".tga" ) ) {
		LoadTGA( name, pic, width, height );          // try tga first
		if ( !*pic ) {                              //
			char altname[MAX_QPATH];                    // try jpg in place of tga
			strcpy( altname, name );
			len = strlen( altname );
			altname[len - 3] = 'j';
			altname[len - 2] = 'p';
			altname[len - 1] = 'g';
			LoadJPG( altname, pic, width, height );
		}
	} else if ( !Q_stricmp( name + len - 4, ".pcx" ) ) {
		LoadPCX32( name, pic, width, height );
	} else if ( !Q_stricmp( name + len - 4, ".bmp" ) ) {
		LoadBMP( name, pic, width, height );
	} else if ( !Q_stricmp( name + len - 4, ".jpg" ) ) {
		LoadJPG( name, pic, width, height );
	}
}


#if defined RTCW_SP
//----(SA)	modified
#endif // RTCW_XX

/*
===============
R_FindImageFile

Finds or loads the given image.
Returns NULL if it fails, not a default image.
==============
*/

#if defined RTCW_SP
image_t *R_FindImageFileExt( const char *name, qboolean mipmap, qboolean allowPicmip, qboolean characterMIP, int glWrapClampMode ) {
#elif defined RTCW_MP
image_t *R_FindImageFile( const char *name, qboolean mipmap, qboolean allowPicmip, int glWrapClampMode ) {
#else
image_t *R_FindImageFile( const char *name, qboolean mipmap, qboolean allowPicmip, int glWrapClampMode, qboolean lightmap ) {
#endif // RTCW_XX

	image_t *image;
	int width, height;
	byte    *pic;
	int32_t hash;

#if defined RTCW_ET
	qboolean allowCompress = qfalse;
#endif // RTCW_XX

	if ( !name ) {
		return NULL;
	}

	hash = generateHashValue( name );

	// Ridah, caching
	if ( r_cacheGathering->integer ) {

#if defined RTCW_SP
		ri.Cmd_ExecuteText( EXEC_NOW, va( "cache_usedfile image %s %i %i %i %i\n", name, mipmap, allowPicmip, characterMIP, glWrapClampMode ) );
#else
		ri.Cmd_ExecuteText( EXEC_NOW, va( "cache_usedfile image %s %i %i %i\n", name, mipmap, allowPicmip, glWrapClampMode ) );
#endif // RTCW_XX

	}

	//
	// see if the image is already loaded
	//
	for ( image = hashTable[hash]; image; image = image->next ) {

#if defined RTCW_SP
		if ( !Q_stricmp( name, image->imgName ) ) {
#else
		if ( !strcmp( name, image->imgName ) ) {
#endif // RTCW_XX

			// the white image can be used with any set of parms, but other mismatches are errors
			if ( strcmp( name, "*white" ) ) {
				if ( image->mipmap != mipmap ) {
					ri.Printf( PRINT_DEVELOPER, "WARNING: reused image %s with mixed mipmap parm\n", name );
				}
				if ( image->allowPicmip != allowPicmip ) {
					ri.Printf( PRINT_DEVELOPER, "WARNING: reused image %s with mixed allowPicmip parm\n", name );
				}

				if ( image->wrapClampMode != glWrapClampMode ) {
					ri.Printf( PRINT_ALL, "WARNING: reused image %s with mixed glWrapClampMode parm\n", name );
				}
			}
			return image;
		}
	}

	// Ridah, check the cache
	// TTimo: assignment used as truth value

#if !defined RTCW_ET
	if ( ( image = R_FindCachedImage( name, hash ) ) ) {
		return image;
	}
	// done.
#else
	// ydnar: don't do this for lightmaps
	if ( !lightmap ) {
		image = R_FindCachedImage( name, hash );
		if ( image != NULL ) {
			return image;
		}
	}
#endif // RTCW_XX

	//
	// load the pic from disk
	//
	R_LoadImage( name, &pic, &width, &height );
	if ( pic == NULL ) {                                    // if we dont get a successful load

#if defined RTCW_SP
// RF, no need to check uppercase on win32 systems
#endif // RTCW_XX

// TTimo: Duane changed to _DEBUG in all cases
// I'd still want that code in the release builds on linux
// (possibly for mod authors)
// /me maintained off for win32, using otherwise but printing diagnostics as developer
#if !_WIN32
		char altname[MAX_QPATH];                            // copy the name
		int len;                                          //
		strcpy( altname, name );                          //
		len = strlen( altname );                          //
		altname[len - 3] = toupper( altname[len - 3] );   // and try upper case extension for unix systems
		altname[len - 2] = toupper( altname[len - 2] );   //
		altname[len - 1] = toupper( altname[len - 1] );   //
		ri.Printf( PRINT_DEVELOPER, "trying %s...", altname );
		R_LoadImage( altname, &pic, &width, &height );      //
		if ( pic == NULL ) {                              // if that fails
			ri.Printf( PRINT_DEVELOPER, "no\n" );
			return NULL;                                  // bail
		}
		ri.Printf( PRINT_DEVELOPER, "yes\n" );
#else
		return NULL;
#endif
	}

#if defined RTCW_SP
	image = R_CreateImageExt( ( char * ) name, pic, width, height, mipmap, allowPicmip, characterMIP, glWrapClampMode );
#else

#if defined RTCW_ET
	// Arnout: apply lightmap colouring
	if ( lightmap ) {
		R_ProcessLightmap( &pic, 4, width, height, &pic );

		// ydnar: no texture compression
		if ( lightmap ) {
			allowCompress = tr.allowCompress;
		}
		tr.allowCompress = -1;
	}

//#ifdef _DEBUG
#define CHECKPOWEROF2
//#endif // _DEBUG

#ifdef CHECKPOWEROF2
	if ( ( ( width - 1 ) & width ) || ( ( height - 1 ) & height ) ) {
		Com_Printf( "^1Image not power of 2 scaled: %s\n", name );
		return NULL;
	}
#endif // CHECKPOWEROF2
#endif // RTCW_XX

	image = R_CreateImage( ( char * ) name, pic, width, height, mipmap, allowPicmip, glWrapClampMode );
#endif // RTCW_XX

	//ri.Free( pic );

#if defined RTCW_ET
	// ydnar: no texture compression
	if ( lightmap ) {
		tr.allowCompress = allowCompress;
	}
#endif // RTCW_XX

	return image;
}


#if defined RTCW_SP
image_t *R_FindImageFile( const char *name, qboolean mipmap, qboolean allowPicmip, int glWrapClampMode ) {
	return R_FindImageFileExt( name, mipmap, allowPicmip, qfalse, glWrapClampMode );
}

//----(SA)	end
#endif // RTCW_XX


/*
================
R_CreateDlightImage
================
*/
#define DLIGHT_SIZE 16
static void R_CreateDlightImage( void ) {
	int x,y;
	byte data[DLIGHT_SIZE][DLIGHT_SIZE][4];
	int b;

	// make a centered inverse-square falloff blob for dynamic lighting
	for ( x = 0 ; x < DLIGHT_SIZE ; x++ ) {
		for ( y = 0 ; y < DLIGHT_SIZE ; y++ ) {
			float d;

			d = ( DLIGHT_SIZE / 2 - 0.5f - x ) * ( DLIGHT_SIZE / 2 - 0.5f - x ) +
				( DLIGHT_SIZE / 2 - 0.5f - y ) * ( DLIGHT_SIZE / 2 - 0.5f - y );
			b = 4000 / d;
			if ( b > 255 ) {
				b = 255;
			} else if ( b < 75 ) {
				b = 0;
			}
			data[y][x][0] =
				data[y][x][1] =
					data[y][x][2] = b;
			data[y][x][3] = 255;
		}
	}

	// BBi
	//tr.dlightImage = R_CreateImage( "*dlight", (byte *)data, DLIGHT_SIZE, DLIGHT_SIZE, qfalse, qfalse, GL_CLAMP );
	tr.dlightImage = R_CreateImage ("*dlight", (byte *)data, DLIGHT_SIZE, DLIGHT_SIZE, false, false, r_get_best_wrap_clamp ());
	// BBi
}


/*
=================
R_InitFogTable
=================
*/
void R_InitFogTable( void ) {
	int i;
	float d;
	float exp;

	exp = 0.5;

	for ( i = 0 ; i < FOG_TABLE_SIZE ; i++ ) {
		d = c::pow( (float)i / ( FOG_TABLE_SIZE - 1 ), exp );

#if defined RTCW_ET
		// ydnar: changed to linear fog
#endif // RTCW_XX

		tr.fogTable[i] = d;

#if defined RTCW_ET
		//%	tr.fogTable[ i ] = (i / 255.0f);
#endif // RTCW_XX

	}
}

/*
================
R_FogFactor

Returns a 0.0 to 1.0 fog density value
This is called for each texel of the fog texture on startup
and for each vertex of transparent shaders in fog dynamically
================
*/
float   R_FogFactor( float s, float t ) {
	float d;

	s -= 1.0 / 512;
	if ( s < 0 ) {
		return 0;
	}
	if ( t < 1.0 / 32 ) {
		return 0;
	}
	if ( t < 31.0 / 32 ) {
		s *= ( t - 1.0f / 32.0f ) / ( 30.0f / 32.0f );
	}

	// we need to leave a lot of clamp range
	s *= 8;

	if ( s > 1.0 ) {
		s = 1.0;
	}

	d = tr.fogTable[ (int)( s * ( FOG_TABLE_SIZE - 1 ) ) ];

	return d;
}

#if defined RTCW_ET
void SaveTGAAlpha( char *name, byte **pic, int width, int height );
#endif // RTCW_XX

/*
================
R_CreateFogImage
================
*/

// BBi
//#if !defined RTCW_ET
//#define FOG_S   256
//#define FOG_T   32
//#else
//#define FOG_S       16
//#define FOG_T       16  // ydnar: used to be 32
//						// arnout: yd changed it to 256, changing to 16
//#endif // RTCW_XX
//
//static void R_CreateFogImage( void ) {
//
//#if !defined RTCW_ET
//	int x,y;
//	byte    *data;
//	float g;
//	float d;
//	float borderColor[4];
//
//	data = static_cast<byte*> (ri.Hunk_AllocateTempMemory( FOG_S * FOG_T * 4 ));
//
//	g = 2.0;
//
//	// S is distance, T is depth
//	for ( x = 0 ; x < FOG_S ; x++ ) {
//		for ( y = 0 ; y < FOG_T ; y++ ) {
//			d = R_FogFactor( ( x + 0.5f ) / FOG_S, ( y + 0.5f ) / FOG_T );
//
//			data[( y * FOG_S + x ) * 4 + 0] =
//				data[( y * FOG_S + x ) * 4 + 1] =
//					data[( y * FOG_S + x ) * 4 + 2] = 255;
//			data[( y * FOG_S + x ) * 4 + 3] = 255 * d;
//		}
//	}
//#else
//	int x, y, alpha;
//	byte    *data;
//	//float	d;
//	float borderColor[4];
//
//
//	// allocate table for image
//	data = static_cast<byte*> (ri.Hunk_AllocateTempMemory( FOG_S * FOG_T * 4 ));
//
//	// ydnar: old fog texture generating algo
//
//	// S is distance, T is depth
//	/*for (x=0 ; x<FOG_S ; x++) {
//		for (y=0 ; y<FOG_T ; y++) {
//			d = R_FogFactor( ( x + 0.5f ) / FOG_S, ( y + 0.5f ) / FOG_T );
//
//			data[(y*FOG_S+x)*4+0] =
//			data[(y*FOG_S+x)*4+1] =
//			data[(y*FOG_S+x)*4+2] = 255;
//			data[(y*FOG_S+x)*4+3] = 255 * d;
//		}
//	}*/
//
//	//%	SaveTGAAlpha( "fog_q3.tga", &data, FOG_S, FOG_T );
//
//	// ydnar: new, linear fog texture generating algo for GL_CLAMP_TO_EDGE (OpenGL 1.2+)
//
//	// S is distance, T is depth
//	for ( x = 0 ; x < FOG_S ; x++ ) {
//		for ( y = 0 ; y < FOG_T ; y++ ) {
//			alpha = 270 * ( (float) x / FOG_S ) * ( (float) y / FOG_T );    // need slop room for fp round to 0
//			if ( alpha < 0 ) {
//				alpha = 0;
//			} else if ( alpha > 255 ) {
//				alpha = 255;
//			}
//
//			// ensure edge/corner cases are fully transparent (at 0,0) or fully opaque (at 1,N where N is 0-1.0)
//			if ( x == 0 ) {
//				alpha = 0;
//			} else if ( x == ( FOG_S - 1 ) ) {
//				alpha = 255;
//			}
//
//			data[( y * FOG_S + x ) * 4 + 0] =
//				data[( y * FOG_S + x ) * 4 + 1] =
//					data[( y * FOG_S + x ) * 4 + 2] = 255;
//			data[( y * FOG_S + x ) * 4 + 3] = alpha;  //%	255*d;
//		}
//	}
//
//	//%	SaveTGAAlpha( "fog_yd.tga", &data, FOG_S, FOG_T );
//#endif // RTCW_XX
//
//	// standard openGL clamping doesn't really do what we want -- it includes
//	// the border color at the edges.  OpenGL 1.2 has clamp-to-edge, which does
//	// what we want.
//	tr.fogImage = R_CreateImage( "*fog", (byte *)data, FOG_S, FOG_T, qfalse, qfalse, GL_CLAMP );
//	ri.Hunk_FreeTempMemory( data );
//
//#if defined RTCW_ET
//	// ydnar: the following lines are unecessary for new GL_CLAMP_TO_EDGE fog
//#endif // RTCW_XX
//
//	borderColor[0] = 1.0;
//	borderColor[1] = 1.0;
//	borderColor[2] = 1.0;
//	borderColor[3] = 1;
//
//	glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor );
//}

static void R_CreateFogImage ()
{
#if !defined RTCW_ET
	const int FOG_S = 256;
	const int FOG_T = 32;
#else
	const int FOG_S = 16;
	const int FOG_T = 16;
#endif // RTCW_XX

	byte* data = 0;

#if !defined RTCW_ET
	data = static_cast<byte*> (ri.Hunk_AllocateTempMemory (FOG_S * FOG_T * 4));

	const float g = 2.0F;

	// S is distance, T is depth
	for (int x = 0; x < FOG_S; ++x) {
		for (int y = 0; y < FOG_T; ++y) {
			float d = R_FogFactor ((x + 0.5F) / FOG_S, (y + 0.5F) / FOG_T);

			data[(((y * FOG_S) + x) * 4) + 0] = 255;
			data[(((y * FOG_S) + x) * 4) + 1] = 255;
			data[(((y * FOG_S) + x) * 4) + 2] = 255;
			data[(((y * FOG_S) + x) * 4) + 3] = 255 * d;
		}
	}
#else
	// allocate table for image
	data = static_cast<byte*> (ri.Hunk_AllocateTempMemory (FOG_S * FOG_T * 4));

	// ydnar: new, linear fog texture generating algo for GL_CLAMP_TO_EDGE (OpenGL 1.2+)

	// S is distance, T is depth
	for (int x = 0; x < FOG_S; ++x) {
		for (int y = 0; y < FOG_T; ++y) {
			int alpha = int (270.0F * (float (x) / FOG_S) * (float (y) / FOG_T)); // need slop room for fp round to 0

			if (alpha > 255)
				alpha = 255;

			// ensure edge/corner cases are fully transparent (at 0,0) or fully opaque (at 1,N where N is 0-1.0)
			if (x == 0)
				alpha = 0;
			else if (x == (FOG_S - 1))
				alpha = 255;

			data[(((y * FOG_S) + x) * 4) + 0] = 255;
			data[(((y * FOG_S) + x) * 4) + 1] = 255;
			data[(((y * FOG_S) + x) * 4) + 2] = 255;
			data[(((y * FOG_S) + x) * 4) + 3] = alpha;  //% 255*d;
		}
	}
#endif // RTCW_XX

	tr.fogImage = R_CreateImage ("*fog", data, FOG_S, FOG_T, false, false, r_get_best_wrap_clamp ());
	ri.Hunk_FreeTempMemory (data);

#if !defined RTCW_ET
	if (!glConfigEx.use_arb_framebuffer_object_) {
		float borderColor[4] = { 1.0F, 1.0F, 1.0F, 1.0F, };

		glTexParameterfv (GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	}
#endif // RTCW_XX
}
// BBi

/*
==================
R_CreateDefaultImage
==================
*/
#define DEFAULT_SIZE    16
static void R_CreateDefaultImage( void ) {

#if !defined RTCW_ET
	int x;
	byte data[DEFAULT_SIZE][DEFAULT_SIZE][4];

	// the default image will be a box, to allow you to see the mapping coordinates
	memset( data, 32, sizeof( data ) );
	for ( x = 0 ; x < DEFAULT_SIZE ; x++ ) {
		data[0][x][0] =
			data[0][x][1] =
				data[0][x][2] = 0; //----(SA) to make the default grid noticable but not blinding
		data[0][x][3] = 255;

		data[x][0][0] =
			data[x][0][1] =
				data[x][0][2] = 0; //----(SA) to make the default grid noticable but not blinding
		data[x][0][3] = 255;

		data[DEFAULT_SIZE - 1][x][0] =
			data[DEFAULT_SIZE - 1][x][1] =
				data[DEFAULT_SIZE - 1][x][2] = 0; //----(SA) to make the default grid noticable but not blinding
		data[DEFAULT_SIZE - 1][x][3] = 255;

		data[x][DEFAULT_SIZE - 1][0] =
			data[x][DEFAULT_SIZE - 1][1] =
				data[x][DEFAULT_SIZE - 1][2] = 0; //----(SA) to make the default grid noticable but not blinding
		data[x][DEFAULT_SIZE - 1][3] = 255;
#else
	int x, y;
	byte data[DEFAULT_SIZE][DEFAULT_SIZE][4];

	// the default image will be a box, to allow you to see the mapping coordinates
	memset( data, 0, sizeof( data ) );
	for ( x = 0 ; x < DEFAULT_SIZE ; x++ ) {
		for ( y = 0 ; y < 2; y++ ) {
			data[y][x][0] = 255;
			data[y][x][1] = 128;
			data[y][x][2] = 0;
			data[y][x][3] = 255;

			data[x][y][0] = 255;
			data[x][y][1] = 128;
			data[x][y][2] = 0;
			data[x][y][3] = 255;

			data[DEFAULT_SIZE - 1 - y][x][0] = 255;
			data[DEFAULT_SIZE - 1 - y][x][1] = 128;
			data[DEFAULT_SIZE - 1 - y][x][2] = 0;
			data[DEFAULT_SIZE - 1 - y][x][3] = 255;

			data[x][DEFAULT_SIZE - 1 - y][0] = 255;
			data[x][DEFAULT_SIZE - 1 - y][1] = 128;
			data[x][DEFAULT_SIZE - 1 - y][2] = 0;
			data[x][DEFAULT_SIZE - 1 - y][3] = 255;
		}
#endif // RTCW_XX

	}
	tr.defaultImage = R_CreateImage( "*default", (byte *)data, DEFAULT_SIZE, DEFAULT_SIZE, qtrue, qfalse, GL_REPEAT );
}

/*
==================
R_CreateBuiltinImages
==================
*/
void R_CreateBuiltinImages( void ) {
	int x,y;
	byte data[DEFAULT_SIZE][DEFAULT_SIZE][4];

	R_CreateDefaultImage();

	// we use a solid white image instead of disabling texturing
	memset( data, 255, sizeof( data ) );
	tr.whiteImage = R_CreateImage( "*white", (byte *)data, 8, 8, qfalse, qfalse, GL_REPEAT );

	// with overbright bits active, we need an image which is some fraction of full color,
	// for default lightmaps, etc
	for ( x = 0 ; x < DEFAULT_SIZE ; x++ ) {
		for ( y = 0 ; y < DEFAULT_SIZE ; y++ ) {
			data[y][x][0] =
				data[y][x][1] =
					data[y][x][2] = tr.identityLightByte;
			data[y][x][3] = 255;
		}
	}

	tr.identityLightImage = R_CreateImage( "*identityLight", (byte *)data, 8, 8, qfalse, qfalse, GL_REPEAT );


	for ( x = 0; x < 32; x++ ) {
		// scratchimage is usually used for cinematic drawing

		// BBi
		//tr.scratchImage[x] = R_CreateImage( "*scratch", (byte *)data, DEFAULT_SIZE, DEFAULT_SIZE, qfalse, qtrue, GL_CLAMP );
		tr.scratchImage[x] = R_CreateImage ("*scratch", (byte*) data, DEFAULT_SIZE, DEFAULT_SIZE, false, true, r_get_best_wrap_clamp ());
		// BBi
	}

	R_CreateDlightImage();
	R_CreateFogImage();
}


/*
===============
R_SetColorMappings
===============
*/
void R_SetColorMappings( void ) {
	int i, j;
	float g;
	int inf;
	int shift;

	// setup the overbright lighting
	tr.overbrightBits = r_overBrightBits->integer;

#ifndef RTCW_VANILLA
	if (glConfigEx.is_path_ogl_1_x())
	{
#endif // RTCW_VANILLA
	if ( !glConfig.deviceSupportsGamma ) {
		tr.overbrightBits = 0;      // need hardware gamma for overbright
	}

	// never overbright in windowed mode
	if ( !glConfig.isFullscreen ) {
		tr.overbrightBits = 0;
	}
#ifndef RTCW_VANILLA
	}
#endif // RTCW_VANILLA

#ifdef RTCW_VANILLA
	// allow 2 overbright bits in 24 bit, but only 1 in 16 bit
	if ( glConfig.colorBits > 16 ) {
		if ( tr.overbrightBits > 2 ) {
			tr.overbrightBits = 2;
		}
	} else {
		if ( tr.overbrightBits > 1 ) {
			tr.overbrightBits = 1;
		}
	}
#else // RTCW_VANILLA
	tr.overbrightBits = std::min(tr.overbrightBits, 2);
#endif // RTCW_VANILLA
	if ( tr.overbrightBits < 0 ) {
		tr.overbrightBits = 0;
	}

	tr.identityLight = 1.0f / ( 1 << tr.overbrightBits );
	tr.identityLightByte = 255 * tr.identityLight;


	if ( r_intensity->value <= 1 ) {
		ri.Cvar_Set( "r_intensity", "1" );
	}

	if ( r_gamma->value < 0.5f ) {
		ri.Cvar_Set( "r_gamma", "0.5" );
	} else if ( r_gamma->value > 3.0f ) {
		ri.Cvar_Set( "r_gamma", "3.0" );
	}

#ifndef RTCW_VANILLA
	if (glConfigEx.is_path_ogl_1_x())
	{
#endif // RTCW_VANILLA
	g = r_gamma->value;

	shift = tr.overbrightBits;

	for ( i = 0; i < 256; i++ ) {
		if ( g == 1 ) {
			inf = i;
		} else {
			inf = 255 * c::pow( i / 255.0f, 1.0f / g ) + 0.5f;
		}
		inf <<= shift;
		if ( inf < 0 ) {
			inf = 0;
		}
		if ( inf > 255 ) {
			inf = 255;
		}
		s_gammatable[i] = inf;
	}

	for ( i = 0 ; i < 256 ; i++ ) {
		j = i * r_intensity->value;
		if ( j > 255 ) {
			j = 255;
		}
		s_intensitytable[i] = j;
	}

	if ( glConfig.deviceSupportsGamma ) {
		GLimp_SetGamma( s_gammatable, s_gammatable, s_gammatable );
	}
#ifndef RTCW_VANILLA
	}
	else
	{
		tr.identityLight = 1.0F;
		tr.identityLightByte = 255;

		const int overbright_bits = std::min(std::max(r_overBrightBits->integer, 0), 2);
		const float overbright = static_cast<float>(1 << overbright_bits);

		ogl_tess_state.intensity = r_intensity->value;
		ogl_tess_state.overbright = overbright;
		ogl_tess_state.gamma = r_gamma->value;
	}
#endif // RTCW_VANILLA
}

/*
===============
R_InitImages
===============
*/
void    R_InitImages( void ) {
	memset( hashTable, 0, sizeof( hashTable ) );

	// Ridah, caching system

// BBi
//#if !defined RTCW_ET
//	R_InitTexnumImages( qfalse );
//#else
//	//%	R_InitTexnumImages(qfalse);
//#endif // RTCW_XX
// BBi

	// done.

	// build brightness translation tables
	R_SetColorMappings();

	// create default texture and white texture
	R_CreateBuiltinImages();

	// Ridah, load the cache media, if they were loaded previously, they'll be restored from the backupImages
	R_LoadCacheImages();
	// done.
}

/*
===============
R_DeleteTextures
===============
*/
void R_DeleteTextures( void ) {
	int i;

	for ( i = 0; i < tr.numImages ; i++ ) {
		glDeleteTextures( 1, &tr.images[i]->texnum );
	}
	memset( tr.images, 0, sizeof( tr.images ) );
	// Ridah

// BBi
//#if !defined RTCW_ET
//	R_InitTexnumImages( qtrue );
//#else
//	//%	R_InitTexnumImages(qtrue);
//#endif // RTCW_XX
// BBi

	// done.

	memset( glState.currenttextures, 0, sizeof( glState.currenttextures ) );

	if ( glConfigEx.use_arb_multitexture_ ) {
		GL_SelectTexture( 1 );
		glBindTexture( GL_TEXTURE_2D, 0 );
		GL_SelectTexture( 0 );
		glBindTexture( GL_TEXTURE_2D, 0 );
	} else {
		glBindTexture( GL_TEXTURE_2D, 0 );
	}
}

/*
============================================================================

SKINS

============================================================================
*/

/*
==================
CommaParse

This is unfortunate, but the skin files aren't
compatable with our normal parsing rules.
==================
*/
static const char *CommaParse( char **data_p ) {
	int c = 0, len;
	char *data;
	static char com_token[MAX_TOKEN_CHARS];

	data = *data_p;
	len = 0;
	com_token[0] = 0;

	// make sure incoming data is valid
	if ( !data ) {
		*data_p = NULL;
		return com_token;
	}

	while ( 1 ) {
		// skip whitespace
		while ( ( c = *data ) <= ' ' ) {
			if ( !c ) {
				break;
			}
			data++;
		}


		c = *data;

		// skip double slash comments
		if ( c == '/' && data[1] == '/' ) {
			while ( *data && *data != '\n' )
				data++;
		}
		// skip /* */ comments
		else if ( c == '/' && data[1] == '*' ) {
			while ( *data && ( *data != '*' || data[1] != '/' ) )
			{
				data++;
			}
			if ( *data ) {
				data += 2;
			}
		} else
		{
			break;
		}
	}

	if ( c == 0 ) {
		return "";
	}

	// handle quoted strings
	if ( c == '\"' ) {
		data++;
		while ( 1 )
		{
			c = *data++;
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
	} while ( c > 32 && c != ',' );

	if ( len == MAX_TOKEN_CHARS ) {
//		Com_Printf ("Token exceeded %i chars, discarded.\n", MAX_TOKEN_CHARS);
		len = 0;
	}
	com_token[len] = 0;

	*data_p = ( char * ) data;
	return com_token;
}


//----(SA) added so client can see what model or scale for the model was specified in a skin
/*
==============
RE_GetSkinModel
==============
*/
qboolean RE_GetSkinModel( qhandle_t skinid, const char *type, char *name ) {
	int i;

#if !defined RTCW_ET
	skin_t      *bar;

	bar = tr.skins[skinid];

	if ( !Q_stricmp( type, "playerscale" ) ) {    // client is requesting scale from the skin rather than a model
		Com_sprintf( name, MAX_QPATH, "%.2f %.2f %.2f", bar->scale[0], bar->scale[1], bar->scale[2] );
		return qtrue;
	}

	for ( i = 0; i < bar->numModels; i++ )
	{
		if ( !Q_stricmp( bar->models[i]->type, type ) ) { // (SA) whoops, should've been this way
			Q_strncpyz( name, bar->models[i]->model, sizeof( bar->models[i]->model ) );
#else
	int hash;
	skin_t  *skin;

	skin = tr.skins[skinid];
	hash = Com_HashKey( (char *)type, strlen( type ) );

	for ( i = 0; i < skin->numModels; i++ ) {
		if ( hash != skin->models[i]->hash ) {
			continue;
		}
		if ( !Q_stricmp( skin->models[i]->type, type ) ) {
			// (SA) whoops, should've been this way
			Q_strncpyz( name, skin->models[i]->model, sizeof( skin->models[i]->model ) );
#endif // RTCW_XX

			return qtrue;
		}
	}
	return qfalse;
}

/*
==============
RE_GetShaderFromModel
	return a shader index for a given model's surface
	'withlightmap' set to '0' will create a new shader that is a copy of the one found
	on the model, without the lighmap stage, if the shader has a lightmap stage

	NOTE: only works for bmodels right now.  Could modify for other models (md3's etc.)
==============
*/
qhandle_t RE_GetShaderFromModel( qhandle_t modelid, int surfnum, int withlightmap ) {
	model_t     *model;
	bmodel_t    *bmodel;
	msurface_t  *surf;
	shader_t    *shd;

	if ( surfnum < 0 ) {
		surfnum = 0;
	}

	model = R_GetModelByHandle( modelid );  // (SA) should be correct now

	if ( model ) {

#if !defined RTCW_ET
		bmodel  = model->bmodel;
#else
		bmodel  = model->model.bmodel;
#endif // RTCW_XX

		if ( bmodel && bmodel->firstSurface ) {
			if ( surfnum >= bmodel->numSurfaces ) { // if it's out of range, return the first surface
				surfnum = 0;
			}

			surf = bmodel->firstSurface + surfnum;

#if defined RTCW_ET
			// RF, check for null shader (can happen on func_explosive's with botclips attached)
			if ( !surf->shader ) {
				return 0;
			}
#endif // RTCW_XX

//			if(surf->shader->lightmapIndex != LIGHTMAP_NONE) {
			if ( surf->shader->lightmapIndex > LIGHTMAP_NONE ) {
				image_t *image;
				int32_t hash;
				qboolean mip = qtrue;   // mip generation on by default

				// get mipmap info for original texture
				hash = generateHashValue( surf->shader->name );
				for ( image = hashTable[hash]; image; image = image->next ) {
					if ( !strcmp( surf->shader->name, image->imgName ) ) {
						mip = image->mipmap;
						break;
					}
				}
				shd = R_FindShader( surf->shader->name, LIGHTMAP_NONE, mip );
				shd->stages[0]->rgbGen = CGEN_LIGHTING_DIFFUSE; // (SA) new
			} else {
				shd = surf->shader;
			}

			return shd->index;
		}
	}

	return 0;
}

//----(SA) end

/*
===============
RE_RegisterSkin

===============
*/
qhandle_t RE_RegisterSkin( const char *name ) {
	qhandle_t hSkin;
	skin_t      *skin;
	skinSurface_t   *surf;
	skinModel_t *model;          //----(SA) added
	char        *text, *text_p;
	const char        *token;
	char surfName[MAX_QPATH];

	if ( !name || !name[0] ) {
		Com_Printf( "Empty name passed to RE_RegisterSkin\n" );
		return 0;
	}

	if ( strlen( name ) >= MAX_QPATH ) {
		Com_Printf( "Skin name exceeds MAX_QPATH\n" );
		return 0;
	}


	// see if the skin is already loaded
	for ( hSkin = 1; hSkin < tr.numSkins ; hSkin++ ) {
		skin = tr.skins[hSkin];
		if ( !Q_stricmp( skin->name, name ) ) {
			if ( skin->numSurfaces == 0 ) {
				return 0;       // default skin
			}
			return hSkin;
		}
	}

	// allocate a new skin
	if ( tr.numSkins == MAX_SKINS ) {
		ri.Printf( PRINT_WARNING, "WARNING: RE_RegisterSkin( '%s' ) MAX_SKINS hit\n", name );
		return 0;
	}


//----(SA)	moved things around slightly to fix the problem where you restart
//			a map that has ai characters who had invalid skin names entered
//			in thier "skin" or "head" field

	// make sure the render thread is stopped
	R_SyncRenderThread();

#if !defined RTCW_ET
	// If not a .skin file, load as a single shader
	if ( strcmp( name + strlen( name ) - 5, ".skin" ) ) {
		tr.numSkins++;
		skin = static_cast<skin_t*> (ri.Hunk_Alloc( sizeof( skin_t ), h_low ));
		tr.skins[hSkin] = skin;
		Q_strncpyz( skin->name, name, sizeof( skin->name ) );
		skin->numSurfaces   = 0;
		skin->numModels     = 0;    //----(SA) added
		skin->numSurfaces = 1;
		skin->surfaces[0] = static_cast<skinSurface_t*> (ri.Hunk_Alloc( sizeof( skin->surfaces[0] ), h_low ));
		skin->surfaces[0]->shader = R_FindShader( name, LIGHTMAP_NONE, qtrue );
		return hSkin;
	}
#endif // RTCW_XX

	// load and parse the skin file
	ri.FS_ReadFile( name, (void **)&text );
	if ( !text ) {
		return 0;
	}

	tr.numSkins++;
	skin = static_cast<skin_t*> (ri.Hunk_Alloc( sizeof( skin_t ), h_low ));
	tr.skins[hSkin] = skin;
	Q_strncpyz( skin->name, name, sizeof( skin->name ) );
	skin->numSurfaces   = 0;
	skin->numModels     = 0;    //----(SA) added

//----(SA)	end

	text_p = text;
	while ( text_p && *text_p ) {
		// get surface name
		token = CommaParse( &text_p );
		Q_strncpyz( surfName, token, sizeof( surfName ) );

		if ( !token[0] ) {
			break;
		}
		// lowercase the surface name so skin compares are faster
		Q_strlwr( surfName );

		if ( *text_p == ',' ) {
			text_p++;
		}

#if !defined RTCW_ET
		if ( strstr( token, "tag_" ) ) {
#else
		if ( !Q_stricmpn( token, "tag_", 4 ) ) {
#endif // RTCW_XX

			continue;
		}

#if !defined RTCW_ET
		if ( strstr( token, "md3_" ) ) {  // this is specifying a model
#else
		if ( !Q_stricmpn( token, "md3_", 4 ) ) {
			// this is specifying a model
#endif // RTCW_XX

			model = skin->models[ skin->numModels ] = static_cast<skinModel_t*> (ri.Hunk_Alloc( sizeof( *skin->models[0] ), h_low ));
			Q_strncpyz( model->type, token, sizeof( model->type ) );

#if defined RTCW_ET
			model->hash = Com_HashKey( model->type, sizeof( model->type ) );
#endif // RTCW_XX

			// get the model name
			token = CommaParse( &text_p );

			Q_strncpyz( model->model, token, sizeof( model->model ) );

			skin->numModels++;
			continue;
		}

#if !defined RTCW_ET
//----(SA)	added
		if ( strstr( token, "playerscale" ) ) {
			token = CommaParse( &text_p );
			skin->scale[0] = atof( token );   // uniform scaling for now
			skin->scale[1] = atof( token );
			skin->scale[2] = atof( token );
			continue;
		}
//----(SA) end
#endif // RTCW_XX

		// parse the shader name
		token = CommaParse( &text_p );

		surf = skin->surfaces[ skin->numSurfaces ] = static_cast<skinSurface_t*> (ri.Hunk_Alloc( sizeof( *skin->surfaces[0] ), h_low ));
		Q_strncpyz( surf->name, surfName, sizeof( surf->name ) );

#if defined RTCW_ET
		surf->hash = Com_HashKey( surf->name, sizeof( surf->name ) );
#endif // RTCW_XX

		surf->shader = R_FindShader( token, LIGHTMAP_NONE, qtrue );
		skin->numSurfaces++;
	}

	ri.FS_FreeFile( text );


	// never let a skin have 0 shaders

#if !defined RTCW_ET
	//----(SA)	allow this for the (current) special case of the loper's upper body
	//			(it's upper body has no surfaces, only tags)
#endif // RTCW_XX

	if ( skin->numSurfaces == 0 ) {

#if !defined RTCW_ET
		if ( !( strstr( name, "loper" ) && strstr( name, "upper" ) ) ) {
#endif // RTCW_XX

			return 0;       // use default skin
		}

#if !defined RTCW_ET
	}
#endif // RTCW_XX

	return hSkin;
}


/*
===============
R_InitSkins
===============
*/
void    R_InitSkins( void ) {
	skin_t      *skin;

	tr.numSkins = 1;

	// make the default skin have all default shaders
	skin = tr.skins[0] = static_cast<skin_t*> (ri.Hunk_Alloc( sizeof( skin_t ), h_low ));
	Q_strncpyz( skin->name, "<default skin>", sizeof( skin->name )  );
	skin->numSurfaces = 1;
	skin->surfaces[0] = static_cast<skinSurface_t*> (ri.Hunk_Alloc( sizeof( *skin->surfaces ), h_low ));
	skin->surfaces[0]->shader = tr.defaultShader;
}

/*
===============
R_GetSkinByHandle
===============
*/
skin_t  *R_GetSkinByHandle( qhandle_t hSkin ) {
	if ( hSkin < 1 || hSkin >= tr.numSkins ) {
		return tr.skins[0];
	}
	return tr.skins[ hSkin ];
}

/*
===============
R_SkinList_f
===============
*/
void    R_SkinList_f( void ) {
	int i, j;
	skin_t      *skin;

	ri.Printf( PRINT_ALL, "------------------\n" );

	for ( i = 0 ; i < tr.numSkins ; i++ ) {
		skin = tr.skins[i];

		ri.Printf( PRINT_ALL, "%3i:%s\n", i, skin->name );
		for ( j = 0 ; j < skin->numSurfaces ; j++ ) {
			ri.Printf( PRINT_ALL, "       %s = %s\n",
					   skin->surfaces[j]->name, skin->surfaces[j]->shader->name );
		}
	}
	ri.Printf( PRINT_ALL, "------------------\n" );
}

// Ridah, utility for automatically cropping and numbering a bunch of images in a directory
/*
=============
SaveTGA

  saves out to 24 bit uncompressed format (no alpha)
=============
*/
void SaveTGA( char *name, byte **pic, int width, int height ) {
	byte    *inpixel, *outpixel;
	byte    *outbuf, *b;

	outbuf = static_cast<byte*> (ri.Hunk_AllocateTempMemory( width * height * 4 + 18 ));
	b = outbuf;

	memset( b, 0, 18 );
	b[2] = 2;       // uncompressed type
	b[12] = width & 255;
	b[13] = width >> 8;
	b[14] = height & 255;
	b[15] = height >> 8;
	b[16] = 24; // pixel size

	{
		int row, col;
		int rows, cols;

		rows = ( height );
		cols = ( width );

		outpixel = b + 18;

		for ( row = ( rows - 1 ); row >= 0; row-- )
		{
			inpixel = ( ( *pic ) + ( row * cols ) * 4 );

			for ( col = 0; col < cols; col++ )
			{
				*outpixel++ = *( inpixel + 2 );   // blue
				*outpixel++ = *( inpixel + 1 );   // green
				*outpixel++ = *( inpixel + 0 );   // red
				//*outpixel++ = *(inpixel + 3);	// alpha

				inpixel += 4;
			}
		}
	}

	ri.FS_WriteFile( name, outbuf, (int)( outpixel - outbuf ) );

	ri.Hunk_FreeTempMemory( outbuf );

}

/*
=============
SaveTGAAlpha

  saves out to 32 bit uncompressed format (with alpha)
=============
*/
void SaveTGAAlpha( char *name, byte **pic, int width, int height ) {
	byte    *inpixel, *outpixel;
	byte    *outbuf, *b;

	outbuf = static_cast<byte*> (ri.Hunk_AllocateTempMemory( width * height * 4 + 18 ));
	b = outbuf;

	memset( b, 0, 18 );
	b[2] = 2;       // uncompressed type
	b[12] = width & 255;
	b[13] = width >> 8;
	b[14] = height & 255;
	b[15] = height >> 8;
	b[16] = 32; // pixel size

	{
		int row, col;
		int rows, cols;

		rows = ( height );
		cols = ( width );

		outpixel = b + 18;

		for ( row = ( rows - 1 ); row >= 0; row-- )
		{
			inpixel = ( ( *pic ) + ( row * cols ) * 4 );

			for ( col = 0; col < cols; col++ )
			{
				*outpixel++ = *( inpixel + 2 );   // blue
				*outpixel++ = *( inpixel + 1 );   // green
				*outpixel++ = *( inpixel + 0 );   // red
				*outpixel++ = *( inpixel + 3 );   // alpha

				inpixel += 4;
			}
		}
	}

	ri.FS_WriteFile( name, outbuf, (int)( outpixel - outbuf ) );

	ri.Hunk_FreeTempMemory( outbuf );

}

/*
==============
R_CropImage
==============
*/
#define CROPIMAGES_ENABLED
//#define FUNNEL_HACK
#define RESIZE
//#define QUICKTIME_BANNER
#define TWILTB2_HACK

qboolean R_CropImage( char *name, byte **pic, int border, int *width, int *height, int lastBox[2] ) {

#if !defined RTCW_ET
#ifdef CROPIMAGES_ENABLED
	int row, col;
	int rows, cols;
	byte    *inpixel, *temppic, *outpixel;
	int mins[2], maxs[2];
	int diff[2];
	//int	newWidth;
	int /*a, max,*/ i;
	int alpha;
	//int	*center;
	qboolean /*invalid,*/ skip;
	vec3_t fCol, fScale;
	qboolean filterColors = qfalse;
	int fCount;
	float f,c;
#define FADE_BORDER_RANGE   ( ( *width ) / 40 )

	rows = ( *height );
	cols = ( *width );

	mins[0] = 99999;
	mins[1] = 99999;
	maxs[0] = -99999;
	maxs[1] = -99999;

#ifdef TWILTB2_HACK
	// find the filter color (use first few pixels)
	filterColors = qtrue;
	inpixel = ( *pic );
	VectorClear( fCol );
	for ( fCount = 0; fCount < 8; fCount++, inpixel += 4 ) {
		for ( i = 0; i < 3; i++ ) {
			if ( *( inpixel + i ) > 70 ) {
				continue;   // too bright, cant be noise
			}
			if ( fCol[i] < *( inpixel + i ) ) {
				fCol[i] = *( inpixel + i );
			}
		}
	}
	//VectorScale( fCol, 1.0/fCount, fCol );
	for ( i = 0; i < 3; i++ ) {
		fCol[i] += 4;
		fScale[i] = 255.0 / ( 255.0 - fCol[i] );
	}
#endif

	for ( row = 0; row < rows; row++ )
	{
		inpixel = ( ( *pic ) + ( row * cols ) * 4 );

		for ( col = 0; col < cols; col++, inpixel += 4 )
		{
			if ( filterColors ) {
				// special code for filtering the twiltb series
				for ( i = 0; i < 3; i++ ) {
					f = *( inpixel + i );
					f -= fCol[i];
					if ( f <= 0 ) {
						f = 0;
					} else { f *= fScale[i];}
					if ( f > 255 ) {
						f = 255;
					}
					*( inpixel + i ) = f;
				}
				// if this pixel is near the edge, then fade it out (just do a brightness fade)
				if ( (int *)inpixel ) {
					// calc the fade scale
					f = (float)row / FADE_BORDER_RANGE;
					if ( f > (float)( rows - row - 1 ) / FADE_BORDER_RANGE ) {
						f = (float)( rows - row - 1 ) / FADE_BORDER_RANGE;
					}
					if ( f > (float)( cols - col - 1 ) / FADE_BORDER_RANGE ) {
						f = (float)( cols - col - 1 ) / FADE_BORDER_RANGE;
					}
					if ( f > (float)( col ) / FADE_BORDER_RANGE ) {
						f = (float)( col ) / FADE_BORDER_RANGE;
					}
					if ( f < 1.0 ) {
						if ( f <= 0 ) {
							*( (int *)inpixel ) = 0;
						} else {
							f += 0.2 * crandom();
							if ( f < 1.0 ) {
								if ( f <= 0 ) {
									*( (int *)inpixel ) = 0;
								} else {
									f = 1.0 - f;
									for ( i = 0; i < 3; i++ ) {
										c = *( inpixel + i );
										c -= f * c;
										if ( c < 0 ) {
											c = 0;
										}
										*( inpixel + i ) = c;
									}
								}
							}
						}
					}
				}
				continue;
			}

			skip = qfalse;
#ifdef QUICKTIME_BANNER
			// hack for quicktime ripped images
			if ( ( col > cols - 3 || row > rows - 36 ) ) {
				// filter this pixel out
				*( inpixel + 0 ) = 0;
				*( inpixel + 1 ) = 0;
				*( inpixel + 2 ) = 0;
				skip = qtrue;
			}
#endif

			if ( !skip ) {
				if (    ( *( inpixel + 0 ) > 20 )       // blue
						||  ( *( inpixel + 1 ) > 20 ) // green
						||  ( *( inpixel + 2 ) > 20 ) ) { // red

					if ( col < mins[0] ) {
						mins[0] = col;
					}
					if ( col > maxs[0] ) {
						maxs[0] = col;
					}
					if ( row < mins[1] ) {
						mins[1] = row;
					}
					if ( row > maxs[1] ) {
						maxs[1] = row;
					}

				} else {
					// filter this pixel out
					*( inpixel + 0 ) = 0;
					*( inpixel + 1 ) = 0;
					*( inpixel + 2 ) = 0;
				}
			}

#ifdef FUNNEL_HACK  // scale brightness down
			for ( i = 0; i < 3; i++ ) {
				alpha = *( inpixel + i );
				if ( ( alpha -= 20 ) < 0 ) {
					alpha = 0;
				}
				*( inpixel + i ) = alpha;
			}
#endif

			// set the alpha component
			alpha = *( inpixel + 0 ); // + *(inpixel + 1) + *(inpixel + 2);
			if ( *( inpixel + 1 ) > alpha ) {
				alpha = *( inpixel + 1 );
			}
			if ( *( inpixel + 2 ) > alpha ) {
				alpha = *( inpixel + 2 );
			}
			//alpha = (int)((float)alpha / 3.0);
			//alpha /= 3;
			if ( alpha > 255 ) {
				alpha = 255;
			}
			*( inpixel + 3 ) = (byte)alpha;
		}
	}

#ifdef RESIZE
	return qtrue;
#endif

	// convert it so that the center is the center of the image
	// this is used for some explosions
	/*
	for (i=0; i<2; i++) {
		if (i==0)	center = width;
		else		center = height;

		if ((*center/2 - mins[i]) > (maxs[i] - *center/2)) {
			maxs[i] = *center/2 + (*center/2 - mins[i]);
		} else {
			mins[i] = *center/2 - (maxs[i] - *center/2);
		}
	}
	*/

	// HACK for funnel
#ifdef FUNNEL_HACK
	mins[0] = 210;
	maxs[0] = 430;
	mins[1] = 0;
	maxs[1] = *height - 1;

	for ( i = 0; i < 2; i++ ) {
		diff[i] = maxs[i] - mins[i];
	}
#else

#ifndef RESIZE
	// apply the border
	for ( i = 0; i < 2; i++ ) {
		mins[i] -= border;
		if ( mins[i] < 0 ) {
			mins[i] = 0;
		}
		maxs[i] += border;
		if ( i == 0 ) {
			if ( maxs[i] > *width - 1 ) {
				maxs[i] = *width - 1;
			}
		} else {
			if ( maxs[i] > *height - 1 ) {
				maxs[i] = *height - 1;
			}
		}
	}

	// we have the mins/maxs, so work out the best square to crop to
	for ( i = 0; i < 2; i++ ) {
		diff[i] = maxs[i] - mins[i];
	}

	// widen the axis that has the smallest diff
	a = -1;
	if ( diff[1] > diff[0] ) {
		a = 0;
		max = *width - 1;
	} else if ( diff[0] > diff[1] ) {
		a = 1;
		max = *height - 1;
	}
	if ( a >= 0 ) {
		invalid = qfalse;
		while ( diff[a] < diff[!a] ) {
			if ( invalid ) {
				Com_Printf( "unable to find a good crop size\n" );
				return qfalse;
			}
			invalid = qtrue;
			if ( mins[a] > 0 ) {
				mins[a] -= 1;
				diff[a] = maxs[a] - mins[a];
				invalid = qfalse;
			}
			if ( ( diff[a] < diff[!a] ) && ( maxs[a] < max ) ) {
				maxs[a] += 1;
				diff[a] = maxs[a] - mins[a];
				invalid = qfalse;
			}
		}
	}

	// make sure it's bigger or equal to the last one
	for ( i = 0; i < 2; i++ ) {
		if ( ( maxs[i] - mins[i] ) < lastBox[i] ) {
			if ( i == 0 ) {
				center = width;
			} else { center = height;}

			maxs[i] = *center / 2 + ( lastBox[i] / 2 );
			mins[i] = maxs[i] - lastBox[i];
			diff[i] = lastBox[i];
		}
	}
#else
	for ( i = 0; i < 2; i++ ) {
		diff[i] = maxs[i] - mins[i];
		lastBox[i] = diff[i];
	}
#endif  // RESIZE
#endif  // FUNNEL_HACK

#if !defined RTCW_MP
	temppic = static_cast<byte*> (malloc( sizeof( unsigned int ) * diff[0] * diff[1] ));
#else
	temppic = static_cast<byte*> (ri.Z_Malloc( sizeof( unsigned int ) * diff[0] * diff[1] ));
#endif // RTCW_XX

	outpixel = temppic;

	for ( row = mins[1]; row < maxs[1]; row++ )
	{
		inpixel = ( ( *pic ) + ( row * cols ) * 4 );
		inpixel += mins[0] * 4;

		for ( col = mins[0]; col < maxs[0]; col++ )
		{
			*outpixel++ = *( inpixel + 0 );   // blue
			*outpixel++ = *( inpixel + 1 );   // green
			*outpixel++ = *( inpixel + 2 );   // red
			*outpixel++ = *( inpixel + 3 );   // alpha

			inpixel += 4;
		}
	}

	// for some reason this causes memory drop, not worth investigating (dev command only)
	//ri.Free( *pic );

	*pic = temppic;

	*width = diff[0];
	*height = diff[1];

	return qtrue;
#else
	return qtrue;   // shutup the compiler
#endif
#else
	return qtrue;   // shutup the compiler
#endif // RTCW_XX

}


/*
===============
R_CropAndNumberImagesInDirectory
===============
*/
void    R_CropAndNumberImagesInDirectory( const char *dir, const char *ext, int maxWidth, int maxHeight, int withAlpha ) {

#if !defined RTCW_ET
#ifdef CROPIMAGES_ENABLED
	char    **fileList;
	int numFiles, j;
#ifndef RESIZE
	int i;
#endif
	byte    *pic, *temppic;
	int width, height, newWidth, newHeight;
	char    *pch;
	int b,c,d,lastNumber;
	int lastBox[2] = {0,0};

	fileList = ri.FS_ListFiles( dir, ext, &numFiles );

	if ( !numFiles ) {
		ri.Printf( PRINT_ALL, "no '%s' files in directory '%s'\n", ext, dir );
		return;
	}

	ri.Printf( PRINT_ALL, "%i files found, beginning processing..\n", numFiles );

	for ( j = 0; j < numFiles; j++ ) {
		char filename[MAX_QPATH], outfilename[MAX_QPATH];

		if ( !Q_strncmp( fileList[j], "spr", 3 ) ) {
			continue;
		}

		Com_sprintf( filename, sizeof( filename ), "%s/%s", dir, fileList[j] );
		ri.Printf( PRINT_ALL, "...cropping '%s'.. ", filename );

		R_LoadImage( filename, &pic, &width, &height );
		if ( !pic ) {
			ri.Printf( PRINT_ALL, "error reading file, ignoring.\n" );
			continue;
		}

		// file has been read, crop it, resize it down to a power of 2, then save
		if ( !R_CropImage( filename, &pic, 6, &width, &height, lastBox ) ) {
			ri.Printf( PRINT_ALL, "unable to crop image.\n" );
			//ri.Free( pic );
			break;
		}
#ifndef RESIZE
		for ( i = 2; ( 1 << i ) <= maxWidth; i++ ) {
			if ( ( width < ( 1 << i ) ) && ( width > ( 1 << ( i - 1 ) ) ) ) {
				newWidth = ( 1 << ( i - 1 ) );
				if ( newWidth > maxWidth ) {
					newWidth = maxWidth;
				}
				newHeight = newWidth;

#if defined RTCW_SP
				temppic = malloc( sizeof( unsigned int ) * newWidth * newHeight );
#else
				temppic = ri.Z_Malloc( sizeof( unsigned int ) * newWidth * newHeight );
#endif // RTCW_XX

				ResampleTexture( (unsigned int *)pic, width, height, (unsigned int *)temppic, newWidth, newHeight );
				memcpy( pic, temppic, sizeof( unsigned int ) * newWidth * newHeight );
				ri.Free( temppic );
				width = height = newWidth;
				break;
			}
		}
		if ( width > maxWidth ) {
			// we need to force the scale downwards
			newWidth = maxWidth;
			newHeight = maxWidth;

#if defined RTCW_SP
			temppic = malloc( sizeof( unsigned int ) * newWidth * newHeight );
#else
			temppic = ri.Z_Malloc( sizeof( unsigned int ) * newWidth * newHeight );
#endif // RTCW_XX

			ResampleTexture( (unsigned int *)pic, width, height, (unsigned int *)temppic, newWidth, newHeight );
			memcpy( pic, temppic, sizeof( unsigned int ) * newWidth * newHeight );
			ri.Free( temppic );
			width = newWidth;
			height = newHeight;
		}
#else
		newWidth = maxWidth;
		newHeight = maxHeight;

#if !defined RTCW_MP
		temppic = static_cast<byte*> (malloc( sizeof( unsigned int ) * newWidth * newHeight ));
#else
		temppic = static_cast<byte*> (ri.Z_Malloc( sizeof( unsigned int ) * newWidth * newHeight ));
#endif // RTCW_XX

		ResampleTexture( (unsigned int *)pic, width, height, (unsigned int *)temppic, newWidth, newHeight );
		memcpy( pic, temppic, sizeof( unsigned int ) * newWidth * newHeight );

#if !defined RTCW_MP
		free( temppic );
#else
		ri.Free( temppic );
#endif // RTCW_XX

		width = newWidth;
		height = newHeight;
#endif

		// set the new filename
		pch = strrchr( filename, '/' );
		*pch = '\0';
		lastNumber = j;
		b = lastNumber / 100;
		lastNumber -= b * 100;
		c = lastNumber / 10;
		lastNumber -= c * 10;
		d = lastNumber;
		Com_sprintf( outfilename, sizeof( outfilename ), "%s/spr%i%i%i.tga", filename, b, c, d );

		if ( withAlpha ) {
			SaveTGAAlpha( outfilename, &pic, width, height );
		} else {
			SaveTGA( outfilename, &pic, width, height );
		}

		// free the pixel data
		//ri.Free( pic );

		ri.Printf( PRINT_ALL, "done.\n" );
	}
#endif
#endif // RTCW_XX

}

/*
==============
R_CropImages_f
==============
*/
void R_CropImages_f( void ) {

#if !defined RTCW_ET
#ifdef CROPIMAGES_ENABLED
	if ( ri.Cmd_Argc() < 5 ) {
		ri.Printf( PRINT_ALL, "syntax: cropimages <dir> <extension> <maxWidth> <maxHeight> <alpha 0/1>\neg: 'cropimages sprites/fire1 .tga 64 64 0'\n" );
		return;
	}
	R_CropAndNumberImagesInDirectory( ri.Cmd_Argv( 1 ), ri.Cmd_Argv( 2 ), atoi( ri.Cmd_Argv( 3 ) ), atoi( ri.Cmd_Argv( 4 ) ), atoi( ri.Cmd_Argv( 5 ) ) );
#else
	ri.Printf( PRINT_ALL, "This command has been disabled.\n" );
#endif
#endif // RTCW_XX

}
// done.

//==========================================================================================
// Ridah, caching system

static int numBackupImages = 0;
static image_t  *backupHashTable[FILE_HASH_SIZE];

#if !defined RTCW_ET
static image_t  *texnumImages[MAX_DRAWIMAGES * 2];
#else
//%	static image_t	*texnumImages[MAX_DRAWIMAGES*2];
#endif // RTCW_XX

/*
===============
R_CacheImageAlloc

  this will only get called to allocate the image_t structures, not that actual image pixels
===============
*/
void *R_CacheImageAlloc( int size ) {
	if ( r_cache->integer && r_cacheShaders->integer ) {

#if !defined RTCW_ET
		return malloc( size );
#if defined RTCW_SP
		//return ri.Z_Malloc( size );
#else
//DAJ TEST		return ri.Z_Malloc( size );	//DAJ was CO
#endif // RTCW_XX
#else
//		return ri.Z_Malloc( size );
		return malloc( size );  // ri.Z_Malloc causes load times about twice as long?... Gordon
//DAJ TEST		return ri.Z_Malloc( size );	//DAJ was CO
#endif // RTCW_XX

	} else {
		return ri.Hunk_Alloc( size, h_low );
	}
}

/*
===============
R_CacheImageFree
===============
*/
void R_CacheImageFree( void *ptr ) {
	if ( r_cache->integer && r_cacheShaders->integer ) {

#if defined RTCW_ET
//		ri.Free( ptr );
#endif // RTCW_XX

		free( ptr );

#if defined RTCW_SP
		//ri.Free( ptr );
#else
//DAJ TEST		ri.Free( ptr );	//DAJ was CO
#endif // RTCW_XX

	}
}

/*
===============
R_TouchImage

  remove this image from the backupHashTable and make sure it doesn't get overwritten
===============
*/
qboolean R_TouchImage( image_t *inImage ) {
	image_t *bImage, *bImagePrev;
	int hash;
	char *name;

	if ( inImage == tr.dlightImage ||
		 inImage == tr.whiteImage ||
		 inImage == tr.defaultImage ||
		 inImage->imgName[0] == '*' ) { // can't use lightmaps since they might have the same name, but different maps will have different actual lightmap pixels
		return qfalse;
	}

	hash = inImage->hash;
	name = inImage->imgName;

	bImage = backupHashTable[hash];
	bImagePrev = NULL;
	while ( bImage ) {

		if ( bImage == inImage ) {
			// add it to the current images
			if ( tr.numImages == MAX_DRAWIMAGES ) {
				ri.Error( ERR_DROP, "R_CreateImage: MAX_DRAWIMAGES hit\n" );
			}

			tr.images[tr.numImages] = bImage;

			// remove it from the backupHashTable
			if ( bImagePrev ) {
				bImagePrev->next = bImage->next;
			} else {
				backupHashTable[hash] = bImage->next;
			}

			// add it to the hashTable
			bImage->next = hashTable[hash];
			hashTable[hash] = bImage;

			// get the new texture
			tr.numImages++;

			return qtrue;
		}

		bImagePrev = bImage;
		bImage = bImage->next;
	}

	return qtrue;
}

/*
===============
R_PurgeImage
===============
*/
void R_PurgeImage( image_t *image ) {
	glDeleteTextures( 1, &image->texnum );
	image->texnum = 0;

	R_CacheImageFree( image );

	memset( glState.currenttextures, 0, sizeof( glState.currenttextures ) );

	if ( glConfigEx.use_arb_multitexture_ ) {
		GL_SelectTexture( 1 );
		glBindTexture( GL_TEXTURE_2D, 0 );
		GL_SelectTexture( 0 );
		glBindTexture( GL_TEXTURE_2D, 0 );
	} else {
		glBindTexture( GL_TEXTURE_2D, 0 );
	}
}


/*
===============
R_PurgeBackupImages

  Can specify the number of Images to purge this call (used for background purging)
===============
*/
void R_PurgeBackupImages( int purgeCount ) {
	int i, cnt;
	static int lastPurged = 0;
	image_t *image;

	if ( !numBackupImages ) {
		// nothing to purge
		lastPurged = 0;
		return;
	}

	R_SyncRenderThread();

	cnt = 0;
	for ( i = lastPurged; i < FILE_HASH_SIZE; ) {
		lastPurged = i;
		// TTimo: assignment used as truth value
		if ( ( image = backupHashTable[i] ) ) {
			// kill it
			backupHashTable[i] = image->next;
			R_PurgeImage( image );
			cnt++;

			if ( cnt >= purgeCount ) {
				return;
			}
		} else {
			i++;    // no images in this slot, so move to the next one
		}
	}

	// all done
	numBackupImages = 0;
	lastPurged = 0;
}

/*
===============
R_BackupImages
===============
*/
void R_BackupImages( void ) {

	if ( !r_cache->integer ) {
		return;
	}
	if ( !r_cacheShaders->integer ) {
		return;
	}

	// backup the hashTable
	memcpy( backupHashTable, hashTable, sizeof( backupHashTable ) );

	// pretend we have cleared the list
	numBackupImages = tr.numImages;
	tr.numImages = 0;

	memset( glState.currenttextures, 0, sizeof( glState.currenttextures ) );

	if ( glConfigEx.use_arb_multitexture_ ) {
		GL_SelectTexture( 1 );
		glBindTexture( GL_TEXTURE_2D, 0 );
		GL_SelectTexture( 0 );
		glBindTexture( GL_TEXTURE_2D, 0 );
	} else {
		glBindTexture( GL_TEXTURE_2D, 0 );
	}
}

/*
=============
R_FindCachedImage
=============
*/
image_t *R_FindCachedImage( const char *name, int hash ) {
	image_t *bImage, *bImagePrev;

	if ( !r_cacheShaders->integer ) {
		return NULL;
	}

	if ( !numBackupImages ) {
		return NULL;
	}

	bImage = backupHashTable[hash];
	bImagePrev = NULL;
	while ( bImage ) {

		if ( !Q_stricmp( name, bImage->imgName ) ) {
			// add it to the current images
			if ( tr.numImages == MAX_DRAWIMAGES ) {
				ri.Error( ERR_DROP, "R_CreateImage: MAX_DRAWIMAGES hit\n" );
			}

			R_TouchImage( bImage );
			return bImage;
		}

		bImagePrev = bImage;
		bImage = bImage->next;
	}

	return NULL;
}

#if defined RTCW_ET
//bani
/*
R_GetTextureId
*/
int R_GetTextureId( const char *name ) {
	int i;

//	ri.Printf( PRINT_ALL, "R_GetTextureId [%s].\n", name );

	for ( i = 0 ; i < tr.numImages ; i++ ) {
		if ( !strcmp( name, tr.images[ i ]->imgName ) ) {
//			ri.Printf( PRINT_ALL, "Found textureid %d\n", i );
			return i;
		}
	}

//	ri.Printf( PRINT_ALL, "Image not found.\n" );
	return -1;
}

// ydnar: glGenTextures, sir...
#endif // RTCW_XX

// BBi
//#if !defined RTCW_ET
///*
//===============
//R_InitTexnumImages
//===============
//*/
//static int last_i;
//void R_InitTexnumImages( qboolean force ) {
//	if ( force || !numBackupImages ) {
//		memset( texnumImages, 0, sizeof( texnumImages ) );
//		last_i = 0;
//	}
//}
//
///*
//============
//R_FindFreeTexnum
//============
//*/
//
//void R_FindFreeTexnum( image_t *inImage ) {
//	int i, max;
//	image_t **image;
//
//	max = ( MAX_DRAWIMAGES * 2 );
//	if ( last_i && !texnumImages[last_i + 1] ) {
//		i = last_i + 1;
//	} else {
//		i = 0;
//		image = (image_t **)&texnumImages;
//		while ( i < max && *( image++ ) ) {
//			i++;
//		}
//	}
//
//	if ( i < max ) {
//		if ( i < max - 1 ) {
//			last_i = i;
//		} else {
//			last_i = 0;
//		}
//		inImage->texnum = 1024 + i;
//		texnumImages[i] = inImage;
//	} else {
//		ri.Error( ERR_DROP, "R_FindFreeTexnum: MAX_DRAWIMAGES hit\n" );
//	}
//}
//#endif // RTCW_XX
// BBi

/*
===============
R_LoadCacheImages
===============
*/
void R_LoadCacheImages( void ) {
	int len;
	byte *buf;
	char    *token;
	const char* pString;
	char name[MAX_QPATH];

#if defined RTCW_MP
	int parms[3], i;
#else
	int parms[4], i;
#endif // RTCW_XX

	if ( numBackupImages ) {
		return;
	}

	len = ri.FS_ReadFile( "image.cache", NULL );

	if ( len <= 0 ) {
		return;
	}

	buf = (byte *)ri.Hunk_AllocateTempMemory( len );
	ri.FS_ReadFile( "image.cache", (void **)&buf );

	pString = reinterpret_cast<const char*> (buf); //DAJ added (char*)

	while ( ( token = COM_ParseExt( &pString, qtrue ) ) && token[0] ) {
		Q_strncpyz( name, token, sizeof( name ) );

#if !defined RTCW_MP
		for ( i = 0; i < 4; i++ ) {
#else
		for ( i = 0; i < 3; i++ ) {
#endif // RTCW_XX

			token = COM_ParseExt( &pString, qfalse );
			parms[i] = atoi( token );
		}

#if defined RTCW_SP
		R_FindImageFileExt( name, parms[0], parms[1], parms[2], parms[3] );
#elif defined RTCW_MP
		R_FindImageFile( name, parms[0], parms[1], parms[2] );
#else
		R_FindImageFile( name, parms[0], parms[1], parms[2], parms[3] );
#endif // RTCW_XX

	}

	ri.Hunk_FreeTempMemory( buf );
}
// done.
//==========================================================================================

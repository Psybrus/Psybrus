/**************************************************************************
*
* File:		RsTypes.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Generic Shared Types
*		
*
*
* 
**************************************************************************/

#include "RsTypes.h"

//////////////////////////////////////////////////////////////////////////
// RsColour Statics
const RsColour RsColour::WHITE =		RsColour( 1.0f, 1.0f, 1.0f, 1.0f );
const RsColour RsColour::BLACK =		RsColour( 0.0f, 0.0f, 0.0f, 1.0f );
const RsColour RsColour::GRAY =			RsColour( 0.5f, 0.5f, 0.5f, 1.0f );
const RsColour RsColour::RED =			RsColour( 1.0f, 0.0f, 0.0f, 1.0f );
const RsColour RsColour::GREEN =		RsColour( 0.0f, 1.0f, 0.0f, 1.0f );
const RsColour RsColour::BLUE =			RsColour( 0.0f, 0.0f, 1.0f, 1.0f );
const RsColour RsColour::YELLOW =		RsColour( 1.0f, 1.0f, 0.0f, 1.0f );
const RsColour RsColour::MAGENTA =		RsColour( 1.0f, 0.0f, 1.0f, 1.0f );
const RsColour RsColour::CYAN =			RsColour( 0.0f, 1.0f, 1.0f, 1.0f );

//////////////////////////////////////////////////////////////////////////
// premultiplyAlpha
void RsColour::premultiplyAlpha()
{
	X_ = X_ * W_;
	Y_ = Y_ * W_;
	Z_ = Z_ * W_;
}

////////////////////////////////////////////////////////////////////////////////
// RsTextureFormatBPP
BcU32 RsTextureFormatBPP( eRsTextureFormat TextureFormat )
{
	switch( TextureFormat )
	{
		case rsTF_RGBA8:
			return 32;
			break;
		case rsTF_PAL4_RGBA8:
			return 4;
			break;
		case rsTF_PAL8_RGBA8:
			return 8;
			break;
		case rsTF_DXT1:
		case rsTF_DXT2:
		case rsTF_DXT3:
		case rsTF_DXT4:
		case rsTF_DXT5:			
			return 4;
			break;
		case rsTF_PVRTC2_RGB:
		case rsTF_PVRTC2_RGBA:
			return 2;
			break;
		case rsTF_PVRTC4_RGB:
		case rsTF_PVRTC4_RGBA:
			return 4;
			break;
		default:
			return 0;
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// RsTextureFormatSize
BcU32 RsTextureFormatSize( eRsTextureFormat TextureFormat, BcU32 Width, BcU32 Height, BcU32 Levels )
{
	BcU32 Size = 0;
	switch( TextureFormat )
	{
		case rsTF_RGBA8:
			for( BcU32 iLevel = 0; iLevel < Levels; ++iLevel )
			{
				Size += Width * Height;
				Width >>= 1;
				Height >>= 1;
			}
			Size *= 4;
			break;
		
		case rsTF_PAL4_RGBA8:
			for( BcU32 iLevel = 0; iLevel < Levels; ++iLevel )
			{
				Size += Width * Height;
				Width >>= 1;
				Height >>= 1;
			}
			Size = ( Size >> 1 ) + 64;
			break;
						
		case rsTF_PAL8_RGBA8:
			for( BcU32 iLevel = 0; iLevel < Levels; ++iLevel )
			{
				Size += Width * Height;
				Width >>= 1;
				Height >>= 1;
			}
			Size = Size + 1024;
			break;

		case rsTF_DXT1:
		case rsTF_DXT2:
		case rsTF_DXT3:
		case rsTF_DXT4:
		case rsTF_DXT5:			
			for( BcU32 iLevel = 0; iLevel < Levels; ++iLevel )
			{
				Size += Width * Height;
				Width >>= 1;
				Height >>= 1;
			}
			Size >>= 1;
			break;
			
		case rsTF_PVRTC2_RGB:
		case rsTF_PVRTC2_RGBA:
			for( BcU32 iLevel = 0; iLevel < Levels; ++iLevel )
			{
				Size += Width * Height;
				Width >>= 1;
				Height >>= 1;
			}
			Size >>= 2;			
			break;
			
		case rsTF_PVRTC4_RGB:
		case rsTF_PVRTC4_RGBA:
			for( BcU32 iLevel = 0; iLevel < Levels; ++iLevel )
			{
				Size += Width * Height;
				Width >>= 1;
				Height >>= 1;
			}
			Size >>= 1;			
			break;
			
		default:
			break;
	}
	
	return Size;
}

////////////////////////////////////////////////////////////////////////////////
// RsVertexDeclSize
BcU32 RsVertexDeclSize( BcU32 VertexFormat )
{
	BcU32 Size = 0;
#define RS_DECL_APPEND_SIZE( flag, size ) \
	if( VertexFormat & flag ) \
	Size += size

	RS_DECL_APPEND_SIZE( rsVDF_POSITION_XY,		8 );
	RS_DECL_APPEND_SIZE( rsVDF_POSITION_XYZ,	12 );
	RS_DECL_APPEND_SIZE( rsVDF_POSITION_XYZW,	16 );
	RS_DECL_APPEND_SIZE( rsVDF_NORMAL_XYZ,		12 );
	RS_DECL_APPEND_SIZE( rsVDF_TANGENT_XYZ,		12 );
	RS_DECL_APPEND_SIZE( rsVDF_TEXCOORD_UV0,	8 );
	RS_DECL_APPEND_SIZE( rsVDF_TEXCOORD_UV1,	8 );
	RS_DECL_APPEND_SIZE( rsVDF_TEXCOORD_UV2,	8 );
	RS_DECL_APPEND_SIZE( rsVDF_TEXCOORD_UV3,	8 );
	RS_DECL_APPEND_SIZE( rsVDF_COLOUR_RGBA8,	4 );
#undef RS_DECL_APPEND_SIZE
	return Size;
}

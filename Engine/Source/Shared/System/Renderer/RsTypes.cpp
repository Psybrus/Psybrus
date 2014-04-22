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

#include "System/Renderer/RsTypes.h"

#include "Base/BcMath.h"

#include <boost/format.hpp>

void RsColour::StaticRegisterClass()
{
	class RsColourSerialiser:
		public ReClassSerialiser_ComplexType< RsColour >
	{
	public:
		RsColourSerialiser( BcName Name ): 
			ReClassSerialiser_ComplexType< RsColour >( Name )
		{}

		virtual BcBool serialiseToString( const void* pInstance, std::string& OutString ) const
		{
			const RsColour& Vec = *reinterpret_cast< const RsColour* >( pInstance );
			OutString = boost::str( boost::format( "%1%, %2%, %3%, %4%" ) % Vec.x() % Vec.y() % Vec.z() % Vec.w() );
			return true;
		}

		virtual BcBool serialiseFromString( void* pInstance, const std::string& InString ) const
		{
			RsColour& Vec = *reinterpret_cast< RsColour* >( pInstance );
			Vec = RsColour( InString.c_str() );
			return true;
		}

		virtual BcBool serialiseToBinary( const void* pInstance, BcBinaryData::Stream& Serialiser ) const
		{
			const RsColour& Vec = *reinterpret_cast< const RsColour* >( pInstance );
			Serialiser << Vec;
			return true;
		}

		virtual BcBool serialiseFromBinary( void* pInstance, const BcBinaryData::Stream& Serialiser ) const 
		{
			RsColour& Vec = *reinterpret_cast< RsColour* >( pInstance );
			Serialiser >> Vec;
			return true;
		}

		virtual BcBool copy( void* pDst, void* pSrc ) const
		{
			RsColour& Dst = *reinterpret_cast< RsColour* >( pDst );
			RsColour& Src = *reinterpret_cast< RsColour* >( pSrc );
			Dst = Src;
			return true;
		}
	};

	ReRegisterClass< RsColour >( new RsColourSerialiser( "class RsColour" ) );
}

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
const RsColour RsColour::PURPLE =		RsColour( 0.5f, 0.0f, 0.5f, 1.0f );
const RsColour RsColour::CYAN =			RsColour( 0.0f, 1.0f, 1.0f, 1.0f );

RsColour::RsColour()
{
}

RsColour::RsColour( const MaVec4d& Vec ):
	MaVec4d( Vec )
{
}

RsColour::RsColour( BcU32 RGBA ):
	MaVec4d( ( ( RGBA ) & 0xff ) / 255.0f,
				( ( RGBA >> 8 ) & 0xff ) / 255.0f,
				( ( RGBA >> 16 ) & 0xff ) / 255.0f,
				( ( RGBA >> 24 ) & 0xff ) / 255.0f )
				
{
}

RsColour::RsColour( BcF32 R, BcF32 G, BcF32 B, BcF32 A ):
	MaVec4d( R, G, B, A )
{
}


BcU32 RsColour::asRGBA() const
{
	return ( ( BcClamp( static_cast< BcU32 >( r() * 255.0f ), 0, 255 ) << 24 ) |
		        ( BcClamp( static_cast< BcU32 >( g() * 255.0f ), 0, 255 ) << 16 ) |
		        ( BcClamp( static_cast< BcU32 >( b() * 255.0f ), 0, 255 ) << 8 ) |
		        ( BcClamp( static_cast< BcU32 >( a() * 255.0f ), 0, 255 ) ) );
}

BcU32 RsColour::asARGB() const
{
	return ( ( BcClamp( static_cast< BcU32 >( a() * 255.0f ), 0, 255 ) << 24 ) |
		        ( BcClamp( static_cast< BcU32 >( r() * 255.0f ), 0, 255 ) << 16 ) |
		        ( BcClamp( static_cast< BcU32 >( g() * 255.0f ), 0, 255 ) << 8 ) |
		        ( BcClamp( static_cast< BcU32 >( b() * 255.0f ), 0, 255 ) ) );
}

BcU32 RsColour::asABGR() const
{
	return ( ( BcClamp( static_cast< BcU32 >( a() * 255.0f ), 0, 255 ) << 24 ) |
				( BcClamp( static_cast< BcU32 >( b() * 255.0f ), 0, 255 ) << 16 ) |
				( BcClamp( static_cast< BcU32 >( g() * 255.0f ), 0, 255 ) << 8 ) |
				( BcClamp( static_cast< BcU32 >( r() * 255.0f ), 0, 255 ) ) );
}

BcU32 RsColour::asBGRA() const
{
	return ( ( BcClamp( static_cast< BcU32 >( b() * 255.0f ), 0, 255 ) << 24 ) |
		        ( BcClamp( static_cast< BcU32 >( g() * 255.0f ), 0, 255 ) << 16 ) |
		        ( BcClamp( static_cast< BcU32 >( r() * 255.0f ), 0, 255 ) << 8 ) |
		        ( BcClamp( static_cast< BcU32 >( a() * 255.0f ), 0, 255 ) ) );
}

//////////////////////////////////////////////////////////////////////////
// premultiplyAlpha
void RsColour::premultiplyAlpha()
{
	X_ = X_ * W_;
	Y_ = Y_ * W_;
	Z_ = Z_ * W_;
}

////////////////////////////////////////////////////////////////////////////////
// RsTextureFormatSize
BcU32 RsTextureFormatSize( eRsTextureFormat TextureFormat, BcU32 Width, BcU32 Height, BcU32 Depth, BcU32 Levels )
{
	BcU32 Size = 0;
	switch( TextureFormat )
	{
		case rsTF_RGB8:
			for( BcU32 iLevel = 0; iLevel < Levels; ++iLevel )
			{
				Size += Width * Height;
				Width >>= 1;
				Height >>= 1;
			}
			Size *= 3;
			Size *= Depth;
			break;

		case rsTF_RGBA8:
			for( BcU32 iLevel = 0; iLevel < Levels; ++iLevel )
			{
				Size += Width * Height;
				Width >>= 1;
				Height >>= 1;
			}
			Size *= 4;
			Size *= Depth;
			break;
		
		case rsTF_PAL4_RGBA8:
			for( BcU32 iLevel = 0; iLevel < Levels; ++iLevel )
			{
				Size += Width * Height;
				Width >>= 1;
				Height >>= 1;
			}
			Size = ( Size >> 1 ) + 64;
			Size *= Depth;
			break;
						
		case rsTF_PAL8_RGBA8:
			for( BcU32 iLevel = 0; iLevel < Levels; ++iLevel )
			{
				Size += Width * Height;
				Width >>= 1;
				Height >>= 1;
			}
			Size = Size + 1024;
			Size *= Depth;
			break;

		case rsTF_DXT1:
			for( BcU32 iLevel = 0; iLevel < Levels; ++iLevel )
			{
				BcU32 BlockCount = ( ( Width + 3 ) / 4 ) * ( ( Height + 3 ) / 4 );
				Size += BlockCount * 8;
				Width >>= 1;
				Height >>= 1;
			}
			Size *= Depth;
			break;

		case rsTF_DXT3:
		case rsTF_DXT5:			
			for( BcU32 iLevel = 0; iLevel < Levels; ++iLevel )
			{
				BcU32 BlockCount = ( ( Width + 3 ) / 4 ) * ( ( Height + 3 ) / 4 );
				Size += BlockCount * 16;
				Width >>= 1;
				Height >>= 1;
			}
			Size *= Depth;
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
			Size *= Depth;
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
			Size *= Depth;
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
	RS_DECL_APPEND_SIZE( rsVDF_TEXCOORD_UVW0,	12 );
	RS_DECL_APPEND_SIZE( rsVDF_TEXCOORD_UVW1,	12 );
	RS_DECL_APPEND_SIZE( rsVDF_TEXCOORD_UVW2,	12 );
	RS_DECL_APPEND_SIZE( rsVDF_TEXCOORD_UVW3,	12 );
	RS_DECL_APPEND_SIZE( rsVDF_SKIN_INDICES,	16 );
	RS_DECL_APPEND_SIZE( rsVDF_SKIN_WEIGHTS,	16 );
	RS_DECL_APPEND_SIZE( rsVDF_COLOUR_ABGR8,	4 );
#undef RS_DECL_APPEND_SIZE
	return Size;
}

/**************************************************************************
*
* File:		RsTypes.cpp
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
#include <algorithm>

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
			BcChar OutChars[ 128 ] = { 0 };
			BcSPrintf( OutChars, sizeof( OutChars ) - 1, "%.16f, %.16f, %.16f, %.16f", Vec.x(), Vec.y(), Vec.z(), Vec.w() );
			OutString = OutChars;
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
RsColour RsColour::FromHTMLColourCode( const BcChar* String )
{
	BcAssert( String[ 0 ] == '#' );
	BcAssert( String[ 7 ] == '\0' );
	BcAssert( isxdigit( String[ 1 ] ) );
	BcAssert( isxdigit( String[ 2 ] ) );
	BcAssert( isxdigit( String[ 3 ] ) );
	BcAssert( isxdigit( String[ 4 ] ) );
	BcAssert( isxdigit( String[ 5 ] ) );
	BcAssert( isxdigit( String[ 6 ] ) );
	auto Number = strtol( &String[ 1 ], nullptr, 16 );
	return RsColour( 
		( ( Number >> 16 ) & 0xff ) / 255.0f,
		( ( Number >> 8 ) & 0xff ) / 255.0f,
		( ( Number >> 0 ) & 0xff ) / 255.0f,
		1.0f );
}

const RsColour RsColour::WHITE =		RsColour( 1.0f, 1.0f, 1.0f, 1.0f );
const RsColour RsColour::BLACK =		RsColour( 0.0f, 0.0f, 0.0f, 1.0f );
const RsColour RsColour::GRAY =			RsColour( 0.5f, 0.5f, 0.5f, 1.0f );
const RsColour RsColour::RED =			RsColour( 1.0f, 0.0f, 0.0f, 1.0f );
const RsColour RsColour::GREEN =		RsColour( 0.0f, 1.0f, 0.0f, 1.0f );
const RsColour RsColour::BLUE =			RsColour( 0.0f, 0.0f, 1.0f, 1.0f );
const RsColour RsColour::ORANGE =		RsColour( 1.0f, 0.5f, 0.0f, 1.0f );
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
// RsTextureBlockInfo
RsBlockInfo RsTextureBlockInfo( RsTextureFormat TextureFormat )
{
	// Bits per block.
	RsBlockInfo BlockInfo;
	BlockInfo.Bits_ = 8;
	BlockInfo.Width_ = 1;
	BlockInfo.Height_ = 1;
	switch( TextureFormat )
	{
	case RsTextureFormat::R8:
		BlockInfo.Bits_ = 8;
		break;
	case RsTextureFormat::R8G8:
		BlockInfo.Bits_ = 16;
		break;
	case RsTextureFormat::R8G8B8:
		BlockInfo.Bits_ = 24;
		break;
	case RsTextureFormat::R8G8B8A8:
		BlockInfo.Bits_ = 32;
		break;
	case RsTextureFormat::R16F:
		BlockInfo.Bits_ = 16;
		break;
	case RsTextureFormat::R16FG16F:
		BlockInfo.Bits_ = 32;
		break;
	case RsTextureFormat::R16FG16FB16F:
		BlockInfo.Bits_ = 48;
		break;
	case RsTextureFormat::R16FG16FB16FA16F:
		BlockInfo.Bits_ = 64;
		break;
	case RsTextureFormat::R32F:
		BlockInfo.Bits_ = 32;
		break;
	case RsTextureFormat::R32FG32F:
		BlockInfo.Bits_ = 64;
		break;
	case RsTextureFormat::R32FG32FB32F:
		BlockInfo.Bits_ = 96;
		break;
	case RsTextureFormat::R32FG32FB32FA32F:
		BlockInfo.Bits_ = 128;
		break;
	case RsTextureFormat::DXT1:
		BlockInfo.Bits_ = 64;
		BlockInfo.Width_ = 4;
		BlockInfo.Height_ = 4;
		break;
	case RsTextureFormat::DXT3:
	case RsTextureFormat::DXT5:			
		BlockInfo.Bits_ = 128;
		BlockInfo.Width_ = 4;
		BlockInfo.Height_ = 4;
		break;
	case RsTextureFormat::ETC1:
		BlockInfo.Bits_ = 64;
		BlockInfo.Width_ = 4;
		BlockInfo.Height_ = 4;
		break;
	default:
		BcBreakpoint; // Format not defined.
		break;
	}

	return BlockInfo;
}

////////////////////////////////////////////////////////////////////////////////
// RsTextureFormatSize
BcU32 RsTextureFormatSize( RsTextureFormat TextureFormat, BcU32 Width, BcU32 Height, BcU32 Depth, BcU32 Levels )
{
	BcU32 Size = 0;
	BcU32 TotalTexels = 0;
	Width = BcMax( 1, Width );
	Height = BcMax( 1, Height );
	Depth = BcMax( 1, Depth );

	BcU32 HalfWidth = Width;
	BcU32 HalfHeight = Height;
	for( BcU32 iLevel = 0; iLevel < Levels; ++iLevel )
	{
		TotalTexels += HalfWidth * HalfHeight;
		HalfWidth >>= 1;
		HalfHeight >>= 1;
	}
	TotalTexels *= Depth;

	switch( TextureFormat )
	{
	case RsTextureFormat::R8:
		Size = TotalTexels;
		break;
	case RsTextureFormat::R8G8:
		Size = TotalTexels * 2;
		break;
	case RsTextureFormat::R8G8B8:
		Size = TotalTexels * 3;
		break;
	case RsTextureFormat::R8G8B8A8:
		Size = TotalTexels * 4;
		break;
	case RsTextureFormat::R16F:
		Size = TotalTexels * sizeof( BcU16 );
		break;
	case RsTextureFormat::R16FG16F:
		Size = TotalTexels * sizeof( BcU16 ) * 2;
		break;
	case RsTextureFormat::R16FG16FB16F:
		Size = TotalTexels * sizeof( BcU16 ) * 3;
		break;
	case RsTextureFormat::R16FG16FB16FA16F:
		Size = TotalTexels * sizeof( BcU16 ) * 4;
		break;
	case RsTextureFormat::R32F:
		Size = TotalTexels * sizeof( BcU32 );
		break;
	case RsTextureFormat::R32FG32F:
		Size = TotalTexels * sizeof( BcU32 ) * 2;
		break;
	case RsTextureFormat::R32FG32FB32F:
		Size = TotalTexels * sizeof( BcU32 ) * 3;
		break;
	case RsTextureFormat::R32FG32FB32FA32F:
		Size = TotalTexels * sizeof( BcU32 ) * 4;
		break;

	case RsTextureFormat::DXT1:
		for( BcU32 iLevel = 0; iLevel < Levels; ++iLevel )
		{
			BcU32 BlockCount = ( ( Width + 3 ) / 4 ) * ( ( Height + 3 ) / 4 );
			Size += BlockCount * 8;
			Width >>= 1;
			Height >>= 1;
		}
		Size *= Depth;
		break;

	case RsTextureFormat::DXT3:
	case RsTextureFormat::DXT5:			
		for( BcU32 iLevel = 0; iLevel < Levels; ++iLevel )
		{
			BcU32 BlockCount = ( ( Width + 3 ) / 4 ) * ( ( Height + 3 ) / 4 );
			Size += BlockCount * 16;
			Width >>= 1;
			Height >>= 1;
		}
		Size *= Depth;
		break;
			
	default:
		break;

	case RsTextureFormat::ETC1:
		for( BcU32 iLevel = 0; iLevel < Levels; ++iLevel )
		{
			Size += ( Width * Height ) / 2;
			Width >>= 1;
			Height >>= 1;
		}
		Size *= Depth;
		break;
	}
	
	return Size;
}

//////////////////////////////////////////////////////////////////////////
// RsTexturePitch
BcU32 RsTexturePitch( RsTextureFormat TextureFormat, BcU32 Width, BcU32 Height )
{
	BcU32 Pitch = 0;
	const auto BlockInfo = RsTextureBlockInfo( TextureFormat );
	auto WidthByBlock = ( Width / BlockInfo.Width_ );
	Pitch = ( WidthByBlock * BlockInfo.Bits_ ) / 8;
	return Pitch;
}

//////////////////////////////////////////////////////////////////////////
// RsTextureSlicePitch
BcU32 RsTextureSlicePitch( RsTextureFormat TextureFormat, BcU32 Width, BcU32 Height )
{
	BcU32 SlicePitch = 0;
	const auto BlockInfo = RsTextureBlockInfo( TextureFormat );
	auto WidthByBlock = ( Width / BlockInfo.Width_ );
	auto HeightByBlock = ( Height / BlockInfo.Height_ );
	SlicePitch = ( WidthByBlock * HeightByBlock * BlockInfo.Bits_ ) / 8;
	return SlicePitch;
}

////////////////////////////////////////////////////////////////////////////////
// RsShaderCodeTypeToBackendType
RsShaderBackendType RsShaderCodeTypeToBackendType( RsShaderCodeType CodeType )
{
	switch( CodeType )
	{
	case RsShaderCodeType::GLSL_140:
		return RsShaderBackendType::GLSL;
	case RsShaderCodeType::GLSL_150:
		return RsShaderBackendType::GLSL;
	case RsShaderCodeType::GLSL_330:
		return RsShaderBackendType::GLSL;
	case RsShaderCodeType::GLSL_400:
		return RsShaderBackendType::GLSL;
	case RsShaderCodeType::GLSL_410:
		return RsShaderBackendType::GLSL;
	case RsShaderCodeType::GLSL_420:
		return RsShaderBackendType::GLSL;
	case RsShaderCodeType::GLSL_430:
		return RsShaderBackendType::GLSL;
	case RsShaderCodeType::GLSL_440:
		return RsShaderBackendType::GLSL;
	case RsShaderCodeType::GLSL_450:
		return RsShaderBackendType::GLSL;
	case RsShaderCodeType::GLSL_ES_100:
		return RsShaderBackendType::GLSL_ES;
	case RsShaderCodeType::GLSL_ES_300:
		return RsShaderBackendType::GLSL_ES;
	case RsShaderCodeType::GLSL_ES_310:
		return RsShaderBackendType::GLSL_ES;
	case RsShaderCodeType::D3D9_3_0:
		return RsShaderBackendType::D3D9;
	case RsShaderCodeType::D3D11_4_0_LEVEL_9_0:
		return RsShaderBackendType::D3D11;
	case RsShaderCodeType::D3D11_4_0_LEVEL_9_1:
		return RsShaderBackendType::D3D11;
	case RsShaderCodeType::D3D11_4_0_LEVEL_9_3:
		return RsShaderBackendType::D3D11;
	case RsShaderCodeType::D3D11_4_0:
		return RsShaderBackendType::D3D11;
	case RsShaderCodeType::D3D11_4_1:
		return RsShaderBackendType::D3D11;
	case RsShaderCodeType::D3D11_5_0:
		return RsShaderBackendType::D3D11;
	case RsShaderCodeType::D3D11_5_1:
		return RsShaderBackendType::D3D11;
	default:
		BcBreakpoint;
	}

	return RsShaderBackendType::INVALID;
}

////////////////////////////////////////////////////////////////////////////////
// RsShaderCodeTypeToString
std::string RsShaderCodeTypeToString( RsShaderCodeType CodeType )
{
	std::string RetVal;
	switch( CodeType )
	{
	case RsShaderCodeType::GLSL_140:
		RetVal = "GLSL_140";
		break;
	case RsShaderCodeType::GLSL_150:
		RetVal = "GLSL_150";
		break;
	case RsShaderCodeType::GLSL_330:
		RetVal = "GLSL_330";
		break;
	case RsShaderCodeType::GLSL_400:
		RetVal = "GLSL_400";
		break;
	case RsShaderCodeType::GLSL_410:
		RetVal = "GLSL_410";
		break;
	case RsShaderCodeType::GLSL_420:
		RetVal = "GLSL_420";
		break;
	case RsShaderCodeType::GLSL_430:
		RetVal = "GLSL_430";
		break;
	case RsShaderCodeType::GLSL_440:
		RetVal = "GLSL_440";
		break;
	case RsShaderCodeType::GLSL_450:
		RetVal = "GLSL_450";
		break;
	case RsShaderCodeType::GLSL_ES_100:
		RetVal = "GLSL_ES_100";
		break;
	case RsShaderCodeType::GLSL_ES_300:
		RetVal = "GLSL_ES_300";
		break;
	case RsShaderCodeType::GLSL_ES_310:
		RetVal = "GLSL_ES_310";
		break;
	case RsShaderCodeType::D3D9_3_0:
		RetVal = "D3D9_3_0";
		break;
	case RsShaderCodeType::D3D11_4_0_LEVEL_9_0:
		RetVal = "D3D11_4_0_LEVEL_9_0";
		break;
	case RsShaderCodeType::D3D11_4_0_LEVEL_9_1:
		RetVal = "D3D11_4_0_LEVEL_9_1";
		break;
	case RsShaderCodeType::D3D11_4_0_LEVEL_9_3:
		RetVal = "D3D11_4_0_LEVEL_9_3";
		break;
	case RsShaderCodeType::D3D11_4_0:
		RetVal = "D3D11_4_0";
		break;
	case RsShaderCodeType::D3D11_4_1:
		RetVal = "D3D11_4_1";
		break;
	case RsShaderCodeType::D3D11_5_0:
		RetVal = "D3D11_5_0";
		break;
	case RsShaderCodeType::D3D11_5_1:
		RetVal = "D3D11_5_1";
		break;
	default:
		BcBreakpoint;
	}

	return std::move( RetVal );
}

////////////////////////////////////////////////////////////////////////////////
// RsShaderBackendTypeToString
std::string RsShaderBackendTypeToString( RsShaderBackendType BackendType )
{
	std::string RetVal;
	switch( BackendType )
	{
	case RsShaderBackendType::GLSL:
		RetVal = "GLSL";
		break;
	case RsShaderBackendType::GLSL_ES:
		RetVal = "GLSL_ES";
		break;
	case RsShaderBackendType::D3D9:
		RetVal = "D3D9";
		break;
	case RsShaderBackendType::D3D11:
		RetVal = "D3D11";
		break;
	case RsShaderBackendType::D3D12:
		RetVal = "D3D12";
		break;
	case RsShaderBackendType::MANTLE:
		RetVal = "MANTLE";
		break;
	default:
		BcBreakpoint;
	}
	return std::move( RetVal );
}

////////////////////////////////////////////////////////////////////////////////
// RsConvertCodeTypeToBackendCodeType
RsShaderCodeType RsStringToShaderCodeType( std::string String )
{
	std::transform( String.begin(), String.end(), String.begin(), ::toupper );
	
	RsShaderCodeType CodeType = RsShaderCodeType::INVALID;
	if( String == "GLSL_140" )
	{
		CodeType = RsShaderCodeType::GLSL_140;
	}
	else if( String == "GLSL_150" )
	{
		CodeType = RsShaderCodeType::GLSL_150;
	}
	else if( String == "GLSL_330" )
	{
		CodeType = RsShaderCodeType::GLSL_330;
	}
	else if( String == "GLSL_400" )
	{
		CodeType = RsShaderCodeType::GLSL_400;
	}
	else if( String == "GLSL_410" )
	{
		CodeType = RsShaderCodeType::GLSL_410;
	}
	else if( String == "GLSL_420" )
	{
		CodeType = RsShaderCodeType::GLSL_420;
	}
	else if( String == "GLSL_430" )
	{
		CodeType = RsShaderCodeType::GLSL_430;
	}
	else if( String == "GLSL_440" )
	{
		CodeType = RsShaderCodeType::GLSL_440;
	}
	else if( String == "GLSL_450" )
	{
		CodeType = RsShaderCodeType::GLSL_450;
	}
	else if( String == "GLSL_ES_100" )
	{
		CodeType = RsShaderCodeType::GLSL_ES_100;
	}
	else if( String == "GLSL_ES_300" )
	{
		CodeType = RsShaderCodeType::GLSL_ES_300;
	}
	else if( String == "GLSL_ES_310" )
	{
		CodeType = RsShaderCodeType::GLSL_ES_310;
	}
	else if( String == "D3D9_3_0" )
	{
		CodeType = RsShaderCodeType::D3D9_3_0;
	}
	else if( String == "D3D11_4_0_LEVEL_9_3" )
	{
		CodeType = RsShaderCodeType::D3D11_4_0_LEVEL_9_3;
	}
	else if( String == "D3D11_4_0" )
	{
		CodeType = RsShaderCodeType::D3D11_4_0;
	}
	else if( String == "D3D11_4_1" )
	{
		CodeType = RsShaderCodeType::D3D11_4_1;
	}
	else if( String == "D3D11_5_0" )
	{
		CodeType = RsShaderCodeType::D3D11_5_0;
	}
	else if( String == "D3D11_5_1" )
	{
		CodeType = RsShaderCodeType::D3D11_5_1;
	}
	else
	{
		BcBreakpoint;
	}

	return CodeType;
}

////////////////////////////////////////////////////////////////////////////////
// RsConvertCodeTypeToBackendCodeType
RsShaderCodeType RsConvertCodeTypeToBackendCodeType( RsShaderCodeType CodeType, RsShaderBackendType Backend )
{
	if( CodeType < RsShaderCodeType::MAX && Backend < RsShaderBackendType::MAX )
	{
		RsShaderCodeType ConversionTable[ (BcU32)RsShaderCodeType::MAX ][ (BcU32)RsShaderBackendType::MAX ];

		// Clear all to invalid.
		for( BcU32 IdxA = 0; IdxA < (BcU32)RsShaderCodeType::MAX; ++IdxA )
		{
			for( BcU32 IdxB = 0; IdxB < (BcU32)RsShaderBackendType::MAX; ++IdxB )
			{
				ConversionTable[ IdxA ][ IdxB ] = RsShaderCodeType::INVALID;
			}
		}

		// Setup straight conversion to same backend type.
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_140 ][ (BcU32)RsShaderBackendType::GLSL ] = RsShaderCodeType::GLSL_140;
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_150 ][ (BcU32)RsShaderBackendType::GLSL ] = RsShaderCodeType::GLSL_150;
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_330 ][ (BcU32)RsShaderBackendType::GLSL ] = RsShaderCodeType::GLSL_330;
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_400 ][ (BcU32)RsShaderBackendType::GLSL ] = RsShaderCodeType::GLSL_400;
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_410 ][ (BcU32)RsShaderBackendType::GLSL ] = RsShaderCodeType::GLSL_410;
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_420 ][ (BcU32)RsShaderBackendType::GLSL ] = RsShaderCodeType::GLSL_420;
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_430 ][ (BcU32)RsShaderBackendType::GLSL ] = RsShaderCodeType::GLSL_430;
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_440 ][ (BcU32)RsShaderBackendType::GLSL ] = RsShaderCodeType::GLSL_440;
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_450 ][ (BcU32)RsShaderBackendType::GLSL ] = RsShaderCodeType::GLSL_450;

		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_ES_100 ][ (BcU32)RsShaderBackendType::GLSL_ES ] = RsShaderCodeType::GLSL_ES_100;
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_ES_300 ][ (BcU32)RsShaderBackendType::GLSL_ES ] = RsShaderCodeType::GLSL_ES_300;
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_ES_310 ][ (BcU32)RsShaderBackendType::GLSL_ES ] = RsShaderCodeType::GLSL_ES_310;

		ConversionTable[ (BcU32)RsShaderCodeType::D3D9_3_0 ][ (BcU32)RsShaderBackendType::D3D9 ] = RsShaderCodeType::D3D9_3_0;

		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_4_0_LEVEL_9_3 ][ (BcU32)RsShaderBackendType::D3D11 ] = RsShaderCodeType::D3D11_4_0_LEVEL_9_3;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_4_0 ][ (BcU32)RsShaderBackendType::D3D11 ] = RsShaderCodeType::D3D11_4_0;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_4_1 ][ (BcU32)RsShaderBackendType::D3D11 ] = RsShaderCodeType::D3D11_4_1;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_5_0 ][ (BcU32)RsShaderBackendType::D3D11 ] = RsShaderCodeType::D3D11_5_0;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_5_1 ][ (BcU32)RsShaderBackendType::D3D11 ] = RsShaderCodeType::D3D11_5_1;

		// D3D11 to GLSL
		// TODO: Revise that these are correct. Just off the top of my head best guesses for now.
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_4_0_LEVEL_9_0 ][ (BcU32)RsShaderBackendType::GLSL ] = RsShaderCodeType::GLSL_140;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_4_0_LEVEL_9_1 ][ (BcU32)RsShaderBackendType::GLSL ] = RsShaderCodeType::GLSL_140;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_4_0_LEVEL_9_3 ][ (BcU32)RsShaderBackendType::GLSL ] = RsShaderCodeType::GLSL_140;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_4_0 ][ (BcU32)RsShaderBackendType::GLSL ] = RsShaderCodeType::GLSL_150;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_4_1 ][ (BcU32)RsShaderBackendType::GLSL ] = RsShaderCodeType::GLSL_330;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_5_0 ][ (BcU32)RsShaderBackendType::GLSL ] = RsShaderCodeType::GLSL_430;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_5_1 ][ (BcU32)RsShaderBackendType::GLSL ] = RsShaderCodeType::GLSL_430;

		// D3D9 to GLSL ES
		// TODO: Revise that these are correct. Just off the top of my head best guesses for now.
		ConversionTable[ (BcU32)RsShaderCodeType::D3D9_3_0 ][ (BcU32)RsShaderBackendType::GLSL_ES ] = RsShaderCodeType::GLSL_ES_100;

		// D3D11 to GLSL ES
		// TODO: Revise that these are correct. Just off the top of my head best guesses for now.
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_4_0_LEVEL_9_0 ][ (BcU32)RsShaderBackendType::GLSL_ES ] = RsShaderCodeType::GLSL_ES_100;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_4_0_LEVEL_9_1 ][ (BcU32)RsShaderBackendType::GLSL_ES ] = RsShaderCodeType::GLSL_ES_100;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_4_0_LEVEL_9_3 ][ (BcU32)RsShaderBackendType::GLSL_ES ] = RsShaderCodeType::GLSL_ES_100;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_4_0 ][ (BcU32)RsShaderBackendType::GLSL_ES ] = RsShaderCodeType::GLSL_ES_100;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_4_1 ][ (BcU32)RsShaderBackendType::GLSL_ES ] = RsShaderCodeType::GLSL_ES_100;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_5_0 ][ (BcU32)RsShaderBackendType::GLSL_ES ] = RsShaderCodeType::GLSL_ES_100;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_5_1 ][ (BcU32)RsShaderBackendType::GLSL_ES ] = RsShaderCodeType::GLSL_ES_100;

		// GLSL to D3D11
		// TODO: Revise that these are correct. Just off the top of my head best guesses for now.
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_140 ][ (BcU32)RsShaderBackendType::D3D11 ] = RsShaderCodeType::D3D11_4_0_LEVEL_9_3;
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_150 ][ (BcU32)RsShaderBackendType::D3D11 ] = RsShaderCodeType::D3D11_4_0;
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_330 ][ (BcU32)RsShaderBackendType::D3D11 ] = RsShaderCodeType::D3D11_4_1;
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_400 ][ (BcU32)RsShaderBackendType::D3D11 ] = RsShaderCodeType::D3D11_5_0;
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_410 ][ (BcU32)RsShaderBackendType::D3D11 ] = RsShaderCodeType::D3D11_5_0;
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_420 ][ (BcU32)RsShaderBackendType::D3D11 ] = RsShaderCodeType::D3D11_5_0;
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_430 ][ (BcU32)RsShaderBackendType::D3D11 ] = RsShaderCodeType::D3D11_5_0;
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_440 ][ (BcU32)RsShaderBackendType::D3D11 ] = RsShaderCodeType::D3D11_5_0;
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_450 ][ (BcU32)RsShaderBackendType::D3D11 ] = RsShaderCodeType::D3D11_5_0;
		
		// GLSL ES to D3D11
		// TODO: Revise that these are correct. Just off the top of my head best guesses for now.
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_ES_100 ][ (BcU32)RsShaderBackendType::D3D11 ] = RsShaderCodeType::D3D11_4_0_LEVEL_9_0;
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_ES_300 ][ (BcU32)RsShaderBackendType::D3D11 ] = RsShaderCodeType::D3D11_4_0_LEVEL_9_3;
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_ES_310 ][ (BcU32)RsShaderBackendType::D3D11 ] = RsShaderCodeType::D3D11_4_0_LEVEL_9_3;

		// GLSL ES to D3D9
		// TODO: Revise that these are correct. Just off the top of my head best guesses for now.
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_ES_100 ][ (BcU32)RsShaderBackendType::D3D9 ] = RsShaderCodeType::D3D9_3_0;


		//
		return ConversionTable[ (BcU32)CodeType ][ (BcU32)Backend ];
	}

	return RsShaderCodeType::INVALID;
}

////////////////////////////////////////////////////////////////////////////////
// RsIsLowerCodeTypeCompatibleWithHigher
BcBool RsIsLowerCodeTypeCompatibleWithHigher( RsShaderCodeType LowerCodeType, RsShaderCodeType HigherCodeType )
{
	// Check we're the same backend type first.
	if( RsShaderCodeTypeToBackendType( LowerCodeType ) == RsShaderCodeTypeToBackendType( HigherCodeType ) )
	{
		// Now it's just integer value.
		if( (int)LowerCodeType <= (int)HigherCodeType )
		{
			return BcTrue;
		}
	}

	return BcFalse;
}

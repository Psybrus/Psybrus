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

#include "Base/BcHalf.h"
#include "Base/BcMath.h"
#include <algorithm>

////////////////////////////////////////////////////////////////////////////////
// RsFloatToVertexDataType
bool RsFloatToVertexDataType( BcF32* InFloats, BcU32 NoofFloats, RsVertexDataType OutDataType, void* OutData, BcU32& OutDataSize )
{
	BcAssert( InFloats != nullptr );
	BcAssert( NoofFloats > 0 );
	BcAssert( OutDataType != RsVertexDataType::MAX );
	BcAssert( OutDataType != RsVertexDataType::INVALID );

	BcU32 VertexDataBytes[] = 
	{
		4, // FLOAT32 = 0,
		2, // FLOAT16,
		4, // FIXED,
		1, // BYTE,
		1, // BYTE_NORM,
		1, // UBYTE,
		1, // UBYTE_NORM,
		2, // SHORT,
		2, // SHORT_NORM,
		2, // USHORT,
		2, // USHORT_NORM,
		4, // INT,
		4, // INT_NORM,
		4, // UINT,
		4, // UINT_NORM,
	};

	// Calculate total output size.
	const BcU32 ElementBytes = VertexDataBytes[ (BcU32)OutDataType ];
	BcU8* OutDataBytes = reinterpret_cast< BcU8* >( OutData );
	OutDataSize = VertexDataBytes[ (BcU32)OutDataType ] * NoofFloats;
	
	// Early out for total bytes.
	if( OutData == nullptr )
	{
		return true;
	}

#define INTEGER_ELEMENT( _vtxType, _dataType ) \
	case _vtxType: \
		for( BcU32 Idx = 0; Idx < NoofFloats; ++Idx ) \
		{ \
			*reinterpret_cast< _dataType* >( OutDataBytes ) = \
				BcClamp( static_cast< _dataType >( *InFloats++ ), \
					std::numeric_limits< _dataType >::min(), \
					std::numeric_limits< _dataType >::max() ); \
			OutDataBytes += ElementBytes; \
		} \
		break; \
	case _vtxType##_NORM: \
		for( BcU32 Idx = 0; Idx < NoofFloats; ++Idx ) \
		{ \
			*reinterpret_cast< _dataType* >( OutDataBytes ) = \
				BcClamp( static_cast< _dataType >( *InFloats++ * std::numeric_limits< _dataType >::max() ), \
					std::numeric_limits< _dataType >::min(), \
					std::numeric_limits< _dataType >::max() ); \
			OutDataBytes += ElementBytes; \
		} \
		break;


	switch( OutDataType )
	{
	case RsVertexDataType::FLOAT32:
		memcpy( OutData, InFloats, OutDataSize );
		break;

	case RsVertexDataType::FLOAT16:
		for( BcU32 Idx = 0; Idx < NoofFloats; ++Idx )
		{
			*reinterpret_cast< BcU16* >( OutDataBytes ) = BcF32ToHalf( *InFloats++ );
			OutDataBytes += ElementBytes;
		}
		break;

	case RsVertexDataType::FIXED:
		return false;

	INTEGER_ELEMENT( RsVertexDataType::BYTE, BcS8 )
	INTEGER_ELEMENT( RsVertexDataType::UBYTE, BcU8 )
	INTEGER_ELEMENT( RsVertexDataType::SHORT, BcS16 )
	INTEGER_ELEMENT( RsVertexDataType::USHORT, BcU16 )
	INTEGER_ELEMENT( RsVertexDataType::INT, BcS32 )
	INTEGER_ELEMENT( RsVertexDataType::UINT, BcU32 )

	default:
		return false;
	}

#undef INTEGER_ELEMENT

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// RsTextureBlockInfo
RsBlockInfo RsTextureBlockInfo( RsResourceFormat TextureFormat )
{
	// Bits per block.
	RsBlockInfo BlockInfo;
	BlockInfo.Bits_ = 8;
	BlockInfo.Width_ = 1;
	BlockInfo.Height_ = 1;
	switch( TextureFormat )
	{
	case RsResourceFormat::R8_UNORM:
	case RsResourceFormat::R8_UINT:
	case RsResourceFormat::R8_SNORM:
	case RsResourceFormat::R8_SINT:
		BlockInfo.Bits_ = 8;
		break;
	case RsResourceFormat::R8G8_UNORM:
	case RsResourceFormat::R8G8_UINT:
	case RsResourceFormat::R8G8_SNORM:
	case RsResourceFormat::R8G8_SINT:
		BlockInfo.Bits_ = 16;
		break;
	case RsResourceFormat::R8G8B8A8_UNORM:
	case RsResourceFormat::R8G8B8A8_UNORM_SRGB:
	case RsResourceFormat::R8G8B8A8_UINT:
	case RsResourceFormat::R8G8B8A8_SNORM:
	case RsResourceFormat::R8G8B8A8_SINT:
		BlockInfo.Bits_ = 32;
		break;
	case RsResourceFormat::R16_FLOAT:
	case RsResourceFormat::R16_UNORM:
	case RsResourceFormat::R16_UINT:
	case RsResourceFormat::R16_SNORM:
	case RsResourceFormat::R16_SINT:
		BlockInfo.Bits_ = 16;
		break;
	case RsResourceFormat::R16G16_FLOAT:
	case RsResourceFormat::R16G16_UNORM:
	case RsResourceFormat::R16G16_UINT:
	case RsResourceFormat::R16G16_SNORM:
	case RsResourceFormat::R16G16_SINT:
		BlockInfo.Bits_ = 32;
		break;
	case RsResourceFormat::R16G16B16A16_FLOAT:
	case RsResourceFormat::R16G16B16A16_UNORM:
	case RsResourceFormat::R16G16B16A16_UINT:
	case RsResourceFormat::R16G16B16A16_SNORM:
	case RsResourceFormat::R16G16B16A16_SINT:
		BlockInfo.Bits_ = 64;
		break;
	case RsResourceFormat::R32_FLOAT:
	case RsResourceFormat::R32_UINT:
	case RsResourceFormat::R32_SINT:
		BlockInfo.Bits_ = 32;
		break;
	case RsResourceFormat::R32G32_FLOAT:
	case RsResourceFormat::R32G32_UINT:
	case RsResourceFormat::R32G32_SINT:
		BlockInfo.Bits_ = 64;
		break;
	case RsResourceFormat::R32G32B32_FLOAT:
	case RsResourceFormat::R32G32B32_UINT:
	case RsResourceFormat::R32G32B32_SINT:
		BlockInfo.Bits_ = 96;
		break;
	case RsResourceFormat::R32G32B32A32_FLOAT:
	case RsResourceFormat::R32G32B32A32_UINT:
	case RsResourceFormat::R32G32B32A32_SINT:
		BlockInfo.Bits_ = 128;
		break;
	case RsResourceFormat::R10G10B10A2_UNORM:
	case RsResourceFormat::R10G10B10A2_UINT:
		BlockInfo.Bits_ = 32;
		break;
	case RsResourceFormat::R11G11B10_FLOAT:
		BlockInfo.Bits_ = 32;
		break;
	case RsResourceFormat::BC1_UNORM:
	case RsResourceFormat::BC1_UNORM_SRGB:
	case RsResourceFormat::BC4_UNORM:
	case RsResourceFormat::BC4_SNORM:
		BlockInfo.Bits_ = 64;
		BlockInfo.Width_ = 4;
		BlockInfo.Height_ = 4;
		break;
	case RsResourceFormat::BC2_UNORM:
	case RsResourceFormat::BC2_UNORM_SRGB:
	case RsResourceFormat::BC3_UNORM:	
	case RsResourceFormat::BC3_UNORM_SRGB:
	case RsResourceFormat::BC5_UNORM:
	case RsResourceFormat::BC5_SNORM:
	case RsResourceFormat::BC6H_UF16:
	case RsResourceFormat::BC6H_SF16:
	case RsResourceFormat::BC7_UNORM:
	case RsResourceFormat::BC7_UNORM_SRGB:
		BlockInfo.Bits_ = 128;
		BlockInfo.Width_ = 4;
		BlockInfo.Height_ = 4;
		break;
	case RsResourceFormat::ETC1_UNORM:
		BlockInfo.Bits_ = 64;
		BlockInfo.Width_ = 4;
		BlockInfo.Height_ = 4;
		break;
	case RsResourceFormat::ETC2_UNORM:
		BlockInfo.Bits_ = 64;
		BlockInfo.Width_ = 4;
		BlockInfo.Height_ = 4;
		break;
	case RsResourceFormat::ETC2A_UNORM:
		BlockInfo.Bits_ = 128;
		BlockInfo.Width_ = 4;
		BlockInfo.Height_ = 4;
		break;
	case RsResourceFormat::ETC2A1_UNORM:
		BlockInfo.Bits_ = 64;
		BlockInfo.Width_ = 4;
		BlockInfo.Height_ = 4;
		break;
	case RsResourceFormat::D16_UNORM:
		BlockInfo.Bits_ = 16;
		break;
	case RsResourceFormat::D32_FLOAT:
		BlockInfo.Bits_ = 32;
		break;
	case RsResourceFormat::D24_UNORM_S8_UINT:
		BlockInfo.Bits_ = 32;
		break;
	default:
		BcBreakpoint; // Format not defined.
		break;
	}

	return BlockInfo;
}

////////////////////////////////////////////////////////////////////////////////
// RsResourceFormatSize
RsFormatInfo RsResourceFormatInfo( RsResourceFormat TextureFormat )
{
	RsFormatInfo Info;
	switch( TextureFormat )
	{
	case RsResourceFormat::R8_UNORM:
	case RsResourceFormat::R8_UINT:
	case RsResourceFormat::R8_SNORM:
	case RsResourceFormat::R8_SINT:
		Info.RBits_ = 8;
		break;
	case RsResourceFormat::R8G8_UNORM:
	case RsResourceFormat::R8G8_UINT:
	case RsResourceFormat::R8G8_SNORM:
	case RsResourceFormat::R8G8_SINT:
		Info.RBits_ = 8;
		Info.GBits_ = 8;
		break;
	case RsResourceFormat::R8G8B8A8_UNORM:
	case RsResourceFormat::R8G8B8A8_UNORM_SRGB:
	case RsResourceFormat::R8G8B8A8_UINT:
	case RsResourceFormat::R8G8B8A8_SNORM:
	case RsResourceFormat::R8G8B8A8_SINT:
		Info.RBits_ = 8;
		Info.GBits_ = 8;
		Info.BBits_ = 8;
		Info.ABits_ = 8;
		break;
	case RsResourceFormat::R16_FLOAT:
	case RsResourceFormat::R16_UNORM:
	case RsResourceFormat::R16_UINT:
	case RsResourceFormat::R16_SNORM:
	case RsResourceFormat::R16_SINT:
		Info.RBits_ = 16;
		break;
	case RsResourceFormat::R16G16_FLOAT:
	case RsResourceFormat::R16G16_UNORM:
	case RsResourceFormat::R16G16_UINT:
	case RsResourceFormat::R16G16_SNORM:
	case RsResourceFormat::R16G16_SINT:
		Info.RBits_ = 16;
		Info.GBits_ = 16;
		break;
	case RsResourceFormat::R16G16B16A16_FLOAT:
	case RsResourceFormat::R16G16B16A16_UNORM:
	case RsResourceFormat::R16G16B16A16_UINT:
	case RsResourceFormat::R16G16B16A16_SNORM:
	case RsResourceFormat::R16G16B16A16_SINT:
		Info.RBits_ = 16;
		Info.GBits_ = 16;
		Info.BBits_ = 16;
		Info.ABits_ = 16;
		break;
	case RsResourceFormat::R32_FLOAT:
	case RsResourceFormat::R32_UINT:
	case RsResourceFormat::R32_SINT:
		Info.RBits_ = 32;
		break;
	case RsResourceFormat::R32G32_FLOAT:
	case RsResourceFormat::R32G32_UINT:
	case RsResourceFormat::R32G32_SINT:
		Info.RBits_ = 32;
		Info.GBits_ = 32;
		break;
	case RsResourceFormat::R32G32B32_FLOAT:
	case RsResourceFormat::R32G32B32_UINT:
	case RsResourceFormat::R32G32B32_SINT:
		Info.RBits_ = 32;
		Info.GBits_ = 32;
		Info.BBits_ = 32;
		break;
	case RsResourceFormat::R32G32B32A32_FLOAT:
	case RsResourceFormat::R32G32B32A32_UINT:
	case RsResourceFormat::R32G32B32A32_SINT:
		Info.RBits_ = 32;
		Info.GBits_ = 32;
		Info.BBits_ = 32;
		Info.ABits_ = 32;
		break;
	case RsResourceFormat::R10G10B10A2_UNORM:
	case RsResourceFormat::R10G10B10A2_UINT:
		Info.RBits_ = 10;
		Info.GBits_ = 10;
		Info.BBits_ = 10;
		Info.ABits_ = 2;
		break;
	case RsResourceFormat::R11G11B10_FLOAT:
		Info.RBits_ = 11;
		Info.GBits_ = 11;
		Info.BBits_ = 10;
		break;
	case RsResourceFormat::D16_UNORM:
		Info.DepthBits_ = 16;
		break;
	case RsResourceFormat::D32_FLOAT:
		Info.DepthBits_ = 32;
		break;
	case RsResourceFormat::D24_UNORM_S8_UINT:
		Info.DepthBits_ = 24;
		Info.StencilBits_ = 8;
		break;
	default:
		BcBreakpoint; // Format not defined.
		break;
	}
	return Info;
}

////////////////////////////////////////////////////////////////////////////////
// RsResourceFormatSize
BcU32 RsResourceFormatSize( RsResourceFormat ResourceFormat, BcU32 Width, BcU32 Height, BcU32 Depth, BcU32 Levels )
{
	BcU32 Size = 0;
	Width = BcMax( 1, Width );
	Height = BcMax( 1, Height );
	Depth = BcMax( 1, Depth );

	auto BlockInfo = RsTextureBlockInfo( ResourceFormat );
	for( BcU32 iLevel = 0; iLevel < Levels; ++iLevel )
	{
		BcU32 BlocksW = BcPotRoundUp( Width, BlockInfo.Width_ ) / BlockInfo.Width_;
		BcU32 BlocksH = BcPotRoundUp( Height, BlockInfo.Height_ ) / BlockInfo.Height_;
		BcU32 BlocksD = Depth;

		Size += ( BlockInfo.Bits_ * BlocksW * BlocksH * BlocksD ) / 8;

		Width = BcMax( Width / 2, 1 );
		Height = BcMax( Height / 2, 1 );
		Depth = BcMax( Depth / 2, 1 );
	}

	return Size;
}

//////////////////////////////////////////////////////////////////////////
// RsTexturePitch
BcU32 RsTexturePitch( RsResourceFormat TextureFormat, BcU32 Width, BcU32 Height )
{
	BcU32 Pitch = 0;
	const auto BlockInfo = RsTextureBlockInfo( TextureFormat );
	auto WidthByBlock = std::max( BcU32( 1 ), Width / BlockInfo.Width_ );
	Pitch = ( WidthByBlock * BlockInfo.Bits_ ) / 8;
	return Pitch;
}

//////////////////////////////////////////////////////////////////////////
// RsTextureSlicePitch
BcU32 RsTextureSlicePitch( RsResourceFormat TextureFormat, BcU32 Width, BcU32 Height )
{
	BcU32 SlicePitch = 0;
	const auto BlockInfo = RsTextureBlockInfo( TextureFormat );
	auto WidthByBlock = std::max( BcU32( 1 ), Width / BlockInfo.Width_ );
	auto HeightByBlock = std::max( BcU32( 1 ), Height / BlockInfo.Height_ );
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
	case RsShaderCodeType::ESSL_100:
		return RsShaderBackendType::ESSL;
	case RsShaderCodeType::ESSL_300:
		return RsShaderBackendType::ESSL;
	case RsShaderCodeType::ESSL_310:
		return RsShaderBackendType::ESSL;
	case RsShaderCodeType::D3D11_4_0_LEVEL_9_1:
		return RsShaderBackendType::D3D11;
	case RsShaderCodeType::D3D11_4_0_LEVEL_9_2:
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
	case RsShaderCodeType::ESSL_100:
		RetVal = "ESSL_100";
		break;
	case RsShaderCodeType::ESSL_300:
		RetVal = "ESSL_300";
		break;
	case RsShaderCodeType::ESSL_310:
		RetVal = "ESSL_310";
		break;
	case RsShaderCodeType::D3D11_4_0_LEVEL_9_1:
		RetVal = "D3D11_4_0_LEVEL_9_1";
		break;
	case RsShaderCodeType::D3D11_4_0_LEVEL_9_2:
		RetVal = "D3D11_4_0_LEVEL_9_2";
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
	case RsShaderCodeType::SPIRV:
		RetVal = "SPIRV";
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
	case RsShaderBackendType::ESSL:
		RetVal = "ESSL";
		break;
	case RsShaderBackendType::D3D11:
		RetVal = "D3D11";
		break;
	case RsShaderBackendType::D3D12:
		RetVal = "D3D12";
		break;
	case RsShaderBackendType::SPIRV:
		RetVal = "SPIRV";
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
	else if( String == "ESSL_100" )
	{
		CodeType = RsShaderCodeType::ESSL_100;
	}
	else if( String == "ESSL_300" )
	{
		CodeType = RsShaderCodeType::ESSL_300;
	}
	else if( String == "ESSL_310" )
	{
		CodeType = RsShaderCodeType::ESSL_310;
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
	else if( String == "SPIRV" )
	{
		CodeType = RsShaderCodeType::SPIRV;
	}	else
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

		ConversionTable[ (BcU32)RsShaderCodeType::ESSL_100 ][ (BcU32)RsShaderBackendType::ESSL ] = RsShaderCodeType::ESSL_100;
		ConversionTable[ (BcU32)RsShaderCodeType::ESSL_300 ][ (BcU32)RsShaderBackendType::ESSL ] = RsShaderCodeType::ESSL_300;
		ConversionTable[ (BcU32)RsShaderCodeType::ESSL_310 ][ (BcU32)RsShaderBackendType::ESSL ] = RsShaderCodeType::ESSL_310;

		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_4_0_LEVEL_9_3 ][ (BcU32)RsShaderBackendType::D3D11 ] = RsShaderCodeType::D3D11_4_0_LEVEL_9_3;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_4_0 ][ (BcU32)RsShaderBackendType::D3D11 ] = RsShaderCodeType::D3D11_4_0;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_4_1 ][ (BcU32)RsShaderBackendType::D3D11 ] = RsShaderCodeType::D3D11_4_1;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_5_0 ][ (BcU32)RsShaderBackendType::D3D11 ] = RsShaderCodeType::D3D11_5_0;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_5_1 ][ (BcU32)RsShaderBackendType::D3D11 ] = RsShaderCodeType::D3D11_5_1;

		ConversionTable[ (BcU32)RsShaderCodeType::SPIRV ][ (BcU32)RsShaderBackendType::SPIRV ] = RsShaderCodeType::SPIRV;

		// D3D11 to GLSL
		// TODO: Revise that these are correct. Just off the top of my head best guesses for now.
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_4_0_LEVEL_9_1 ][ (BcU32)RsShaderBackendType::GLSL ] = RsShaderCodeType::GLSL_140;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_4_0_LEVEL_9_2 ][ (BcU32)RsShaderBackendType::GLSL ] = RsShaderCodeType::GLSL_140;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_4_0_LEVEL_9_3 ][ (BcU32)RsShaderBackendType::GLSL ] = RsShaderCodeType::GLSL_140;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_4_0 ][ (BcU32)RsShaderBackendType::GLSL ] = RsShaderCodeType::GLSL_150;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_4_1 ][ (BcU32)RsShaderBackendType::GLSL ] = RsShaderCodeType::GLSL_330;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_5_0 ][ (BcU32)RsShaderBackendType::GLSL ] = RsShaderCodeType::GLSL_430;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_5_1 ][ (BcU32)RsShaderBackendType::GLSL ] = RsShaderCodeType::GLSL_430;

		
		// D3D11 to GLSL ES
		// TODO: Revise that these are correct. Just off the top of my head best guesses for now.
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_4_0_LEVEL_9_1 ][ (BcU32)RsShaderBackendType::ESSL ] = RsShaderCodeType::ESSL_100;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_4_0_LEVEL_9_2 ][ (BcU32)RsShaderBackendType::ESSL ] = RsShaderCodeType::ESSL_100;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_4_0_LEVEL_9_3 ][ (BcU32)RsShaderBackendType::ESSL ] = RsShaderCodeType::ESSL_100;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_4_0 ][ (BcU32)RsShaderBackendType::ESSL ] = RsShaderCodeType::ESSL_100;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_4_1 ][ (BcU32)RsShaderBackendType::ESSL ] = RsShaderCodeType::ESSL_100;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_5_0 ][ (BcU32)RsShaderBackendType::ESSL ] = RsShaderCodeType::ESSL_100;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_5_1 ][ (BcU32)RsShaderBackendType::ESSL ] = RsShaderCodeType::ESSL_100;

		// D3D11 to SPIRV
		// TODO: Revise that these are correct. Just off the top of my head best guesses for now.
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_4_0_LEVEL_9_1 ][ (BcU32)RsShaderBackendType::SPIRV ] = RsShaderCodeType::SPIRV;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_4_0_LEVEL_9_2 ][ (BcU32)RsShaderBackendType::SPIRV ] = RsShaderCodeType::SPIRV;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_4_0_LEVEL_9_3 ][ (BcU32)RsShaderBackendType::SPIRV ] = RsShaderCodeType::SPIRV;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_4_0 ][ (BcU32)RsShaderBackendType::SPIRV ] = RsShaderCodeType::SPIRV;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_4_1 ][ (BcU32)RsShaderBackendType::SPIRV ] = RsShaderCodeType::SPIRV;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_5_0 ][ (BcU32)RsShaderBackendType::SPIRV ] = RsShaderCodeType::SPIRV;
		ConversionTable[ (BcU32)RsShaderCodeType::D3D11_5_1 ][ (BcU32)RsShaderBackendType::SPIRV ] = RsShaderCodeType::SPIRV;

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
		
		// GLSL to SPIRV
		// TODO: Revise that these are correct. Just off the top of my head best guesses for now.
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_140 ][ (BcU32)RsShaderBackendType::SPIRV ] = RsShaderCodeType::SPIRV;
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_150 ][ (BcU32)RsShaderBackendType::SPIRV ] = RsShaderCodeType::SPIRV;
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_330 ][ (BcU32)RsShaderBackendType::SPIRV ] = RsShaderCodeType::SPIRV;
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_400 ][ (BcU32)RsShaderBackendType::SPIRV ] = RsShaderCodeType::SPIRV;
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_410 ][ (BcU32)RsShaderBackendType::SPIRV ] = RsShaderCodeType::SPIRV;
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_420 ][ (BcU32)RsShaderBackendType::SPIRV ] = RsShaderCodeType::SPIRV;
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_430 ][ (BcU32)RsShaderBackendType::SPIRV ] = RsShaderCodeType::SPIRV;
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_440 ][ (BcU32)RsShaderBackendType::SPIRV ] = RsShaderCodeType::SPIRV;
		ConversionTable[ (BcU32)RsShaderCodeType::GLSL_450 ][ (BcU32)RsShaderBackendType::SPIRV ] = RsShaderCodeType::SPIRV;

		// GLSL ES to D3D11
		// TODO: Revise that these are correct. Just off the top of my head best guesses for now.
		ConversionTable[ (BcU32)RsShaderCodeType::ESSL_100 ][ (BcU32)RsShaderBackendType::D3D11 ] = RsShaderCodeType::D3D11_4_0_LEVEL_9_1;
		ConversionTable[ (BcU32)RsShaderCodeType::ESSL_300 ][ (BcU32)RsShaderBackendType::D3D11 ] = RsShaderCodeType::D3D11_4_0_LEVEL_9_3;
		ConversionTable[ (BcU32)RsShaderCodeType::ESSL_310 ][ (BcU32)RsShaderBackendType::D3D11 ] = RsShaderCodeType::D3D11_4_0_LEVEL_9_3;

		// GLSL ES to SPIRV
		// TODO: Revise that these are correct. Just off the top of my head best guesses for now.
		ConversionTable[ (BcU32)RsShaderCodeType::ESSL_100 ][ (BcU32)RsShaderBackendType::SPIRV ] = RsShaderCodeType::SPIRV;
		ConversionTable[ (BcU32)RsShaderCodeType::ESSL_300 ][ (BcU32)RsShaderBackendType::SPIRV ] = RsShaderCodeType::SPIRV;
		ConversionTable[ (BcU32)RsShaderCodeType::ESSL_310 ][ (BcU32)RsShaderBackendType::SPIRV ] = RsShaderCodeType::SPIRV;

		// SPIR-V to GLSL
		// TODO: Revise that these are correct. Just off the top of my head best guesses for now.
		ConversionTable[ (BcU32)RsShaderCodeType::SPIRV ][ (BcU32)RsShaderBackendType::GLSL ] = RsShaderCodeType::GLSL_450;

		// SPIR-V to D3D11
		// TODO: Revise that these are correct. Just off the top of my head best guesses for now.
		ConversionTable[ (BcU32)RsShaderCodeType::SPIRV ][ (BcU32)RsShaderBackendType::D3D11 ] = RsShaderCodeType::D3D11_5_1;
		
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

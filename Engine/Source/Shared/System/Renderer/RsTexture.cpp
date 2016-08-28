/**************************************************************************
*
* File:		RsTexture.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/RsTexture.h"

#include "Base/BcMath.h"

//////////////////////////////////////////////////////////////////////////
// RsTextureDesc
RsTextureDesc::RsTextureDesc():
	Type_( RsTextureType::UNKNOWN ),
	CreationFlags_( RsResourceCreationFlags::NONE ),
	BindFlags_( RsResourceBindFlags::NONE ),
	Format_( RsResourceFormat::INVALID ),
	Width_( 0 ),
	Height_( 0 ),
	Depth_( 0 )
{

}

//////////////////////////////////////////////////////////////////////////
// RsTextureDesc
RsTextureDesc::RsTextureDesc( 
		RsTextureType Type, 
		RsResourceCreationFlags CreationFlags,
		RsResourceBindFlags BindFlags,
		RsResourceFormat Format,
		BcU32 Levels, 
		BcU32 Width, 
		BcU32 Height,
		BcU32 Depth ):
	Type_( Type ),
	CreationFlags_( CreationFlags ),
	BindFlags_( BindFlags ),
	Format_( Format ),
	Levels_( Levels ),
	Width_( Width ),
	Height_( Height ),
	Depth_( Depth )
{
#ifdef PSY_DEBUG
	// Check levels is valid.
	BcAssert( Levels_ > 0 );
	BcAssert( Width_ >= 1 );
	BcAssert( Height_ >= 1 );
 	BcAssert( Depth_ >= 1 );

	// Max num of mips...math fail...must be a simpler way to calculate it.
	BcU32 MinWidth = BcMax( 1, Width_ );
	BcU32 MinHeight = BcMax( 1, Height_ );
	BcU32 MinDepth = BcMax( 1, Depth_ );
	BcU32 MaxLevels = 0;
	while( MinWidth >= 1 || MinHeight >= 1 || MinDepth >= 1 )
	{
		++MaxLevels;
		MinWidth >>= 1;
		MinHeight >>= 1;
		MinDepth >>= 1;
	}
	BcAssertMsg( Levels_ <= MaxLevels, "Levels_ (%u) <= MaxLevels (%u). Width (%u), Height (%u), Depth (%u)", Levels_, MaxLevels, Width_, Height_, Depth_ );

	// Calculate minimum dimension.
	BcU32 MinimumDimension = 1;
	switch( Format_ )
	{
	case RsResourceFormat::BC1_UNORM:
	case RsResourceFormat::BC1_UNORM_SRGB:
	case RsResourceFormat::BC2_UNORM:
	case RsResourceFormat::BC2_UNORM_SRGB:
	case RsResourceFormat::BC3_UNORM:
	case RsResourceFormat::BC3_UNORM_SRGB:
	case RsResourceFormat::BC4_UNORM:
	case RsResourceFormat::BC4_SNORM:
	case RsResourceFormat::BC5_UNORM:
	case RsResourceFormat::BC5_SNORM:
	case RsResourceFormat::BC6H_UF16:
	case RsResourceFormat::BC6H_SF16:
	case RsResourceFormat::BC7_UNORM:
	case RsResourceFormat::BC7_UNORM_SRGB:
	case RsResourceFormat::ETC1_UNORM:
	case RsResourceFormat::ETC2_UNORM:
	case RsResourceFormat::ETC2A_UNORM:
	case RsResourceFormat::ETC2A1_UNORM:
		MinimumDimension = 4;
		break;
	default:
		break;
	}

	// Check vs texture type.
	switch( Type_ )
	{
	case RsTextureType::TEX1D:
		BcAssert( Width_ >= MinimumDimension && Height_ == 1 && Depth_ == 1 );
		break;
	case RsTextureType::TEX2D:
		BcAssert( Width_ >= MinimumDimension && Height_ >= MinimumDimension && Depth_ == 1 );
		break;
	case RsTextureType::TEX3D:
		BcAssert( Width_ >= MinimumDimension && Height_ >= MinimumDimension && Depth_ >= 1 );
		break;
	case RsTextureType::TEXCUBE:
		BcAssert( Width_ >= MinimumDimension && Height_ >= MinimumDimension && Depth_ == 1 );
		break;
	default:
		BcBreakpoint;
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
// Ctor
RsTexture::RsTexture( RsContext* pContext, const RsTextureDesc& Desc ):
	RsResource( RsResourceType::TEXTURE, pContext ),
	Desc_( Desc )
{

}

//////////////////////////////////////////////////////////////////////////
// getDesc
const RsTextureDesc& RsTexture::getDesc() const
{
	return Desc_;
}

//////////////////////////////////////////////////////////////////////////
// getSlice
RsTextureSlice RsTexture::getSlice( BcU32 Level, RsTextureFace Face ) const
{
#ifdef PSY_DEBUG
	// Check level validity.
	BcAssert( Level >= 0 && Level < Desc_.Levels_ );
#endif

	RsTextureSlice Slice = 
	{
		Level,
		Face
	};
	
	return Slice;
}

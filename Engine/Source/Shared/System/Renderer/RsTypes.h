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

#ifndef __RSTYPES_MANGLE_H__
#define __RSTYPES_MANGLE_H__

#include "Base/BcTypes.h"
#include "Math/MaVec4d.h"

//////////////////////////////////////////////////////////////////////////
// Domain is used by us. Undefine it.
#ifdef DOMAIN
#undef DOMAIN
#endif

//////////////////////////////////////////////////////////////////////////
// Blending Modes
enum class RsBlendingMode : BcU32
{
	NONE = 0,			// (+) ONE, ZERO
	BLEND,				// (+) ONE, INVSRCALPHA 
	ADD,				// (+) ONE, ONE
	SUBTRACT,			// (-) ONE, ONE

	MAX,
	INVALID = BcErrorCode
};


//////////////////////////////////////////////////////////////////////////
// Fill mode
enum class RsFillMode : BcU32
{
	SOLID = 0,
	WIREFRAME,
	
	MAX,
	INVALID = BcErrorCode
};

//////////////////////////////////////////////////////////////////////////
// Cull mode
enum class RsCullMode : BcU32
{
	NONE = 0,
	CCW,
	CW, 

	MAX,
	INVALID = BcErrorCode
};

//////////////////////////////////////////////////////////////////////////
// Blend type
enum class RsBlendType : BcU32
{
	ZERO = 0,
	ONE,
	SRC_COLOUR,
	INV_SRC_COLOUR,
	SRC_ALPHA,
	INV_SRC_ALPHA,
	DEST_COLOUR,
	INV_DEST_COLOUR,
	DEST_ALPHA,
	INV_DEST_ALPHA,
	
	MAX,
	INVALID = BcErrorCode
};

//////////////////////////////////////////////////////////////////////////
// Blend op
enum class RsBlendOp : BcU32
{
	ADD = 0,
	SUBTRACT,
	REV_SUBTRACT,
	MINIMUM,
	MAXIMUM,

	MAX,
	INVALID = BcErrorCode
};

//////////////////////////////////////////////////////////////////////////
// Compare modes
enum class RsCompareMode : BcU32
{
	NEVER = 0,
	LESS,
	EQUAL,
	LESSEQUAL,
	GREATER,
	NOTEQUAL,
	GREATEREQUAL,
	ALWAYS,
	
	MAX,
	INVALID = BcErrorCode
};

//////////////////////////////////////////////////////////////////////////
// Stencil operations
enum class RsStencilOp : BcU32
{
	KEEP = 0,
	ZERO,
	REPLACE,
	INCR,
	INCR_WRAP,
	DECR,
	DECR_WRAP,
	INVERT,
	
	MAX,
	INVALID = BcErrorCode
};

//////////////////////////////////////////////////////////////////////////
// Vertex data type.
enum class RsVertexDataType : BcU32
{
	FLOAT32 = 0,
	FLOAT16,
	FIXED,
	BYTE,
	BYTE_NORM,
	UBYTE,
	UBYTE_NORM,
	SHORT,
	SHORT_NORM,
	USHORT,
	USHORT_NORM,
	INT,
	INT_NORM,
	UINT,
	UINT_NORM,
	
	MAX,
	INVALID = BcErrorCode
};

/**
 * Convert a stream of floats to output vertex data format.
 * @param InFloats Input floats.
 * @param NoofFloats Number of floats.
 * @param OutDataType Output data type.
 * @param OutData Output data. Can be nullptr.
 * @param OutDataSize Output data size in bytes.
 * @return true for success.
 */
bool RsFloatToVertexDataType( BcF32* InFloats, BcU32 NoofFloats, RsVertexDataType OutDataType, void* OutData, BcU32& OutDataSize );

//////////////////////////////////////////////////////////////////////////
// Vertex data type.
enum class RsVertexUsage : BcU32
{
	POSITION = 0,
	BLENDWEIGHTS,
	BLENDINDICES,
	NORMAL,
	PSIZE,
	TEXCOORD,
	TANGENT,
	BINORMAL,
	TESSFACTOR,
	POSITIONT,
	COLOUR,
	FOG,
	DEPTH,
	SAMPLE,
	
	MAX,
	INVALID = BcErrorCode
};

//////////////////////////////////////////////////////////////////////////
// Resource Format
enum class RsResourceFormat : BcU32
{
	UNKNOWN,

	// Colour.
	R8_UNORM,
	R8_UINT,
	R8_SNORM,
	R8_SINT,

	R8G8_UNORM,
	R8G8_UINT,
	R8G8_SNORM,
	R8G8_SINT,
	
	R8G8B8A8_UNORM,
	R8G8B8A8_UNORM_SRGB,
	R8G8B8A8_UINT,
	R8G8B8A8_SNORM,
	R8G8B8A8_SINT,
	
	R16_FLOAT,
	R16_UNORM,
	R16_UINT,
	R16_SNORM,
	R16_SINT,

	R16G16_FLOAT,
	R16G16_UNORM,
	R16G16_UINT,
	R16G16_SNORM,
	R16G16_SINT,	

	R16G16B16A16_FLOAT,
	R16G16B16A16_UNORM,
	R16G16B16A16_UINT,
	R16G16B16A16_SNORM,
	R16G16B16A16_SINT,

	R32_FLOAT,
	R32_UINT,
	R32_SINT,

	R32G32_FLOAT,
	R32G32_UINT,
	R32G32_SINT,
	
	R32G32B32_FLOAT,
	R32G32B32_UINT,
	R32G32B32_SINT,
	
	R32G32B32A32_FLOAT,
	R32G32B32A32_UINT,
	R32G32B32A32_SINT,
		
	R10G10B10A2_UNORM,
	R10G10B10A2_UINT,

	R11G11B10_FLOAT,

	// Compressed formats.
	BC1_UNORM,
	BC1_UNORM_SRGB,

	BC2_UNORM,
	BC2_UNORM_SRGB,

	BC3_UNORM,
	BC3_UNORM_SRGB,

	BC4_UNORM,
	BC4_SNORM,

	BC5_UNORM,
	BC5_SNORM,

	BC6H_UF16,
	BC6H_SF16,

	BC7_UNORM,
	BC7_UNORM_SRGB,

	ETC1_UNORM,
	ETC2_UNORM,
	ETC2A_UNORM,
	ETC2A1_UNORM,

	// Depth stencil.
	D16_UNORM,
	D24_UNORM_S8_UINT,
	D32_FLOAT,
	
	MAX,
	INVALID = BcErrorCode
};

//////////////////////////////////////////////////////////////////////////
// RsTextureBlockInfo
struct RsBlockInfo
{
	BcU32 Width_ = 0;
	BcU32 Height_ = 0;
	BcU32 Bits_ = 0;
};

extern RsBlockInfo RsTextureBlockInfo( RsResourceFormat TextureFormat );

//////////////////////////////////////////////////////////////////////////
// RsTextureFormatInfo
struct RsFormatInfo
{
	BcU32 RBits_ = 0;
	BcU32 GBits_ = 0;
	BcU32 BBits_ = 0;
	BcU32 ABits_ = 0;
	BcU32 DepthBits_ = 0;
	BcU32 StencilBits_ = 0;
};

extern RsFormatInfo RsResourceFormatInfo( RsResourceFormat TextureFormat );

//////////////////////////////////////////////////////////////////////////
// RsResourceFormatSize
extern BcU32 RsResourceFormatSize( RsResourceFormat TextureFormat, BcU32 Width, BcU32 Height, BcU32 Depth, BcU32 Levels );

//////////////////////////////////////////////////////////////////////////
// RsTexturePitch
extern BcU32 RsTexturePitch( RsResourceFormat TextureFormat, BcU32 Width, BcU32 Height );

//////////////////////////////////////////////////////////////////////////
// RsTextureSlicePitch
extern BcU32 RsTextureSlicePitch( RsResourceFormat TextureFormat, BcU32 Width, BcU32 Height );

//////////////////////////////////////////////////////////////////////////
// RsTextureFace
enum class RsTextureFace : BcU32
{
	NONE = 0,
	POSITIVE_X = 0,
	NEGATIVE_X,
	POSITIVE_Y,
	NEGATIVE_Y,
	POSITIVE_Z,
	NEGATIVE_Z,
};

//////////////////////////////////////////////////////////////////////////
// RsTextureSlice
struct RsTextureSlice
{
	BcU32 Level_;
	RsTextureFace Face_;
};

//////////////////////////////////////////////////////////////////////////
// Texture filtering mode.
enum class RsTextureFilteringMode : BcU32
{
	NEAREST = 0,
	LINEAR,
	NEAREST_MIPMAP_NEAREST,
	LINEAR_MIPMAP_NEAREST,
	NEAREST_MIPMAP_LINEAR,
	LINEAR_MIPMAP_LINEAR,

	MAX,
	INVALID = BcErrorCode
};

//////////////////////////////////////////////////////////////////////////
// Texture sampling mode.
enum class RsTextureSamplingMode : BcU32
{
	WRAP = 0,
	MIRROR,
	CLAMP,
	DECAL,

	MAX,	
	INVALID = BcErrorCode
};


//////////////////////////////////////////////////////////////////////////
// Texture params.
struct RsTextureParams
{
	RsTextureFilteringMode		MinFilter_;
	RsTextureFilteringMode		MagFilter_;
	RsTextureSamplingMode		UMode_;
	RsTextureSamplingMode		VMode_;
	RsTextureSamplingMode		WMode_;
	
	bool operator == ( const RsTextureParams& Other )
	{
		return ( MinFilter_ == Other.MinFilter_ ) &&
		       ( MagFilter_ == Other.MagFilter_ ) &&
		       ( UMode_ == Other.UMode_ ) &&
			   ( VMode_ == Other.VMode_ ) &&
		       ( WMode_ == Other.WMode_ );
	}

	bool operator != ( const RsTextureParams& Other )
	{
		return ( MinFilter_ != Other.MinFilter_ ) ||
		       ( MagFilter_ != Other.MagFilter_ ) ||
		       ( UMode_ != Other.UMode_ ) ||
		       ( VMode_ != Other.VMode_ ) ||
		       ( WMode_ != Other.WMode_ );
	}
};

//////////////////////////////////////////////////////////////////////////
// Primitive Types.
enum class RsTopologyType : BcU32
{
	POINTS = 0,
	LINE_LIST,
	LINE_STRIP,
	LINE_LIST_ADJACENCY,
	LINE_STRIP_ADJACENCY,
	TRIANGLE_LIST,
	TRIANGLE_STRIP,
	TRIANGLE_LIST_ADJACENCY,
	TRIANGLE_STRIP_ADJACENCY,
	TRIANGLE_FAN,
	PATCH_LIST,

	MAX,	
	INVALID = BcErrorCode
};

//////////////////////////////////////////////////////////////////////////
// Handy defines
enum class RsShaderType : BcU32
{
	VERTEX,
	PIXEL,
	HULL,
	DOMAIN,
	GEOMETRY,
	COMPUTE,
		
	MAX,
	INVALID = BcErrorCode
};

enum class RsShaderDataType : BcU32
{
	BINARY = 0,
	SOURCE,

	MAX,
	INVALID = BcErrorCode
};

enum class RsShaderResourceType : BcU32
{
	BUFFER = 0,
	TEXTURE,
	
	MAX,
	INVALID = BcErrorCode
};

enum class RsUnorderedAccessType : BcU32
{
	BUFFER = 0,
	TEXTURE,
	
	MAX,
	INVALID = BcErrorCode
};

//////////////////////////////////////////////////////////////////////////
// RsAPI
enum class RsAPI : BcU32
{
	NULL_RENDERER,
	OPENGL,
	D3D12,
	VULKAN,

	MAX,
	INVALID = BcErrorCode
};

//////////////////////////////////////////////////////////////////////////
// RsShaderBackendType
enum class RsShaderBackendType : BcU32
{
	GLSL,
	GLSL_ES,
	D3D11,
	D3D12,
	SPIRV,

	MAX,
	INVALID = BcErrorCode
};

//////////////////////////////////////////////////////////////////////////
// RsShaderCodeType
enum class RsShaderCodeType : BcU32
{
	// GLSL
	GLSL_140,		// GL 3.1+
	GLSL_150,		// GL 3.2+
	GLSL_330,		// GL 3.3+
	GLSL_400,		// GL 4.0+
	GLSL_410,		// GL 4.1+
	GLSL_420,		// GL 4.2+
	GLSL_430,		// GL 4.3+
	GLSL_440,		// GL 4.4+
	GLSL_450,		// GL 4.5+
	
	// GLSL ES
	GLSL_ES_100,	// GL ES 2.0+
	GLSL_ES_300,	// GL ES 3.0+
	GLSL_ES_310,	// GL ES 3.0+

	// D3D11
	D3D11_4_0_LEVEL_9_1,
	D3D11_4_0_LEVEL_9_2,
	D3D11_4_0_LEVEL_9_3,
	D3D11_4_0,
	D3D11_4_1,
	D3D11_5_0,
	D3D11_5_1,

	// D3D12

	// SPIR-V
	SPIRV,

	//
	MAX,
	INVALID = BcErrorCode
};

inline RsShaderCodeType& operator ++ ( RsShaderCodeType& A )
{
	((int&)A)++;
	return A;
}

/**
 * Convert shader code type to backend.
 */
RsShaderBackendType RsShaderCodeTypeToBackendType( RsShaderCodeType CodeType );

/**
 * Convert shader code type to string.
 */
std::string RsShaderCodeTypeToString( RsShaderCodeType CodeType );

/**
 * Convert backend type to string.
 */
std::string RsShaderBackendTypeToString( RsShaderBackendType BackendType );

/**
 * Convert string to shader code type.
 */
RsShaderCodeType RsStringToShaderCodeType( std::string String );

/**
 * Convert code type to another code type of a specific backend.
 */
RsShaderCodeType RsConvertCodeTypeToBackendCodeType( RsShaderCodeType CodeType, RsShaderBackendType Backend );

/**
 * Is code type compatible with a higher code type?
 */
BcBool RsIsLowerCodeTypeCompatibleWithHigher( RsShaderCodeType LowerCodeType, RsShaderCodeType HigherCodeType );

//////////////////////////////////////////////////////////////////////////
// RsProgramUniformType
enum class RsProgramUniformType : BcU32
{
	FLOAT = 0,
	FLOAT_VEC2,
	FLOAT_VEC3,
	FLOAT_VEC4,
	FLOAT_MAT2,
	FLOAT_MAT3,
	FLOAT_MAT4,
	INT,
	INT_VEC2,
	INT_VEC3,
	INT_VEC4,
	UINT,
	UINT_VEC2,
	UINT_VEC3,
	UINT_VEC4,
	BOOL,
	BOOL_VEC2,
	BOOL_VEC3,
	BOOL_VEC4,
	SAMPLER_1D,
	SAMPLER_2D,
	SAMPLER_3D,
	SAMPLER_CUBE,
	SAMPLER_1D_SHADOW,
	SAMPLER_2D_SHADOW,
	TEXTURE,
	
	MAX,
	INVALID = BcErrorCode,
};

//////////////////////////////////////////////////////////////////////////
// RsProgramVertexAttribute
struct RsProgramVertexAttribute
{
	BcU32							Channel_;
	RsVertexUsage					Usage_;
	BcU32							UsageIdx_;
};

typedef std::vector< RsProgramVertexAttribute > RsProgramVertexAttributeList;

//////////////////////////////////////////////////////////////////////////
// RsProgramParameterType
enum class RsProgramParameterType : BcU32
{
	UNKNOWN,
	SAMPLER,
	SHADER_RESOURCE,
	UNORDERED_ACCESS,
	UNIFORM_BLOCK,
};

//////////////////////////////////////////////////////////////////////////
// RsProgramParameter
struct RsProgramParameter
{
	/// Name of parameter.
	BcChar Name_[ 64 ];
	/// Type of parameter.
	RsProgramParameterType Type_;
	/// Size of parameter.
	BcU32 Size_;
	/// Internal type of parameter.
	BcU32 InternalType_;
};

typedef std::vector< RsProgramParameter > RsProgramParameterList;

//////////////////////////////////////////////////////////////////////////
// Resource stuff
enum class RsResourceCreationFlags : BcU32
{
	NONE			= 0x00000000,
	STATIC			= 0x00000001,
	DYNAMIC			= 0x00000002,
	STREAM			= 0x00000004,
};

DEFINE_ENUM_CLASS_FLAG_OPERATOR( RsResourceCreationFlags, | );
DEFINE_ENUM_CLASS_FLAG_OPERATOR( RsResourceCreationFlags, & );

enum class RsResourceBindFlags : BcU32
{
	NONE				= 0x00000000,
	VERTEX_BUFFER		= 0x00000001,
	INDEX_BUFFER		= 0x00000002,
	UNIFORM_BUFFER		= 0x00000004,
	SHADER_RESOURCE		= 0x00000008,
	STREAM_OUTPUT		= 0x00000010,
	RENDER_TARGET		= 0x00000020,
	DEPTH_STENCIL		= 0x00000040,
	UNORDERED_ACCESS	= 0x00000080,
	PRESENT				= 0x00000100,
	TRANSIENT			= 0x00000200
};

DEFINE_ENUM_CLASS_FLAG_OPERATOR( RsResourceBindFlags, | );
DEFINE_ENUM_CLASS_FLAG_OPERATOR( RsResourceBindFlags, & );

enum class RsResourceUpdateFlags : BcU32
{
	NONE			= 0x00000000,
	ASYNC			= 0x00000001,
};

DEFINE_ENUM_CLASS_FLAG_OPERATOR( RsResourceUpdateFlags, | );
DEFINE_ENUM_CLASS_FLAG_OPERATOR( RsResourceUpdateFlags, & );

//////////////////////////////////////////////////////////////////////////
// Buffer stuff
struct RsBufferLock
{
	void* Buffer_;
};

typedef std::function< void( class RsBuffer*, const RsBufferLock& ) > RsBufferUpdateFunc;


//////////////////////////////////////////////////////////////////////////
// Texture stuff

enum class RsTextureType : BcU32
{
	UNKNOWN = 0,
	TEX1D,
	TEX2D,
	TEX3D,
	TEXCUBE,
};

// 1D: (const char*)Buffer_ + ( X * BytesPerPixel )
// 2D: (const char*)Buffer_ + Pitch_ * X + ( X  * BytesPerPixel )
// 3D: (const char*)Buffer_ + SlicePitch_ * D + Pitch_ * Y + ( X * BytesPerPixel )
struct RsTextureLock
{
	void* Buffer_;
	BcU32 Pitch_;
	BcU32 SlicePitch_;
};

typedef std::function< void( class RsTexture*, const RsTextureLock& ) > RsTextureUpdateFunc;

//////////////////////////////////////////////////////////////////////////
// Screenshot

struct RsScreenshot
{
	void* Data_;
	BcU32 Width_;
	BcU32 Height_;
	RsResourceFormat Format_;
};

typedef std::function< BcBool( const RsScreenshot& ) > RsScreenshotFunc;

//////////////////////////////////////////////////////////////////////////
// RsScissorRect
struct RsScissorRect
{
	RsScissorRect()
	{}

	RsScissorRect( BcS32 X, BcS32 Y, BcS32 Width, BcS32 Height ):
		X_( X ),
		Y_( Y ),
		Width_( Width ),
		Height_( Height )
	{}

	bool operator == ( const RsScissorRect& B ) const
	{
		return  X_ == B.X_ &&
			Y_ == B.Y_ &&
			Width_ == B.Width_ &&
			Height_ == B.Height_;
	}

	bool operator != ( const RsScissorRect& B ) const
	{
		return  X_ != B.X_ ||
			Y_ != B.Y_ ||
			Width_ != B.Width_ ||
			Height_ != B.Height_;
	}

	BcS32 X_ = 0;
	BcS32 Y_ = 0;
	BcS32 Width_ = 0;
	BcS32 Height_ = 0;
};

//////////////////////////////////////////////////////////////////////////
// RsQueryType
enum class RsQueryType : BcU32
{
	UNKNOWN = 0,
	OCCLUSION,
	OCCLUSION_BINARY,
	TIMESTAMP,
};


#endif

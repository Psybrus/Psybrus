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
// Datatypes
class RsColour:
	public MaVec4d
{
public:
	REFLECTION_DECLARE_BASIC( RsColour );

public:
	RsColour();
	RsColour( const MaVec4d& Vec );
	RsColour( BcU32 RGBA );
	RsColour( BcF32 R, BcF32 G, BcF32 B, BcF32 A );

	BcU32 asRGBA() const;
	BcU32 asARGB() const;
	BcU32 asABGR() const;
	BcU32 asBGRA() const;

	inline BcF32 r() const { return x(); };
	inline BcF32 g() const { return y(); };
	inline BcF32 b() const { return z(); };
	inline BcF32 a() const { return w(); };

	inline void r( BcF32 V ){ x( V ); };
	inline void g( BcF32 V ){ y( V ); };
	inline void b( BcF32 V ){ z( V ); };
	inline void a( BcF32 V ){ w( V ); };

	void premultiplyAlpha();

public:
	static RsColour FromHTMLColourCode( const BcChar* String );

	static const RsColour WHITE;
	static const RsColour BLACK;
	static const RsColour GRAY;
	static const RsColour RED;
	static const RsColour GREEN;
	static const RsColour BLUE;
	static const RsColour ORANGE;
	static const RsColour YELLOW;
	static const RsColour MAGENTA;
	static const RsColour PURPLE;
	static const RsColour CYAN;
};

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
	CW, 
	CCW,

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
// Colour Format (RT)
enum class RsColourFormat : BcU32
{
	NONE = 0,
	A2R10G10B10,
	A8R8G8B8,
	X8R8G8B8,
	A1R5G5B5,
	X1R5G5B5,
	R5G6B5,
	R16F,
	G16R16F,
	A16B16G16R16F,
	R32F,
	G32R32F,
	A32B32G32R32F,
	
	MAX,
	INVALID = BcErrorCode
};

//////////////////////////////////////////////////////////////////////////
// Depth Stencil Format (RT)
enum class RsDepthStencilFormat : BcU32
{
	NONE = 0,
	D16,
	D32,
	D15S1,
	D24S8,
	D24X8,
	D24X4S4,
	D32F,
	D24FS8,
	
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
// Texture Format
enum class RsTextureFormat : BcU32
{
	UNKNOWN,

	// Colour.
	R8,
	R8G8,
	R8G8B8,
	R8G8B8A8,
	R16F,
	R16FG16F,
	R16FG16FB16F,
	R16FG16FB16FA16F,
	R32F,
	R32FG32F,
	R32FG32FB32F,
	R32FG32FB32FA32F,
	DXT1,
	DXT3,
	DXT5,
	ETC1,

	// Depth stencil.
	D16,
	D24,
	D32,
	D24S8,
	D32F,
	
	MAX,
	INVALID = BcErrorCode
};

//////////////////////////////////////////////////////////////////////////
// RsTextureBlockInfo
struct RsBlockInfo
{
	BcU32 Width_;
	BcU32 Height_;
	BcU32 Bits_;
};

extern RsBlockInfo RsTextureBlockInfo( RsTextureFormat TextureFormat );

//////////////////////////////////////////////////////////////////////////
// RsTextureFormatSize
extern BcU32 RsTextureFormatSize( RsTextureFormat TextureFormat, BcU32 Width, BcU32 Height, BcU32 Depth, BcU32 Levels );

//////////////////////////////////////////////////////////////////////////
// RsTexturePitch
extern BcU32 RsTexturePitch( RsTextureFormat TextureFormat, BcU32 Width, BcU32 Height );

//////////////////////////////////////////////////////////////////////////
// RsTextureSlicePitch
extern BcU32 RsTextureSlicePitch( RsTextureFormat TextureFormat, BcU32 Width, BcU32 Height );

//////////////////////////////////////////////////////////////////////////
// RsTextureFace
enum class RsTextureFace : BcU32
{
	NONE,
	POSITIVE_X,
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
// Render state
enum class RsRenderStateType : BcU32
{
	DEPTH_WRITE_ENABLE = 0,		///!< Depth write enable/disable. true or false.
	DEPTH_TEST_ENABLE,				///!< Depth test enable/disable. true or false.
	DEPTH_TEST_COMPARE,			///!< Depth test compare. RsCompareMode
	STENCIL_WRITE_MASK,			///!< Stencil write mask.
	STENCIL_TEST_ENABLE,			///!< Stencil test enable.
	STENCIL_TEST_FUNC_COMPARE,		///!< Stencil test compare.
	STENCIL_TEST_FUNC_REF,			///!< Stencil test reference value.
	STENCIL_TEST_FUNC_MASK,		///!< Stencil test mask.
	STENCIL_TEST_OP_SFAIL,			///!< Stencil test fail operation.
	STENCIL_TEST_OP_DPFAIL,		///!< Stencil test passes, but depth fails operation.
	STENCIL_TEST_OP_DPPASS,		///!< Stencil test and depth pass operation.
	COLOR_WRITE_MASK_0,			///!< Color write mask, RT 0. Bits 0-3, RGBA
	COLOR_WRITE_MASK_1,			///!< Color write mask, RT 1. Bits 0-3, RGBA
	COLOR_WRITE_MASK_2,			///!< Color write mask, RT 2. Bits 0-3, RGBA
	COLOR_WRITE_MASK_3,			///!< Color write mask, RT 3. Bits 0-3, RGBA
	BLEND_MODE,					///!< Blend mode (simple). eRsBlendMode.
	FILL_MODE,					///!< Fill mode. RsFillMode.
	
	MAX,
	INVALID = BcErrorCode
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

//////////////////////////////////////////////////////////////////////////
// RsShaderBackendType
enum class RsShaderBackendType : BcU32
{
	GLSL,
	GLSL_ES,
	D3D9,
	D3D11,
	D3D12,
	MANTLE,

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

	// D3D9
	D3D9_3_0,
	
	// D3D11
	D3D11_4_0_LEVEL_9_0,
	D3D11_4_0_LEVEL_9_1,
	D3D11_4_0_LEVEL_9_3,
	D3D11_4_0,
	D3D11_4_1,
	D3D11_5_0,
	D3D11_5_1,

	// D3D12

	// Mantle

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
// RsShaderParameterType
enum class RsShaderParameterType : BcU32
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
// Resource stuff
enum class RsResourceCreationFlags : BcU32
{
	NONE			= 0x00000000,
	STATIC			= 0x00000001,
	DYNAMIC			= 0x00000002,
	STREAM			= 0x00000004,
};

inline RsResourceCreationFlags operator & ( RsResourceCreationFlags A, RsResourceCreationFlags B )
{
	return (RsResourceCreationFlags)( (BcU32)A & (BcU32)B );
}

inline RsResourceCreationFlags operator | ( RsResourceCreationFlags A, RsResourceCreationFlags B )
{
	return (RsResourceCreationFlags)( (BcU32)A | (BcU32)B );
}

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
	PRESENT				= 0x00000100
};

inline RsResourceBindFlags operator & ( RsResourceBindFlags A, RsResourceBindFlags B )
{
	return (RsResourceBindFlags)( (BcU32)A & (BcU32)B );
}

inline RsResourceBindFlags operator | ( RsResourceBindFlags A, RsResourceBindFlags B )
{
	return (RsResourceBindFlags)( (BcU32)A | (BcU32)B );
}

enum class RsResourceUpdateFlags : BcU32
{
	NONE			= 0x00000000,
	ASYNC			= 0x00000001,
};

inline RsResourceUpdateFlags operator & ( RsResourceUpdateFlags A, RsResourceUpdateFlags B )
{
	return (RsResourceUpdateFlags)( (BcU32)A & (BcU32)B );
}

inline RsResourceUpdateFlags operator | ( RsResourceUpdateFlags A, RsResourceUpdateFlags B )
{
	return (RsResourceUpdateFlags)( (BcU32)A | (BcU32)B );
}

//////////////////////////////////////////////////////////////////////////
// Buffer stuff
enum class RsBufferType
{
	UNKNOWN = 0,
	VERTEX,
	INDEX,
	UNIFORM,
	UNORDERED_ACCESS,
	DRAW_INDIRECT,
	STREAM_OUT,
};

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

#endif

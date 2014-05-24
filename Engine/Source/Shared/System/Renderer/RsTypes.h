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
	static const RsColour WHITE;
	static const RsColour BLACK;
	static const RsColour GRAY;
	static const RsColour RED;
	static const RsColour GREEN;
	static const RsColour BLUE;
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
	MAX
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
	INV_DEST_ALPHA
};

//////////////////////////////////////////////////////////////////////////
// Blend op
enum class RsBlendOp : BcU32
{
	ADD = 0,
	SUBTRACT,
	REV_SUBTRACT,
	MIN,
	MAX
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
	INVALID
};

//////////////////////////////////////////////////////////////////////////
// Texture Type
enum class RsTextureType : BcU32
{
	TEX1D = 0,
	TEX2D,
	TEX3D,
	TEXCUBE,
	MAX
};

//////////////////////////////////////////////////////////////////////////
// Texture Format
enum class RsTextureFormat : BcU32
{
	RGB8 = 0,
	RGBA8,
	I8,

	
	DXT1,
	DXT3,
	DXT5,

	
	PAL4_RGBA8,
	PAL8_RGBA8,
	
	
	PVRTC2_RGB,
	PVRTC2_RGBA,
	PVRTC4_RGB,
	PVRTC4_RGBA,
	
	
	INVALID = BcErrorCode
};

//////////////////////////////////////////////////////////////////////////
// RsTextureFormatSize
extern BcU32 RsTextureFormatSize( RsTextureFormat TextureFormat, BcU32 Width, BcU32 Height, BcU32 Depth, BcU32 Levels );

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
	
	
	MAX
};

//////////////////////////////////////////////////////////////////////////
// Primitive Types.
enum class RsPrimitiveType : BcU32
{
	POINTLIST = 0,
	LINELIST,
	LINESTRIP,
	TRIANGLELIST,
	TRIANGLESTRIP,
	TRIANGLEFAN,
	PATCHES,
	MAX,
	
	INVALID = BcErrorCode
};

//////////////////////////////////////////////////////////////////////////
// Handy defines
enum class RsShaderType : BcU32
{
	INVALID = 0,
	VERTEX,
	TESSELATION_CONTROL,
	TESSELATION_EVALUATION,
	GEOMETRY,
	FRAGMENT,
	COMPUTE,
	
	
	MAX,
	FORCE_DWORD = 0x7fffffff
};

enum class RsShaderDataType : BcU32
{
	BINARY = 0,
	SOURCE
};

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
// Lock flags
enum eRsLockFlags
{
	rsLF_READ = 0x00000001,
	rsLF_WRITE = 0x00000002,
	
	//
	rsLF_FORCE_DWORD = 0x7fffffff
};

//////////////////////////////////////////////////////////////////////////
// Handy defines
#define RS_MAX_LIGHTINGSTAGES			1024
#define RS_MAX_PROJECTORSTAGES			1024
#define RS_MAX_LIGHTS					4

#define RS_RENDEROBJECTMEM				( 1024 * 1024 * 8 )

#endif

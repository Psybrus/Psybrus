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
#include "Base/BcVectors.h"
#include "Base/BcMat4d.h"
#include "Base/BcName.h"

//////////////////////////////////////////////////////////////////////////
// Datatypes
struct RsColour:
	public BcVec4d
{
	RsColour(){};
	RsColour( const BcVec4d& Vec ):
		BcVec4d( Vec )
	{};
	RsColour( BcU32 RGBA ):
		BcVec4d( ( ( RGBA ) & 0xff ) / 255.0f,
				 ( ( RGBA >> 8 ) & 0xff ) / 255.0f,
				 ( ( RGBA >> 16 ) & 0xff ) / 255.0f,
				 ( ( RGBA >> 24 ) & 0xff ) / 255.0f )
				
	{};
	RsColour( BcF32 R, BcF32 G, BcF32 B, BcF32 A ):
		BcVec4d( R, G, B, A )
	{};

	BcU32 asRGBA() const
	{
		return ( ( BcClamp( static_cast< BcU32 >( r() * 255.0f ), 0, 255 ) << 24 ) |
		         ( BcClamp( static_cast< BcU32 >( g() * 255.0f ), 0, 255 ) << 16 ) |
		         ( BcClamp( static_cast< BcU32 >( b() * 255.0f ), 0, 255 ) << 8 ) |
		         ( BcClamp( static_cast< BcU32 >( a() * 255.0f ), 0, 255 ) ) );
	}

	BcU32 asARGB() const
	{
		return ( ( BcClamp( static_cast< BcU32 >( a() * 255.0f ), 0, 255 ) << 24 ) |
		         ( BcClamp( static_cast< BcU32 >( r() * 255.0f ), 0, 255 ) << 16 ) |
		         ( BcClamp( static_cast< BcU32 >( g() * 255.0f ), 0, 255 ) << 8 ) |
		         ( BcClamp( static_cast< BcU32 >( b() * 255.0f ), 0, 255 ) ) );
	}

	BcU32 asABGR() const
	{
		return ( ( BcClamp( static_cast< BcU32 >( a() * 255.0f ), 0, 255 ) << 24 ) |
				 ( BcClamp( static_cast< BcU32 >( b() * 255.0f ), 0, 255 ) << 16 ) |
				 ( BcClamp( static_cast< BcU32 >( g() * 255.0f ), 0, 255 ) << 8 ) |
				 ( BcClamp( static_cast< BcU32 >( r() * 255.0f ), 0, 255 ) ) );
	}

	BcU32 asBGRA() const
	{
		return ( ( BcClamp( static_cast< BcU32 >( b() * 255.0f ), 0, 255 ) << 24 ) |
		         ( BcClamp( static_cast< BcU32 >( g() * 255.0f ), 0, 255 ) << 16 ) |
		         ( BcClamp( static_cast< BcU32 >( r() * 255.0f ), 0, 255 ) << 8 ) |
		         ( BcClamp( static_cast< BcU32 >( a() * 255.0f ), 0, 255 ) ) );
	}

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
enum eRsBlendingMode
{
	rsBM_NONE = 0,			// (+) ONE, ZERO
	rsBM_BLEND,				// (+) ONE, INVSRCALPHA 
	rsBM_ADD,				// (+) ONE, ONE
	rsBM_SUBTRACT,			// (-) ONE, ONE
	rsBM_MAX,
	
	rsBM_FORCE_DWORD = 0x7fffffff
};

//////////////////////////////////////////////////////////////////////////
// Blend type
enum eRsBlendType
{
	rsBT_ZERO = 0,
	rsBT_ONE,
	rsBT_SRC_COLOUR,
	rsBT_INV_SRC_COLOUR,
	rsBT_SRC_ALPHA,
	rsBT_INV_SRC_ALPHA,
	rsBT_DEST_COLOUR,
	rsBT_INV_DEST_COLOUR,
	rsBT_DEST_ALPHA,
	rsBT_INV_DEST_ALPHA,

	rsBT_FORCE_DWORD = 0x7fffffff
};

//////////////////////////////////////////////////////////////////////////
// Blend op
enum eRsBlendOp
{
	rsBO_ADD = 0,
	rsBO_SUBTRACT,
	rsBO_REV_SUBTRACT,
	rsBO_MIN,
	rsBO_MAX,

	rsBO_FORCE_DWORD = 0x7fffffff
};

//////////////////////////////////////////////////////////////////////////
// Compare modes
enum eRsCompareMode
{
	rsCM_NEVER = 0,
	rsCM_LESS,
	rsCM_EQUAL,
	rsCM_LESSEQUAL,
	rsCM_GREATER,
	rsCM_NOTEQUAL,
	rsCM_GREATEREQUAL,
	rsCM_ALWAYS,
	rsCM_MAX,
	//
	rsCM_INVALID = BcErrorCode,
	rsCM_FORCE_DWORD = 0x7fffffff
};

//////////////////////////////////////////////////////////////////////////
// Stencil operations
enum eRsStencilOp
{
	rsSO_KEEP = 0,
	rsSO_ZERO,
	rsSO_REPLACE,
	rsSO_INCR,
	rsSO_INCR_WRAP,
	rsSO_DECR,
	rsSO_DECR_WRAP,
	rsSO_INVERT,
	//
	rsSO_INVALID = BcErrorCode,
	rsSO_FORCE_DWORD = 0x7fffffff
};


//////////////////////////////////////////////////////////////////////////
// Colour Format (RT)
enum eRsColourFormat
{
	rsCF_A2R10G10B10 = 0,
	rsCF_A8R8G8B8,
	rsCF_X8R8G8B8,
	rsCF_A1R5G5B5,
	rsCF_X1R5G5B5,
	rsCF_R5G6B5,
	rsCF_R16F,
	rsCF_G16R16F,
	rsCF_A16B16G16R16F,
	rsCF_R32F,
	rsCF_G32R32F,
	rsCF_A32B32G32R32F,
	rsCF_MAX,
	//
	rsCF_INVALID = BcErrorCode,
	rsCF_FORCE_DWORD = 0x7fffffff
};

//////////////////////////////////////////////////////////////////////////
// Depth Stencil Format (RT)
enum eRsDepthStencilFormat
{
	rsDSF_D16 = 0,
	rsDSF_D32,
	rsDSF_D15S1,
	rsDSF_D24S8,
	rsDSF_D24X8,
	rsDSF_D24X4S4,
	rsDSF_D32F,
	rsDSF_D24FS8,
	rsDSF_MAX,
	//
	rsDSF_INVALID = BcErrorCode,
	rsDSF_FORCE_DWORD = 0x7fffffff
};

//////////////////////////////////////////////////////////////////////////
// Vertex data type.
enum eRsVertexDataType
{
	rsVDT_FLOAT32 = 0,
	rsVDT_FLOAT16,
	rsVDT_FIXED,
	rsVDT_BYTE,
	rsVDT_BYTE_NORM,
	rsVDT_UBYTE,
	rsVDT_UBYTE_NORM,
	rsVDT_SHORT,
	rsVDT_SHORT_NORM,
	rsVDT_USHORT,
	rsVDT_USHORT_NORM,
	rsVDT_INT,
	rsVDT_INT_NORM,
	rsVDT_UINT,
	rsVDT_UINT_NORM,
	
	rsVDF_MAX,
};

//////////////////////////////////////////////////////////////////////////
// Texture Type
enum eRsTextureType
{
	rsTT_1D = 0,
	rsTT_2D,
	rsTT_3D,
	rsTT_CUBEMAP,
	rsTT_MAX
};

//////////////////////////////////////////////////////////////////////////
// Texture Format
enum eRsTextureFormat
{
	//
	rsTF_RGB8 = 0,
	rsTF_RGBA8,
	rsTF_I8,

	// DXT
	rsTF_DXT1,
	rsTF_DXT3,
	rsTF_DXT5,

	// Palettised
	rsTF_PAL4_RGBA8,
	rsTF_PAL8_RGBA8,
	
	// PVRTC
	rsTF_PVRTC2_RGB,
	rsTF_PVRTC2_RGBA,
	rsTF_PVRTC4_RGB,
	rsTF_PVRTC4_RGBA,
	
	//
	rsTF_INVALID = BcErrorCode,
	rsTF_FORCE_DWORD = 0x7fffffff
};

//////////////////////////////////////////////////////////////////////////
// RsTextureFormatSize
extern BcU32 RsTextureFormatSize( eRsTextureFormat TextureFormat, BcU32 Width, BcU32 Height, BcU32 Depth, BcU32 Levels );

//////////////////////////////////////////////////////////////////////////
// Texture filtering mode.
enum eRsTextureFilteringMode
{
	rsTFM_NEAREST = 0,
	rsTFM_LINEAR,
	rsTFM_NEAREST_MIPMAP_NEAREST,
	rsTFM_LINEAR_MIPMAP_NEAREST,
	rsTFM_NEAREST_MIPMAP_LINEAR,
	rsTFM_LINEAR_MIPMAP_LINEAR,
	
	//
	rsTFM_MAX,
	
	//
	rsTFM_INVALID = BcErrorCode,
	rsTFM_FORCE_DWORD = 0x7fffffff
};

//////////////////////////////////////////////////////////////////////////
// Texture sampling mode.
enum eRsTextureSamplingMode
{
	rsTSM_WRAP = 0,
	rsTSM_MIRROR,
	rsTSM_CLAMP,
	rsTSM_DECAL,
	rsTSM_MAX,
	//
	rsTSM_INVALID = BcErrorCode,
	rsTSM_FORCE_DWORD = 0x7fffffff
};


//////////////////////////////////////////////////////////////////////////
// Texture params.
struct RsTextureParams
{
	eRsTextureFilteringMode		MinFilter_;
	eRsTextureFilteringMode		MagFilter_;
	eRsTextureSamplingMode		UMode_;
	eRsTextureSamplingMode		VMode_;
	eRsTextureSamplingMode		WMode_;
	
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
enum eRsRenderState
{
	rsRS_DEPTH_WRITE_ENABLE = 0,		///!< Depth write enable/disable. true or false.
	rsRS_DEPTH_TEST_ENABLE,				///!< Depth test enable/disable. true or false.
	rsRS_DEPTH_TEST_COMPARE,			///!< Depth test compare. eRsCompareMode
	rsRS_STENCIL_WRITE_MASK,			///!< Stencil write mask.
	rsRS_STENCIL_TEST_ENABLE,			///!< Stencil test enable.
	rsRS_STENCIL_TEST_FUNC_COMPARE,		///!< Stencil test compare.
	rsRS_STENCIL_TEST_FUNC_REF,			///!< Stencil test reference value.
	rsRS_STENCIL_TEST_FUNC_MASK,		///!< Stencil test mask.
	rsRS_STENCIL_TEST_OP_SFAIL,			///!< Stencil test fail operation.
	rsRS_STENCIL_TEST_OP_DPFAIL,		///!< Stencil test passes, but depth fails operation.
	rsRS_STENCIL_TEST_OP_DPPASS,		///!< Stencil test and depth pass operation.
	rsRS_COLOR_WRITE_MASK_0,			///!< Color write mask, RT 0. Bits 0-3, RGBA
	rsRS_COLOR_WRITE_MASK_1,			///!< Color write mask, RT 1. Bits 0-3, RGBA
	rsRS_COLOR_WRITE_MASK_2,			///!< Color write mask, RT 2. Bits 0-3, RGBA
	rsRS_COLOR_WRITE_MASK_3,			///!< Color write mask, RT 3. Bits 0-3, RGBA
	rsRS_BLEND_MODE,					///!< Blend mode (simple). eRsBlendMode.
	
	//
	rsRS_MAX,
	rsRS_FORCE_DWORD = 0x7fffffff
};

//////////////////////////////////////////////////////////////////////////
// Primitive Types.
enum eRsPrimitiveType
{
	rsPT_POINTLIST = 0,
	rsPT_LINELIST,
	rsPT_LINESTRIP,
	rsPT_TRIANGLELIST,
	rsPT_TRIANGLESTRIP,
	rsPT_TRIANGLEFAN,
	rsPT_PATCHES,
	rsPT_MAX,
	//
	rsPT_INVALID = BcErrorCode,
	rsPT_FORCE_DWORD = 0x7fffffff
};

//////////////////////////////////////////////////////////////////////////
// Vertex enums
enum eRsVertexDeclFlags
{
	// Position: 0x1
	rsVDF_POSITION_XY		= 0x00000001,
	rsVDF_POSITION_XYZ		= 0x00000002,
	rsVDF_POSITION_XYZW		= 0x00000004,

	// Normal: 0x10
	rsVDF_NORMAL_XYZ		= 0x00000010,

	// Tangent: 0x100
	rsVDF_TANGENT_XYZ		= 0x00000100,

	// Texcoord UV: 0x1000
	rsVDF_TEXCOORD_UV0		= 0x00001000,
	rsVDF_TEXCOORD_UV1		= 0x00002000,
	rsVDF_TEXCOORD_UV2		= 0x00004000,
	rsVDF_TEXCOORD_UV3		= 0x00008000,

	// Texcoord UVW: 0x10000
	rsVDF_TEXCOORD_UVW0		= 0x00010000,
	rsVDF_TEXCOORD_UVW1		= 0x00020000,
	rsVDF_TEXCOORD_UVW2		= 0x00040000,
	rsVDF_TEXCOORD_UVW3		= 0x00080000,

	// Skinning: 0x100000
	rsVDF_SKIN_INDICES		= 0x00100000,
	rsVDF_SKIN_WEIGHTS		= 0x00200000,

	// Colour: 0x1000000
	rsVDF_COLOUR_ABGR8		= 0x01000000,

	//
	rsVDF_FORCE_DWORD		= 0x7fffffff
};

enum eRsVertexChannel
{
	rsVC_POSITION = 0,
	rsVC_NORMAL,
	rsVC_TANGENT,
	rsVC_TEXCOORD0,
	rsVC_TEXCOORD1,
	rsVC_TEXCOORD2,
	rsVC_TEXCOORD3,
	rsVC_SKIN_INDICES,
	rsVC_SKIN_WEIGHTS,
	rsVC_COLOUR,

	//
	rsVC_MAX,
	rsVC_FORCE_DWORD = 0x7fffffff
};

enum eRsVertexChannelMask
{
	rsVCM_POSITION = rsVDF_POSITION_XY | rsVDF_POSITION_XYZ | rsVDF_POSITION_XYZW,
	rsVCM_NORMAL = rsVDF_NORMAL_XYZ,
	rsVCM_TANGENT = rsVDF_TANGENT_XYZ,
	rsVCM_TEXCOORD0 = rsVDF_TEXCOORD_UV0,
	rsVCM_TEXCOORD1 = rsVDF_TEXCOORD_UV1,
	rsVCM_TEXCOORD2 = rsVDF_TEXCOORD_UV2,
	rsVCM_TEXCOORD3 = rsVDF_TEXCOORD_UV3,
	rsVCM_COLOUR = rsVDF_COLOUR_ABGR8,

	//
	rsVCM_FORCE_DWORD = 0x7fffffff
};

extern BcU32 RsVertexDeclSize( BcU32 VertexFormat );

//////////////////////////////////////////////////////////////////////////
// Handy defines
enum eRsShaderType
{
	rsST_VERTEX = 0,
	rsST_TESSELATION_CONTROL,
	rsST_TESSELATION_EVALUATION,
	rsST_GEOMETRY,
	rsST_FRAGMENT,
	rsST_COMPUTE,
	
	//
	rsST_MAX,
	rsST_FORCE_DWORD = 0x7fffffff
};

enum eRsShaderDataType
{
	rsSDT_BINARY = 0,
	rsSDT_SOURCE
};

enum eRsShaderParameterType
{
	rsSPT_FLOAT = 0,
	rsSPT_FLOAT_VEC2,
	rsSPT_FLOAT_VEC3,
	rsSPT_FLOAT_VEC4,
	rsSPT_FLOAT_MAT2,
	rsSPT_FLOAT_MAT3,
	rsSPT_FLOAT_MAT4,
	rsSPT_INT,
	rsSPT_INT_VEC2,
	rsSPT_INT_VEC3,
	rsSPT_INT_VEC4,
	rsSPT_BOOL,
	rsSPT_BOOL_VEC2,
	rsSPT_BOOL_VEC3,
	rsSPT_BOOL_VEC4,
	rsSPT_SAMPLER_1D,
	rsSPT_SAMPLER_2D,
	rsSPT_SAMPLER_3D,
	rsSPT_SAMPLER_CUBE,
	rsSPT_SAMPLER_1D_SHADOW,
	rsSPT_SAMPLER_2D_SHADOW,
	
	rsSPT_MAX,
	rsSPT_INVALID = BcErrorCode,
	rsSPT_FORCE_DWORD = 0x7fffffff
};

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


//////////////////////////////////////////////////////////////////////////
// RsRenderSort
union RsRenderSort
{
	RsRenderSort()
	{
	
	}
	
	RsRenderSort( BcU64 Value ):
		Value_( Value )
	{
	
	}
	
	RsRenderSort( const RsRenderSort& Sort ):
		Value_( Sort.Value_ )
	{
	
	}
	
	BcU64				Value_;
	
#if PSY_ENDIAN_LITTLE
	struct 
	{
		BcU64			MaterialID_		: 16;		// 16
		BcU64			Depth_			: 24;		// 40
		BcU64			Blend_			: 2;		// 42
		BcU64			Layer_			: 4;		// 46
		BcU64			Pass_			: 2;		// 48
		BcU64			Viewport_		: 8;		// 56
		BcU64			RenderTarget_	: 4;		// 60
		BcU64			NodeType_		: 4;		// 64
	};
#elif PSY_ENDIAN_BIG
	struct 
	{
		BcU64			NodeType_		: 4;		// 64
		BcU64			RenderTarget_	: 4;		// 60
		BcU64			Viewport_		: 8;		// 56
		BcU64			Pass_			: 2;		// 48
		BcU64			Layer_			: 4;		// 46
		BcU64			Blend_			: 2;		// 42
		BcU64			Depth_			: 24;		// 40
		BcU64			MaterialID_		: 16;		// 16
	};
#endif
};

#define RS_SORT_MATERIAL_SHIFT			BcU64( 0 )
#define RS_SORT_DEPTH_SHIFT				BcU64( 16 )
#define RS_SORT_BLEND_SHIFT				BcU64( 40 )
#define RS_SORT_LAYER_SHIFT				BcU64( 42 )
#define RS_SORT_PASS_SHIFT				BcU64( 46 )
#define RS_SORT_VIEWPORT_SHIFT			BcU64( 48 )
#define RS_SORT_RENDERTARGET_SHIFT		BcU64( 56 )
#define RS_SORT_NODETYPE_SHIFT			BcU64( 60 )

#define RS_SORT_MATERIAL_MAX			BcU64( 0x000000000000ffff )
#define RS_SORT_DEPTH_MAX				BcU64( 0x0000000000ffffff )
#define RS_SORT_BLEND_MAX				BcU64( 0x0000000000000003 )
#define RS_SORT_LAYER_MAX				BcU64( 0x000000000000000f )
#define RS_SORT_PASS_MAX				BcU64( 0x0000000000000003 )
#define RS_SORT_VIEWPORT_MAX			BcU64( 0x00000000000000ff )
#define RS_SORT_RENDERTARGET_MAX		BcU64( 0x000000000000000f )
#define RS_SORT_NODETYPE_MAX			BcU64( 0x000000000000000f )

#define RS_SORT_PASS_DEFERRED			BcU64( 0x0000000000000000 )
#define RS_SORT_PASS_FORWARD			BcU64( 0x0000000000000001 )
#define RS_SORT_PASS_POSTPROCESS		BcU64( 0x0000000000000002 )
#define RS_SORT_PASS_OVERLAY			BcU64( 0x0000000000000003 )

#define RS_SORT_NODETYPE_ z		RS_SORT_NODETYPE_MAX
#define RS_SORT_NODETYPE_RESOURCE		BcU64( 0 )

#define RS_SORT_MACRO_VIEWPORT_RENDERTARGET( _V, _T ) \
	BcU64(	( ( BcU64( _V ) & RS_SORT_VIEWPORT_MAX ) << RS_SORT_VIEWPORT_SHIFT ) | \
	( ( BcU64( _T ) & RS_SORT_RENDERTARGET_MAX ) << RS_SORT_RENDERTARGET_SHIFT ) )

#endif

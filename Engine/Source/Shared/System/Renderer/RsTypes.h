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

#include "BcTypes.h"
#include "BcVectors.h"
#include "BcMat4d.h"

//////////////////////////////////////////////////////////////////////////
// Datatypes
struct RsColour:
	public BcVec4d
{
	RsColour(){};
	RsColour( const BcVec4d& Vec ):
		BcVec4d( Vec )
	{};
	RsColour( BcReal R, BcReal G, BcReal B, BcReal A ):
		BcVec4d( R, G, B, A )
	{};

	BcU32 asRGBA() const
	{
		return ( ( static_cast< BcU32 >( r() * 255.0f ) << 24 ) |
		         ( static_cast< BcU32 >( g() * 255.0f ) << 16 ) |
		         ( static_cast< BcU32 >( b() * 255.0f ) << 8 ) |
		         ( static_cast< BcU32 >( a() * 255.0f ) ) );
	}

	BcU32 asARGB() const
	{
		return ( ( static_cast< BcU32 >( a() * 255.0f ) << 24 ) |
		         ( static_cast< BcU32 >( r() * 255.0f ) << 16 ) |
		         ( static_cast< BcU32 >( g() * 255.0f ) << 8 ) |
		         ( static_cast< BcU32 >( b() * 255.0f ) ) );
	}

	BcU32 asBGRA() const
	{
		return ( ( static_cast< BcU32 >( b() * 255.0f ) << 24 ) |
		         ( static_cast< BcU32 >( g() * 255.0f ) << 16 ) |
		         ( static_cast< BcU32 >( r() * 255.0f ) << 8 ) |
		         ( static_cast< BcU32 >( a() * 255.0f ) ) );
	}

	inline BcReal r() const { return x(); };
	inline BcReal g() const { return y(); };
	inline BcReal b() const { return z(); };
	inline BcReal a() const { return w(); };

	inline void r( BcReal V ){ x( V ); };
	inline void g( BcReal V ){ y( V ); };
	inline void b( BcReal V ){ z( V ); };
	inline void a( BcReal V ){ w( V ); };

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
	static const RsColour CYAN;
};

//////////////////////////////////////////////////////////////////////////
// Texture Types
enum eRsTextureType
{
	rsTT_DIFFUSE = 0,
	rsTT_SPECULAR,
	rsTT_NORMAL,
	rsTT_PROJECTED,
	rsTT_MAX,
	
	rsTT_FORCE_DWORD = 0x7fffffff
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
// Compare modes
enum eRsCompareMode
{
	rsCM_NEVER,
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
// Texture Format
enum eRsTextureFormat
{
	// 32bit
	rsTF_RGBA8 = 0,

	// Palettised
	rsTF_PAL4_RGBA8,
	rsTF_PAL8_RGBA8,
	
	// DXT
	rsTF_DXT1,
	rsTF_DXT2,
	rsTF_DXT3,
	rsTF_DXT4,
	rsTF_DXT5,
	
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
// RsTextureFormatBPP
extern BcU32 RsTextureFormatBPP( eRsTextureFormat TextureFormat );

//////////////////////////////////////////////////////////////////////////
// RsTextureFormatSize
extern BcU32 RsTextureFormatSize( eRsTextureFormat TextureFormat, BcU32 Width, BcU32 Height, BcU32 Levels );

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
	eRsTextureSamplingMode		UMode_;
	eRsTextureSamplingMode		VMode_;
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

	// Texcoord: 0x1000
	rsVDF_TEXCOORD_UV0		= 0x00001000,
	rsVDF_TEXCOORD_UV1		= 0x00002000,
	rsVDF_TEXCOORD_UV2		= 0x00004000,
	rsVDF_TEXCOORD_UV3		= 0x00008000,

	// Colour: 0x10000
	rsVDF_COLOUR_RGBA8		= 0x00010000,
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
	rsVC_COLOUR
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
	rsVCM_COLOUR = rsVDF_COLOUR_RGBA8
};

extern BcU32 RsVertexDeclSize( BcU32 VertexFormat );

//////////////////////////////////////////////////////////////////////////
// Handy defines
enum eRsShaderType
{
	rsST_GEOMETRY = 0,
	rsST_VERTEX,
	rsST_FRAGMENT,
	
	rsST_MAX
};

enum eRsShaderDataType
{
	rsSDT_BINARY = 0,
	rsSDT_SOURCE
};

//////////////////////////////////////////////////////////////////////////
// Lock flags
enum eRsLockFlags
{
	rsLF_READ = 0x00000001,
	rsLF_WRITE = 0x00000002,
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
	
#if defined( PSY_ENDIAN_LITTLE )
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
#elif defined( PSY_ENDIAN_BIG )
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

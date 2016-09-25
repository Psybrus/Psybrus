/**************************************************************************
*
* File:		RsRenderNode.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Renderable instance. Used to perform rendering.
*		
*
*
* 
**************************************************************************/

#ifndef __RSRENDERNODE_H__
#define __RSRENDERNODE_H__

#include "Base/BcMath.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class RsFrame;
class RsCore;
class RsContext;

//////////////////////////////////////////////////////////////////////////
// RsRenderSort
class RsRenderSort
{
public:
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
	
	union
	{
		BcU64				Value_;
	
		struct 
		{
			BcU64			MaterialID_		: 16;		// 16
			BcU64			Depth_			: 24;		// 40
			BcU64			Blend_			: 2;		// 42
			BcU64			Layer_			: 4;		// 46
			BcU64			Pass_			: 4;		// 50
			BcU64			Viewport_		: 8;		// 58
			BcU64			RenderTarget_	: 4;		// 62
		};
	};
};

#define RS_SORT_MATERIAL_SHIFT			BcU64( 0 )
#define RS_SORT_DEPTH_SHIFT				BcU64( 16 )
#define RS_SORT_BLEND_SHIFT				BcU64( 40 )
#define RS_SORT_LAYER_SHIFT				BcU64( 42 )
#define RS_SORT_PASS_SHIFT				BcU64( 46 )
#define RS_SORT_VIEWPORT_SHIFT			BcU64( 50 )
#define RS_SORT_RENDERTARGET_SHIFT		BcU64( 58 )

#define RS_SORT_MATERIAL_MAX			BcU64( 0x000000000000ffff )
#define RS_SORT_DEPTH_MAX				BcU64( 0x0000000000ffffff )
#define RS_SORT_BLEND_MAX				BcU64( 0x0000000000000003 )
#define RS_SORT_LAYER_MAX				BcU64( 0x000000000000000f )
#define RS_SORT_PASS_MAX				BcU64( 0x000000000000000f )
#define RS_SORT_VIEWPORT_MAX			BcU64( 0x00000000000000ff )
#define RS_SORT_RENDERTARGET_MAX		BcU64( 0x000000000000000f )


//////////////////////////////////////////////////////////////////////////
// RsRenderSortPass
enum class RsRenderSortPassType : BcU32
{
	SHADOW,
	DEPTH,
	OPAQUE,
	TRANSPARENT,
	OVERLAY,

	INVALID = BcErrorCode
};

enum class RsRenderSortPassFlags : BcU32
{
	NONE = 0,
	SHADOW = 1 << BcU32( RsRenderSortPassType::SHADOW ),
	DEPTH = 1 << BcU32( RsRenderSortPassType::DEPTH ),
	OPAQUE = 1 << BcU32( RsRenderSortPassType::OPAQUE ),
	TRANSPARENT = 1 << BcU32( RsRenderSortPassType::TRANSPARENT ),
	OVERLAY = 1 << BcU32( RsRenderSortPassType::OVERLAY ),
	ALL = SHADOW | DEPTH | OPAQUE | TRANSPARENT | OVERLAY
};

DEFINE_ENUM_CLASS_FLAG_OPERATOR( RsRenderSortPassFlags, | );
DEFINE_ENUM_CLASS_FLAG_OPERATOR( RsRenderSortPassFlags, & );

//////////////////////////////////////////////////////////////////////////
/**	\class RsRenderNode
*	\brief Renderable node.
*	
*	Sortable render node. Used to queue up high level render commands in 
*	to be called later on.
*/
class RsRenderNode
{
public:
	struct Capture {};
	typedef void( Capture::*Callback )( RsContext* );

public:
	RsRenderNode( RsRenderSort InSort, Callback InCallback );

	/**
	 * Perform render of node.
	 */
	void render( RsContext* Context );

public:
	RsRenderSort Sort_;
	Callback Callback_;
};

static_assert( std::is_trivially_destructible< RsRenderNode >::value, "RsRenderNode must be trivially destructible." );

//////////////////////////////////////////////////////////////////////////
// Inline
inline void RsRenderNode::render( RsContext* Context )
{
	const auto RenderNodeSize = sizeof( RsRenderNode );
	auto* CapturePtr = 
		reinterpret_cast< Capture* >( 
			reinterpret_cast< BcU8* >( this ) + RenderNodeSize );
	(CapturePtr->*Callback_)( Context );
}

#endif

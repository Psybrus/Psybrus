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
#include "System/Renderer/RsTypes.h"

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
			BcU64			Pass_			: 2;		// 48
			BcU64			Viewport_		: 8;		// 56
			BcU64			RenderTarget_	: 4;		// 60
			BcU64			NodeType_		: 4;		// 64
		};
	};
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

#define RS_SORT_NODETYPE_RESOURCE		BcU64( 0 )

#define RS_SORT_MACRO_VIEWPORT_RENDERTARGET( _V, _T ) \
	BcU64(	( ( BcU64( _V ) & RS_SORT_VIEWPORT_MAX ) << RS_SORT_VIEWPORT_SHIFT ) | \
	( ( BcU64( _T ) & RS_SORT_RENDERTARGET_MAX ) << RS_SORT_RENDERTARGET_SHIFT ) )


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

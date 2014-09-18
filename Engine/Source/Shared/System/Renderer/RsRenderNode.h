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
*	Deriving from this object will allow it to render how it likes.
*	Renderable resource instances will all have their own render() call,
*	removing the need for a nasty resource type switch.
*	Generally renderable instances will be platform/API specific, allowing
*	for both high-level and low-level optimisations to be implemented.
*	This object is allocated by the frame object itself when a request to render
*	a resource is specified. This means that you can not change an instance
*	once it has been submitted to the frame object for rendering.
*	The upside is you can derive your own if you use the rendering API
*	that is exposed by RsCoreImpl and other objects. This does not require
*	deallocation, and gives more flexibility.	
*/
class RsRenderNode
{
public:
	RsRenderNode();
	virtual ~RsRenderNode();

	/**
	*	Perform default render.
	*/
	virtual void			render() = 0;

public:
	RsContext*				pContext_;
	RsRenderSort			Sort_;
};

#endif

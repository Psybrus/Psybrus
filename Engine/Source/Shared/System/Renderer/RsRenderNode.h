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

#include "RsTypes.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class RsFrame;
class RsCore;


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

	/**
	*	Perform default render.
	*/
	virtual void			render() = 0;

public:
	RsRenderSort			Sort_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines
inline RsRenderNode::RsRenderNode()
{
	Sort_.Value_ = 0;
}

#endif

/**************************************************************************
*
* File:		RsFrame.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Frame object. All render commands are pushed onto here.
*		
*
*
* 
**************************************************************************/

#ifndef __RSFRAME_H__
#define __RSFRAME_H__

#include "Base/BcTypes.h"

#include "System/Renderer/RsContext.h"
#include "System/Renderer/RsRenderNode.h"
#include "System/Renderer/RsRenderTarget.h"

#include "System/Renderer/RsViewport.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class RsCore;
class RsMaterial;
class RsEffect;
class RoModel;

//////////////////////////////////////////////////////////////////////////
// Enums
enum eRsFramePrimitiveMode
{
	rsFPM_2D = 0,
	rsFPM_3D
};

//////////////////////////////////////////////////////////////////////////
/**	\class RsFrame
*	\brief Render System Frame
*
*	This object encapsulates all rendering tasks and resources for a frame.
*/
class RsFrame
{
public:
	virtual ~RsFrame(){};
	
	/**
	*	Get context.
	*/
	virtual RsContext* getContext() const = 0;
	
	/**
	*	Set current render target.\n
	*	NON-JOB FUNCTION: Called by game thread.
	*/
	virtual void	setRenderTarget( RsRenderTarget* pRenderTarget ) = 0;
	
	/**
	*	Set current viewport.\n
	*	NON-JOB FUNCTION: Called by game thread.
	*/
	virtual void	setViewport( const RsViewport& Viewport ) = 0;
	
	/**
	*	Add render node.\n
	*	NON-JOB FUNCTION: Placeholder.
	*/
	virtual void	addRenderNode( RsRenderNode* pInstance ) = 0;
	
	/**
	*	Allocate from instance memory.
	*/
	virtual void*	allocMem( BcSize Bytes ) = 0;

	//////////////////////////////////////////////////////////////////////////
	// Debug routines.
	/*
	void			debugLine( const MaVec3d& PointA, const MaVec3d& PointB, const RsColour& Colour = RsColour::WHITE );
	void			debugAxis( const MaVec3d& Point, BcF32 Size, const RsColour& Colour = RsColour::WHITE );
	void			debugGrid( const MaVec3d& Centre, BcF32 Step, BcU32 Axis, BcU32 Elements, const RsColour& Colour = RsColour::WHITE );
	void			debugAABB( const MaAABB& AABB, const RsColour& Colour = RsColour::WHITE );
	void			debugMatrix( const MaMat4d& Matrix, BcF32 Scale = 1.0f, const RsColour& Colour = RsColour::WHITE  );
	void			debugEllipsoid( const MaVec3d& Centre, const MaVec3d& Scale, BcU32 LOD = 8, const RsColour& Colour = RsColour::WHITE );
	void			debugCube( const MaMat4d& Transform, const RsColour& Colour = RsColour::WHITE );
	void			debugCone( const MaMat4d& Transform, const RsColour& Colour = RsColour::WHITE );
	 */
	
public:
	/**
	*	Allocate from instance memory.\n
	*	NON-JOB FUNCTION: Called by specific frame implementation.
	*/
	template< typename _Ty >
	_Ty*			alloc( BcU32 NoofObjects  = 1 );
	
	/**
	*	Create new object from instance memory.
	*/
	template< typename _Ty >
	_Ty*			newObject();

protected:

};

//////////////////////////////////////////////////////////////////////////
// Inlines

template< typename _Ty >
inline _Ty* RsFrame::alloc( BcU32 NoofObjects )
{
	return reinterpret_cast< _Ty* >( allocMem( sizeof( _Ty ) * NoofObjects ) );
}

template< typename _Ty >
inline _Ty* RsFrame::newObject()
{
	return new ( alloc< _Ty >() ) _Ty();
}

#endif

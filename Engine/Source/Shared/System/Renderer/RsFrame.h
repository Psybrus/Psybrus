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

#include "BcTypes.h"

#include "RsRenderNode.h"
#include "RsRenderTarget.h"

#include "RsViewport.h"

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
	*	Get device handle.
	*/
	virtual BcHandle deviceHandle() const = 0;
	
	/**
	*	Get width.
	*/
	virtual BcU32	width() const = 0;
	
	/**
	*	Get height.
	*/
	virtual BcU32	height() const = 0;
	
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
	void			debugLine( const BcVec3d& PointA, const BcVec3d& PointB, const RsColour& Colour = RsColour::WHITE );
	void			debugAxis( const BcVec3d& Point, BcReal Size, const RsColour& Colour = RsColour::WHITE );
	void			debugGrid( const BcVec3d& Centre, BcReal Step, BcU32 Axis, BcU32 Elements, const RsColour& Colour = RsColour::WHITE );
	void			debugAABB( const BcAABB& AABB, const RsColour& Colour = RsColour::WHITE );
	void			debugMatrix( const BcMat4d& Matrix, BcReal Scale = 1.0f, const RsColour& Colour = RsColour::WHITE  );
	void			debugEllipsoid( const BcVec3d& Centre, const BcVec3d& Scale, BcU32 LOD = 8, const RsColour& Colour = RsColour::WHITE );
	void			debugCube( const BcMat4d& Transform, const RsColour& Colour = RsColour::WHITE );
	void			debugCone( const BcMat4d& Transform, const RsColour& Colour = RsColour::WHITE );
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

	/*
	template< typename _Ty, typename _P1 >
	_Ty*			newObject( _P1 P1 );

	template< typename _Ty, typename _P1, typename _P2 >
	_Ty*			newObject( _P1 P1, _P2 P2 );

	template< typename _Ty, typename _P1, typename _P2, typename _P3 >
	_Ty*			newObject( _P1 P1, _P2 P2, _P3 P3 );

	template< typename _Ty, typename _P1, typename _P2, typename _P3, typename _P4 >
	_Ty*			newObject( _P1 P1, _P2 P2, _P3 P3, _P4 P4 );

	template< typename _Ty, typename _P1, typename _P2, typename _P3, typename _P4, typename _P5 >
	_Ty*			newObject( _P1 P1, _P2 P2, _P3 P3, _P4 P4, _P5 P5 );

	 */
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
	return placement_new< _Ty >( alloc< _Ty >() );
}

/*
template< typename _Ty, typename _P1 >
inline _Ty* RsFrame::newObject( _P1 P1 )
{
	return placement_new ( alloc< _Ty >() ) _Ty( P1 );
}

template< typename _Ty, typename _P1, typename _P2 >
inline _Ty* RsFrame::newObject( _P1 P1, _P2 P2 )
{
	return placement_new ( alloc< _Ty >() ) _Ty( P1, P2 );
}

template< typename _Ty, typename _P1, typename _P2, typename _P3 >
inline _Ty* RsFrame::newObject( _P1 P1, _P2 P2, _P3 P3 )
{
	return placement_new ( alloc< _Ty >() ) _Ty( P1, P2, P3 );
}

template< typename _Ty, typename _P1, typename _P2, typename _P3, typename _P4 >
inline _Ty* RsFrame::newObject( _P1 P1, _P2 P2, _P3 P3, _P4 P4 )
{
	return placement_new ( alloc< _Ty >() ) _Ty( P1, P2, P3, P4 );
}

template< typename _Ty, typename _P1, typename _P2, typename _P3, typename _P4, typename _P5 >
inline _Ty* RsFrame::newObject( _P1 P1, _P2 P2, _P3 P3, _P4 P4, _P5 P5 )
{
	return placement_new ( alloc< _Ty >() ) _Ty( P1, P2, P3, P4, P5 );
}
*/
#endif

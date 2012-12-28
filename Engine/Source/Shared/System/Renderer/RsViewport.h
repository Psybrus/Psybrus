/**************************************************************************
*
* File:		RsViewport.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSVIEWPORT_H__
#define __RSVIEWPORT_H__

#include "Base/BcMat4d.h"
#include "Base/BcAABB.h"
#include "Base/BcPlane.h"

//////////////////////////////////////////////////////////////////////////
// RsViewport
class RsViewport
{
public:
	RsViewport();
	RsViewport( BcU32 X, BcU32 Y, BcU32 Width, BcU32 Height, BcF32 ZNear = 1.0f, BcF32 ZFar = 1024.0f );
	~RsViewport();

	//
	void viewport( BcU32 X, BcU32 Y, BcU32 Width, BcU32 Height, BcF32 ZNear = 1.0f, BcF32 ZFar = 1024.0f );

	//
	void view( const BcMat4d& ModelView );
	const BcMat4d& view() const;

	void projection( const BcMat4d& Projection );
	const BcMat4d& projection() const;

	/**
	*	Return viewproj matrix.
	*/
	const BcMat4d& viewProjection() const;

	/**
	*	Transform viewport.
	*/
	void transform( const BcMat4d& Transform );

	/**
	*	Get 3D world space coordinates from a 2D point in the viewport.
	*/
	void unProject( const BcVec2d& ScreenCoord, BcVec3d& WorldNearPos, BcVec3d& WorldFarPos ) const;
	
	/**
	*	Get 2D screen space coordinate from a 3D world space point in the viewport.
	*/
	BcVec2d project( const BcVec3d& WorldPos );
	
	/**
	*	Get integer depth value.
	*/
	BcU32 depth( const BcVec3d& WorldPos ) const;

	/**
	*	Intersect AABB with frustum.
	*/
	BcBool intersect( const BcAABB& AABB ) const;

	//
	BcU32 x() const;
	BcU32 y() const;

	BcU32 width() const;
	BcU32 height() const;

	BcF32 zFar() const;
	BcF32 zNear() const;

	void clearDepth( BcBool ClearDepth );
	BcBool clearDepth() const;

private:
	void buildFrustum();

private:
	// Viewport
	BcU32 Top_;
	BcU32 Bottom_;
	BcU32 Left_;
	BcU32 Right_;
	
	//
	BcF32 ZFar_;
	BcF32 ZNear_;
	
	// Transforms
	BcMat4d ModelView_;
	BcMat4d Projection_;
	BcMat4d ViewProj_;

	// Properties.
	BcBool ClearDepth_;

	//
	BcPlane FrustumPlanes_[ 6 ];
};

//////////////////////////////////////////////////////////////////////////
// Inlines
inline RsViewport::RsViewport( BcU32 X, BcU32 Y, BcU32 Width, BcU32 Height, BcF32 ZNear, BcF32 ZFar )
{
	viewport( X, Y, Width, Height, ZNear, ZFar );
	ClearDepth_ = BcFalse;
}

inline void RsViewport::viewport( BcU32 X, BcU32 Y, BcU32 Width, BcU32 Height, BcF32 ZNear, BcF32 ZFar )
{
	Top_ = Y;
	Bottom_ = ( Y + Height ) - 1;
	Left_ = X;
	Right_ = ( X + Width ) - 1;
	ZNear_ = ZNear;
	ZFar_ = ZFar;
}

inline void RsViewport::view( const BcMat4d& ModelView )
{
	ModelView_ = ModelView;
}

inline const BcMat4d& RsViewport::view() const
{
	return ModelView_;
}

inline void RsViewport::projection( const BcMat4d& Projection )
{
	Projection_ = Projection;
	ViewProj_ = ModelView_ * Projection_;
	buildFrustum();
}

inline const BcMat4d& RsViewport::projection() const
{
	return Projection_;
}

inline const BcMat4d& RsViewport::viewProjection() const
{
	return ViewProj_;
}

inline void RsViewport::transform( const BcMat4d& Transform )
{
	ModelView_ = Transform * ModelView_;
	ViewProj_ = ModelView_ * Projection_;
	buildFrustum();
}

inline BcU32 RsViewport::x() const
{
	return Left_;
}

inline BcU32 RsViewport::y() const
{
	return Top_;
}

inline BcU32 RsViewport::width() const
{
	return ( Right_ - Left_ ) + 1;
}

inline BcU32 RsViewport::height() const
{
	return ( Bottom_ - Top_ ) + 1;
}

inline BcF32 RsViewport::zFar() const
{
	return ZFar_;
}

inline BcF32 RsViewport::zNear() const
{
	return ZNear_;
}

inline void RsViewport::clearDepth( BcBool ClearDepth )
{
	ClearDepth_ = ClearDepth;
}

inline BcBool RsViewport::clearDepth() const
{
	return ClearDepth_;
}

#endif

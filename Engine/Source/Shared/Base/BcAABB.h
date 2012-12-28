/**************************************************************************
*
* File:		BcAABB.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Axis Aligned Bounding Box
*		
*
*
* 
**************************************************************************/

#ifndef __BCAABB_H__
#define __BCAABB_H__

#include "Base/BcVectors.h"
#include "Base/BcMat4d.h"
#include "Base/BcPlane.h"

//////////////////////////////////////////////////////////////////////////
// BcAABB
class BcAABB
{
public:
	enum eClassify
	{
		bcBC_INSIDE = 0,
		bcBC_OUTSIDE,
		bcBC_SPANNING
	};

	enum eCorners
	{
		LBB = 0,
		RBB,
		LTB,
		RTB,
		LBF,
		RBF,
		LTF,
		RTF,
	};

	enum eFaces
	{
		LEFT = 0,
		RIGHT,
		TOP,
		BOTTOM,
		FRONT,
		BACK,
	};


public:
	BcAABB();
	BcAABB( const BcAABB& Other );
	BcAABB( const BcVec3d& Min, const BcVec3d& Max );
	~BcAABB();

	// Accessors
	void				min( const BcVec3d& Min );
	void				max( const BcVec3d& Max );

	const BcVec3d&		min() const;
	const BcVec3d&		max() const;

	BcF32				width() const;
	BcF32				height() const;
	BcF32				depth() const;
	BcF32				volume() const;

	BcVec3d				corner( BcU32 i ) const;
	BcPlane				facePlane( BcU32 i ) const;
	BcVec3d				faceCentre( BcU32 i ) const;
	BcVec3d				centre() const;
	BcVec3d				dimensions() const;
	BcF32				diameter() const;

	BcBool				isEmpty() const;

	// Construction
	void				empty();
	void				expandBy( const BcVec3d& Point );
	void				expandBy( const BcAABB& AABB );

	// Intersection
	BcBool				lineIntersect( const BcVec3d& Start, const BcVec3d& End, BcVec3d* pIntersectionPoint, BcVec3d* pIntersectionNormal ) const;
	BcBool				boxIntersect( const BcAABB& AABB, BcAABB* pIntersectionBox ) const;

	// Classification
	eClassify			classify( const BcVec3d& Point ) const;
	eClassify			classify( const BcAABB& AABB ) const;

	// Transform
	BcAABB				transform( const BcMat4d& Transform ) const;

private:
	BcVec3d Min_;
	BcVec3d Max_;

};

//////////////////////////////////////////////////////////////////////////
// Inlines
inline void BcAABB::min( const BcVec3d& Min )
{
	Min_ = Min;
}

inline void BcAABB::max( const BcVec3d& Max )
{
	Max_ = Max;
}

inline const BcVec3d& BcAABB::min() const
{
	return Min_;
}

inline const BcVec3d& BcAABB::max() const
{
	return Max_;
}

inline BcVec3d BcAABB::corner( BcU32 i ) const
{
	return BcVec3d( ( i & 1 ) ? Min_.x() : Max_.x(),
	                ( i & 2 ) ? Min_.y() : Max_.y(),
	                ( i & 4 ) ? Min_.z() : Max_.z() );
}

inline BcVec3d BcAABB::centre() const
{
	return ( ( Min_ + Max_ ) * 0.5f );
}

inline BcVec3d BcAABB::dimensions() const
{
	return ( Max_ - Min_ );
}

inline BcF32 BcAABB::diameter() const
{
	return ( ( Max_ - Min_ ).magnitude() );
}

inline void BcAABB::empty()
{
	Min_.set( 1e24f, 1e24f, 1e24f );
	Max_.set( -1e24f, -1e24f, -1e24f );
}

inline BcBool BcAABB::isEmpty() const
{
	return ( ( Min_.x() > Max_.x() ) ||
	         ( Min_.y() > Max_.y() ) ||
	         ( Min_.z() > Max_.z() ) );
}

inline void BcAABB::expandBy( const BcVec3d& Point )
{
	Min_.x( BcMin( Min_.x(), Point.x() ) );
	Min_.y( BcMin( Min_.y(), Point.y() ) );
	Min_.z( BcMin( Min_.z(), Point.z() ) );

	Max_.x( BcMax( Max_.x(), Point.x() ) );
	Max_.y( BcMax( Max_.y(), Point.y() ) );
	Max_.z( BcMax( Max_.z(), Point.z() ) );
}

inline void BcAABB::expandBy( const BcAABB& AABB )
{
	Min_.x( BcMin( Min_.x(), AABB.Min_.x() ) );
	Min_.y( BcMin( Min_.y(), AABB.Min_.y() ) );
	Min_.z( BcMin( Min_.z(), AABB.Min_.z() ) );

	Max_.x( BcMax( Max_.x(), AABB.Max_.x() ) );
	Max_.y( BcMax( Max_.y(), AABB.Max_.y() ) );
	Max_.z( BcMax( Max_.z(), AABB.Max_.z() ) );
}

inline BcF32 BcAABB::width() const
{
	return Max_.x() - Min_.x();
}

inline BcF32 BcAABB::height() const
{
	return Max_.y() - Min_.y();
}

inline BcF32 BcAABB::depth() const
{
	return Max_.z() - Min_.z();
}

inline BcF32 BcAABB::volume() const
{
	return ( width() * height() * depth() );
}

#endif

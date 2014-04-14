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

#endif

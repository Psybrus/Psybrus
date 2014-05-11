/**************************************************************************
*
* File:		MaAABB.h
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

#include "Math/MaMat4d.h"
#include "Math/MaPlane.h"
#include "Reflection/ReReflection.h"

//////////////////////////////////////////////////////////////////////////
// MaAABB
class MaAABB
{
public:
	REFLECTION_DECLARE_BASIC( MaAABB );

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
	MaAABB();
	MaAABB( const MaAABB& Other );
	MaAABB( const MaVec3d& Min, const MaVec3d& Max );
	~MaAABB();

	// Accessors
	void				min( const MaVec3d& Min );
	void				max( const MaVec3d& Max );

	const MaVec3d&		min() const;
	const MaVec3d&		max() const;

	BcF32				width() const;
	BcF32				height() const;
	BcF32				depth() const;
	BcF32				volume() const;

	MaVec3d				corner( BcU32 i ) const;
	MaPlane				facePlane( BcU32 i ) const;
	MaVec3d				faceCentre( BcU32 i ) const;
	MaVec3d				centre() const;
	MaVec3d				dimensions() const;
	BcF32				diameter() const;

	BcBool				isEmpty() const;

	// Construction
	void				empty();
	void				expandBy( const MaVec3d& Point );
	void				expandBy( const MaAABB& AABB );

	// Intersection
	BcBool				lineIntersect( const MaVec3d& Start, const MaVec3d& End, MaVec3d* pIntersectionPoint, MaVec3d* pIntersectionNormal ) const;
	BcBool				boxIntersect( const MaAABB& AABB, MaAABB* pIntersectionBox ) const;

	// Classification
	eClassify			classify( const MaVec3d& Point ) const;
	eClassify			classify( const MaAABB& AABB ) const;

	// Transform
	MaAABB				transform( const MaMat4d& Transform ) const;

private:
	MaVec3d Min_;
	MaVec3d Max_;

};

#endif

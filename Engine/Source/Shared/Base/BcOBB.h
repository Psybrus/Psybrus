/**************************************************************************
*
* File:		BcOBB.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Oriented Bounding Box
*		
*
*
* 
**************************************************************************/

#ifndef __BcOBB_H__
#define __BcOBB_H__

#include "Base/BcVec3d.h"
#include "Base/BcMat4d.h"
#include "Base/BcPlane.h"
#include "Base/BcAABB.h"

//////////////////////////////////////////////////////////////////////////
// BcOBB
class BcOBB
{
public:
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
	BcOBB();
	BcOBB( const BcOBB& Other );
	BcOBB( const BcAABB& AABB, const BcMat4d& Transform = BcMat4d() );
	~BcOBB();

	// Accessors
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

	// Intersection
	BcBool				lineIntersect( const BcVec3d& Start, const BcVec3d& End, BcVec3d* pIntersectionPoint, BcVec3d* pIntersectionNormal ) const;

	// Transform
	BcOBB				transform( const BcMat4d& Transform ) const;

private:
	BcVec3d				Corners_[ 8 ];

};

//////////////////////////////////////////////////////////////////////////
// Inlines
inline BcVec3d BcOBB::corner( BcU32 i ) const
{
	return Corners_[ i ];
}

inline BcF32 BcOBB::width() const
{
	return ( corner( LBB ) - corner( RBB ) ).magnitude();
}

inline BcF32 BcOBB::height() const
{
	return ( corner( LBB ) - corner( LTB ) ).magnitude();
}

inline BcF32 BcOBB::depth() const
{
	return ( corner( LBB ) - corner( LBF ) ).magnitude();
}

inline BcF32 BcOBB::volume() const
{
	return ( width() * height() * depth() );
}

#endif

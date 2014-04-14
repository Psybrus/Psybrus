/**************************************************************************
*
* File:		BcPlane.h
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*		
*  
*
*
* 
**************************************************************************/

#ifndef	__BCPLANE_H__
#define __BCPLANE_H__

#include "Base/BcVec3d.h"
#include "Base/BcMat4d.h"

//////////////////////////////////////////////////////////////////////////
// BcPlane
class BcPlane
{
public:
	enum eClassify
	{
		bcPC_FRONT = 0,
		bcPC_BACK,
		bcPC_COINCIDING,
		bcPC_SPANNING
	};

public:
	BcPlane(){}
	BcPlane( const BcVec3d& Normal, BcF32 D );
	BcPlane( BcF32 A, BcF32 B, BcF32 C, BcF32 D );
	~BcPlane(){}

	void normalise();
	void transform( const BcMat4d& Transform );

	void fromPoints( const BcVec3d& V1, const BcVec3d& V2, const BcVec3d& V3 );
	void fromPointNormal( const BcVec3d& Point, const BcVec3d& Normal );

	// Intersection
	BcBool lineIntersection( const BcVec3d& Point, const BcVec3d& Dir, BcF32& Distance ) const;
	BcBool lineIntersection( const BcVec3d& A, const BcVec3d& B, BcF32& Distance, BcVec3d& Intersection ) const;

	// Classification
	BcF32 distance( const BcVec3d& Point ) const;
	eClassify classify( const BcVec3d& Point, BcF32 Radius = 1e-3f ) const;

	// Operator
	BcBool operator == (const BcPlane& Other ) const;
	BcPlane operator -() const;

	// Utility
	static BcBool intersect( const BcPlane& A, const BcPlane& B, const BcPlane& C, BcVec3d& Point );

	// Accesors
	void a( const BcF32 A ){ Normal_.x( A ); };
	void b( const BcF32 B ){ Normal_.y( B ); };
	void c( const BcF32 C ){ Normal_.z( C ); };
	void d( const BcF32 D ){ D_ = D; };

	const BcVec3d& normal() const;
	BcF32 d() const;

private:
	BcVec3d			Normal_;
	BcF32			D_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines
inline BcPlane::BcPlane( const BcVec3d& Normal, BcF32 D )
{
	Normal_ = Normal;
	D_ = D;
}

inline BcPlane::BcPlane( BcF32 A, BcF32 B, BcF32 C, BcF32 D )
{
	Normal_ = BcVec3d( A, B, C );
	D_ = D;
}

#endif

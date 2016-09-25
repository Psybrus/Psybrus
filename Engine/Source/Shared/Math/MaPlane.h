/**************************************************************************
*
* File:		MaPlane.h
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

#include "Math/MaVec3d.h"
#include "Math/MaMat4d.h"
#include "Reflection/ReUtility.h"

//////////////////////////////////////////////////////////////////////////
// MaPlane
class MaPlane
{
public:
	REFLECTION_DECLARE_BASIC( MaPlane );

public:
	enum eClassify
	{
		bcPC_FRONT = 0,
		bcPC_BACK,
		bcPC_COINCIDING,
		bcPC_SPANNING
	};

public:
	MaPlane(){}
	MaPlane( const MaVec3d& Normal, BcF32 D );
	MaPlane( BcF32 A, BcF32 B, BcF32 C, BcF32 D );
	MaPlane( const MaVec3d& V1, const MaVec3d& V2, const MaVec3d& V3 );
	~MaPlane(){}

	void normalise();
	void transform( const MaMat4d& Transform );

	void fromPoints( const MaVec3d& V1, const MaVec3d& V2, const MaVec3d& V3 );
	void fromPointNormal( const MaVec3d& Point, const MaVec3d& Normal );

	// Intersection
	BcBool lineIntersection( const MaVec3d& Point, const MaVec3d& Dir, BcF32& Distance ) const;
	BcBool lineIntersection( const MaVec3d& A, const MaVec3d& B, BcF32& Distance, MaVec3d& Intersection ) const;

	// Classification
	BcF32 distance( const MaVec3d& Point ) const;
	eClassify classify( const MaVec3d& Point, BcF32 Radius = 1e-3f ) const;

	// Operator
	BcBool operator == (const MaPlane& Other ) const;
	BcBool operator != (const MaPlane& Other ) const;
	MaPlane operator -() const;

	// Utility
	static BcBool intersect( const MaPlane& A, const MaPlane& B, const MaPlane& C, MaVec3d& Point );

	// Accesors
	void a( const BcF32 A ){ Normal_.x( A ); };
	void b( const BcF32 B ){ Normal_.y( B ); };
	void c( const BcF32 C ){ Normal_.z( C ); };
	void d( const BcF32 D ){ D_ = D; };

	const MaVec3d& normal() const;
	BcF32 d() const;

private:
	MaVec3d			Normal_;
	BcF32			D_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines
inline MaPlane::MaPlane( const MaVec3d& Normal, BcF32 D )
{
	Normal_ = Normal;
	D_ = D;
}

inline MaPlane::MaPlane( BcF32 A, BcF32 B, BcF32 C, BcF32 D )
{
	Normal_ = MaVec3d( A, B, C );
	D_ = D;
}

inline MaPlane::MaPlane( const MaVec3d& V1, const MaVec3d& V2, const MaVec3d& V3 )
{
	fromPoints( V1, V2, V3 );
}

#endif

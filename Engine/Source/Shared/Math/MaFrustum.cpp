#include "Math/MaFrustum.h"

//////////////////////////////////////////////////////////////////////////
// Reflection.
REFLECTION_DEFINE_BASIC( MaFrustum );

void MaFrustum::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Planes_",		&MaFrustum::Planes_ ),
	};
		
	ReRegisterClass< MaFrustum >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// MaFrustum
MaFrustum::MaFrustum():
	Planes_()
{

}

//////////////////////////////////////////////////////////////////////////
// MaFrustum
MaFrustum::MaFrustum( const MaMat4d& ClipTransform )
{
	Planes_[ 0 ] = MaPlane( 
		( ClipTransform[0][3] + ClipTransform[0][0] ),
		( ClipTransform[1][3] + ClipTransform[1][0] ),
		( ClipTransform[2][3] + ClipTransform[2][0] ),
		( ClipTransform[3][3] + ClipTransform[3][0]) );

	Planes_[ 1 ] = MaPlane( 
		( ClipTransform[0][3] - ClipTransform[0][0] ),
		( ClipTransform[1][3] - ClipTransform[1][0] ),
		( ClipTransform[2][3] - ClipTransform[2][0] ),
		( ClipTransform[3][3] - ClipTransform[3][0] ) );

	Planes_[ 2 ] = MaPlane( 
		( ClipTransform[0][3] + ClipTransform[0][1] ),
		( ClipTransform[1][3] + ClipTransform[1][1] ),
		( ClipTransform[2][3] + ClipTransform[2][1] ),
		( ClipTransform[3][3] + ClipTransform[3][1] ) );

	Planes_[ 3 ] = MaPlane( 
		( ClipTransform[0][3] - ClipTransform[0][1] ),
		( ClipTransform[1][3] - ClipTransform[1][1] ),
		( ClipTransform[2][3] - ClipTransform[2][1] ),
		( ClipTransform[3][3] - ClipTransform[3][1] ) );

	Planes_[ 4 ] = MaPlane( 
		( ClipTransform[0][3] - ClipTransform[0][2] ),
		( ClipTransform[1][3] - ClipTransform[1][2] ),
		( ClipTransform[2][3] - ClipTransform[2][2] ),
		( ClipTransform[3][3] - ClipTransform[3][2] ) );
	
	Planes_[ 5 ] = MaPlane( 
		( ClipTransform[0][3] ),
		( ClipTransform[1][3] ),
		( ClipTransform[2][3] ),
		( ClipTransform[3][3] ) );

	for ( BcU32 i = 0; i < 6; ++i )
	{
		MaVec3d Normal = Planes_[ i ].normal();
		BcF32 Scale = 1.0f / -Normal.magnitude();
		Planes_[ i ] = MaPlane( 
			Planes_[ i ].normal().x() * Scale,
			Planes_[ i ].normal().y() * Scale,
			Planes_[ i ].normal().z() * Scale,
			Planes_[ i ].d() * Scale );
	}
}

//////////////////////////////////////////////////////////////////////////
// MaFrustum
bool MaFrustum::intersect( const MaVec3d& SphereCentre, BcF32 Radius ) const
{
	BcF32 Distance;
	for( BcU32 i = 0; i < Planes_.size(); ++i )
	{
		Distance = Planes_[ i ].distance( SphereCentre );
		if( Distance > Radius )
		{
			return false;
		}
	}
	return true;
}
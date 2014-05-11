/**************************************************************************
*
* File:		RsFrame.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Frame object. All renderable instances are pushed onto here.
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/RsFrame.h"

/*
//////////////////////////////////////////////////////////////////////////
// TVertex
struct TVertex
{
	BcF32 X_, Y_, Z_;
	BcU32 ARGB_;
};

//////////////////////////////////////////////////////////////////////////
// debugLine
void RsFrame::debugLine( const MaVec3d& PointA, const MaVec3d& PointB, const RsColour& Colour )
{
	addLine( PointA, PointB, Colour, Colour.a() < 1.0f ? 1 : 0 );
}

//////////////////////////////////////////////////////////////////////////
// debugAxis/*
void RsFrame::debugAxis( const MaVec3d& Point, BcF32 Size, const RsColour& Colour )
{
	beginPrimitive( rsPT_LINELIST, rsFPM_3D, Colour.a() < 1.0f ? 1 : 0 );
	addLine( Point - MaVec3d( Size, 0.0f, 0.0f ), Point + MaVec3d( Size, 0.0f, 0.0f ), Colour, 0 );
	addLine( Point - MaVec3d( 0.0f, Size, 0.0f ), Point + MaVec3d( 0.0f, Size, 0.0f ), Colour, 0 );
	addLine( Point - MaVec3d( 0.0f, 0.0f, Size ), Point + MaVec3d( 0.0f, 0.0f, Size ), Colour, 0 );
	endPrimitive();
}

//////////////////////////////////////////////////////////////////////////
// debugGrid
void RsFrame::debugGrid( const MaVec3d& Centre, BcF32 Step, BcU32 Axis, BcU32 Elements, const RsColour& Colour )
{
	BcF32 LBound = -Step * BcF32( Elements >> 1 );
	BcF32 UBound =  Step * BcF32( Elements >> 1 );

	beginPrimitive( rsPT_LINELIST, rsFPM_3D, Colour.a() < 1.0f ? 1 : 0 );

	switch( Axis )
	{
	case 0:
		for( BcF32 i = LBound; i <= UBound; i += Step )
		{
			addLine( MaVec3d( 0.0f, i, LBound ) + Centre, MaVec3d( 0.0f, i, UBound ) + Centre, Colour, 0 );
			addLine( MaVec3d( 0.0f, LBound, i ) + Centre, MaVec3d( 0.0f, UBound, i ) + Centre, Colour, 0 );
		}
		break;

	case 1:
		for( BcF32 i = LBound; i <= UBound; i += Step )
		{
			addLine( MaVec3d( i, 0.0f, LBound ) + Centre, MaVec3d( i, 0.0f, UBound ) + Centre, Colour, 0 );
			addLine( MaVec3d( LBound, 0.0f, i ) + Centre, MaVec3d( UBound, 0.0f, i ) + Centre, Colour, 0 );
		}
		break;

	case 2:
		for( BcF32 i = LBound; i <= UBound; i += Step )
		{
			addLine( MaVec3d( i, LBound, 0.0f ) + Centre, MaVec3d( i, UBound, 0.0f ) + Centre, Colour, 0 );
			addLine( MaVec3d( LBound, i, 0.0f ) + Centre, MaVec3d( UBound, i, 0.0f ) + Centre, Colour, 0 );
		}
		break;
	}

	endPrimitive();
}

//////////////////////////////////////////////////////////////////////////
// debugAABB
void RsFrame::debugAABB( const MaAABB& AABB, const RsColour& Colour )
{
	beginPrimitive( rsPT_LINELIST, rsFPM_3D, Colour.a() < 1.0f ? 1 : 0 );

	addLine( AABB.corner( 0 ), AABB.corner( 1 ), Colour, 0 );
	addLine( AABB.corner( 1 ), AABB.corner( 3 ), Colour, 0 );
	addLine( AABB.corner( 2 ), AABB.corner( 0 ), Colour, 0 );
	addLine( AABB.corner( 3 ), AABB.corner( 2 ), Colour, 0 );

	addLine( AABB.corner( 4 ), AABB.corner( 5 ), Colour, 0 );
	addLine( AABB.corner( 5 ), AABB.corner( 7 ), Colour, 0 );
	addLine( AABB.corner( 6 ), AABB.corner( 4 ), Colour, 0 );
	addLine( AABB.corner( 7 ), AABB.corner( 6 ), Colour, 0 );

	addLine( AABB.corner( 0 ), AABB.corner( 4 ), Colour, 0 );
	addLine( AABB.corner( 1 ), AABB.corner( 5 ), Colour, 0 );
	addLine( AABB.corner( 2 ), AABB.corner( 6 ), Colour, 0 );
	addLine( AABB.corner( 3 ), AABB.corner( 7 ), Colour, 0 );

	endPrimitive();
}

//////////////////////////////////////////////////////////////////////////
// debugMatrix
void RsFrame::debugMatrix( const MaMat4d& Matrix, BcF32 Scale, const RsColour& Colour )
{
	MaVec3d Position( Matrix.row3().x(), Matrix.row3().y(), Matrix.row3().z() );
	MaVec3d X( Matrix.row0().x(), Matrix.row0().y(), Matrix.row0().z() );
	MaVec3d Y( Matrix.row1().x(), Matrix.row1().y(), Matrix.row1().z() );
	MaVec3d Z( Matrix.row2().x(), Matrix.row2().y(), Matrix.row2().z() );

	X = X * Scale;
	Y = Y * Scale;
	Z = Z * Scale;

	beginPrimitive( rsPT_LINELIST, rsFPM_3D, Colour.a() < 1.0f ? 1 : 0 );

	addLine( Position, Position + X, RsColour::RED * Colour, 0 );
	addLine( Position, Position + Y, RsColour::GREEN * Colour, 0 );
	addLine( Position, Position + Z, RsColour::BLUE * Colour, 0 );

	endPrimitive();
}

//////////////////////////////////////////////////////////////////////////
// debugEllipsoid
void RsFrame::debugEllipsoid( const MaVec3d& Centre, const MaVec3d& Scale, BcU32 LOD, const RsColour& Colour )
{
	// Draw outer circles for all axis.
	BcF32 Angle = 0.0f;
	BcF32 AngleInc = ( BcPI * 2.0f ) / BcF32( LOD );

	beginPrimitive( rsPT_LINELIST, rsFPM_3D, Colour.a() < 1.0f ? 1 : 0 );

	// Draw axis lines.
	for( BcU32 i = 0; i < LOD; ++i )
	{
		MaVec2d PosA( BcCos( Angle ), -BcSin( Angle ) );
		MaVec2d PosB( BcCos( Angle + AngleInc ), -BcSin( Angle + AngleInc ) );

		MaVec3d XAxisA = MaVec3d( 0.0f,                 PosA.x() * Scale.y(), PosA.y() * Scale.z() );
		MaVec3d YAxisA = MaVec3d( PosA.x() * Scale.x(), 0.0f,                 PosA.y() * Scale.z() );
		MaVec3d ZAxisA = MaVec3d( PosA.x() * Scale.x(), PosA.y() * Scale.y(), 0.0f                 );
		MaVec3d XAxisB = MaVec3d( 0.0f,                 PosB.x() * Scale.y(), PosB.y() * Scale.z() );
		MaVec3d YAxisB = MaVec3d( PosB.x() * Scale.x(), 0.0f,                 PosB.y() * Scale.z() );
		MaVec3d ZAxisB = MaVec3d( PosB.x() * Scale.x(), PosB.y() * Scale.y(), 0.0f                 );

		addLine( XAxisA + Centre, XAxisB + Centre, Colour, 0 );
		addLine( YAxisA + Centre, YAxisB + Centre, Colour, 0 );
		addLine( ZAxisA + Centre, ZAxisB + Centre, Colour, 0 );

		Angle += AngleInc;
	}

	// Draw a cross down centre.
	MaVec3d XAxis = MaVec3d( Scale.x(), 0.0f, 0.0f );
	MaVec3d YAxis = MaVec3d( 0.0f, Scale.y(), 0.0f );
	MaVec3d ZAxis = MaVec3d( 0.0f, 0.0f, Scale.z() );
	addLine( Centre - XAxis, Centre + XAxis, Colour, 0 );
	addLine( Centre - YAxis, Centre + YAxis, Colour, 0 );
	addLine( Centre - ZAxis, Centre + ZAxis, Colour, 0 );

	endPrimitive();
}

//////////////////////////////////////////////////////////////////////////
// debugCube
void RsFrame::debugCube( const MaMat4d& Transform, const RsColour& Colour )
{
	TVertex* pVertices = alloc< TVertex >( 36 );

	// NOTE: Highly wasteful now that I've just seen it -_-
	BcU32 ColourARGB = Colour.asARGB();
	TVertex CubeFaces[] = 
	{
		// Back
		{ -0.5f, -0.5f, -0.5f, ColourARGB },
		{ -0.5f,  0.5f, -0.5f, ColourARGB },
		{  0.5f,  0.5f, -0.5f, ColourARGB },
		{  0.5f, -0.5f, -0.5f, ColourARGB },

		// Front
		{ -0.5f, -0.5f,  0.5f, ColourARGB },
		{  0.5f, -0.5f,  0.5f, ColourARGB },
		{  0.5f,  0.5f,  0.5f, ColourARGB },
		{ -0.5f,  0.5f,  0.5f, ColourARGB },

		// Bottom
		{ -0.5f, -0.5f, -0.5f, ColourARGB },
		{  0.5f, -0.5f, -0.5f, ColourARGB },
		{  0.5f, -0.5f,  0.5f, ColourARGB },
		{ -0.5f, -0.5f,  0.5f, ColourARGB },

		// Top
		{ -0.5f,  0.5f,  0.5f, ColourARGB },
		{  0.5f,  0.5f,  0.5f, ColourARGB },
		{  0.5f,  0.5f, -0.5f, ColourARGB },
		{ -0.5f,  0.5f, -0.5f, ColourARGB },

		// Right

		{  0.5f, -0.5f, -0.5f, ColourARGB },
		{  0.5f,  0.5f, -0.5f, ColourARGB },
		{  0.5f,  0.5f,  0.5f, ColourARGB },
		{  0.5f, -0.5f,  0.5f, ColourARGB },

		// Left
		{ -0.5f, -0.5f,  0.5f, ColourARGB },
		{ -0.5f,  0.5f,  0.5f, ColourARGB },
		{ -0.5f,  0.5f, -0.5f, ColourARGB },
		{ -0.5f, -0.5f, -0.5f, ColourARGB },

	};

	BcU32 NoofIndices = 36;
	BcU32 Indices[] = 
	{
		0,  1,  2,  2,  3,  0,
		4,  5,  6,  6,  7,  4,
		8,  9,  10, 10, 11, 8,
		12, 13, 14, 14, 15, 12,
		16, 17, 18, 18, 19, 16,
		20, 21, 22, 22, 23, 20
	};

	// Copy in indexed data.
	for( BcU32 i = 0; i < NoofIndices; ++i )
	{
		TVertex CachedVert = CubeFaces[ Indices[i] ];
		MaVec3d Position = MaVec3d( CachedVert.X_, CachedVert.Y_, CachedVert.Z_ ) * Transform;
		CachedVert.X_ = Position.x();
		CachedVert.Y_ = Position.y();
		CachedVert.Z_ = Position.z();
		pVertices[i] = CachedVert;
	}
	
	//
	addPrimitive( NULL, NULL, rsPT_TRIANGLELIST, NoofIndices / 3, rsVDF_POSITION_XYZ | rsVDF_COLOUR_ABGR8, pVertices, Colour.a() < 1.0f ? 1 : 0 );
}

//////////////////////////////////////////////////////////////////////////
// debugCube
void RsFrame::debugCone( const MaMat4d& Transform, const RsColour& Colour )
{
	TVertex* pVertices = alloc< TVertex >( 18 );
	
	//
	BcU32 ColourARGB = Colour.asARGB();
	TVertex CubeFaces[] = 
	{
		// Bottom
		{ -0.5f,  0.0f, -0.5f, ColourARGB },
		{  0.5f,  0.0f, -0.5f, ColourARGB },
		{  0.5f,  0.0f,  0.5f, ColourARGB },
		{ -0.5f,  0.0f,  0.5f, ColourARGB },
		
		// Top
		{  0.0f,  1.0f,  0.0f, ColourARGB },

	};
	
	BcU32 NoofIndices = 18;
	BcU32 Indices[] = 
	{
		0, 1, 2, 
		2, 3, 0,
		4, 1, 0,
		4, 2, 1,
		4, 3, 2,
		4, 0 ,3,
	};
	
	// Copy in indexed data.
	for( BcU32 i = 0; i < NoofIndices; ++i )
	{
		TVertex CachedVert = CubeFaces[ Indices[i] ];
		MaVec3d Position = MaVec3d( CachedVert.X_, CachedVert.Y_, CachedVert.Z_ ) * Transform;
		CachedVert.X_ = Position.x();
		CachedVert.Y_ = Position.y();
		CachedVert.Z_ = Position.z();
		pVertices[i] = CachedVert;
	}

	addPrimitive( NULL, NULL, rsPT_TRIANGLELIST, NoofIndices / 3, rsVDF_POSITION_XYZ | rsVDF_COLOUR_ABGR8, pVertices, Colour.a() < 1.0f ? 1 : 0 );
}
*/

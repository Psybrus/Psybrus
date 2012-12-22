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
void RsFrame::debugLine( const BcVec3d& PointA, const BcVec3d& PointB, const RsColour& Colour )
{
	addLine( PointA, PointB, Colour, Colour.a() < 1.0f ? 1 : 0 );
}

//////////////////////////////////////////////////////////////////////////
// debugAxis/*
void RsFrame::debugAxis( const BcVec3d& Point, BcReal Size, const RsColour& Colour )
{
	beginPrimitive( rsPT_LINELIST, rsFPM_3D, Colour.a() < 1.0f ? 1 : 0 );
	addLine( Point - BcVec3d( Size, 0.0f, 0.0f ), Point + BcVec3d( Size, 0.0f, 0.0f ), Colour, 0 );
	addLine( Point - BcVec3d( 0.0f, Size, 0.0f ), Point + BcVec3d( 0.0f, Size, 0.0f ), Colour, 0 );
	addLine( Point - BcVec3d( 0.0f, 0.0f, Size ), Point + BcVec3d( 0.0f, 0.0f, Size ), Colour, 0 );
	endPrimitive();
}

//////////////////////////////////////////////////////////////////////////
// debugGrid
void RsFrame::debugGrid( const BcVec3d& Centre, BcReal Step, BcU32 Axis, BcU32 Elements, const RsColour& Colour )
{
	BcReal LBound = -Step * BcReal( Elements >> 1 );
	BcReal UBound =  Step * BcReal( Elements >> 1 );

	beginPrimitive( rsPT_LINELIST, rsFPM_3D, Colour.a() < 1.0f ? 1 : 0 );

	switch( Axis )
	{
	case 0:
		for( BcReal i = LBound; i <= UBound; i += Step )
		{
			addLine( BcVec3d( 0.0f, i, LBound ) + Centre, BcVec3d( 0.0f, i, UBound ) + Centre, Colour, 0 );
			addLine( BcVec3d( 0.0f, LBound, i ) + Centre, BcVec3d( 0.0f, UBound, i ) + Centre, Colour, 0 );
		}
		break;

	case 1:
		for( BcReal i = LBound; i <= UBound; i += Step )
		{
			addLine( BcVec3d( i, 0.0f, LBound ) + Centre, BcVec3d( i, 0.0f, UBound ) + Centre, Colour, 0 );
			addLine( BcVec3d( LBound, 0.0f, i ) + Centre, BcVec3d( UBound, 0.0f, i ) + Centre, Colour, 0 );
		}
		break;

	case 2:
		for( BcReal i = LBound; i <= UBound; i += Step )
		{
			addLine( BcVec3d( i, LBound, 0.0f ) + Centre, BcVec3d( i, UBound, 0.0f ) + Centre, Colour, 0 );
			addLine( BcVec3d( LBound, i, 0.0f ) + Centre, BcVec3d( UBound, i, 0.0f ) + Centre, Colour, 0 );
		}
		break;
	}

	endPrimitive();
}

//////////////////////////////////////////////////////////////////////////
// debugAABB
void RsFrame::debugAABB( const BcAABB& AABB, const RsColour& Colour )
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
void RsFrame::debugMatrix( const BcMat4d& Matrix, BcReal Scale, const RsColour& Colour )
{
	BcVec3d Position( Matrix.row3().x(), Matrix.row3().y(), Matrix.row3().z() );
	BcVec3d X( Matrix.row0().x(), Matrix.row0().y(), Matrix.row0().z() );
	BcVec3d Y( Matrix.row1().x(), Matrix.row1().y(), Matrix.row1().z() );
	BcVec3d Z( Matrix.row2().x(), Matrix.row2().y(), Matrix.row2().z() );

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
void RsFrame::debugEllipsoid( const BcVec3d& Centre, const BcVec3d& Scale, BcU32 LOD, const RsColour& Colour )
{
	// Draw outer circles for all axis.
	BcReal Angle = 0.0f;
	BcReal AngleInc = ( BcPI * 2.0f ) / BcReal( LOD );

	beginPrimitive( rsPT_LINELIST, rsFPM_3D, Colour.a() < 1.0f ? 1 : 0 );

	// Draw axis lines.
	for( BcU32 i = 0; i < LOD; ++i )
	{
		BcVec2d PosA( BcCos( Angle ), -BcSin( Angle ) );
		BcVec2d PosB( BcCos( Angle + AngleInc ), -BcSin( Angle + AngleInc ) );

		BcVec3d XAxisA = BcVec3d( 0.0f,                 PosA.x() * Scale.y(), PosA.y() * Scale.z() );
		BcVec3d YAxisA = BcVec3d( PosA.x() * Scale.x(), 0.0f,                 PosA.y() * Scale.z() );
		BcVec3d ZAxisA = BcVec3d( PosA.x() * Scale.x(), PosA.y() * Scale.y(), 0.0f                 );
		BcVec3d XAxisB = BcVec3d( 0.0f,                 PosB.x() * Scale.y(), PosB.y() * Scale.z() );
		BcVec3d YAxisB = BcVec3d( PosB.x() * Scale.x(), 0.0f,                 PosB.y() * Scale.z() );
		BcVec3d ZAxisB = BcVec3d( PosB.x() * Scale.x(), PosB.y() * Scale.y(), 0.0f                 );

		addLine( XAxisA + Centre, XAxisB + Centre, Colour, 0 );
		addLine( YAxisA + Centre, YAxisB + Centre, Colour, 0 );
		addLine( ZAxisA + Centre, ZAxisB + Centre, Colour, 0 );

		Angle += AngleInc;
	}

	// Draw a cross down centre.
	BcVec3d XAxis = BcVec3d( Scale.x(), 0.0f, 0.0f );
	BcVec3d YAxis = BcVec3d( 0.0f, Scale.y(), 0.0f );
	BcVec3d ZAxis = BcVec3d( 0.0f, 0.0f, Scale.z() );
	addLine( Centre - XAxis, Centre + XAxis, Colour, 0 );
	addLine( Centre - YAxis, Centre + YAxis, Colour, 0 );
	addLine( Centre - ZAxis, Centre + ZAxis, Colour, 0 );

	endPrimitive();
}

//////////////////////////////////////////////////////////////////////////
// debugCube
void RsFrame::debugCube( const BcMat4d& Transform, const RsColour& Colour )
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
		BcVec3d Position = BcVec3d( CachedVert.X_, CachedVert.Y_, CachedVert.Z_ ) * Transform;
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
void RsFrame::debugCone( const BcMat4d& Transform, const RsColour& Colour )
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
		BcVec3d Position = BcVec3d( CachedVert.X_, CachedVert.Y_, CachedVert.Z_ ) * Transform;
		CachedVert.X_ = Position.x();
		CachedVert.Y_ = Position.y();
		CachedVert.Z_ = Position.z();
		pVertices[i] = CachedVert;
	}

	addPrimitive( NULL, NULL, rsPT_TRIANGLELIST, NoofIndices / 3, rsVDF_POSITION_XYZ | rsVDF_COLOUR_ABGR8, pVertices, Colour.a() < 1.0f ? 1 : 0 );
}
*/

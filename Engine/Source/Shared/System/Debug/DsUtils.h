#pragma once

#include "Base/BcTypes.h"
#include "Math/MaAABB.h"
#include "Math/MaMat4d.h"
#include "Math/MaVec3d.h"
#include "System/Renderer/RsTypes.h"

//////////////////////////////////////////////////////////////////////////
// Debug Utilities
#if !PSY_PRODUCTION
namespace Debug
{
	class DrawCategory
	{
	public:
		DrawCategory( const char* Name );
		DrawCategory( BcU32 Category );
		~DrawCategory();

	private:
		BcU32 OldMask_;
	};

	bool CanDraw( const char* CategoryName = nullptr );
	void DrawLine( const MaVec3d& PointA, const MaVec3d& PointB, const RsColour& Colour = RsColour::WHITE, BcU32 Layer = 0 );
	void DrawLines( const MaVec3d* pPoints, BcU32 NoofLines, const RsColour& Colour = RsColour::WHITE, BcU32 Layer = 0 );
	void DrawMatrix( const MaMat4d& Matrix, const RsColour& Colour = RsColour::WHITE, BcU32 Layer = 0 );
	void DrawGrid( const MaVec3d& Position, const MaVec3d& Size, BcF32 StepSize, BcF32 SubDivideMultiple, BcU32 Layer = 0 );
	void DrawEllipsoid( const MaVec3d& Position, const MaVec3d& Size, const RsColour& Colour = RsColour::WHITE, BcU32 Layer = 0 );
	void DrawCircle( const MaVec3d& Position, const MaVec3d& Size, const RsColour& Colour = RsColour::WHITE, BcU32 Layer = 0 );
	void DrawAABB( const MaAABB& AABB, const RsColour& Colour = RsColour::WHITE, BcU32 Layer = 0 );
}
#endif

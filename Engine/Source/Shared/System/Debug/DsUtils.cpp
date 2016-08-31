#include "System/Debug/DsUtils.h"

#include "System/Scene/Rendering/ScnDebugRenderComponent.h"

//////////////////////////////////////////////////////////////////////////
// Debug utilities.
namespace Debug
{
	DrawCategory::DrawCategory( const char* Name )
	{
		if( ScnDebugRenderComponent::pImpl() )
		{
			OldMask_ = ScnDebugRenderComponent::pImpl()->getCurrentCategoryMask();
			ScnDebugRenderComponent::pImpl()->setCurrentCategoryMask( ScnDebugRenderComponent::pImpl()->getCategoryMask( Name ) );
		}
	}

	DrawCategory::DrawCategory( BcU32 Category )
	{
		if( ScnDebugRenderComponent::pImpl() )
		{
			OldMask_ = ScnDebugRenderComponent::pImpl()->getCurrentCategoryMask();
			ScnDebugRenderComponent::pImpl()->setCurrentCategoryMask( Category );
		}
	}

	DrawCategory::~DrawCategory()
	{
		if( ScnDebugRenderComponent::pImpl() )
		{
			ScnDebugRenderComponent::pImpl()->setCurrentCategoryMask( OldMask_ );
		}
	}

	bool CanDraw( const char* CategoryName )
	{
		if( ScnDebugRenderComponent::pImpl() )
		{
			if( CategoryName != nullptr )
			{
				auto CategoryMask = ScnDebugRenderComponent::pImpl()->getCategoryMask( CategoryName );
				if( CategoryMask & ScnDebugRenderComponent::pImpl()->getDrawCategoryMask() )
				{
					return true;
				}
			}
			else
			{
				return true;
			}
		}
		return false;
	}

	void DrawLine( const MaVec3d& PointA, const MaVec3d& PointB, const RsColour& Colour, BcU32 Layer )
	{
		if( CanDraw() ) ScnDebugRenderComponent::pImpl()->drawLine( PointA, PointB, Colour, Layer );
	}

	void DrawLines( const MaVec3d* pPoints, BcU32 NoofLines, const RsColour& Colour, BcU32 Layer )
	{
		if( CanDraw() ) ScnDebugRenderComponent::pImpl()->drawLines( pPoints, NoofLines, Colour, Layer );
	}

	void DrawMatrix( const MaMat4d& Matrix, const RsColour& Colour, BcU32 Layer )
	{
		if( CanDraw() ) ScnDebugRenderComponent::pImpl()->drawMatrix( Matrix,  Colour, Layer );
	}

	void DrawGrid( const MaVec3d& Position, const MaVec3d& Size, BcF32 StepSize, BcF32 SubDivideMultiple, BcU32 Layer )
	{
		if( CanDraw() ) ScnDebugRenderComponent::pImpl()->drawGrid( Position, Size, StepSize, SubDivideMultiple, Layer );
	}

	void DrawEllipsoid( const MaVec3d& Position, const MaVec3d& Size, const RsColour& Colour, BcU32 Layer )
	{
		if( CanDraw() ) ScnDebugRenderComponent::pImpl()->drawEllipsoid( Position, Size, Colour, Layer );
	}

	void DrawCircle( const MaVec3d& Position, const MaVec3d& Size, const RsColour& Colour, BcU32 Layer )
	{
		if( CanDraw() ) ScnDebugRenderComponent::pImpl()->drawCircle( Position, Size, Colour, Layer );
	}

	void DrawAABB( const MaAABB& AABB, const RsColour& Colour, BcU32 Layer )
	{
		if( CanDraw() ) ScnDebugRenderComponent::pImpl()->drawAABB( AABB, Colour, Layer );
	}
}

/**************************************************************************
*
* File:		GaWorldBSPComponent.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		World BSP
*		
*
*
* 
**************************************************************************/

#include "GaWorldBSPComponent.h"

//////////////////////////////////////////////////////////////////////////
// Define
DEFINE_RESOURCE( GaWorldBSPComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void GaWorldBSPComponent::initialise( const Json::Value& Object )
{
	InEditorMode_ = BcTrue;
	EditorState_ = ES_IDLE;

	Projection_.orthoProjection( -32.0f, 32.0f, 18.0f, -18.0f, -1.0f, 1.0f );

	NearestPoint_ = BcErrorCode;
	NearestEdge_ = BcErrorCode;
	pBSPTree_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void GaWorldBSPComponent::create()
{

}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void GaWorldBSPComponent::destroy()
{

}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool GaWorldBSPComponent::isReady()
{
	return BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaWorldBSPComponent::update( BcReal Tick )
{
	//
	if( InEditorMode_ )
	{
		const BcReal NormalSize = 0.5f;
		const BcReal HintSize = 0.125f;
		const BcVec2d HintBoxSize( HintSize, HintSize );

		Canvas_->clear();
		Canvas_->pushMatrix( Projection_ );
		Canvas_->setMaterialComponent( Material_ );

		//Canvas_->drawLine( BcVec2d( -32.0f, -32.0f ), BcVec2d( 32.0f, 32.0f ), RsColour::GREEN, 0 );
		for( BcU32 Idx = 0; Idx < Edges_.size(); ++Idx )
		{
			const GaWorldBSPEdge& Edge( Edges_[ Idx ] );
			const GaWorldBSPPoint& PointA( Points_[ Edge.A_ ] );
			const GaWorldBSPPoint& PointB( Points_[ Edge.B_ ] );

			// Draw edge.
			Canvas_->drawLine( PointA.Position_, PointB.Position_, RsColour::WHITE, 0 );

			// Draw normal.
			BcVec2d Centre( ( PointA.Position_ + PointB.Position_ ) * 0.5f );
			BcVec2d Normal( ( PointB.Position_ - PointA.Position_ ).normal().cross() );
			Canvas_->drawLine( Centre, Centre + Normal * NormalSize, RsColour::BLUE, 0 );
		}

		for( BcU32 Idx = 0; Idx < Points_.size(); ++Idx )
		{
			const GaWorldBSPPoint& Point( Points_[ Idx ] );
			Canvas_->drawLineBox( Point.Position_ - HintBoxSize, Point.Position_ + HintBoxSize, RsColour::WHITE, 0 );
		}

		// Interface.
		switch( EditorState_ )
		{
		case ES_IDLE:
			{
				if( pBSPTree_ != NULL )
				{
					BcBSPInfo BSPInfo;
					if( pBSPTree_->checkPointFront( BcVec3d( MousePointPosition_, 0.0f ) ) )
					{
						Canvas_->drawLineBox( MousePointPosition_ - HintBoxSize, MousePointPosition_ + HintBoxSize, RsColour::GREEN, 0 );
					}
					else
					{
						Canvas_->drawLineBox( MousePointPosition_ - HintBoxSize, MousePointPosition_ + HintBoxSize, RsColour::RED, 0 );
					}
				}
				else
				{
					Canvas_->drawLineBox( MousePointPosition_ - HintBoxSize, MousePointPosition_ + HintBoxSize, RsColour::BLUE, 0 );
				}

				// Highlight nearest point, if none then nearest edge.
				if( NearestPoint_ != BcErrorCode )
				{
					const GaWorldBSPPoint& Point( Points_[ NearestPoint_ ] );
					Canvas_->drawLineBox( Point.Position_ - HintBoxSize, Point.Position_ + HintBoxSize, RsColour::RED, 0 );
				}
				else
				{
					if( NearestEdge_ != BcErrorCode )
					{
						const GaWorldBSPEdge& Edge( Edges_[ NearestEdge_ ] );
						const GaWorldBSPPoint& PointA( Points_[ Edge.A_ ] );
						const GaWorldBSPPoint& PointB( Points_[ Edge.B_ ] );
	
						// Draw edge.
						Canvas_->drawLine( PointA.Position_, PointB.Position_, RsColour::RED, 0 );
					}
				}

				// Line intersections out.
				/*
				BcBSPPointInfo PointInfo;
				if( pBSPTree_ != NULL )
				{
					for( BcReal Angle = 0.0f; Angle < BcPIMUL2; Angle += BcPI * 0.025f )
					{
						BcVec3d Normal( BcVec3d( BcCos( Angle ) * 128.0f, BcSin( Angle ) * 128.0f, 4.0f ) );
						pBSPTree_->lineIntersection( BcVec3d( MousePosition_, 1.0f ), BcVec3d( MousePosition_, 0.0f ) + Normal, &PointInfo );
						Canvas_->drawLine( MousePosition_, BcVec2d( PointInfo.Point_.x(), PointInfo.Point_.y() ), RsColour::RED, 0 );
					}
				}
				*/
			}
			break;

		case ES_ADD_POINTS:
			{
				BcAssert( LastPointIdx_ != BcErrorCode );
				const GaWorldBSPPoint& Point( Points_[ LastPointIdx_ ] );

				// Draw edge.
				Canvas_->drawLine( Point.Position_, MousePointPosition_, RsColour::GREEN, 0 );

				// Draw normal.
				BcVec2d Centre( ( Point.Position_ + MousePointPosition_ ) * 0.5f );
				BcVec2d Normal( ( MousePointPosition_ - Point.Position_ ).normal().cross() );
				Canvas_->drawLine( Centre, Centre + Normal * NormalSize, RsColour::GREEN, 0 );
			}
			break;
		}
		
		//Canvas_->popMatrix(); // hack!
	}
}

//////////////////////////////////////////////////////////////////////////
// render
//virtual
void GaWorldBSPComponent::render( class ScnViewComponent* pViewComponent, RsFrame* pFrame, RsRenderSort Sort )
{

}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaWorldBSPComponent::onAttach( ScnEntityWeakRef Parent )
{
	OsEventInputKeyboard::Delegate OnKeyboardEvent = OsEventInputKeyboard::Delegate::bind< GaWorldBSPComponent, &GaWorldBSPComponent::onKeyboardEvent >( this );
	OsEventInputMouse::Delegate OnMouseEvent = OsEventInputMouse::Delegate::bind< GaWorldBSPComponent, &GaWorldBSPComponent::onMouseEvent >( this );

	OsCore::pImpl()->subscribe( osEVT_INPUT_KEYDOWN, OnKeyboardEvent );
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEDOWN, OnMouseEvent );
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEMOVE, OnMouseEvent );

	//
	Canvas_ = Parent->getComponentByType< ScnCanvasComponent >( 0 );

	ScnMaterialRef Material;
	if( CsCore::pImpl()->requestResource( "default", "default", Material ) && CsCore::pImpl()->createResource( BcName::INVALID, Material_, Material, BcErrorCode ) )
	{
		Parent->attach( Material_ );
	}
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaWorldBSPComponent::onDetach( ScnEntityWeakRef Parent )
{
	OsCore::pImpl()->unsubscribeAll( this );

	Canvas_ = NULL;
	Parent->detach( Material_ );
}

//////////////////////////////////////////////////////////////////////////
// onMouseEvent
eEvtReturn GaWorldBSPComponent::onKeyboardEvent( EvtID ID, const OsEventInputKeyboard& Event )
{
	switch( Event.AsciiCode_ )
	{
	case 'S':
	case 's':
		saveJson();
		break;

	case 'L':
	case 'l':
		loadJson();
		break;

	case 'B':
	case 'b':
		buildBSP();
		break;

	default:
		break;
	}

	return evtRET_PASS;
}

//////////////////////////////////////////////////////////////////////////
// onMouseEvent
eEvtReturn GaWorldBSPComponent::onMouseEvent( EvtID ID, const OsEventInputMouse& Event )
{
	if( InEditorMode_ )
	{
		// Picking.
		OsClient* pClient = OsCore::pImpl()->getClient( 0 );
		BcVec2d Position( Event.MouseX_, Event.MouseY_ );
		const BcVec2d RealScreen( ( Position.x() / pClient->getWidth() ) * 2.0f - 1.0f, ( Position.y() / pClient->getHeight() ) * 2.0f - 1.0f );
	
		BcMat4d InvProjMat = Projection_;
		InvProjMat.inverse();
		BcVec3d WorldNearPos( RealScreen.x(), -RealScreen.y(), 0.0f );
		BcVec3d WorldFarPos( RealScreen.x(), -RealScreen.y(), 1.0f );
	
		WorldNearPos = WorldNearPos * InvProjMat;
		WorldFarPos = WorldFarPos * InvProjMat;
	
		if( Projection_[3][3] == 0.0f )
		{
			WorldNearPos *= -1.0f;
			WorldFarPos *= 1.0f;
		}

		Position.set( WorldNearPos.x(), WorldNearPos.y() );

		// Find nearest point for 
		BcU32 PointIdx = nearestPoint( Position, 0.25f );
		BcU32 EdgeIdx = nearestEdge( Position, 0.25f );

		// Cache for UI use.
		NearestPoint_ = PointIdx;
		NearestEdge_ = EdgeIdx;
		
		switch( ID )
		{
		case osEVT_INPUT_MOUSEDOWN:
			{
				switch( EditorState_ )
				{
				case ES_IDLE:
					{
						// Start edging.
						if( Event.ButtonCode_ == 0 )
						{
							// No point? Add.
							if( PointIdx == BcErrorCode )
							{
								PointIdx = addPoint( Position );
							}

							LastPointIdx_ = PointIdx;
							EditorState_ = ES_ADD_POINTS;
						}
						// Remove point or edge.
						else if ( Event.ButtonCode_ == 1 )
						{
							if( NearestPoint_ != BcErrorCode )
							{
								removePoint( NearestPoint_ );
								NearestPoint_ = BcErrorCode;
								NearestEdge_ = BcErrorCode;
							}
							else if( NearestEdge_ != BcErrorCode )
							{
								removeEdge( NearestEdge_ );
								NearestEdge_ = BcErrorCode;
							}
						}
						// Invert edge.
						else if ( Event.ButtonCode_ == 2 )
						{
							if( NearestPoint_ == BcErrorCode && NearestEdge_ != BcErrorCode )
							{
								invertEdge( NearestEdge_ );
							}
						}

					}
					break;

				case ES_ADD_POINTS:
					{
						if( Event.ButtonCode_ == 0 )
						{
							if( PointIdx == BcErrorCode )
							{
								PointIdx = addPoint( Position );
							}
							else
							{
								// If the point exists, terminate.
								EditorState_ = ES_IDLE;
							}

							// Add edge.
							if( addEdge( LastPointIdx_, PointIdx ) )
							{
								LastPointIdx_ = PointIdx;
							}
							else
							{
								// Couldn't add edge, bail out.
								EditorState_ = ES_IDLE;
								LastPointIdx_ = BcErrorCode;
							}				
						}
						else if ( Event.ButtonCode_ == 1 )
						{
							EditorState_ = ES_IDLE;
							LastPointIdx_ = BcErrorCode;
						}
					}
					break;
				}
			}
			break;

		case osEVT_INPUT_MOUSEMOVE:
			{
				// Store mouse position.
				MousePosition_ = Position;
				MousePointPosition_.x( BcRound( Position.x() ) );
				MousePointPosition_.y( BcRound( Position.y() ) );
			}
			break;
		}
	}

	return evtRET_PASS;
}

//////////////////////////////////////////////////////////////////////////
// nearestPoint
BcU32 GaWorldBSPComponent::nearestPoint( BcVec2d Position, BcReal Radius )
{
	// Round up the position to the nearest int.
	Position.x( BcRound( Position.x() ) );
	Position.y( BcRound( Position.y() ) );

	// Find.
	BcU32 FoundIdx = BcErrorCode;
	BcReal Closest = Radius;

	for( BcU32 Idx = 0; Idx < Points_.size(); ++Idx )
	{
		GaWorldBSPPoint& Point( Points_[ Idx ] );
		BcReal Distance = ( Point.Position_ - Position ).magnitude();

		if( Distance < Closest )
		{
			Closest = Distance;
			FoundIdx = Idx;
		}
	}
	
	return FoundIdx;
}

//////////////////////////////////////////////////////////////////////////
// addPoint
BcU32 GaWorldBSPComponent::addPoint( BcVec2d Position )
{
	// Round up the position to the nearest int.
	Position.x( BcRound( Position.x() ) );
	Position.y( BcRound( Position.y() ) );

	// Setup point.
	GaWorldBSPPoint Point = 
	{
		Position
	};

	Points_.push_back( Point );
	return Points_.size() - 1;
}

//////////////////////////////////////////////////////////////////////////
// addEdge
BcBool GaWorldBSPComponent::addEdge( BcU32 IdxA, BcU32 IdxB )
{
	// Check edge doesn't exist already.
	for( BcU32 Idx = 0; Idx < Edges_.size(); ++Idx )
	{
		GaWorldBSPEdge& Edge( Edges_[ Idx ] );

		// If it exists, we can't add it.
		if( Edge.A_ == IdxA && Edge.B_ == IdxB ||
			Edge.A_ == IdxB && Edge.B_ == IdxA )
		{
			return BcFalse;
		}
	}
	
	Edges_.push_back( GaWorldBSPEdge( IdxA, IdxB ) );
	
	return BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// removePoint
void GaWorldBSPComponent::removePoint( BcU32 Idx )
{
	// Remove point at index.
	BcU32 PointIdx = 0;
	for( std::vector< GaWorldBSPPoint >::iterator It( Points_.begin() ); It != Points_.end(); ++PointIdx, ++It )
	{
		if( PointIdx == Idx )
		{
			Points_.erase( It );
			break;
		}
	}

	// Remove any edges that reference this point.
	for( std::vector< GaWorldBSPEdge >::iterator It( Edges_.begin() ); It != Edges_.end(); )
	{
		if( (*It).A_ == Idx || (*It).B_ == Idx )
		{
			It = Edges_.erase( It );
		}
		else
		{
			++It;
		}
	}

	// Update edge indices.
	for( std::vector< GaWorldBSPEdge >::iterator It( Edges_.begin() ); It != Edges_.end(); ++It )
	{
		if( (*It).A_ > Idx )
		{
			(*It).A_--;
		}
		if( (*It).B_ > Idx )
		{
			(*It).B_--;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// removeEdge
void GaWorldBSPComponent::removeEdge( BcU32 Idx )
{
	// Remove point at index.
	BcU32 EdgeIdx = 0;
	for( std::vector< GaWorldBSPEdge >::iterator It( Edges_.begin() ); It != Edges_.end(); ++EdgeIdx, ++It )
	{
		if( EdgeIdx == Idx )
		{
			Edges_.erase( It );
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// invertEdge
void GaWorldBSPComponent::invertEdge( BcU32 Idx )
{
	GaWorldBSPEdge& Edge( Edges_[ Idx ] );
	BcU32 Temp = Edge.B_;
	Edge.B_ = Edge.A_;
	Edge.A_ = Temp;
}

//////////////////////////////////////////////////////////////////////////
// nearestEdge
BcU32 GaWorldBSPComponent::nearestEdge( const BcVec2d& Position, BcReal Radius )
{
	BcU32 NearestIdx = BcErrorCode;
	BcReal NearestDistance = Radius;

	// Check edge doesn't exist already.
	for( BcU32 Idx = 0; Idx < Edges_.size(); ++Idx )
	{
		const BcVec2d& NearestPosition( nearestPositionOnEdge( Position, Idx ) );

		BcReal Distance = ( NearestPosition - Position ).magnitude();
		if( Distance < NearestDistance )
		{
			NearestIdx = Idx;
			NearestDistance = Distance;
		}
	}

	return NearestIdx;
}

//////////////////////////////////////////////////////////////////////////
// nearestPositionOnEdge
BcVec2d GaWorldBSPComponent::nearestPositionOnEdge( const BcVec2d& Position, BcU32 Idx )
{
	const GaWorldBSPEdge& Edge( Edges_[ Idx ] );
	const BcVec2d& PointA( Points_[ Edge.A_ ].Position_ );
	const BcVec2d& PointB( Points_[ Edge.B_ ].Position_ );

	BcVec2d AP = Position - PointA;
	BcVec2d AB = PointB - PointA;
	BcReal AB2 = AB.dot( AB );
	BcReal APAB = AP.dot( AB );
	BcReal T = BcClamp( APAB / AB2, 0.0f, 1.0f );
	return ( PointA + AB * T );
}

//////////////////////////////////////////////////////////////////////////
// saveJson
void GaWorldBSPComponent::saveJson()
{
	Json::Value LevelData;
	Json::Value PointsData( Json::arrayValue );
	Json::Value EdgesData( Json::arrayValue );
	
	PointsData.resize( Points_.size() );
	for( BcU32 Idx = 0; Idx < Points_.size(); ++Idx )
	{
		const GaWorldBSPPoint& Point( Points_[ Idx ] );
		BcChar Buffer[ 256 ];
		BcSPrintf(Buffer, "%.2f,%.2f", Point.Position_.x(), Point.Position_.y() );
		PointsData[ Idx ] = Buffer;
	}

	EdgesData.resize( Edges_.size() );
	for( BcU32 Idx = 0; Idx < Edges_.size(); ++Idx )
	{
		const GaWorldBSPEdge& Edge( Edges_[ Idx ] );
		BcChar Buffer[ 256 ];
		BcSPrintf(Buffer, "%u,%u", Edge.A_, Edge.B_ );
		EdgesData[ Idx ] = Buffer;
	}

	LevelData["points"] = PointsData;
	LevelData["edges"] = EdgesData;

	Json::FastWriter Writer;
	std::string JsonOutput = Writer.write( LevelData );


}

//////////////////////////////////////////////////////////////////////////
// loadJson
void GaWorldBSPComponent::loadJson()
{

}

//////////////////////////////////////////////////////////////////////////
// buildBSP
void GaWorldBSPComponent::buildBSP()
{
	delete pBSPTree_;
	pBSPTree_ = new BcBSPTree();

	// TODO: Add top and bottom? Do we need?
	
	// Add edges.
	for( BcU32 Idx = 0; Idx < Edges_.size(); ++Idx )
	{
		const GaWorldBSPEdge& Edge( Edges_[ Idx ] );
		const GaWorldBSPPoint& PointA( Points_[ Edge.A_ ] );
		const GaWorldBSPPoint& PointB( Points_[ Edge.B_ ] );

		BcVec3d Vertices[ 4 ] = 
		{
			BcVec3d( PointA.Position_, 0.0f ),
			BcVec3d( PointB.Position_, 0.0f ),
			BcVec3d( PointB.Position_, 8.0f ),
			BcVec3d( PointA.Position_, 8.0f ),
		};

		BcPlane Plane;
		Plane.fromPoints( Vertices[ 0 ], Vertices[ 1 ], Vertices [ 2 ] );

		pBSPTree_->addNode( Plane, Vertices, 4 );
	}

	pBSPTree_->buildTree();
}

//////////////////////////////////////////////////////////////////////////
// checkPointFront
BcBool GaWorldBSPComponent::checkPointFront( const BcVec3d& Point, BcBSPInfo* pData, BcBSPNode* pNode )
{
	if( pBSPTree_ )
	{
		return pBSPTree_->checkPointFront( Point, pData, pNode );
	}
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// checkPointBack
BcBool GaWorldBSPComponent::checkPointBack( const BcVec3d& Point, BcBSPInfo* pData, BcBSPNode* pNode )
{
	if( pBSPTree_ )
	{
		return pBSPTree_->checkPointBack( Point, pData, pNode );
	}
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// lineIntersection
BcBool GaWorldBSPComponent::lineIntersection( const BcVec3d& A, const BcVec3d& B, BcBSPPointInfo* pPointInfo, BcBSPNode* pNode )
{
	if( pBSPTree_ )
	{
		return pBSPTree_->lineIntersection( A, B, pPointInfo, pNode );
	}
	return BcFalse;
}

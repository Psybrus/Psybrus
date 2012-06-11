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
// Round
static BcVec2d RoundVector( BcVec2d Vector )
{
	const BcReal Factor = 1.0f;
	return BcVec2d( BcRound( Vector.x() / Factor ) * Factor, BcRound( Vector.y() / Factor ) * Factor );
}

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

	pVertexArray_ = NULL;
	pVertexBuffer_ = NULL;
	pPrimitive_ = NULL;

	CurrentLevel_ = 0;
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
	OsCore::pImpl()->getClient( 0 )->setMouseLock( !InEditorMode_ );

	Canvas_->clear();
	Canvas_->pushMatrix( Projection_ );
	Canvas_->setMaterialComponent( Material_ );

	if( InEditorMode_ )
	{
		const BcReal NormalSize = 0.5f;
		const BcReal HintSize = 0.125f;
		const BcVec2d HintBoxSize( HintSize, HintSize );


		{
			Canvas_->drawBox( BcVec2d( -32.0f, -18.0f ), BcVec2d( 32.0f, 18.0f ), RsColour( 0.0f, 0.0f, 0.0f, 1.0f ), 0 );
		}

		// Draw editor grid.
		BcReal Size = 16.0f;
		for( BcReal T = -Size; T <= Size; T += 1.0f )
		{
			Canvas_->drawLine( BcVec2d( T, -Size ), BcVec2d( T, Size ), RsColour( 1.0f, 1.0f, 1.0f, 0.05f ), 1 );
			Canvas_->drawLine( BcVec2d( -Size,T ), BcVec2d( Size, T ), RsColour( 1.0f, 1.0f, 1.0f, 0.05f ), 1 );
		}
		for( BcReal T = -Size; T <= Size; T += 4.0f )
		{
			Canvas_->drawLine( BcVec2d( T, -Size ), BcVec2d( T, Size ), RsColour( 1.0f, 1.0f, 1.0f, 0.05f ), 1 );
			Canvas_->drawLine( BcVec2d( -Size,T ), BcVec2d( Size, T ), RsColour( 1.0f, 1.0f, 1.0f, 0.05f ), 1 );
		}

		for( BcU32 Idx = 0; Idx < Edges_.size(); ++Idx )
		{
			const GaWorldBSPEdge& Edge( Edges_[ Idx ] );
			const GaWorldBSPPoint& PointA( Points_[ Edge.A_ ] );
			const GaWorldBSPPoint& PointB( Points_[ Edge.B_ ] );

			// Draw edge.
			Canvas_->drawLine( PointA.Position_, PointB.Position_, RsColour::WHITE, 1 );

			// Draw normal.
			BcVec2d Centre( ( PointA.Position_ + PointB.Position_ ) * 0.5f );
			BcVec2d Normal( ( PointB.Position_ - PointA.Position_ ).normal().cross() );
			Canvas_->drawLine( Centre, Centre + Normal * NormalSize, RsColour::BLUE, 1 );
		}

		for( BcU32 Idx = 0; Idx < Points_.size(); ++Idx )
		{
			const GaWorldBSPPoint& Point( Points_[ Idx ] );
			Canvas_->drawLineBox( Point.Position_ - HintBoxSize, Point.Position_ + HintBoxSize, RsColour::WHITE, 1 );
		}

		// Interface.
		switch( EditorState_ )
		{
		case ES_IDLE:
			{
				if( pBSPTree_ != NULL )
				{
					BcBSPInfo BSPInfo;
					if( pBSPTree_->checkPointFront( BcVec3d( MousePosition_, 0.0f ), 1e-3f ) )
					{
						Canvas_->drawLineBox( MousePosition_ - HintBoxSize, MousePosition_ + HintBoxSize, RsColour::GREEN, 2 );
					}
					else
					{
						Canvas_->drawLineBox( MousePosition_ - HintBoxSize, MousePosition_ + HintBoxSize, RsColour::RED, 2 );
					}
				}
				Canvas_->drawLineBox( MousePointPosition_ - HintBoxSize, MousePointPosition_ + HintBoxSize, RsColour::BLUE, 2 );

				// Highlight nearest point, if none then nearest edge.
				if( NearestPoint_ != BcErrorCode )
				{
					const GaWorldBSPPoint& Point( Points_[ NearestPoint_ ] );
					Canvas_->drawLineBox( Point.Position_ - HintBoxSize, Point.Position_ + HintBoxSize, RsColour::RED, 2 );
				}
				else
				{
					if( NearestEdge_ != BcErrorCode )
					{
						const GaWorldBSPEdge& Edge( Edges_[ NearestEdge_ ] );
						const GaWorldBSPPoint& PointA( Points_[ Edge.A_ ] );
						const GaWorldBSPPoint& PointB( Points_[ Edge.B_ ] );
	
						// Draw edge.
						Canvas_->drawLine( PointA.Position_, PointB.Position_, RsColour::RED, 2 );
					}
				}

				// Line intersections out.
				BcBSPPointInfo PointInfo;
				if( pBSPTree_ != NULL )
				{
					for( BcReal Angle = 0.0f; Angle < BcPIMUL2; Angle += BcPI * 0.025f )
					{
						BcVec3d Normal( BcVec3d( BcCos( Angle ) * 128.0f, BcSin( Angle ) * 128.0f, 0.0f ) );
						PointInfo.Point_ = BcVec3d( MousePosition_, 0.0f ) + Normal;
						pBSPTree_->lineIntersection( BcVec3d( MousePosition_, 0.0f ), PointInfo.Point_, &PointInfo );
						Canvas_->drawLine( MousePosition_, BcVec2d( PointInfo.Point_.x(), PointInfo.Point_.y() ), RsColour( 1.0f, 0.0f, 0.0f, 0.25f ), 0 );
					}
				}
			}
			break;

		case ES_ADD_POINTS:
			{
				BcAssert( LastPointIdx_ != BcErrorCode );
				const GaWorldBSPPoint& Point( Points_[ LastPointIdx_ ] );

				// Draw edge.
				Canvas_->drawLine( Point.Position_, MousePointPosition_, RsColour::GREEN, 2 );

				// Draw normal.
				BcVec2d Centre( ( Point.Position_ + MousePointPosition_ ) * 0.5f );
				BcVec2d Normal( ( MousePointPosition_ - Point.Position_ ).normal().cross() );
				Canvas_->drawLine( Centre, Centre + Normal * NormalSize, RsColour::GREEN, 2 );
			}
			break;
		}
		
		//Canvas_->popMatrix(); // hack!
	}
}

//////////////////////////////////////////////////////////////////////////
// render
class GaWorldBSPComponenttRenderNode: public RsRenderNode
{
public:
	void render()
	{
		pPrimitive_->render( rsPT_TRIANGLELIST, 0, NoofIndices_ );
	}
	
	RsPrimitive* pPrimitive_;
	BcU32 NoofIndices_;
};

//virtual
void GaWorldBSPComponent::render( class ScnViewComponent* pViewComponent, RsFrame* pFrame, RsRenderSort Sort )
{	
	if( pVertexArray_ != NULL )
	{
		// Bind material.
		MaterialWorld_->setWorldTransform( BcMat4d() );
		MaterialWorld_->bind( pFrame, Sort );

		// Setup render node.
		GaWorldBSPComponenttRenderNode* pRenderNode = pFrame->newObject< GaWorldBSPComponenttRenderNode >();
		pRenderNode->pPrimitive_ = pPrimitive_;
		pRenderNode->NoofIndices_ = pVertexBuffer_->getNoofVertices();

		// Add to frame.
		pRenderNode->Sort_ = Sort;
		//pFrame->addRenderNode( pRenderNode );
	}
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
	
	if( CsCore::pImpl()->requestResource( "default", "airsolid", Material ) && CsCore::pImpl()->createResource( BcName::INVALID, MaterialWorld_, Material, BcErrorCode ) )
	{
		Parent->attach( MaterialWorld_ );
	}

	// Ok setup player.
	// TODO: Move to a game state component.
	// Attach player to world.
	ScnEntityRef PlayerEntity = ScnCore::pImpl()->createEntity( "default", "PlayerEntity", "PlayerEntity_0" );
	Parent->attach( PlayerEntity );

	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaWorldBSPComponent::onDetach( ScnEntityWeakRef Parent )
{
	OsCore::pImpl()->unsubscribeAll( this );

	Canvas_ = NULL;
	Parent->detach( Material_ );

	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onMouseEvent
eEvtReturn GaWorldBSPComponent::onKeyboardEvent( EvtID ID, const OsEventInputKeyboard& Event )
{
	switch( Event.AsciiCode_ )
	{
	case 0x9:
		InEditorMode_ = !InEditorMode_;
		break;

	case 'X':
	case 'x':
		if( InEditorMode_ )
		{
			saveJson();
			buildBSP();
		}
		break;

	case 'B':
	case 'b':
		if( InEditorMode_ )
			buildBSP();
		break;

	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		if( InEditorMode_ )
		{
			CurrentLevel_ = Event.AsciiCode_ - '0';
			loadJson();
			buildBSP();
		}
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
				MousePointPosition_ = RoundVector( Position );
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
	Position = RoundVector( Position );

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
	Position = RoundVector( Position );

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

	BcFile OutputFile;
	BcChar FileName[ 128 ];
	BcSPrintf( FileName, "./LevelData%u.json", CurrentLevel_ );
	if( OutputFile.open( FileName, bcFM_WRITE ) )
	{
		OutputFile.write( JsonOutput.c_str(), JsonOutput.size() );
		OutputFile.close();
	}
}

//////////////////////////////////////////////////////////////////////////
// loadJson
void GaWorldBSPComponent::loadJson()
{
	Json::Value LevelData;
	Json::Value PointsData( Json::arrayValue );
	Json::Value EdgesData( Json::arrayValue );

	// Clear out old level.
	Points_.clear();
	Edges_.clear();

	// Load new in.
	BcFile InputFile;
	BcChar FileName[ 128 ];
	BcSPrintf( FileName, "./LevelData%u.json", CurrentLevel_ );
	if( InputFile.open( FileName, bcFM_READ ) )
	{
		BcChar* pBuffer = new BcChar[ InputFile.size() + 1 ];
		BcMemZero( pBuffer, InputFile.size() + 1 );
		InputFile.read( pBuffer, InputFile.size() );
		InputFile.close();

		Json::Reader Reader;
		if( Reader.parse( pBuffer, pBuffer + InputFile.size(), LevelData ) )
		{
			PointsData = LevelData[ "points" ];
			EdgesData = LevelData[ "edges" ];

			for( BcU32 Idx = 0; Idx < PointsData.size(); ++Idx )
			{
				Json::Value& PointData( PointsData[ Idx ] );
				BcReal X, Y;
				BcSScanf( PointData.asCString(), "%f,%f", &X, &Y );

				GaWorldBSPPoint Point = { BcVec2d( X, Y ) };
				Points_.push_back( Point );
			}

			for( BcU32 Idx = 0; Idx < EdgesData.size(); ++Idx )
			{
				Json::Value& EdgeData( EdgesData[ Idx ] );

				BcU32 A, B;
				BcSScanf( EdgeData.asCString(), "%u,%u", &A, &B );
				GaWorldBSPEdge Edge( A, B );
				Edges_.push_back( Edge );
			}
		}
		else
		{
			BcPrintf( "GaWorldBSPComponent: Failed to parse Json:\n %s\n", Reader.getFormatedErrorMessages().c_str() );
			BcAssertMsg( BcFalse, "Failed to parse \"%s\", see log for more details.", FileName );
			return;
		}

		delete [] pBuffer;
	}
}

//////////////////////////////////////////////////////////////////////////
// buildBSP
void GaWorldBSPComponent::buildBSP()
{
	delete pBSPTree_;
	pBSPTree_ = NULL;

	// Free old vertex buffer.
	if( pVertexArray_ != NULL )
	{
		RsCore::pImpl()->destroyResource( pVertexBuffer_ );
		pVertexBuffer_ = NULL;

		RsCore::pImpl()->destroyResource( pPrimitive_ );
		pVertexBuffer_ = NULL;

		// Wait for renderer.
		SysFence Fence( RsCore::WORKER_MASK );
		
		// Delete working data.
		delete [] pVertexArray_;
		pVertexArray_ = NULL;
	}

	if( Edges_.size() > 0 )
	{
		pBSPTree_ = new BcBSPTree();

		BcU32 NoofVertices = Edges_.size() * 6;
		BcU32 VertexDescriptor = rsVDF_POSITION_XYZ | rsVDF_TEXCOORD_UVW0 | rsVDF_COLOUR_RGBA8;

		// Setup vertex array.
		pVertexArray_ =  new GaWorldBSPVertex[ NoofVertices ];
		GaWorldBSPVertex* pVertex =  pVertexArray_;
		
		// Add edges.
		for( BcU32 Idx = 0; Idx < Edges_.size(); ++Idx )
		{
			const GaWorldBSPEdge& Edge( Edges_[ Idx ] );
			const GaWorldBSPPoint& PointA( Points_[ Edge.A_ ] );
			const GaWorldBSPPoint& PointB( Points_[ Edge.B_ ] );
			
			BcVec3d Vertices[ 4 ] = 
			{
				BcVec3d( PointA.Position_, -4.0f ),
				BcVec3d( PointB.Position_, -4.0f ),
				BcVec3d( PointB.Position_,  4.0f ),
				BcVec3d( PointA.Position_,  4.0f ),
			};

			BcU32 Indices[ 6 ] =
			{
				0, 1, 2, 2, 3, 0
			};

			BcVec3d Offset = BcVec3d( 64.0f, 64.0f, 4.0f );
			for( BcU32 Vert = 0; Vert < 6; ++Vert )
			{
				BcVec3d Vertex = Vertices[ Indices[ Vert ] ];
				BcVec3d Index = ( Vertex + Offset );
				pVertex->X_ = Vertex.x();
				pVertex->Y_ = Vertex.y();
				pVertex->Z_ = Vertex.z();
				pVertex->U_ = ( Index.x() / 128.0f );
				pVertex->V_ = ( Index.y() / 128.0f );
				pVertex->W_ = ( Index.z() / 8.0f );
				pVertex->RGBA_ = 0xffffffff;
				++pVertex;
			}
	
			BcPlane Plane;
			Plane.fromPoints( Vertices[ 0 ], Vertices[ 1 ], Vertices [ 2 ] );
			pBSPTree_->addNode( Plane, Vertices, 4 );
		}

		// Build tree.
		pBSPTree_->buildTree();

		// Setup primitive and vertex buffer.
		pVertexBuffer_ = RsCore::pImpl()->createVertexBuffer( VertexDescriptor, NoofVertices, pVertexArray_ ); 
		pPrimitive_ = RsCore::pImpl()->createPrimitive( pVertexBuffer_, NULL );
	}
}

//////////////////////////////////////////////////////////////////////////
// checkPointFront
BcBool GaWorldBSPComponent::checkPointFront( const BcVec3d& Point, BcReal Radius, BcBSPInfo* pData, BcBSPNode* pNode )
{
	if( pBSPTree_ )
	{
		return pBSPTree_->checkPointFront( Point, Radius, pData, pNode );
	}
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// checkPointBack
BcBool GaWorldBSPComponent::checkPointBack( const BcVec3d& Point, BcReal Radius, BcBSPInfo* pData, BcBSPNode* pNode )
{
	if( pBSPTree_ )
	{
		return pBSPTree_->checkPointBack( Point, Radius, pData, pNode );
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

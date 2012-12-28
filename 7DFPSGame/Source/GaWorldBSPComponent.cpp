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

#include "GaPawnComponent.h"

//////////////////////////////////////////////////////////////////////////
// Round
static BcVec2d RoundVector( BcVec2d Vector )
{
	const BcF32 Factor = 1.0f;
	return BcVec2d( BcRound( Vector.x() / Factor ) * Factor, BcRound( Vector.y() / Factor ) * Factor );
}

//////////////////////////////////////////////////////////////////////////
// Define
DEFINE_RESOURCE( GaWorldBSPComponent );

BCREFLECTION_EMPTY_REGISTER( GaWorldBSPComponent );
/*
BCREFLECTION_DERIVED_BEGIN( ScnComponent, GaWorldBSPComponent )
	BCREFLECTION_MEMBER( BcName,							Name_,							bcRFF_DEFAULT | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( BcU32,								Index_,							bcRFF_DEFAULT | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( CsPackage,							pPackage_,						bcRFF_POINTER | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( BcU32,								RefCount_,						bcRFF_DEFAULT | bcRFF_TRANSIENT ),
BCREFLECTION_DERIVED_END();
*/

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void GaWorldBSPComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );

	Json::Value Level = Object[ "level" ];
	Json::Value IsEditor = Object[ "editor" ];

	if( IsEditor.type() == Json::booleanValue )
	{
		IsEditor_ = IsEditor.asBool();
	}

	InEditorMode_ = IsEditor_;
	EditorState_ = ES_IDLE;

	Projection_.orthoProjection( -32.0f, 32.0f, 18.0f, -18.0f, -1.0f, 1.0f );

	NearestPoint_ = BcErrorCode;
	NearestEdge_ = BcErrorCode;
	NearestEnemy_ = BcErrorCode;
	pBSPTree_ = NULL;

	pVertexArray_ = NULL;
	pVertexBuffer_ = NULL;
	pPrimitive_ = NULL;

	CurrentLevel_ = 1;
	TotalLevels_ = 7;

	StartPosition_ = BcVec2d( 0.0f, 0.0f );
	QuitPosition_ = BcVec2d( 0.0f, 0.0f );

	if( Level.type() == Json::intValue )
	{
		CurrentLevel_ = Level.asInt();
	}

	TextTimer_ = 0.0f;
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
void GaWorldBSPComponent::update( BcF32 Tick )
{
	OsCore::pImpl()->getClient( 0 )->setMouseLock( !InEditorMode_ );

	Canvas_->clear();
	Canvas_->pushMatrix( Projection_ );


	if( !InEditorMode_ && TextList_.size() > 0 )
	{
		BcMat4d TextScaleMatrix;
		TextScaleMatrix.scale( BcVec4d( 0.1f, 0.1f, 1.0f, 1.0f ) );

		TextTimer_ += Tick;
		BcF32 TextThing = BcAbs( BcSin( TextTimer_ ) ) ;
		Font_->getMaterialComponent()->setParameter( FontScaleParam_, BcVec2d( 0.4f, 0.6f ) );

		Canvas_->pushMatrix( TextScaleMatrix );

		std::string Text = TextList_[ 0 ];
		BcVec2d Size = Font_->draw( Canvas_, BcVec2d( 0.0f, 0.0f ), Text, RsColour::WHITE, BcTrue, 16 );
		Font_->draw( Canvas_, Size * -0.5f, Text, RsColour( 1.0f, 1.0f, 1.0f, TextThing ), BcFalse, 16 );
		Canvas_->popMatrix();

		if( TextTimer_ > BcPI )
		{
			TextTimer_ = 0.0f;
			TextList_.erase( TextList_.begin() );
		}
	}


	//
	Canvas_->setMaterialComponent( Material_ );

	if( InEditorMode_ )
	{
		const BcF32 NormalSize = 0.5f;
		const BcF32 HintSize = 0.125f;
		const BcVec2d HintBoxSize( HintSize, HintSize );

		{
			Canvas_->drawBox( BcVec2d( -32.0f, -18.0f ), BcVec2d( 32.0f, 18.0f ), RsColour( 0.0f, 0.0f, 0.0f, 1.0f ), 0 );
		}

		// Draw editor grid.
		BcF32 Size = 16.0f;
		for( BcF32 T = -Size; T <= Size; T += 1.0f )
		{
			Canvas_->drawLine( BcVec2d( T, -Size ), BcVec2d( T, Size ), RsColour( 1.0f, 1.0f, 1.0f, 0.05f ), 1 );
			Canvas_->drawLine( BcVec2d( -Size,T ), BcVec2d( Size, T ), RsColour( 1.0f, 1.0f, 1.0f, 0.05f ), 1 );
		}
		for( BcF32 T = -Size; T <= Size; T += 4.0f )
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

		BcVec2d EnemySize( 0.5f, 0.5f );
		BcVec2d EnemySizeOther( -EnemySize.x(), EnemySize.y() );
		for( BcU32 Idx = 0; Idx < Enemies_.size(); ++Idx )
		{
			BcVec2d Enemy( Enemies_[ Idx ] );
			Canvas_->drawLineBox( Enemy - EnemySize, Enemy + EnemySize, RsColour::WHITE, 1 );
			Canvas_->drawLine( Enemy - EnemySize, Enemy + EnemySize, RsColour::WHITE, 1 );
			Canvas_->drawLine( Enemy - EnemySizeOther, Enemy + EnemySizeOther, RsColour::WHITE, 1 );

			// Do ray casts for enemy position in all 4 directions.

		}

		for( BcU32 Idx = 0; Idx < Points_.size(); ++Idx )
		{
			const GaWorldBSPPoint& Point( Points_[ Idx ] );
			Canvas_->drawLineBox( Point.Position_ - HintBoxSize, Point.Position_ + HintBoxSize, RsColour::WHITE, 1 );
		}

		Canvas_->drawBox( StartPosition_ - HintBoxSize, StartPosition_ + HintBoxSize, RsColour::GREEN, 1 );
		Canvas_->drawBox( QuitPosition_ - HintBoxSize, QuitPosition_ + HintBoxSize, RsColour::RED, 1 );

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
					else if ( NearestEnemy_ != BcErrorCode )
					{
						BcVec2d Enemy( Enemies_[ NearestEnemy_ ] );
						Canvas_->drawLineBox( Enemy - EnemySize, Enemy + EnemySize, RsColour::RED, 1 );
						Canvas_->drawLine( Enemy - EnemySize, Enemy + EnemySize, RsColour::RED, 1 );
						Canvas_->drawLine( Enemy - EnemySizeOther, Enemy + EnemySizeOther, RsColour::RED, 1 );
					}
				}

				// Line intersections out.
				BcBSPPointInfo PointInfo;
				if( pBSPTree_ != NULL )
				{
					for( BcF32 Angle = 0.0f; Angle < BcPIMUL2; Angle += BcPI * 0.025f )
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

	// Next level stuff.
	if( !InEditorMode_ )
	{
		BcVec3d PlayerPosition = PlayerEntity_->getPosition();
		BcF32 Distance = ( BcVec3d( QuitPosition_, 0.0f ) - PlayerPosition ).magnitude();

		if( Distance < 2.0f )
		{
			CurrentLevel_++;

			if( CurrentLevel_ <= TotalLevels_ )
			{
				loadJson();
			}
			else
			{
				BcBreakpoint;
			}
		}
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
	Canvas_ = Parent->getComponentAnyParentByType< ScnCanvasComponent >( 0 );
	WorldInfo_ = Parent->getComponentAnyParentByType< GaWorldInfoComponent >( 0 );

	ScnMaterialRef Material;
	if( CsCore::pImpl()->requestResource( "materials", "default", Material ) && CsCore::pImpl()->createResource( BcName::INVALID, getPackage(), Material_, Material, scnSPF_2D ) )
	{
		Parent->attach( Material_ );
	}
	
	if( CsCore::pImpl()->requestResource( "materials", "airsolid", Material ) && CsCore::pImpl()->createResource( BcName::INVALID, getPackage(), MaterialWorld_, Material, scnSPF_3D ) )
	{
		Parent->attach( MaterialWorld_ );
	}

	ScnFontRef Font;
	if( CsCore::pImpl()->requestResource( "fonts", "default", Font ) && 
		CsCore::pImpl()->requestResource( "materials", "font", Material ) &&
		CsCore::pImpl()->createResource( BcName::INVALID, getPackage(), Font_, Font, Material ) )
	{
		FontScaleParam_ = Font_->getMaterialComponent()->findParameter( "aAlphaTestStep" );
		Parent->attach( Font_ );
	}

	// Ok setup player.
	// TODO: Move to a game state component.
	// Attach player to world.
	PlayerEntity_ = ScnCore::pImpl()->createEntity( "default", "PlayerEntity", "PlayerEntity_0" );
	Parent->attach( PlayerEntity_ );
	
	Super::onAttach( Parent );

	// Load in level stuff.
	loadJson();
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaWorldBSPComponent::onDetach( ScnEntityWeakRef Parent )
{
	OsCore::pImpl()->unsubscribeAll( this );
	
	Canvas_ = NULL;

	if( PlayerEntity_.isValid() )
	{
		getParentEntity()->detach( PlayerEntity_ );
		PlayerEntity_ = NULL;
	}

	Parent->detach( Font_ );
	Font_ = NULL;
	
	destroyEntities();
	
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onMouseEvent
eEvtReturn GaWorldBSPComponent::onKeyboardEvent( EvtID ID, const OsEventInputKeyboard& Event )
{
	switch( Event.AsciiCode_ )
	{
	case 0x9:
		{
			InEditorMode_ = !InEditorMode_ && IsEditor_;

			if( !InEditorMode_ )
			{
				createEntities();
			}
			else
			{
				destroyEntities();
			}
		}
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
		}
		break;

	case 'E':
	case 'e':
		if( InEditorMode_ )
		{
			addEnemy( MousePointPosition_ );			
		}
		break;

	case 'S':
	case 's':
		if( InEditorMode_ )
		{
			StartPosition_ = MousePointPosition_;
		}
		break;

	case 'Q':
	case 'q':
		if( InEditorMode_ )
		{
			QuitPosition_ = MousePointPosition_;
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
		BcU32 PointIdx = nearestPoint( Position, 0.5f );
		BcU32 EdgeIdx = nearestEdge( Position, 0.5f );
		BcU32 EnemyIdx = nearestEnemy( Position, 0.5f );

		// Cache for UI use.
		NearestPoint_ = PointIdx;
		NearestEdge_ = EdgeIdx;
		NearestEnemy_ = EnemyIdx;
		
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
							else if( NearestEnemy_ != BcErrorCode )
							{
								removeEnemy( NearestEnemy_ );
								NearestEnemy_ = BcErrorCode;
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
BcU32 GaWorldBSPComponent::nearestPoint( BcVec2d Position, BcF32 Radius )
{
	// Round up the position to the nearest int.
	Position = RoundVector( Position );

	// Find.
	BcU32 FoundIdx = BcErrorCode;
	BcF32 Closest = Radius;

	for( BcU32 Idx = 0; Idx < Points_.size(); ++Idx )
	{
		GaWorldBSPPoint& Point( Points_[ Idx ] );
		BcF32 Distance = ( Point.Position_ - Position ).magnitude();

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
// addEnemy
BcBool GaWorldBSPComponent::addEnemy( const BcVec2d& Position )
{
	// Check it doesn't exist.
	for( BcU32 Idx = 0; Idx < Enemies_.size(); ++Idx )
	{
		if( ( Enemies_[ Idx ] - Position ).magnitudeSquared() < 2.0f )
		{
			return BcFalse;
		}
	}
	
	// Add!
	Enemies_.push_back( Position );

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
// removeEnemy
void GaWorldBSPComponent::removeEnemy( BcU32 Idx )
{
	// Remove enemy at index.
	BcU32 EnemyIdx = 0;
	for( std::vector< BcVec2d >::iterator It( Enemies_.begin() ); It != Enemies_.end(); ++EnemyIdx, ++It )
	{
		if( EnemyIdx == Idx )
		{
			Enemies_.erase( It );
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
BcU32 GaWorldBSPComponent::nearestEdge( const BcVec2d& Position, BcF32 Radius )
{
	BcU32 NearestIdx = BcErrorCode;
	BcF32 NearestDistance = Radius;

	// Check edge doesn't exist already.
	for( BcU32 Idx = 0; Idx < Edges_.size(); ++Idx )
	{
		const BcVec2d& NearestPosition( nearestPositionOnEdge( Position, Idx ) );

		BcF32 Distance = ( NearestPosition - Position ).magnitude();
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
	BcF32 AB2 = AB.dot( AB );
	BcF32 APAB = AP.dot( AB );
	BcF32 T = BcClamp( APAB / AB2, 0.0f, 1.0f );
	return ( PointA + AB * T );
}

//////////////////////////////////////////////////////////////////////////
// nearestEnemy
BcU32 GaWorldBSPComponent::nearestEnemy( BcVec2d Position, BcF32 Radius )
{
	BcU32 NearestIdx = BcErrorCode;
	BcF32 NearestDistance = Radius;

	// Check edge doesn't exist already.
	for( BcU32 Idx = 0; Idx < Enemies_.size(); ++Idx )
	{
		const BcVec2d& NearestPosition( Enemies_[ Idx ] );

		BcF32 Distance = ( NearestPosition - Position ).magnitude();
		if( Distance < NearestDistance )
		{
			NearestIdx = Idx;
			NearestDistance = Distance;
		}
	}

	return NearestIdx;
}

//////////////////////////////////////////////////////////////////////////
// killEnemy
BcBool GaWorldBSPComponent::killEnemy( const BcVec3d& Position, BcF32 Radius )
{
	BcF32 NearestDistanceSquared = Radius * Radius;
	std::vector< ScnEntityRef >::iterator NearestIt = EnemyEntities_.end();
	for( std::vector< ScnEntityRef >::iterator It = EnemyEntities_.begin(); It != EnemyEntities_.end(); ++It )
	{
		BcF32 DistanceSquared = ( (*It)->getPosition() - Position ).magnitudeSquared();
		if( DistanceSquared < NearestDistanceSquared )
		{
			NearestDistanceSquared = DistanceSquared;
			NearestIt = It;
		}
	}

	if( NearestIt != EnemyEntities_.end() )
	{
		ScnEntityRef Entity = (*NearestIt);
		EnemyEntities_.erase( NearestIt );
		getParentEntity()->detach( Entity );
		BcPrintf( "Killed enemy!\n" );
		return BcTrue;
	}

	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// killPlayer
void GaWorldBSPComponent::killPlayer()
{
	BcPrintf( "Kill player :(!\n" );
	loadJson();
}

//////////////////////////////////////////////////////////////////////////
// canSeePlayer
BcBool GaWorldBSPComponent::canSeePlayer( const BcVec3d& From )
{
	BcBSPPointInfo BSPPointInfo;
	if( !lineIntersection( From, PlayerEntity_->getPosition(), &BSPPointInfo ) )
	{
		return BcTrue;
	}

	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// saveJson
void GaWorldBSPComponent::saveJson()
{
	BcChar Buffer[ 256 ];
	Json::Value LevelData;
	Json::Value PointsData( Json::arrayValue );
	Json::Value EdgesData( Json::arrayValue );
	Json::Value EnemiesData( Json::arrayValue );
	Json::Value StartData( Json::stringValue );
	Json::Value QuitData( Json::stringValue );
	
	PointsData.resize( Points_.size() );
	for( BcU32 Idx = 0; Idx < Points_.size(); ++Idx )
	{
		const GaWorldBSPPoint& Point( Points_[ Idx ] );
		BcSPrintf(Buffer, "%.2f,%.2f", Point.Position_.x(), Point.Position_.y() );
		PointsData[ Idx ] = Buffer;
	}

	EdgesData.resize( Edges_.size() );
	for( BcU32 Idx = 0; Idx < Edges_.size(); ++Idx )
	{
		const GaWorldBSPEdge& Edge( Edges_[ Idx ] );
		BcSPrintf(Buffer, "%u,%u", Edge.A_, Edge.B_ );
		EdgesData[ Idx ] = Buffer;
	}

	EnemiesData.resize( Enemies_.size() );
	for( BcU32 Idx = 0; Idx < Enemies_.size(); ++Idx )
	{
		const BcVec2d& Enemy( Enemies_[ Idx ] );
		BcSPrintf(Buffer, "%.2f,%.2f", Enemy.x(), Enemy.y() );
		EnemiesData[ Idx ] = Buffer;
	}

	BcSPrintf(Buffer, "%.2f,%.2f", StartPosition_.x(), StartPosition_.y() );
	StartData = Buffer;
	BcSPrintf(Buffer, "%.2f,%.2f", QuitPosition_.x(), QuitPosition_.y() );
	QuitData = Buffer;

	LevelData["points"] = PointsData;
	LevelData["edges"] = EdgesData;
	LevelData["enemies"] = EnemiesData;
	LevelData["start"] = StartData;
	LevelData["quit"] = QuitData;

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
	Json::Value EnemiesData( Json::arrayValue );
	Json::Value StartData;
	Json::Value QuitData;

	// Hacky reset of state.
	EditorState_ = ES_IDLE;
	LastPointIdx_ = BcErrorCode;
	NearestPoint_ = BcErrorCode;
	NearestEdge_ = BcErrorCode;
	NearestEnemy_ = BcErrorCode;

	// Clear out old level.
	Points_.clear();
	Edges_.clear();
	Enemies_.clear();

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
			BcF32 X, Y;
			PointsData = LevelData[ "points" ];
			EdgesData = LevelData[ "edges" ];
			EnemiesData = LevelData[ "enemies" ];
			StartData = LevelData[ "start" ];
			QuitData = LevelData[ "quit" ];

			for( BcU32 Idx = 0; Idx < PointsData.size(); ++Idx )
			{
				Json::Value& PointData( PointsData[ Idx ] );
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

			for( BcU32 Idx = 0; Idx < EnemiesData.size(); ++Idx )
			{
				Json::Value& EnemyData( EnemiesData[ Idx ] );
				BcSScanf( EnemyData.asCString(), "%f,%f", &X, &Y );

				BcVec2d Enemy = BcVec2d( X, Y );
				Enemies_.push_back( Enemy );
			}

			if( StartData.type() == Json::stringValue )
			{
				BcSScanf( StartData.asCString(), "%f,%f", &X, &Y );
				StartPosition_ = BcVec2d( X, Y );
			}
		
			if( QuitData.type() == Json::stringValue )
			{
				BcSScanf( QuitData.asCString(), "%f,%f", &X, &Y );
				QuitPosition_ = BcVec2d( X, Y );
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

	// HACK.
	if( PlayerEntity_.isValid() )
	{
		GaPawnComponentRef Pawn = PlayerEntity_->getComponentByType< GaPawnComponent >( 0 );
		if( Pawn.isValid() )
		{
			Pawn->setPosition( BcVec3d( StartPosition_.x(), StartPosition_.y(), 0.0f ) );
		}
	}

	//
	buildBSP();

	// If not in editor mode, create entities!
	if(!InEditorMode_)
	{
		createEntities();
	}

	// Reset.
	GaWorldResetEvent Event;

	// Do level context.
	switch( CurrentLevel_ )
	{
	case 1:
		clearMessages();
		addMessage( "Resense - By NeiloGD for 7DFPS" );
		addMessage( "" );
		addMessage( "\"W...what in the hell?\"" );
		addMessage( "\"My head...why does it hurt so much?\"" );
		addMessage( "\"Have my implants malfunctioned?\"" );
		addMessage( "\"I must make my way to the lab...\"" );
		addMessage( "\"The elevator...I remember a circular room\"" );
		addMessage( "\"I need to sense my way...\" [Use Ctrl Key]" );
		break;

	case 2:
		clearMessages();
		addMessage( "\"Ok.\"" );
		addMessage( "\"One step at a time...\"" );
		addMessage( "\"Can I remember?\"" );
		addMessage( "\"The elevator is just around the corner.\"" );
		break;

	case 3:
		clearMessages();
		addMessage( "\"Something has happened here.\"" );
		addMessage( "\"I sense I am the only one in the building.\"" );
		addMessage( "\"I need to find out what...\"" );
		break;

	case 4:
		clearMessages();
		addMessage( "\"*sigh*\"" );
		addMessage( "\"The security system is active.\"" );
		addMessage( "\"I must be careful, the bots may attack me.\"" );
		addMessage( "\"I should still have my gun...\" [LMB/RMB to shoot]" );
		break;

	case 5:
		clearMessages();
		addMessage( "\"It just keeps getting worse.\"" );
		addMessage( "\"This really isn't my day.\"" );
		break;

	case 6:
		clearMessages();
		addMessage( "\"Several more bots...\"" );
		addMessage( "\"I can do this...come on!\"" );
		break;

	case 7:
		clearMessages();
		addMessage( "To be continued..." );
		addMessage( "" );
		addMessage( "This is the end of the game :'(" );
		addMessage( "Ran short on time, tired, and feeling sick." );
		addMessage( "However, I do dare you to try kill all the bots!" );
		addMessage( "Thanks so much for playing..." );
		addMessage( "...if you got this far..." );
		addMessage( "...YOU ARE AWESOME ^_^ <3" );
		addMessage( "...and always a winner to me - NeiloGD" );
		break;

	default:
		break;
	}

	Event.Position_ = StartPosition_;
	Event.HasWeapon_ = CurrentLevel_ >= 4 ? BcTrue : BcFalse;
	getParentEntity()->publish( gaEVT_CORE_RESET, Event );

}

//////////////////////////////////////////////////////////////////////////
// createEntities
void GaWorldBSPComponent::createEntities()
{
	destroyEntities();
	
	//
	for( BcU32 Idx = 0; Idx < Enemies_.size(); ++Idx )
	{
		BcVec2d Enemy( Enemies_[ Idx ] );
		ScnEntityRef EnemyEntity = ScnCore::pImpl()->createEntity( "default", "EnemyEntity", "EnemyEntity" );
		EnemyEntity->setPosition( BcVec3d( Enemy.x(), Enemy.y(), 1.0f ) );
		EnemyEntities_.push_back( EnemyEntity );
		getParentEntity()->attach( EnemyEntity );
	}
}

//////////////////////////////////////////////////////////////////////////
// destroyEntities
void GaWorldBSPComponent::destroyEntities()
{
	// Clean up old entities.
	for( BcU32 Idx = 0; Idx < EnemyEntities_.size(); ++Idx )
	{
		ScnEntityRef EnemyEntity = EnemyEntities_[ Idx ];
		getParentEntity()->detach( EnemyEntity );
	}
	EnemyEntities_.clear();
}

//////////////////////////////////////////////////////////////////////////
// buildBSP
void GaWorldBSPComponent::buildBSP()
{
	delete pBSPTree_;
	pBSPTree_ = NULL;

	BcF32 Width = static_cast< BcF32 >( WorldInfo_->getWidth() );
	BcF32 Height = static_cast< BcF32 >( WorldInfo_->getHeight() );
	BcF32 Depth = static_cast< BcF32 >( WorldInfo_->getDepth() );

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
		BcU32 VertexDescriptor = rsVDF_POSITION_XYZ | rsVDF_TEXCOORD_UVW0 | rsVDF_COLOUR_ABGR8;

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

			BcVec3d Offset = BcVec3d( Width, Height, Depth ) * 0.5f;
			for( BcU32 Vert = 0; Vert < 6; ++Vert )
			{
				BcVec3d Vertex = Vertices[ Indices[ Vert ] ];
				BcVec3d Index = ( Vertex + Offset );
				pVertex->X_ = Vertex.x();
				pVertex->Y_ = Vertex.y();
				pVertex->Z_ = Vertex.z();
				pVertex->U_ = ( Index.x() / Width );
				pVertex->V_ = ( Index.y() / Height );
				pVertex->W_ = ( Index.z() / Depth );
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
BcBool GaWorldBSPComponent::checkPointFront( const BcVec3d& Point, BcF32 Radius, BcBSPInfo* pData, BcBSPNode* pNode )
{
	if( pBSPTree_ )
	{
		return pBSPTree_->checkPointFront( Point, Radius, pData, pNode );
	}
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// checkPointBack
BcBool GaWorldBSPComponent::checkPointBack( const BcVec3d& Point, BcF32 Radius, BcBSPInfo* pData, BcBSPNode* pNode )
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

//////////////////////////////////////////////////////////////////////////
// clearMessages
void GaWorldBSPComponent::clearMessages()
{
	if( TextList_.size() > 0 )
	{
		std::string CurrMessage = TextList_[ 0 ];
		TextList_.clear();
		TextList_.push_back( CurrMessage );
	}
}

//////////////////////////////////////////////////////////////////////////
// addMessage
void GaWorldBSPComponent::addMessage( const BcChar* pMessage )
{
	TextList_.push_back( pMessage );

}
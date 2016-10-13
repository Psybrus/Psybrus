#include "System/Scene/Rendering/ScnTileMap.h"
#include "System/Scene/Rendering/ScnCanvasComponent.h"
#include "System/Scene/ScnComponentProcessor.h"
#include "System/Scene/ScnEntity.h"
#include "System/Renderer/RsCore.h"

#include "System/Content/CsCore.h"
#include "System/Os/OsCore.h"

#include "Base/BcMath.h"

#ifdef PSY_IMPORT_PIPELINE
#include "System/Scene/Import/ScnTileMapImport.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnTileMap );

void ScnTileMap::StaticRegisterClass()
{
	auto& Class = ReRegisterClass< ScnTileMap, Super >();
	BcUnusedVar( Class );

#ifdef PSY_IMPORT_PIPELINE
	//// Add importer attribute to class for resource system to use.
	Class.addAttribute( new CsResourceImporterAttribute( 
		ScnTileMapImport::StaticGetClass(), 0 ) );
#endif
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnTileMap::ScnTileMap():
	TileMapData_( nullptr )
{
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
ScnTileMap::~ScnTileMap()
{
}

//////////////////////////////////////////////////////////////////////////
// findLayerIndexByName
BcU32 ScnTileMap::findLayerIndexByName( const BcChar* Name )
{
	for( BcU32 Idx = 0; Idx < TileMapData_->NoofLayers_; ++Idx )
	{
		auto& Layer = TileMapData_->Layers_[ Idx ];
		if( strcmp( Name, getString( Layer.Name_ ) ) == 0 )
		{
			return Idx;
		}
	}
	return BcErrorCode;
}

//////////////////////////////////////////////////////////////////////////
// isTileValid
bool ScnTileMap::isTileValid( BcU32 LayerIdx, BcS32 X, BcS32 Y ) const
{
	if( LayerIdx < TileMapData_->NoofLayers_ )
	{
		auto& Layer = TileMapData_->Layers_[ LayerIdx ];
		if( X >= 0 && X < (BcS32)Layer.Width_ && 
		    Y >= 0 && Y < (BcS32)Layer.Height_ )
		{
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
// getTile
const ScnTileMapTile* ScnTileMap::getTile( BcU32 LayerIdx, BcS32 X, BcS32 Y ) const
{
	BcAssert( isTileValid( LayerIdx, X, Y ) );
	auto& Layer = TileMapData_->Layers_[ LayerIdx ];
	BcU32 Index = X + ( Y * Layer.Width_ );
	return &Layer.Tiles_[ Index ];
}

//////////////////////////////////////////////////////////////////////////
// fileReady
void ScnTileMap::fileReady()
{
	// File is ready, get the header chunk.
	requestChunk( 0, TileMapData_ );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnTileMap::fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData )
{
	if( ChunkID == BcHash( "data" ) )
	{
		TileMapData_ = reinterpret_cast< ScnTileMapData* >( pData );

		markCreate();
		markReady();
	}
}

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnTileMapComponent );

void ScnTileMapComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "TileMap_", &ScnTileMapComponent::TileMap_, bcRFF_IMPORTER | bcRFF_SHALLOW_COPY ),

		new ReField( "Canvas_", &ScnTileMapComponent::Canvas_, bcRFF_TRANSIENT )
	};

	using namespace std::placeholders;
	ReRegisterClass< ScnTileMapComponent, Super >( Fields )
		.addAttribute( new ScnComponentProcessor( 
			{
				ScnComponentProcessFuncEntry(
					"Update",
					ScnComponentPriority::TILEMAP_UPDATE,
					std::bind( &ScnTileMapComponent::update, _1 ) ),
			} ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnTileMapComponent::ScnTileMapComponent():
	TileMap_( nullptr ),
	Canvas_( nullptr ),
	TileMaterials_(),
	TileTileSets_()
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnTileMapComponent::~ScnTileMapComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// getTileLocalPosition
const ScnTileMapTile* ScnTileMapComponent::getTileLocalPosition( 
		BcU32 LayerIdx, const MaVec2d& LocalPosition ) const
{	
	BcS32 X = (BcS32)LocalPosition.x();
	BcS32 Y = (BcS32)LocalPosition.y();
	if( TileMap_->isTileValid( LayerIdx, X, Y ) )
	{
		return TileMap_->getTile( LayerIdx, X, Y );
	}
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// getTileWorldPosition
const ScnTileMapTile* ScnTileMapComponent::getTileWorldPosition( 
		BcU32 LayerIdx, const MaVec2d& WorldPosition ) const
{	
	// Transform to appropriate position.
	MaMat4d Transform = getParentEntity()->getWorldMatrix();
	Transform.inverse();
	auto LocalPosition = WorldPosition * Transform; 
	return getTileLocalPosition( LayerIdx, LocalPosition );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
void ScnTileMapComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );

	// Find a canvas to use for rendering (someone in ours, or our parent's hierarchy).
	Canvas_ = Parent->getComponentAnyParentByType< ScnCanvasComponent >( 0 );
	BcAssertMsg( Canvas_ != nullptr, "Sprite component needs to be attached to an entity with a canvas component in any parent!" );

	// Add first null material + tileset.
	TileMaterials_.emplace_back( nullptr, ScnRect() );
	TileTileSets_.emplace_back( 0 );

	ScnTileMapData* TileMapData = TileMap_->TileMapData_;
	for( BcU32 MaterialIdx = 0; MaterialIdx < TileMapData->NoofTileSets_; ++MaterialIdx )
	{
		const auto& TileSet = TileMapData->TileSets_[ MaterialIdx ];
		BcAssert( TileSet.FirstGID_ == TileMaterials_.size() );
		BcAssert( TileSet.NoofImages_ == 1 );

		// Grab resources for tileset.
		ScnTextureRef Texture = TileMap_->getPackage()->getCrossRefResource( TileSet.Images_[ 0 ].TextureRef_ );
		ScnMaterialRef Material = TileMap_->getPackage()->getCrossRefResource( TileSet.Images_[ 0 ].MaterialRef_ );
		BcAssert( Texture.isValid() );
		BcAssert( Material.isValid() );

		// Attach a new material for this tileset.
		auto MaterialComponent = getParentEntity()->attach< ScnMaterialComponent >( 
			TileMap_->getString( TileSet.Name_ ), Material, 
			ScnShaderPermutationFlags::MESH_STATIC_2D );

		// Set texture on material and setup rects using tileset data.
		MaterialComponent->setTexture( 0, Texture );

		// Add rects.
		for( BcU32 RectIdx = 0; RectIdx < Texture->noofRects(); ++RectIdx )
		{
			TileMaterials_.emplace_back( MaterialComponent, Texture->getRect( RectIdx ) );
			TileTileSets_.emplace_back( MaterialIdx );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// onDetach
void ScnTileMapComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

	Canvas_ = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// draw
void ScnTileMapComponent::draw()
{
	ScnTileMapData* TileMapData = TileMap_->TileMapData_;

	const MaMat4d& Transform = getParentEntity()->getWorldMatrix();

	MaVec2d TileSize( (BcF32)TileMapData->TileWidth_, (BcF32)TileMapData->TileHeight_ );

	// Grab first vertex availible.
	auto* FirstVert = Canvas_->allocVertices( 0 );
	auto* Vert = FirstVert;
	BcU32 NoofVerts = 0;

	Canvas_->pushMatrix( Transform );
	for( BcU32 LayerIdx = 0; LayerIdx < TileMapData->NoofLayers_; ++LayerIdx )
	{
		auto& Layer = TileMapData->Layers_[ LayerIdx ];
		auto LayerSize = MaVec2d( (BcF32)Layer.Width_, (BcF32)Layer.Height_ );

		for( BcU32 Y = 0; Y < Layer.Height_; ++Y )
		{
			for( BcU32 X = 0; X < Layer.Width_; ++X )
			{
				auto& Tile = Layer.Tiles_[ X + ( Y * Layer.Width_ ) ];
				if( Tile.GID_ != 0 )
				{
					const auto& Material = TileMaterials_[ Tile.GID_ ];
					const auto& TileSet = TileMapData->TileSets_[ TileTileSets_[ Tile.GID_ ] ];

					MaVec2d TileSetTileSize( (BcF32)TileSet.TileWidth_, (BcF32)TileSet.TileHeight_ );
					MaVec2d PositionTL( 0.0f, 0.0f );
					MaVec2d PositionBR( 0.0f, 0.0f );

					switch( TileMapData->Orientation_ )
					{
					case ScnTileMapOrientation::ORTHOGONAL:
						{
							PositionTL = 
								MaVec2d( 
									static_cast< BcF32 >( X ) - LayerSize.x() * 0.5f, 
									static_cast< BcF32 >( Y ) - LayerSize.y() * 0.5f ) * TileSize;
							PositionBR = PositionTL + TileSetTileSize;
						}
						break;

					case ScnTileMapOrientation::HEXAGONAL:
						{
							BcF32 SideLengthX = 0.0f;//TileMapData->HexSideLength_;
							BcF32 SideLengthY = (BcF32)TileMapData->HexSideLength_;
							BcF32 SideOffsetX = ( TileSize.x() - SideLengthX ) / 2.0f;
							BcF32 SideOffsetY = ( TileSize.y() - SideLengthY ) / 2.0f;

							MaVec2d HexTileSize(
								TileSize.x() + SideLengthX,
								SideOffsetY + SideLengthY );

							if( ( Y & 1 ) == 1 )
							{
								PositionTL += MaVec2d( SideOffsetX + SideLengthX, 0.0f );
							}

							PositionTL += 
								MaVec2d( 
									static_cast< BcF32 >( X ) - LayerSize.x() * 0.5f, 
									static_cast< BcF32 >( Y ) - LayerSize.y() * 0.5f ) * HexTileSize;
							PositionBR = PositionTL + TileSetTileSize;
						}
						break;

					default:
						break;
					}

					PositionTL.x( floorf( PositionTL.x() ) );
					PositionTL.y( floorf( PositionTL.y() ) );
					PositionBR.x( floorf( PositionBR.x() ) );
					PositionBR.y( floorf( PositionBR.y() ) );

					Canvas_->setMaterialComponent( Material.first );
					const ScnRect& Rect = Material.second;
					BcU32 ABGR = RsColour( 1.0f, 1.0f, 1.0f, Layer.Opacity_ ).asRGBA();
					
					Vert->X_ = PositionTL.x();
					Vert->Y_ = PositionTL.y();
					Vert->Z_ = 0.0f;
					Vert->W_ = 1.0f;
					Vert->U_ = Rect.X_;
					Vert->V_ = Rect.Y_;
					Vert->RGBA_ = ABGR;
					Vert++;
					Vert->X_ = PositionBR.x();
					Vert->Y_ = PositionTL.y();
					Vert->Z_ = 0.0f;
					Vert->W_ = 1.0f;
					Vert->U_ = Rect.X_ + Rect.W_;
					Vert->V_ = Rect.Y_;
					Vert->RGBA_ = ABGR;
					Vert++;
					Vert->X_ = PositionTL.x();
					Vert->Y_ = PositionBR.y();
					Vert->Z_ = 0.0f;
					Vert->W_ = 1.0f;
					Vert->U_ = Rect.X_;
					Vert->V_ = Rect.Y_ + Rect.H_;
					Vert->RGBA_ = ABGR;
					Vert++;
					Vert->X_ = PositionTL.x();
					Vert->Y_ = PositionBR.y();
					Vert->Z_ = 0.0f;
					Vert->W_ = 1.0f;
					Vert->U_ = Rect.X_;
					Vert->V_ = Rect.Y_ + Rect.H_;
					Vert->RGBA_ = ABGR;
					Vert++;
					Vert->X_ = PositionBR.x();
					Vert->Y_ = PositionTL.y();
					Vert->Z_ = 0.0f;
					Vert->W_ = 1.0f;
					Vert->U_ = Rect.X_ + Rect.W_;
					Vert->V_ = Rect.Y_;
					Vert->RGBA_ = ABGR;
					Vert++;
					Vert->X_ = PositionBR.x();
					Vert->Y_ = PositionBR.y();
					Vert->Z_ = 0.0f;
					Vert->W_ = 1.0f;
					Vert->U_ = Rect.X_ + Rect.W_;
					Vert->V_ = Rect.Y_ + Rect.H_;
					Vert->RGBA_ = ABGR;
					Vert++;

					NoofVerts += 6;
				}
			}
		}

		if( NoofVerts > 0 )
		{
			Canvas_->addPrimitive( RsTopologyType::TRIANGLE_LIST, Vert - NoofVerts, NoofVerts, LayerIdx, BcTrue );

			// Allocate vertices we've used.
			Canvas_->allocVertices( NoofVerts );

			// Reset.
			NoofVerts = 0;
		}
	}

	Canvas_->popMatrix();

}

//////////////////////////////////////////////////////////////////////////
// update
//static
void ScnTileMapComponent::update( const ScnComponentList& Components )
{
	for( auto Component : Components )
	{
		BcAssert( Component->isTypeOf< ScnTileMapComponent >() );
		auto* TileMapComponent = static_cast< ScnTileMapComponent* >( Component.get() );

		TileMapComponent->draw();
	}
}

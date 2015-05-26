#pragma once

#include "System/Renderer/RsCore.h"
#include "System/Content/CsResource.h"

#include "System/Scene/ScnComponent.h"
#include "System/Scene/Rendering/ScnTileMapFileData.h"

//////////////////////////////////////////////////////////////////////////
// ScnTileMap
class ScnTileMap:
	public CsResource
{
public:
	REFLECTION_DECLARE_DERIVED( ScnTileMap, CsResource );
	
	ScnTileMap();
	virtual ~ScnTileMap();

	/**
	 * Find layer index by name.
	 */
	BcU32 findLayerIndexByName( const BcChar* Name );

	/**
	 * @return True if valid tile.
	 */
	bool isTileValid( BcU32 LayerIdx, BcS32 X, BcS32 Y ) const;
	
	/**
	 * Get tile.
	 * @param LayerIdx.
	 * @param X.
	 * @param Y.
	 */
	const ScnTileMapTile* getTile( BcU32 LayerIdx, BcS32 X, BcS32 Y ) const;

private:
	virtual void fileReady();
	virtual void fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData );

private:
	friend class ScnTileMapComponent;

	ScnTileMapData* TileMapData_;
};


//////////////////////////////////////////////////////////////////////////
// ScnTileMapComponent
class ScnTileMapComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( ScnTileMapComponent, ScnComponent );
	
	ScnTileMapComponent();
	virtual ~ScnTileMapComponent();

	/**
	 * Get tile using local position.
	 * @param LayerIdx Index of layer.
	 * @param Position Position local to this component's entity.
	 */
	const ScnTileMapTile* getTileLocalPosition( BcU32 LayerIdx, const MaVec2d& LocalPosition ) const;
	
	/**
	 * Get tile using world position.
	 * @param LayerIdx Index of layer.
	 * @param Position Position in world.
	 */
	const ScnTileMapTile* getTileWorldPosition( BcU32 LayerIdx, const MaVec2d& WorldPosition ) const;

private:
	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

private:
	void draw();

	static void update( const ScnComponentList& Components );

	std::string MaterialName_;
	class ScnTileMap* TileMap_;

	class ScnCanvasComponent* Canvas_;
	class ScnMaterialComponent* Material_;

	using MaterialRectPair = std::pair< ScnMaterialComponent*, ScnRect >;
	std::vector< MaterialRectPair > Materials_;
};


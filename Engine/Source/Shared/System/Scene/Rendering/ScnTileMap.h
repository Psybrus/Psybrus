#pragma once

#include "System/Renderer/RsCore.h"
#include "System/Content/CsResource.h"

#include "System/Scene/ScnTypes.h"
#include "System/Scene/Rendering/ScnTileMapFileData.h"

//////////////////////////////////////////////////////////////////////////
// ScnTileMapRef
typedef ReObjectRef< class ScnTileMap > ScnTileMapRef;
typedef std::vector< ScnTileMapRef > ScnTileMapList;
typedef ScnTileMapList::iterator ScnTileMapListIterator;
typedef ScnTileMapList::const_iterator ScnTileMapListConstIterator;
typedef std::map< BcName, ScnTileMapRef > ScnTileMapMap;
typedef ScnTileMapMap::iterator ScnTileMapMapIterator;
typedef ScnTileMapMap::const_iterator ScnTileMapMapConstIterator;

//////////////////////////////////////////////////////////////////////////
// ScnTileMap
class ScnTileMap:
	public CsResource
{
public:
	REFLECTION_DECLARE_DERIVED( ScnTileMap, CsResource );
	
	ScnTileMap();
	virtual ~ScnTileMap();
	
private:
	virtual void fileReady();
	virtual void fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData );

	ScnTileMapData* TileMapData_;
};


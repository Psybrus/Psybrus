#pragma once

#include "Base/BcTypes.h"
#include "Base/BcRelativePtr.h"

//////////////////////////////////////////////////////////////////////////
// ScnTileMapOrientation
enum class ScnTileMapOrientation : BcU32
{
	ORTHOGONAL,
	ISOMETRIC,
	ISOMETRIC_STAGGERED,
	HEXAGONAL_STAGGERED
};

//////////////////////////////////////////////////////////////////////////
// ScnTileMapData
struct ScnTileMapData
{
	ScnTileMapOrientation Orientation_ = ScnTileMapOrientation::ORTHOGONAL;
	BcU32 Width_ = 0;
	BcU32 Height_ = 0;
	BcU32 TileWidth_ = 0;
	BcU32 TileHeight_ = 0;
	RsColour BackgroundColour_ = RsColour::BLACK;

	BcU32 NoofTileSets_ = 0;
	BcRelativePtrU32< struct ScnTileMapTileSet > TileSets_ = nullptr;

	BcU32 NoofLayers_ = 0;
	BcRelativePtrU32< struct ScnTileMapLayer > Layers_ = nullptr;

	BcU32 NoofObjectGroups_ = 0;
	BcRelativePtrU32< struct ScnTileMapLayer > ObjectGroups_ = nullptr;
};

//////////////////////////////////////////////////////////////////////////
// ScnTileMapTileSet
struct ScnTileMapTileSet
{
	BcU32 Name_ = BcErrorCode;
	BcU32 FirstGID_ = 0;
	BcU32 TileWidth_ = 0;
	BcU32 TileHeight_ = 0;

	BcU32 NoofImages_ = 0;
	BcRelativePtrU32< struct ScnTileMapTileSetImage > Images_ = nullptr;

	BcU32 NoofProperties_ = 0;
	BcRelativePtrU32< struct ScnTileMapProperty > Properties_ = nullptr;
};

//////////////////////////////////////////////////////////////////////////
// ScnTileMapTileSetImage
struct ScnTileMapTileSetImage
{
	BcU32 Width_ = 0;
	BcU32 Height_ = 0;
};

//////////////////////////////////////////////////////////////////////////
// ScnTileMapLayer
struct ScnTileMapLayer
{
	BcU32 Name_ = BcErrorCode;
	BcU32 Width_ = 0;
	BcU32 Height_ = 0;
	BcF32 Opacity_ = 1.0f;

	BcRelativePtrU32< struct ScnTileMapTile > Tiles_ = nullptr;

	BcU32 NoofProperties_ = 0;
	BcRelativePtrU32< struct ScnTileMapProperty > Properties_ = nullptr;
};

//////////////////////////////////////////////////////////////////////////
// ScnTileMapTile
struct ScnTileMapTile
{
	BcU32 GID_ = 0;
};

//////////////////////////////////////////////////////////////////////////
// ScnTileMapObjectGroup
struct ScnTileMapObjectGroup
{
	BcU32 Name_ = BcErrorCode;
	BcU32 Width_ = 0;
	BcU32 Height_ = 0;

	BcU32 NoofObjects_ = 0;
};

//////////////////////////////////////////////////////////////////////////
// ScnTileMapObject
struct ScnTileMapObject
{
	BcU32 Name_ = BcErrorCode;
	BcU32 Type_ = BcErrorCode;
	BcU32 GID_= 0;
	BcF32 X_;
	BcF32 Y_;

	BcU32 NoofProperties_ = 0;
	BcRelativePtrU32< struct ScnTileMapProperty > Properties_ = nullptr;
};

//////////////////////////////////////////////////////////////////////////
// ScnTileMapProperty
struct ScnTileMapProperty
{
	BcU32 Name_ = BcErrorCode;
	BcU32 Value_ = BcErrorCode;
};

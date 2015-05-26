#pragma once

#ifndef __ScnTileMapImport_H__
#define __ScnTileMapImport_H__

#include "System/Content/CsCore.h"
#include "System/Content/CsResourceImporter.h"
#include "System/Renderer/RsTypes.h"

#include "Base/BcStream.h"

#include "System/Scene/Rendering/ScnTileMapFileData.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations.
namespace rapidxml
{
    // Forward declarations
    template<class Ch> class xml_node;
    template<class Ch> class xml_attribute;
    template<class Ch> class xml_document;
}

//////////////////////////////////////////////////////////////////////////
// ScnTileMapImport
class ScnTileMapImport:
	public CsResourceImporter
{
public:
	REFLECTION_DECLARE_DERIVED_MANUAL_NOINIT( ScnTileMapImport, CsResourceImporter );

public:
	ScnTileMapImport();
	ScnTileMapImport( ReNoInit );
	virtual ~ScnTileMapImport();

	/**
	 * Import.
	 */
	BcBool import( const Json::Value& );

private:
	void parseMap( 
		class BcStream& Stream, 
		rapidxml::xml_node<char>& Node, 
		BcStream::Object< ScnTileMapData > Header );

	void parseTileSet( 
		class BcStream& Stream, 
		rapidxml::xml_node<char>& Node, 
		BcStream::Object< ScnTileMapTileSet > TileSet );
	
	void parseLayer( 
		class BcStream& Stream, 
		rapidxml::xml_node<char>& Node, 
		BcStream::Object< ScnTileMapLayer >Layer );
	
	void parseLayerTile( 
		class BcStream& Stream, 
		rapidxml::xml_node<char>& Node, 
		BcStream::Object< ScnTileMapTile > Tile );

	void parseProperty( 
		class BcStream& Stream, 
		rapidxml::xml_node<char>& Node, 
		BcStream::Object< ScnTileMapProperty > Property );

private:
	std::string Source_;
};

#endif

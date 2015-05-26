/**************************************************************************
*
* File:		ScnTileMapImport.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Import/ScnTileMapImport.h"

#if PSY_IMPORT_PIPELINE

#include "System/Renderer/RsTypes.h"

#include "Base/BcFile.h"
#include "Base/BcMath.h"

#include <rapidxml.hpp>

#endif // PSY_IMPORT_PIPELINE

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( ScnTileMapImport )
	
void ScnTileMapImport::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Source_", &ScnTileMapImport::Source_, bcRFF_IMPORTER ),
	};
		
	ReRegisterClass< ScnTileMapImport, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnTileMapImport::ScnTileMapImport():
	Source_()
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnTileMapImport::ScnTileMapImport( ReNoInit ):
	Source_()
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnTileMapImport::~ScnTileMapImport()
{

}

//////////////////////////////////////////////////////////////////////////
// import
BcBool ScnTileMapImport::import( const Json::Value& )
{
#if PSY_IMPORT_PIPELINE

	BcStream Stream;

	CsResourceImporter:addDependency( Source_.c_str() );

	auto Header = Stream.alloc< ScnTileMapData >();
	BcFile SourceFile;
	if( SourceFile.open( Source_.c_str(), bcFM_READ ) )
	{
		auto SourceText = SourceFile.readAllBytes();
		rapidxml::xml_document<> SourceDoc;
		SourceDoc.parse< 0 >( reinterpret_cast< char* >( SourceText.get() ) );
		parseMap( Stream, *SourceDoc.first_node(), Header );

		CsResourceImporter::addChunk( BcHash( "data" ), Stream.pData(), Stream.dataSize() );
		return BcTrue;
	}

#endif // PSY_IMPORT_PIPELINE
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// parseMap
void ScnTileMapImport::parseMap( 
		class BcStream& Stream, 
		rapidxml::xml_node<char>& Node, 
		BcStream::Object< ScnTileMapData > Header )
{
	BcAssertMsg( std::string( "map" ) == Node.name(), "Node is not a map node." );

	// Parse attributes.
	auto* ChildAttrib = Node.first_attribute();
	while( ChildAttrib != nullptr )
	{
		if( std::string( "version" ) == ChildAttrib->name() )
		{
			BcAssert( std::string( "1.0" ) == ChildAttrib->value() );
		}
		else if( std::string( "orientation" ) == ChildAttrib->name() )
		{
			if( std::string( "orthogonal" ) == ChildAttrib->value() )
			{
				Header->Orientation_ = ScnTileMapOrientation::ORTHOGONAL;
			}
			else
			{
				// Unsupported.
				BcAssert( false );
			}
		}
		else if( std::string( "width" ) == ChildAttrib->name() )
		{
			Header->Width_ = std::stol( ChildAttrib->value(), 0 );
		}
		else if( std::string( "height" ) == ChildAttrib->name() )
		{
			Header->Height_ = std::stol( ChildAttrib->value(), 0 );
		}
		else if( std::string( "tilewidth" ) == ChildAttrib->name() )
		{
			Header->TileWidth_ = std::stol( ChildAttrib->value(), 0 );
		}
		else if( std::string( "tileheight" ) == ChildAttrib->name() )
		{
			Header->TileHeight_ = std::stol( ChildAttrib->value(), 0 );
		}
		else if( std::string( "backgroundcolor" ) == ChildAttrib->name() )
		{
			Header->BackgroundColour_ = RsColour::FromHTMLColourCode( ChildAttrib->value() );
		}
		ChildAttrib = ChildAttrib->next_attribute();
	}

	// Count nodes.
	auto* ChildNode = Node.first_node();
	while( ChildNode != nullptr )
	{
		if( std::string( "tileset" ) == ChildNode->name() )
		{
			Header->NoofTileSets_++;
		}
		else if( std::string( "layer" ) == ChildNode->name() )
		{
			Header->NoofLayers_++;
		}
		ChildNode = ChildNode->next_sibling();
	}

	// Allocate tilesets + layers.
	auto TileSetsPtr = Stream.alloc< ScnTileMapTileSet >( Header->NoofTileSets_ );
	auto LayersPtr = Stream.alloc< ScnTileMapLayer >( Header->NoofLayers_ );
	Header->TileSets_.reset( TileSetsPtr.get() );
	Header->Layers_.reset( LayersPtr.get() ) ;

	// Parse nodes.
	ChildNode = Node.first_node();
	BcU32 TileSetIdx = 0;
	BcU32 LayerIdx = 0;
	while( ChildNode != nullptr )
	{
		if( std::string( "tileset" ) == ChildNode->name() )
		{
			parseTileSet( Stream, *ChildNode, Stream.get( &Header->TileSets_[ TileSetIdx++ ] ) );
		}
		else if( std::string( "layer" ) == ChildNode->name() )
		{
			parseLayer( Stream, *ChildNode, Stream.get( &Header->Layers_[ LayerIdx++ ] ) );
		}

		ChildNode = ChildNode->next_sibling();
	}
}

//////////////////////////////////////////////////////////////////////////
// parseTileSet
void ScnTileMapImport::parseTileSet( 
		class BcStream& Stream, 
		rapidxml::xml_node<char>& Node, 
		BcStream::Object< ScnTileMapTileSet > TileSet )
{
	BcAssertMsg( std::string( "tileset" ) == Node.name(), "Node is not a tileset node." );

	// Parse attributes.
	auto* ChildAttrib = Node.first_attribute();
	while( ChildAttrib != nullptr )
	{
		if( std::string( "firstgid" ) == ChildAttrib->name() )
		{
			TileSet->FirstGID_ = std::stol( ChildAttrib->value() );
		}
		else if( std::string( "name" ) == ChildAttrib->name() )
		{
			TileSet->Name_ = addString( ChildAttrib->value() );
		}
		else if( std::string( "tilewidth" ) == ChildAttrib->name() )
		{
			TileSet->TileWidth_ = std::stol( ChildAttrib->value() );
		}
		else if( std::string( "tileheight" ) == ChildAttrib->name() )
		{
			TileSet->TileHeight_ = std::stol( ChildAttrib->value() );
		}
		ChildAttrib = ChildAttrib->next_attribute();
	}

	// Count nodes.
	auto* ChildNode = Node.first_node();
	while( ChildNode != nullptr )
	{
		if( std::string( "image" ) == ChildNode->name() )
		{
			TileSet->NoofImages_++;
		}
		ChildNode = ChildNode->next_sibling();
	}

	// Parse nodes.
	ChildNode = Node.first_node();
	BcU32 ImageIdx = 0;
	while( ChildNode != nullptr )
	{
		if( std::string( "image" ) == ChildNode->name() )
		{
			// TODO.
		}
		ChildNode = ChildNode->next_sibling();
	}

}

//////////////////////////////////////////////////////////////////////////
// parseLayer
void ScnTileMapImport::parseLayer( 
		class BcStream& Stream, 
		rapidxml::xml_node<char>& Node, 
		BcStream::Object< ScnTileMapLayer > Layer )
{
	BcAssertMsg( std::string( "layer" ) == Node.name(), "Node is not a layer node." );

	// Parse attributes.
	auto* ChildAttrib = Node.first_attribute();
	while( ChildAttrib != nullptr )
	{
		if( std::string( "name" ) == ChildAttrib->name() )
		{
			Layer->Name_ = addString( ChildAttrib->value() );
		}
		else if( std::string( "width" ) == ChildAttrib->name() )
		{
			Layer->Width_ = std::stol( ChildAttrib->value() );
		}
		else if( std::string( "height" ) == ChildAttrib->name() )
		{
			Layer->Height_ = std::stol( ChildAttrib->value() );
		}
		else if( std::string( "opacity" ) == ChildAttrib->name() )
		{
			Layer->Opacity_ = std::stof( ChildAttrib->value() );
		}
		ChildAttrib = ChildAttrib->next_attribute();
	}

	// Allocate tiles.
	auto TilePtr = Stream.alloc< ScnTileMapTile >( Layer->Width_ * Layer->Height_ ); Layer->Tiles_.reset( TilePtr.get() );
	Layer->Tiles_.reset( TilePtr.get() );

	// Find data node.
	auto* DataNode = Node.first_node( "data" );
	BcAssertMsg( DataNode != nullptr, "Missing data node in layer." );

	// Parse nodes.
	auto* ChildNode = DataNode->first_node();
	BcU32 TileIdx = 0;
	while( ChildNode != nullptr )
	{
		if( std::string( "tile" ) == ChildNode->name() )
		{
			parseLayerTile( Stream, *ChildNode, Stream.get( &Layer->Tiles_[ TileIdx++ ] ) );
		}
		ChildNode = ChildNode->next_sibling();
	}

}

///////////////////////////////////////////////////////////
///////////////
// parseLayerTile
void ScnTileMapImport::parseLayerTile( 
		class BcStream& Stream, 
		rapidxml::xml_node<char>& Node, 
		BcStream::Object< ScnTileMapTile > Tile )
{
	BcAssertMsg( std::string( "tile" ) == Node.name(), "Node is not a tile node." );

	// Parse attributes.
	auto* ChildAttrib = Node.first_attribute();
	while( ChildAttrib != nullptr )
	{
		if( std::string( "gid" ) == ChildAttrib->name() )
		{
			Tile->GID_ = std::stol( ChildAttrib->value() );
		}
		ChildAttrib = ChildAttrib->next_attribute();
	}
}

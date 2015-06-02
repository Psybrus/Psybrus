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
#include "System/Scene/Import/ScnTextureImport.h"

#if PSY_IMPORT_PIPELINE

#include "System/Renderer/RsTypes.h"

#include "Base/BcCompression.h"
#include "Base/BcFile.h"
#include "Base/BcMath.h"

#include <boost/filesystem/path.hpp>
#include <regex>

#include <rapidxml.hpp>

extern "C"
{
	#include "b64/cdecode.h"
}

#endif // PSY_IMPORT_PIPELINE

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( ScnTileMapImport )
	
void ScnTileMapImport::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Source_", &ScnTileMapImport::Source_, bcRFF_IMPORTER ),
		new ReField( "Textures_", &ScnTileMapImport::Textures_, bcRFF_IMPORTER ),
		new ReField( "Materials_", &ScnTileMapImport::Materials_, bcRFF_IMPORTER ),
	};
	
	ReRegisterClass< ScnTileMapImport, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnTileMapImport::ScnTileMapImport():
	Source_(),
	Textures_(),
	Materials_()
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnTileMapImport::ScnTileMapImport( ReNoInit ):
	Source_(),
	Textures_(),
	Materials_()
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
	BcBool CanImport = BcTrue;
	BcStream Stream;

	CsResourceImporter:addDependency( Source_.c_str() );

	if( Source_.empty() )
	{
		CsResourceImporter::addMessage( CsMessageCategory::CRITICAL, "Missing 'source' field." );
		CanImport = BcFalse;
	}

	if( Materials_.empty() )
	{
		CsResourceImporter::addMessage( CsMessageCategory::CRITICAL, "Missing 'materials' list." );
		CanImport = BcFalse;
	}

	if( CanImport)
	{
		TileMapData_ = Stream.alloc< ScnTileMapData >();
		BcFile SourceFile;
		if( SourceFile.open( Source_.c_str(), bcFM_READ ) )
		{
			auto SourceText = SourceFile.readAllBytes();
			rapidxml::xml_document<> SourceDoc;
			SourceDoc.parse< 0 >( reinterpret_cast< char* >( SourceText.get() ) );
			parseMap( Stream, *SourceDoc.first_node(), TileMapData_ );

			CsResourceImporter::addChunk( BcHash( "data" ), Stream.pData(), Stream.dataSize() );
			return BcTrue;
		}
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
#if PSY_IMPORT_PIPELINE
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
			else if( std::string( "isometric" ) == ChildAttrib->value() )
			{
				Header->Orientation_ = ScnTileMapOrientation::ISOMETRIC;
			}
			else if( std::string( "hexagonal" ) == ChildAttrib->value() )
			{
				Header->Orientation_ = ScnTileMapOrientation::HEXAGONAL;
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
		else if( std::string( "hexsidelength" ) == ChildAttrib->name() )
		{
			Header->HexSideLength_ = std::stol( ChildAttrib->value() );
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
#endif // PSY_IMPORT_PIPELINE
}

//////////////////////////////////////////////////////////////////////////
// parseTileSet
void ScnTileMapImport::parseTileSet( 
		class BcStream& Stream, 
		rapidxml::xml_node<char>& Node, 
		BcStream::Object< ScnTileMapTileSet > TileSet )
{
#if PSY_IMPORT_PIPELINE
	BcAssertMsg( std::string( "tileset" ) == Node.name(), "Node is not a tileset node." );

	// Parse attributes.
	const char* TileSetName = nullptr;
	auto* ChildAttrib = Node.first_attribute();
	while( ChildAttrib != nullptr )
	{
		if( std::string( "firstgid" ) == ChildAttrib->name() )
		{
			TileSet->FirstGID_ = std::stol( ChildAttrib->value() );
		}
		else if( std::string( "name" ) == ChildAttrib->name() )
		{
			TileSetName = ChildAttrib->value();
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
	BcAssert( TileSetName != nullptr );

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

	auto ImagesPtr = Stream.alloc< ScnTileMapTileSetImage >( TileSet->NoofImages_ );
	TileSet->Images_.reset( ImagesPtr.get() );

	// Parse nodes.
	ChildNode = Node.first_node();
	BcU32 ImageIdx = 0;
	while( ChildNode != nullptr )
	{
		if( std::string( "image" ) == ChildNode->name() )
		{
			parseImage( Stream, 
				*ChildNode, 
				TileSetName, 
				TileSet, 
				Stream.get( &TileSet->Images_[ ImageIdx++ ] ) );
		}
		ChildNode = ChildNode->next_sibling();
	}
#endif // PSY_IMPORT_PIPELINE
}

//////////////////////////////////////////////////////////////////////////
// parseImage
void ScnTileMapImport::parseImage( 
		class BcStream& Stream, 
		rapidxml::xml_node<char>& Node, 
		const char* TileSetName,
		BcStream::Object< ScnTileMapTileSet > TileSet, 
		BcStream::Object< ScnTileMapTileSetImage > Image )
{
#if PSY_IMPORT_PIPELINE
	BcAssertMsg( std::string( "image" ) == Node.name(), "Node is not an image node." );

	// Parse attributes.
	auto* ChildAttrib = Node.first_attribute();
	while( ChildAttrib != nullptr )
	{
		if( std::string( "source" ) == ChildAttrib->name() )
		{
			// Try by texture file name then tileset name.
			Image->TextureRef_ = findTexture( ChildAttrib->value() );
			if( Image->TextureRef_ == CSCROSSREFID_INVALID )
			{ 
				Image->TextureRef_ = findTexture( TileSetName );
			}

			if( Image->TextureRef_ == CSCROSSREFID_INVALID )
			{
				using namespace boost::filesystem;
				path TMXSourcePath = Source_;
				path TexSourcePath = TMXSourcePath.parent_path() / path( ChildAttrib->value() );

				// Create texture importer.
				auto TextureImporter = CsResourceImporterUPtr(
					new ScnTextureImport( 
						TexSourcePath.c_str(), "ScnTextureAtlas",
						TexSourcePath.c_str(), RsTextureFormat::R8G8B8A8,
						TileSet->TileWidth_,
						TileSet->TileHeight_ ) );

				Image->TextureRef_ = CsResourceImporter::addImport( std::move( TextureImporter ) );
			}

			// Try texture file name then tileset name.
			Image->MaterialRef_ = findMaterialMatch( ChildAttrib->value() );
			if( Image->MaterialRef_ == CSCROSSREFID_INVALID )
			{ 
				Image->MaterialRef_ = findMaterialMatch( TileSetName );
			}
		}
		else if( std::string( "width" ) == ChildAttrib->name() )
		{
			Image->Width_ = addString( ChildAttrib->value() );
		}
		else if( std::string( "height" ) == ChildAttrib->name() )
		{
			Image->Height_ = std::stol( ChildAttrib->value() );
		}
		ChildAttrib = ChildAttrib->next_attribute();
	}
#endif // PSY_IMPORT_PIPELINE
}

//////////////////////////////////////////////////////////////////////////
// parseLayer
void ScnTileMapImport::parseLayer( 
		class BcStream& Stream, 
		rapidxml::xml_node<char>& Node, 
		BcStream::Object< ScnTileMapLayer > Layer )
{
#if PSY_IMPORT_PIPELINE
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

	auto EncodingAttrib = DataNode->first_attribute( "encoding" );
	if( EncodingAttrib == nullptr )
	{
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
	else if( std::string( "base64" ) == EncodingAttrib->value() )
	{
		BcU32 NoofTiles = Layer->Width_ * Layer->Height_;

		// Decode data.
		size_t DataLength = strlen( DataNode->value() );
		size_t BytesRequired = ( DataLength * 3 / 4 );
		BcAssert( BytesRequired < std::numeric_limits< size_t >::max() );
		std::unique_ptr< BcU8[] > DecodedData( new BcU8[ BytesRequired ] );
		base64_decodestate DecodeState;
		base64_init_decodestate( &DecodeState );
		base64_decode_block( 
			(char*)DataNode->value(), 
			(const int)DataLength, 
			(char*)DecodedData.get(), 
			&DecodeState );

		// Check compression type.
		auto CompressionAttrib = DataNode->first_attribute( "compression" );
		if( CompressionAttrib != nullptr )
		{
			if( std::string( "zlib" ) == CompressionAttrib->value() )
			{
				auto Success = BcDecompressData( 
					DecodedData.get(), 
					DataLength, 
					reinterpret_cast< BcU8* >( &Layer->Tiles_[ 0 ] ),
					sizeof( ScnTileMapTile ) * NoofTiles );
				BcAssertMsg( Success, "Failed to decompress layer data." );
			}
			else
			{
				CsResourceImporter::addMessage( CsMessageCategory::ERROR, "Invalid compression for layer." );
			}
		}
		else
		{
			memcpy( &Layer->Tiles_[ 0 ], DecodedData.get(), sizeof( ScnTileMapTile ) * NoofTiles );
		}
	}
	else
	{
		CsResourceImporter::addMessage( CsMessageCategory::ERROR, "Invalid encoding for layer." );
	}
#endif // PSY_IMPORT_PIPELINE
}

//////////////////////////////////////////////////////////////////////////
// parseLayerTile
void ScnTileMapImport::parseLayerTile( 
		class BcStream& Stream, 
		rapidxml::xml_node<char>& Node, 
		BcStream::Object< ScnTileMapTile > Tile )
{
#if PSY_IMPORT_PIPELINE
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
#endif // PSY_IMPORT_PIPELINE
}

//////////////////////////////////////////////////////////////////////////
// parseProperty
void ScnTileMapImport::parseProperty( 
		class BcStream& Stream, 
		rapidxml::xml_node<char>& Node, 
		BcStream::Object< ScnTileMapProperty > Property )
{
#if PSY_IMPORT_PIPELINE
	// Parse attributes.
	auto* ChildAttrib = Node.first_attribute();
	while( ChildAttrib != nullptr )
	{
		if( std::string( "name" ) == ChildAttrib->name() )
		{
			Property->Name_ = addString( ChildAttrib->value() );
		}
		else if( std::string( "value" ) == ChildAttrib->name() )
		{
			Property->Value_ = addString( ChildAttrib->value() );
		}
		ChildAttrib = ChildAttrib->next_attribute();
	}	
#endif // PSY_IMPORT_PIPELINE
}

//////////////////////////////////////////////////////////////////////////
// findTexture
CsCrossRefId ScnTileMapImport::findTexture( const std::string& TextureName )
{
	CsCrossRefId RetVal = CSCROSSREFID_INVALID;
#if PSY_IMPORT_PIPELINE
	for( const auto& TextureEntry : Textures_ )
	{
		if( std::regex_match( TextureName, std::regex( TextureEntry.first ) ) )
		{
			RetVal = TextureEntry.second;
		}
	}
#endif // PSY_IMPORT_PIPELINE
	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// findMaterialMatch
CsCrossRefId ScnTileMapImport::findMaterialMatch( const std::string& Path )
{
	CsCrossRefId RetVal = CSCROSSREFID_INVALID;
#if PSY_IMPORT_PIPELINE
	for( const auto& MaterialEntry : Materials_ )
	{
		if( std::regex_match( Path, std::regex( MaterialEntry.first ) ) )
		{
			RetVal = MaterialEntry.second;
		}
	}

	// Can't find match? Throw exception.
	if( RetVal == CSCROSSREFID_INVALID )
	{
		auto ErrorString = std::string( "Unable to find match for \"" ) + Path + std::string( "\"" );
		
		CsResourceImporter::addMessage( CsMessageCategory::ERROR, ErrorString );
	}
#endif // PSY_IMPORT_PIPELINE
	return RetVal;
}
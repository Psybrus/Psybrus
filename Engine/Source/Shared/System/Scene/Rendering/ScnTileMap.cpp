#include "System/Scene/Rendering/ScnTileMap.h"
#include "System/Renderer/RsCore.h"

#include "System/Content/CsCore.h"
#include "System/Os/OsCore.h"

#include "Base/BcMath.h"

#ifdef PSY_IMPORT_PIPELINE
#include "System/Scene/Import/ScnTileMapImport.h"
#endif

#include "System/Debug/DsImGuiFieldEditor.h"

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

	// Add editor.
	/*
	Class.addAttribute( 
		new DsImGuiFieldEditor( 
			[]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags )
			{
				ScnTileMap* Value = (ScnTileMap*)Object;
				if( Value != nullptr )
				{
					ImGui::Text( "Width: %u", Value->Width_ );
					ImGui::Text( "Height: %u", Value->Height_ );
					ImGui::Text( "Depth: %u", Value->Depth_ );
					ImGui::Text( "Format: TODO" );
					MaVec2d Size( Value->Width_, Value->Height_ );
					const auto WidthRequirement = 256.0f;
					Size *= WidthRequirement / Size.x();
					ImGui::Image( Value->getTexture(), Size );
				}
			} ) );
	*/
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnTileMap::ScnTileMap()
{
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
ScnTileMap::~ScnTileMap()
{

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


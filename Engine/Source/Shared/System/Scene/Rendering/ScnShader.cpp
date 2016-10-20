/**************************************************************************
*
* File:		ScnShader.cpp
* Author:	Neil Richardson 
* Ver/Date:	11/04/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Rendering/ScnShader.h"

#include "System/Content/CsCore.h"

#include "Base/BcMath.h"
#include "Base/BcFile.h"

#ifdef PSY_IMPORT_PIPELINE
#include "System/Scene/Import/ScnShaderImport.h"
#endif

#define LAZY_SHADER_CREATION ( 1 )

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnShader );

void ScnShader::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "pHeader_", &ScnShader::pHeader_, bcRFF_SHALLOW_COPY | bcRFF_CHUNK_DATA ),
		new ReField( "ShaderMappings_", &ScnShader::ShaderMappings_, bcRFF_TRANSIENT | bcRFF_CONST ),
		new ReField( "ProgramMap_", &ScnShader::ProgramMap_, bcRFF_TRANSIENT | bcRFF_CONST ),
		new ReField( "TargetCodeType_", &ScnShader::TargetCodeType_, bcRFF_CONST ),
		new ReField( "TotalProgramsLoaded_", &ScnShader::TotalProgramsLoaded_, bcRFF_CONST ),
	};
		
	auto& Class = ReRegisterClass< ScnShader, Super >( Fields );
	BcUnusedVar( Class );

	ReEnumConstant* ScnShaderPermutationFlagsEnumConstants[] = 
	{
		new ReEnumConstant( "RENDER_FORWARD", ScnShaderPermutationFlags::RENDER_FORWARD ),
		new ReEnumConstant( "RENDER_DEFERRED", ScnShaderPermutationFlags::RENDER_DEFERRED ),
		new ReEnumConstant( "RENDER_POST_PROCESS", ScnShaderPermutationFlags::RENDER_POST_PROCESS ),
		new ReEnumConstant( "PASS_SHADOW", ScnShaderPermutationFlags::PASS_SHADOW ),
		new ReEnumConstant( "PASS_DEPTH", ScnShaderPermutationFlags::PASS_DEPTH ),
		new ReEnumConstant( "PASS_MAIN", ScnShaderPermutationFlags::PASS_MAIN ),
		new ReEnumConstant( "MESH_STATIC_2D", ScnShaderPermutationFlags::MESH_STATIC_2D ),
		new ReEnumConstant( "MESH_STATIC_3D", ScnShaderPermutationFlags::MESH_STATIC_3D ),
		new ReEnumConstant( "MESH_SKINNED_3D", ScnShaderPermutationFlags::MESH_SKINNED_3D ),
		new ReEnumConstant( "MESH_PARTICLE_3D", ScnShaderPermutationFlags::MESH_PARTICLE_3D ),
		new ReEnumConstant( "MESH_INSTANCED_3D", ScnShaderPermutationFlags::MESH_INSTANCED_3D ),


	};
	ReRegisterEnum< ScnShaderPermutationFlags >( ScnShaderPermutationFlagsEnumConstants );

	

#ifdef PSY_IMPORT_PIPELINE
	// Add importer attribute to class for resource system to use.
	Class.addAttribute( new CsResourceImporterAttribute( 
		ScnShaderImport::StaticGetClass(), 1 ) );
#endif
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnShader::ScnShader():
	pHeader_ ( nullptr ),
	TotalProgramsLoaded_( 0 ),
	TargetCodeType_( RsShaderCodeType::INVALID )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnShader::~ScnShader()
{

}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnShader::create()
{
	// TODO: Move some of the fileChunk(...) code into here. Will do for now.
	markReady();
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnShader::destroy()
{
	for( auto& ProgramEntry : ProgramMap_ )
	{
		RsCore::pImpl()->destroyResource( ProgramEntry.second.Program_ );
	}

	for( auto& ShaderMapping : ShaderMappings_ )
	{
		for( auto& ShaderEntry : ShaderMapping.Shaders_ )
		{
			RsCore::pImpl()->destroyResource( ShaderEntry.second.Shader_ );
		}

		ShaderMapping.Shaders_.clear();
	}

	ProgramMap_.clear();
	pHeader_ = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// hasPermutation
bool ScnShader::hasPermutation( ScnShaderPermutationFlags PermutationFlags )
{
	for( auto It : ProgramMap_ )
	{
		if( BcContainsAllFlags( It.first, PermutationFlags ) )
		{
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
// getProgram
RsProgram* ScnShader::getProgram( ScnShaderPermutationFlags PermutationFlags )
{
	// Find best matching permutation.
	auto BestIter = ProgramMap_.find( PermutationFlags );
	
	if( BestIter == ProgramMap_.end() )
	{
		// Iterate over map and find best matching permutation (most matching bits).
		BcU32 BestFlagsSet = 0;
		for( auto Iter = ProgramMap_.begin(); Iter != ProgramMap_.end(); ++Iter )
		{
			BcU32 FlagsSet = BcBitsSet( (BcU32)(*Iter).first & (BcU32)PermutationFlags );
			if( FlagsSet > BestFlagsSet )
			{
				BestIter = Iter;
				BestFlagsSet = FlagsSet;
			}
		}
	}
	
	// We should have found one, must contain all requested permutations.
	if( BestIter != ProgramMap_.end() && BcContainsAllFlags( BestIter->first, PermutationFlags ) )
	{
		auto& ProgramData = BestIter->second;

#if LAZY_SHADER_CREATION
		// If we don't have program, create it.
		if( ProgramData.Program_ == nullptr )
		{
			createProgram( ProgramData );
		}
#else
		BcAssert( ProgramData.Program_ );
#endif

		return ProgramData.Program_;
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// getShader
ScnShaderComplexity ScnShader::getComplexity( RsShaderType ShaderType, ScnShaderPermutationFlags PermutationFlags )
{
	// Find best matching permutation.
	auto BestIter = ProgramMap_.find( PermutationFlags );
	
	if( BestIter == ProgramMap_.end() )
	{
		// Iterate over map and find best matching permutation (most matching bits).
		BcU32 BestFlagsSet = 0;
		for( auto Iter = ProgramMap_.begin(); Iter != ProgramMap_.end(); ++Iter )
		{
			BcU32 FlagsSet = BcBitsSet( (BcU32)(*Iter).first & (BcU32)PermutationFlags );
			if( FlagsSet > BestFlagsSet )
			{
				BestIter = Iter;
				BestFlagsSet = FlagsSet;
			}
		}
	}
	
	// We should have found one, must contain all requested permutations.
	if( BestIter != ProgramMap_.end() && BcContainsAllFlags( BestIter->first, PermutationFlags ) )
	{
		auto& ProgramData = BestIter->second;

		auto& Shaders = ShaderMappings_[ (BcU32)ShaderType ].Shaders_;
		auto ShaderHash = ProgramData.Header_->ShaderHashes_[ (BcU32)ShaderType ];
		auto ShaderIt = Shaders.find( ShaderHash );
		BcAssert( ShaderIt != Shaders.end() );

		return ShaderIt->second.Header_->Complexity_;
	}

	return ScnShaderComplexity();
}

//////////////////////////////////////////////////////////////////////////
// getShader
RsShader* ScnShader::getShader( BcU32 Hash, ScnShader::TShaderMap& ShaderMap )
{
	// Find best matching permutation.
	auto BestIter = ShaderMap.find( Hash );
		
	// We should have found one.
	if( BestIter != ShaderMap.end() )
	{
		auto& ShaderData = BestIter->second;
#if LAZY_SHADER_CREATION
		if( ShaderData.Shader_ == nullptr )
		{
			createShader( ShaderData );
		}
#else
		BcAssert( ShaderData.Shader_ );
#endif

		return ShaderData.Shader_;
	}
	
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// createProgram
void ScnShader::createProgram( ProgramData& ProgramData )
{
	BcAssert( ProgramData.Program_ == nullptr );

	std::vector< RsShader* > Shaders;
	Shaders.reserve( (size_t)RsShaderType::MAX );
		
	for( BcU32 Idx = 0; Idx < (BcU32)RsShaderType::MAX; ++Idx )
	{
		if( ProgramData.Header_->ShaderHashes_[ Idx ] != 0 )
		{
			auto& ShaderMapping( ShaderMappings_[ Idx ] );
			RsShader* pShader = getShader( ProgramData.Header_->ShaderHashes_[ Idx ], ShaderMapping.Shaders_ );
			BcAssertMsg( pShader != nullptr, "Shader for permutation %x is invalid in ScnShader %s\n", ProgramData.Header_->ProgramPermutationFlags_, getFullName().c_str() );
			Shaders.push_back( pShader );
		}
	}

	// Shrink to fit.
	Shaders.shrink_to_fit();

#if PSY_DEBUG
	if( Shaders.size() == 0)
	{
		PSY_LOG( "Unable to find shaders:" );
		for( BcU32 Idx = 0; Idx < (BcU32)RsShaderType::MAX; ++Idx )
		{
			PSY_LOG( " - Shader %u: %x", Idx, ProgramData.Header_->ShaderHashes_[ Idx ] != 0 );
		}
	}
#endif

	RsProgramVertexAttribute* VertexAttributes_ = (RsProgramVertexAttribute*)( ProgramData.Header_ + 1 );
	RsProgramVertexAttributeList VertexAttributes;
	VertexAttributes.reserve( ProgramData.Header_->NoofVertexAttributes_ );
	for( BcU32 Idx = 0; Idx < ProgramData.Header_->NoofVertexAttributes_; ++Idx )
	{
		VertexAttributes.push_back( VertexAttributes_[ Idx ] );
	}

	RsProgramParameter* Parameters_ = (RsProgramParameter*)( VertexAttributes_ + ProgramData.Header_->NoofVertexAttributes_ );
	RsProgramParameterList Parameters;
	Parameters.reserve( ProgramData.Header_->NoofParameters_ );
	for( BcU32 Idx = 0; Idx < ProgramData.Header_->NoofParameters_; ++Idx )
	{
		Parameters.push_back( Parameters_[ Idx ] );
	}

	// Create program.
#if PSY_DEBUG
	std::string Flags;
	ReManager::GetEnum( "ScnShaderPermutationFlags" )->getTypeSerialiser()->serialiseToString( &ProgramData.Header_->ProgramPermutationFlags_, Flags );
	PSY_LOG( "Creating program: %s %s (%x)", (*getName()).c_str(), Flags.c_str(), ProgramData.Header_->ProgramPermutationFlags_ );
#endif // PSY_DEBUG
	RsProgramUPtr Program = RsCore::pImpl()->createProgram( 
		std::move( Shaders ), 
		std::move( VertexAttributes ),
		std::move( Parameters ),
		getFullName().c_str() );			
	ProgramData.Program_ = Program.release();
}

//////////////////////////////////////////////////////////////////////////
// createShader
void ScnShader::createShader( ShaderData& ShaderData )
{	
	BcAssert( ShaderData.Shader_ == nullptr );

	// Check for file loading.
	BcU32* pFileLoadTag = (BcU32*)ShaderData.Data_;
	BcBool FreeShaderData = BcFalse;
	BcUnusedVar( pFileLoadTag );
	BcUnusedVar( FreeShaderData );
#if !defined( PSY_PRODUCTION )
	if( *pFileLoadTag == ScnShader::LOAD_FROM_FILE_TAG )
	{
		BcFile InputFile;
		auto Filename = (const char*)( pFileLoadTag + 1 );
		InputFile.open( Filename );
		ShaderData.Size_ = static_cast< BcU32 >( InputFile.size() );
		ShaderData.Data_ = InputFile.readAllBytes().release();
		FreeShaderData = BcTrue;
	}
#endif
	RsShaderUPtr Shader = RsCore::pImpl()->createShader(
		RsShaderDesc( ShaderData.Header_->ShaderType_, ShaderData.Header_->ShaderCodeType_ ), 
		ShaderData.Data_, ShaderData.Size_,
		getFullName().c_str() );
	ShaderData.Shader_ = Shader.release();
}

//////////////////////////////////////////////////////////////////////////
// fileReady
void ScnShader::fileReady()
{
	// File is ready, get the header chunk.
	requestChunk( 0 );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnShader::fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData )
{
	// If we have no render core get chunk 0 so we keep getting entered into.
	if( RsCore::pImpl() == nullptr )
	{
		requestChunk( 0 );
		return;
	}

	if( ChunkID == BcHash( "header" ) )
	{
		pHeader_ = (ScnShaderHeader*)pData;

		// Find out what target code type we should load.
		// Always load in maximum supported.
		RsContext* pContext = RsCore::pImpl()->getContext( nullptr );
		RsShaderCodeType* pCodeTypes = (RsShaderCodeType*)( pHeader_ + 1 );
		for( BcU32 Idx = 0; Idx < pHeader_->NoofShaderCodeTypes_; ++Idx )
		{
			if( pContext->isShaderCodeTypeSupported( pCodeTypes[ Idx ] ) )
			{
				TargetCodeType_ = pCodeTypes[ Idx ];
			}
		}

#if PSY_DEBUG
		if( TargetCodeType_ == RsShaderCodeType::INVALID )
		{
			PSY_LOG( "Supported shader types in %s:", getFullName().c_str() );
			const ReEnum* Enum = ReManager::GetEnum( "RsShaderCodeType" );
			for( BcU32 Idx = 0; Idx < pHeader_->NoofShaderCodeTypes_; ++Idx )
			{
				auto EnumConstant = Enum->getEnumConstant( static_cast< BcU32 >( pCodeTypes[ Idx ] ) );
				PSY_LOG( "- %s", (*EnumConstant->getName()).c_str() );
			}

		}

#endif
		if( TargetCodeType_ == RsShaderCodeType::INVALID )
		{
			PSY_LOG( "ERROR: No valid code type built in shader. Please one for \"%s\"", getFullName().c_str() );	
		}

		// Grab the rest of the chunks.
		const BcU32 TotalChunks = pHeader_->NoofProgramPermutations_ + pHeader_->NoofShaderPermutations_;
		for( BcU32 Idx = 0; Idx < TotalChunks; ++Idx )
		{
			requestChunk( ++ChunkIdx );
		}
	}
	else if( ChunkID == BcHash( "shader" ) )
	{
		ShaderData ShaderData;
		ShaderData.Header_ = (ScnShaderUnitHeader*)pData;
		ShaderData.Data_ = ShaderData.Header_ + 1;
		ShaderData.Size_ = getChunkSize( ChunkIdx ) - sizeof( ScnShaderUnitHeader );
		ShaderData.Shader_ = nullptr;

		if( ShaderData.Header_->ShaderCodeType_ == TargetCodeType_ )
		{
#if !LAZY_SHADER_CREATION
			createShader( ShaderData );
#endif
			ShaderMappings_[ (BcU32)ShaderData.Header_->ShaderType_ ].Shaders_[ ShaderData.Header_->ShaderHash_ ] = ShaderData;
		}
	}
	else if( ChunkID == BcHash( "program" ) )
	{
		ProgramData ProgramData;
		ProgramData.Header_ = (ScnShaderProgramHeader*)pData;
		ProgramData.Program_ = nullptr;
		++TotalProgramsLoaded_;

		if( ProgramData.Header_->ShaderCodeType_ == TargetCodeType_ )
		{
#if !LAZY_SHADER_CREATION
			createProgram( ProgramData );
#endif
			ProgramMap_[ ProgramData.Header_->ProgramPermutationFlags_ ] = ProgramData;
		}
	}

	// Mark ready if we've got all the programs we expect.
	if( TotalProgramsLoaded_ == pHeader_->NoofProgramPermutations_ )
	{
		markCreate();

		if( ProgramMap_.size() == 0 )
		{
			PSY_LOG( "WARNING: No programs created for $(ScnShader:%s.%s)",
				(*getPackage()->getName()).c_str(),
				(*getName()).c_str() );
		}
	}
}

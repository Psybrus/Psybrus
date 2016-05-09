/**************************************************************************
*
* File:		ScnShaderImport.cpp
* Author:	Neil Richardson 
* Ver/Date: 
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "ScnShaderImport.h"

#if PSY_IMPORT_PIPELINE

#include "System/SysKernel.h"

#include "Base/BcStream.h"
#include "Math/MaQuat.h"

#include "System/Renderer/RsTypes.h"

#define EXCLUDE_PSTDINT
#include <glsl/glsl_optimizer.h>

#if PLATFORM_WINDOWS
#pragma warning ( disable : 4512 ) // Can't generate assignment operator (for boost)
#endif

#include <bitset>
#include <regex>
#include <sstream>

#undef DOMAIN // This is defined somewhere in a core header.

namespace
{
	//////////////////////////////////////////////////////////////////////////
	// Shader permutations.
	static ScnShaderPermutationEntry GPermutationsRenderType[] = 
	{
		{ ScnShaderPermutationFlags::RENDER_FORWARD,			"PERM_RENDER_FORWARD",				"1" },
		{ ScnShaderPermutationFlags::RENDER_DEFERRED,			"PERM_RENDER_DEFERRED",				"1" },
		{ ScnShaderPermutationFlags::RENDER_FORWARD_PLUS,		"PERM_RENDER_FORWARD_PLUS",			"1" },
		{ ScnShaderPermutationFlags::RENDER_POST_PROCESS,		"PERM_RENDER_POST_PROCESS",			"1" },
	};

	static ScnShaderPermutationEntry GPermutationsPassType[] = 
	{
		{ ScnShaderPermutationFlags::PASS_SHADOW,				"PERM_PASS_SHADOW",					"1" },
		{ ScnShaderPermutationFlags::PASS_DEPTH,				"PERM_PASS_DEPTH",					"1" },
		{ ScnShaderPermutationFlags::PASS_MAIN,					"PERM_PASS_MAIN",					"1" },
	};

	static ScnShaderPermutationEntry GPermutationsMeshType[] = 
	{
		{ ScnShaderPermutationFlags::MESH_STATIC_2D,			"PERM_MESH_STATIC_2D",				"1" },
		{ ScnShaderPermutationFlags::MESH_STATIC_3D,			"PERM_MESH_STATIC_3D",				"1" },
		{ ScnShaderPermutationFlags::MESH_SKINNED_3D,			"PERM_MESH_SKINNED_3D",				"1" },
		{ ScnShaderPermutationFlags::MESH_PARTICLE_3D,			"PERM_MESH_PARTICLE_3D",			"1" },
		{ ScnShaderPermutationFlags::MESH_INSTANCED_3D,			"PERM_MESH_INSTANCED_3D",			"1" },
	};

	static ScnShaderPermutationEntry GPermutationsLightingType[] = 
	{
		{ ScnShaderPermutationFlags::LIGHTING_NONE,				"PERM_LIGHTING_NONE",				"1" },
		{ ScnShaderPermutationFlags::LIGHTING_DIFFUSE,			"PERM_LIGHTING_DIFFUSE",			"1" },
	};

	static ScnShaderPermutationGroup GPermutationGroups[] =
	{
		ScnShaderPermutationGroup( GPermutationsRenderType ),
		ScnShaderPermutationGroup( GPermutationsPassType ),
		ScnShaderPermutationGroup( GPermutationsMeshType ),
		ScnShaderPermutationGroup( GPermutationsLightingType ),
	};

	static const BcU32 GNoofPermutationGroups = ( sizeof( GPermutationGroups ) / sizeof( GPermutationGroups[ 0 ] ) );

	// NOTE: Put these in the order that HLSLCC needs to build them.
	static const ScnShaderLevelEntry GShaderLevelEntries[] =
	{
		{ "ps_4_0_level_9_1", RsShaderType::PIXEL, RsShaderCodeType::D3D11_4_0_LEVEL_9_1 },
		{ "ps_4_0_level_9_2", RsShaderType::PIXEL, RsShaderCodeType::D3D11_4_0_LEVEL_9_2 },
		{ "ps_4_0_level_9_3", RsShaderType::PIXEL, RsShaderCodeType::D3D11_4_0_LEVEL_9_3 },
		{ "ps_4_0", RsShaderType::PIXEL, RsShaderCodeType::D3D11_4_0 },
		{ "ps_4_1", RsShaderType::PIXEL, RsShaderCodeType::D3D11_4_1 },
		{ "ps_5_0", RsShaderType::PIXEL, RsShaderCodeType::D3D11_5_0 },
		
		{ "hs_5_0", RsShaderType::HULL, RsShaderCodeType::D3D11_5_0 },

		{ "ds_5_0", RsShaderType::DOMAIN, RsShaderCodeType::D3D11_5_0 },
		
		{ "gs_4_0", RsShaderType::GEOMETRY, RsShaderCodeType::D3D11_4_0 },
		{ "gs_4_1", RsShaderType::GEOMETRY, RsShaderCodeType::D3D11_4_1 },
		{ "gs_5_0", RsShaderType::GEOMETRY, RsShaderCodeType::D3D11_5_0 },
		
		{ "vs_4_0_level_9_1", RsShaderType::VERTEX, RsShaderCodeType::D3D11_4_0_LEVEL_9_1 },
		{ "vs_4_0_level_9_2", RsShaderType::VERTEX, RsShaderCodeType::D3D11_4_0_LEVEL_9_2 },
		{ "vs_4_0_level_9_3", RsShaderType::VERTEX, RsShaderCodeType::D3D11_4_0_LEVEL_9_3 },
		{ "vs_4_0", RsShaderType::VERTEX, RsShaderCodeType::D3D11_4_0 },
		{ "vs_4_1", RsShaderType::VERTEX, RsShaderCodeType::D3D11_4_1 },
		{ "vs_5_0", RsShaderType::VERTEX, RsShaderCodeType::D3D11_5_0 },
		
		{ "cs_4_0", RsShaderType::COMPUTE, RsShaderCodeType::D3D11_4_0 },
		{ "cs_4_1", RsShaderType::COMPUTE, RsShaderCodeType::D3D11_4_1 },
		{ "cs_5_0", RsShaderType::COMPUTE, RsShaderCodeType::D3D11_5_0 },
	};

	static const BcU32 GNoofShaderLevelEntries = ( sizeof( GShaderLevelEntries ) / sizeof( GShaderLevelEntries[ 0 ] ) ); 

}

#endif // PSY_IMPORT_PIPELINE

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( ScnShaderImport )
	
void ScnShaderImport::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Source_", &ScnShaderImport::Source_, bcRFF_IMPORTER ),
		new ReField( "Sources_", &ScnShaderImport::Sources_, bcRFF_IMPORTER ),
		new ReField( "Defines_", &ScnShaderImport::Defines_, bcRFF_IMPORTER ),
		new ReField( "Entrypoints_", &ScnShaderImport::Entrypoints_, bcRFF_IMPORTER ),
		new ReField( "UsePermutations_", &ScnShaderImport::UsePermutations_, bcRFF_IMPORTER ),
		new ReField( "ExcludePermutations_", &ScnShaderImport::ExcludePermutations_, bcRFF_IMPORTER ),
		new ReField( "IncludePermutations_", &ScnShaderImport::IncludePermutations_, bcRFF_IMPORTER ),
		new ReField( "CodeTypes_", &ScnShaderImport::CodeTypes_, bcRFF_IMPORTER ),
		new ReField( "BackendTypes_", &ScnShaderImport::BackendTypes_, bcRFF_IMPORTER ),
	};
	ReRegisterClass< ScnShaderImport, Super >( Fields );

	{
		ReEnumConstant* EnumConstants[] = 
		{
			new ReEnumConstant( "NONE", ScnShaderPermutationFlags::NONE ),

			new ReEnumConstant( "RENDER_FORWARD", ScnShaderPermutationFlags::RENDER_FORWARD ),
			new ReEnumConstant( "RENDER_DEFERRED", ScnShaderPermutationFlags::RENDER_DEFERRED ),
			new ReEnumConstant( "RENDER_FORWARD_PLUS", ScnShaderPermutationFlags::RENDER_FORWARD_PLUS ),
			new ReEnumConstant( "RENDER_POST_PROCESS", ScnShaderPermutationFlags::RENDER_POST_PROCESS ),
			new ReEnumConstant( "RENDER_ALL", ScnShaderPermutationFlags::RENDER_ALL ),

			new ReEnumConstant( "PASS_SHADOW", ScnShaderPermutationFlags::PASS_SHADOW ),
			new ReEnumConstant( "PASS_DEPTH", ScnShaderPermutationFlags::PASS_DEPTH ),
			new ReEnumConstant( "PASS_MAIN", (BcU32)ScnShaderPermutationFlags::PASS_MAIN ),
			new ReEnumConstant( "PASS_ALL", ScnShaderPermutationFlags::PASS_ALL ),

			new ReEnumConstant( "MESH_STATIC_2D", (BcU32)ScnShaderPermutationFlags::MESH_STATIC_2D ),
			new ReEnumConstant( "MESH_STATIC_3D", (BcU32)ScnShaderPermutationFlags::MESH_STATIC_3D ),
			new ReEnumConstant( "MESH_SKINNED_3D", (BcU32)ScnShaderPermutationFlags::MESH_SKINNED_3D ),
			new ReEnumConstant( "MESH_PARTICLE_3D", (BcU32)ScnShaderPermutationFlags::MESH_PARTICLE_3D ),
			new ReEnumConstant( "MESH_INSTANCED_3D", (BcU32)ScnShaderPermutationFlags::MESH_INSTANCED_3D ),
			new ReEnumConstant( "MESH_ALL", (BcU32)ScnShaderPermutationFlags::MESH_ALL ),

			new ReEnumConstant( "LIGHTING_NONE", (BcU32)ScnShaderPermutationFlags::LIGHTING_NONE ),
			new ReEnumConstant( "LIGHTING_DIFFUSE", (BcU32)ScnShaderPermutationFlags::LIGHTING_DIFFUSE ),
			new ReEnumConstant( "LIGHTING_ALL", (BcU32)ScnShaderPermutationFlags::LIGHTING_ALL ),
		};
		ReRegisterEnum< ScnShaderPermutationFlags >( EnumConstants );
	}
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnShaderImport::ScnShaderImport() :
	ShaderCompileId_( 0 ),
	UsePermutations_( BcTrue )
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnShaderImport::ScnShaderImport( ReNoInit ) :
	ShaderCompileId_( 0 ),
	UsePermutations_( BcTrue )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnShaderImport::~ScnShaderImport()
{

}

//////////////////////////////////////////////////////////////////////////
// import
BcBool ScnShaderImport::import()
{
	// Regenerate shader data header.
	regenerateShaderDataHeader();

#if PSY_IMPORT_PIPELINE
	if( Source_.empty() && Sources_.empty() )
	{
		PSY_LOG( "ERROR: Missing 'source' or 'sources' fields.\n" );
		return BcFalse;
	}

	// Entry points.
	if( Entrypoints_.size() == 0 )
	{
		PSY_LOG( "ERROR: Missing entry points.\n" );
		return BcFalse;
	}

	// Reset errors building.
	GotErrorBuilding_.store( 0 );

	// Reset pending permutations.
	PendingPermutations_.store( 0 );

	
#if PLATFORM_WINDOWS
	// Setup include paths.
	IncludePaths_.clear();
	IncludePaths_.push_back( ".\\" );
	IncludePaths_.push_back( ".\\Content\\Engine\\" );
	IncludePaths_.push_back( "..\\Psybrus\\Dist\\Content\\Engine\\" );
#elif PLATFORM_LINUX || PLATFORM_OSX
	// Setup include paths.
	IncludePaths_.clear();
	IncludePaths_.push_back( "./" );
	IncludePaths_.push_back( "./Content/Engine/" );
	IncludePaths_.push_back( "../Psybrus/Dist/Content/Engine/" );
#endif
	
	// Cache intermediate path.
	IntermediatePath_ = getIntermediatePath();

	// Do appropriate pipeline.
	BcBool RetVal = BcFalse;
	if( !Source_.empty() )
	{
		RetVal = oldPipeline();
	}
	else
	{
		RetVal = newPipeline();
	}

	// Wait for permutation building jobs.
	while( PendingPermutations_ > 0 )
	{
		std::this_thread::yield();
	}

	// No errors hit?
	if( GotErrorBuilding_ > 0 )
	{
		RetVal = BcFalse;
	}

	// Export.
	if( RetVal == BcTrue )
	{
		BcStream Stream;
		ScnShaderHeader Header;
		ScnShaderUnitHeader ShaderUnit;
		BcMemZero( &Header, sizeof( Header ) );
		BcMemZero( &ShaderUnit, sizeof( ShaderUnit ) );

		// Export header.
		Header.NoofShaderPermutations_ = static_cast< BcU32 >( BuiltShaderData_.size() );
		Header.NoofProgramPermutations_ = static_cast< BcU32 >( BuiltProgramData_.size() );

		// Determine shader code types built.
		std::set< RsShaderCodeType > ShaderCodeTypes;
		for( const auto& BuiltProgramData : BuiltProgramData_ )
		{
			ShaderCodeTypes.insert( BuiltProgramData.ShaderCodeType_ );
		}

		Header.NoofShaderCodeTypes_ = static_cast< BcU32 >( ShaderCodeTypes.size() );
		
		Stream << Header;
		for( auto ShaderCodeType : ShaderCodeTypes )
		{
			Stream << ShaderCodeType;
		}

		CsResourceImporter::addChunk( BcHash( "header" ), Stream.pData(), Stream.dataSize() );

		// Export shaders.
		for( auto& ShaderData : BuiltShaderData_ )
		{
			ShaderUnit.ShaderType_ = ShaderData.second.ShaderType_;
			ShaderUnit.ShaderDataType_ = RsShaderDataType::SOURCE;
			ShaderUnit.ShaderCodeType_ = ShaderData.second.CodeType_;
			ShaderUnit.ShaderHash_ = ShaderData.second.Hash_;
			ShaderUnit.PermutationFlags_ = ScnShaderPermutationFlags::NONE;

			Stream.clear();
			Stream.push( &ShaderUnit, sizeof( ShaderUnit ) );
			Stream.push( ShaderData.second.Code_.getData< BcU8* >(), ShaderData.second.Code_.getDataSize() );

			CsResourceImporter::addChunk( BcHash( "shader" ), Stream.pData(), Stream.dataSize() );
		}

		// Export programs.
		BcU32 VertexAttributeIdx = 0;
		BcU32 ParameterIdx = 0;
		for( BcU32 Idx = 0; Idx < BuiltProgramData_.size(); ++Idx )
		{
			auto& ProgramData = BuiltProgramData_[ Idx ];

			Stream.clear();
			Stream.push( &ProgramData, sizeof( ProgramData ) );

			// Export vertex attributes, uniforms, and uniform blocks.
			if( ProgramData.NoofVertexAttributes_ > 0 )
			{
				auto& VertexAttributes = BuiltVertexAttributes_[ VertexAttributeIdx++ ];
				BcAssert( VertexAttributes.size() > 0 );
				Stream.push( &VertexAttributes[ 0 ], VertexAttributes.size() * sizeof( RsProgramVertexAttribute ) );
			}

			if( ProgramData.NoofParameters_ > 0 )
			{
				auto& Parameters = BuiltParameters_[ ParameterIdx++ ];
				BcAssert( Parameters.size() > 0 );
				Stream.push( &Parameters[ 0 ], Parameters.size() * sizeof( RsProgramParameter ) );
			}

			CsResourceImporter::addChunk( BcHash( "program" ), Stream.pData(), Stream.dataSize() );			
		}
	}

	return RetVal;
#else
	return BcFalse;
#endif // PSY_IMPORT_PIPELINE
}

//////////////////////////////////////////////////////////////////////////
// regenerateShaderDataHeader
void ScnShaderImport::regenerateShaderDataHeader()
{
#if PSY_IMPORT_PIPELINE
	// Check if we can bail out.
	static std::atomic< bool > RegeneratedShaderData( false );
	if( RegeneratedShaderData.exchange( true ) == true )
	{
		return;
	}
	
	// Setup class-psf type mapping.
	ShaderClassMapping_[ ReManager::GetClass< BcU32 >() ] = "uint";
	ShaderClassMapping_[ ReManager::GetClass< BcS32 >() ] = "int";
	ShaderClassMapping_[ ReManager::GetClass< BcF32 >() ] = "float";
	ShaderClassMapping_[ ReManager::GetClass< MaVec2d >() ] = "float2";
	ShaderClassMapping_[ ReManager::GetClass< MaVec3d >() ] = "float3";
	ShaderClassMapping_[ ReManager::GetClass< MaVec4d >() ] = "float4";
	ShaderClassMapping_[ ReManager::GetClass< MaMat4d >() ] = "float4x4";
	ShaderClassMapping_[ ReManager::GetClass< MaQuat >() ] = "float4";
	ShaderClassMapping_[ ReManager::GetClass< RsColour >() ] = "float4";

	// Find all classes with the ScnShaderDataAttribute.
	std::vector< const ReClass* > ShaderDataClasses;
	CsResourceImporter::addDependency( ScnShaderDataAttribute::StaticGetClass() );
	auto Classes = ReManager::GetClasses();
	std::set< BcName > FoundNames;
	for( const auto* Class : Classes )
	{
		if( auto Attribute = Class->getAttribute< ScnShaderDataAttribute >() )
		{
			BcAssertMsg( Attribute->getName() != BcName::INVALID, "ScnShaderDataAttribute missing name." );
			BcAssertMsg( FoundNames.find( Attribute->getName() ) == FoundNames.end(), "ScnShaderDataAttribute \"%s\" already taken.", (*Attribute->getName()).c_str() );
			ShaderDataClasses.push_back( Class );
			BcAssert( Class->getFlags() & bcRFF_POD );
			CsResourceImporter::addDependency( Class );
		}
	}

	auto writeShaderClass = [ this ]( RsShaderBackendType OutputBackend, const ReClass* Class, BcU32 MaxInstances, std::string& OutString )
	{
		std::array< BcChar, 1024 > OutBuffer;
		static std::array< const char*, 5 > UniformBufferDecls =
		{
			"layout(std140) uniform",
			nullptr,
			"cbuffer",
			"cbuffer",
			"layout(std140) uniform",
		};

		auto UniformBufferDecl = UniformBufferDecls[ (int)OutputBackend ];

		const auto* Attribute = Class->getAttribute< ScnShaderDataAttribute >();
		switch( OutputBackend )
		{
			case RsShaderBackendType::D3D11:
			case RsShaderBackendType::GLSL:
			{
				BcAssert( UniformBufferDecl );

				{
					OutString += std::string( UniformBufferDecl ) + " " + (*Class->getName()) + "\n";
					OutString += "{\n";

					for( const auto* Field : Class->getFields() )
					{
						writeField( OutString, Class, Field, "\t", OutputBackend );
					}

					OutString += "};\n\n";
				}

				if( Attribute->isInstancable() )
				{
					OutString += "struct " + (*Class->getName()) + "Instance\n";
					OutString += "{\n";

					for( const auto* Field : Class->getFields() )
					{
						writeField( OutString, Class, Field, "\t", OutputBackend );
					}

					OutString += "};\n\n";

					OutString += std::string( UniformBufferDecl ) + " " + (*Class->getName()) + "Instanced\n";
					OutString += "{\n";

					BcSPrintf( OutBuffer.data(), OutBuffer.size(), "\t%sInstance %sInstances_[%u];\n",
						(*Class->getName()).c_str(),
						(*Attribute->getName()).c_str(),
						MaxInstances );
					OutString += OutBuffer.data();

					OutString += "};\n\n";
				}
			}
			break;

			case RsShaderBackendType::GLSL_ES:
			{
				for( const auto* Field : Class->getFields() )
				{
					writeField( OutString, Class, Field, "\t", OutputBackend );

					BcSPrintf( OutBuffer.data(), OutBuffer.size(), "#define %s %sVS_X%s\n",
						(*Field->getName()).c_str(),
						(*Class->getName()).c_str(),
						(*Field->getName()).c_str() );
					OutString += OutBuffer.data();
				}

				OutString += "\n\n";	
			}
			break;

			default:
				BcBreakpoint;
		}
	};

	auto writeShaderHeader = [ this, &writeShaderClass ]( const ReClass* Class )
	{
		std::array< RsShaderBackendType, 3 > OutputBackends = 
		{
			RsShaderBackendType::GLSL_ES,
			RsShaderBackendType::GLSL,
			RsShaderBackendType::D3D11,
		};

		// TODO: Calculate max.
		const int MAX_INSTANCES = 128;

		std::array< BcChar, 1024 > OutBuffer;

		const auto* Attribute = Class->getAttribute< ScnShaderDataAttribute >();

		// Write out some shaders.
		std::string OutString;
		OutString += "// DO NOT MODIFY. Autogenerated by ScnShaderImport::regenerateShaderDataHeader.\n";

		BcSPrintf( OutBuffer.data(), OutBuffer.size(), "#ifndef __Uniform%s_PSH__\n", (*Attribute->getName()).c_str() );
		OutString += OutBuffer.data();
		BcSPrintf( OutBuffer.data(), OutBuffer.size(), "#define __Uniform%s_PSH__\n", (*Attribute->getName()).c_str() );
		OutString += OutBuffer.data();

		OutString += "#include <PsybrusTypes.psh>\n\n";

		CsResourceImporter::addDependency( Class );
		for( size_t Idx = 0; Idx < OutputBackends.size(); ++Idx )
		{
			auto OutputBackend = OutputBackends[ Idx ];

			auto BackendTypeString = RsShaderBackendTypeToString( OutputBackend );
			BcSPrintf( OutBuffer.data(),  OutBuffer.size(), "#if PSY_INPUT_BACKEND_TYPE == PSY_BACKEND_TYPE_%s\n", 
				BackendTypeString.data() );
			OutString += OutBuffer.data();

			writeShaderClass( OutputBackend, Class, MAX_INSTANCES, OutString );

			OutString += "#endif\n\n";
		}

		OutString += "#endif // include guard\n";

		// Check output folder exists.
		bool ShouldExport = false;
		if( !BcFileSystemExists( "./Content/Engine" ) )
		{
			BcFileSystemCreateDirectories( "./Content/Engine" );
			ShouldExport = true;
		}

		// Check if file currently exists.
		BcSPrintf( OutBuffer.data(), OutBuffer.size(), "./Content/Engine/Uniform%s.psh", (*Attribute->getName()).c_str() );
		const std::string FileName = OutBuffer.data();
		if( BcFileSystemExists( FileName.c_str() ) )
		{
			BcFile ExistingHeader;
			if( ExistingHeader.open( FileName.c_str(), bcFM_READ ) )
			{
				auto FileData = ExistingHeader.readAllBytes();
				std::string ExistingString =  reinterpret_cast< const char* >( FileData.get() );
				ExistingString = BcStrReplace( ExistingString, "\r\n", "\n" ); // Silly windows line endings.
				if( OutString != ExistingString )
				{
					ShouldExport = true;
				}
			}
		}
		else
		{
			ShouldExport = true;
		}

		if( ShouldExport )
		{
			BcFile OutputHeader;
			if( OutputHeader.open( FileName.c_str(), bcFM_WRITE_TEXT ) )
			{
				OutputHeader.write( OutString.data(), OutString.size() );
			}
		}
	};

	// Write out shader headers.
	for( auto ShaderClass : ShaderDataClasses )
	{
		writeShaderHeader( ShaderClass );
	}
#endif // PSY_IMPORT_PIPELINE
}

//////////////////////////////////////////////////////////////////////////
// writeField
void ScnShaderImport::writeField( std::string& OutString, const ReClass* InClass, const ReField* InField, std::string Indentation, RsShaderBackendType OutputBackend )
{
	BcU32 FieldSize = static_cast< BcU32 >( InField->getSize() );
	BcU32 ClassSize = static_cast< BcU32 >( InField->getType()->getSize() );
	BcU32 NumElements = FieldSize / ClassSize;
	std::array< BcChar, 1024 > OutBuffer;
	OutBuffer.fill( 0 );

	// Check if it's a base type.
	auto It = ShaderClassMapping_.find( InField->getType() );
	if( It == ShaderClassMapping_.end() )
	{
		BcAssertMsg( false, "Nested structs not supported by Psybrus for any current shading language." );

		OutString += Indentation + "struct\n";
		OutString += Indentation + "{\n";
		for( const auto* Field : InField->getType()->getFields() )
		{
			writeField( OutString, InField->getClass(), Field, Indentation + "\t", OutputBackend );
		}
		if( NumElements == 1 )
		{
			BcSPrintf( OutBuffer.data(), OutBuffer.size(), 
				"%s} %s;\n",
				Indentation.c_str(), 
				(*InField->getName()).c_str() );
			OutString += OutBuffer.data();
		}
		else
		{
			BcSPrintf( OutBuffer.data(), OutBuffer.size(), 
				"%s} %s[%u];\n",
				Indentation.c_str(), 
				(*InField->getName()).c_str(),
				NumElements );
			OutString += OutBuffer.data();
		}
	}
	else
	{
		if( OutputBackend == RsShaderBackendType::GLSL_ES )
		{
			if( NumElements == 1 )
			{
				BcSPrintf( OutBuffer.data(), OutBuffer.size(), 
					"uniform %s %sVS_X%s;\n",
					It->second.c_str(),
					(*InClass->getName()).c_str(),
					(*InField->getName()).c_str() );
			}
			else
			{
				BcSPrintf( OutBuffer.data(), OutBuffer.size(), 
					"uniform %s %sVS_X%s[%u];\n",
					It->second.c_str(),
					(*InClass->getName()).c_str(),
					(*InField->getName()).c_str(),
					NumElements );
			}
		}
		else
		{
			if( NumElements == 1 )
			{
				BcSPrintf( OutBuffer.data(), OutBuffer.size(), 
					"%s%s %s;\n",
					Indentation.c_str(),
					It->second.c_str(),
					(*InField->getName()).c_str() );
			}
			else
			{
				BcSPrintf( OutBuffer.data(), OutBuffer.size(), 
					"%s%s %s[%u];\n",
					Indentation.c_str(),
					It->second.c_str(),
					(*InField->getName()).c_str(),
					NumElements );
			}
		}

		OutString += OutBuffer.data();
	}
}

//////////////////////////////////////////////////////////////////////////
// oldPipeline
BcBool ScnShaderImport::oldPipeline()
{
	BcBool RetVal = BcFalse;
#if PSY_IMPORT_PIPELINE
	// Read in source.
	if( !Source_.empty() )
	{
		BcFile SourceFile;
		if( SourceFile.open( Source_.c_str(), bcFM_READ ) )
		{
			std::vector< char > FileData( SourceFile.size() + 1 );
			BcMemZero( FileData.data(), FileData.size() );
			SourceFile.read( FileData.data(), FileData.size() );
			SourceFileData_ = FileData.data();

			addDependency( Source_.c_str() );
		}
	}
	else
	{
		return BcFalse;
	}

	// Generate permutations.
	generatePermutations( 0, GNoofPermutationGroups, GPermutationGroups, getDefaultPermutation() );

	// Sort input types from lowest to highest.
	std::sort( CodeTypes_.begin(), CodeTypes_.end(), 
		[]( RsShaderCodeType A, RsShaderCodeType B )
		{
			return A < B;
		} );

	// Backend types. If it's empty, default to all.
	if( BackendTypes_.empty() )
	{
		BackendTypes_.push_back( RsShaderBackendType::D3D11 );
		BackendTypes_.push_back( RsShaderBackendType::GLSL );
		BackendTypes_.push_back( RsShaderBackendType::GLSL_ES );
	}

	// Kick off all permutation building jobs.
	for( auto& Permutation : Permutations_ )
	{
		for( const auto& InputCodeType : CodeTypes_ )
		{
			// Setup entries for input code type.
			std::vector< ScnShaderLevelEntry > Entries;
			for( auto& ShaderLevelEntry : GShaderLevelEntries )
			{
				const auto& Entry = Entrypoints_[ ShaderLevelEntry.Type_ ];
				if( ShaderLevelEntry.CodeType_ == InputCodeType &&
					!Entry.empty() )
				{
					ScnShaderLevelEntry NewEntry = ShaderLevelEntry;
					Entries.push_back( NewEntry );
				}
			}

			// If we've got valid entries, continue.
			if( Entries.size() > 0 )
			{
				for( const auto& BackendType : BackendTypes_ )
				{
					RsShaderCodeType OutputCodeType = RsConvertCodeTypeToBackendCodeType( InputCodeType, BackendType );

					// If it isn't invalid, add and build.
					if( OutputCodeType != RsShaderCodeType::INVALID )
					{
						// Add output code type.
						if( std::find( OutputCodeTypes_.begin(), OutputCodeTypes_.end(), OutputCodeType ) == OutputCodeTypes_.end() )
						{
							OutputCodeTypes_.push_back( OutputCodeType );
						}

						// Build on a job.
						ScnShaderPermutationJobParams JobParams =
						{
							InputCodeType,
							OutputCodeType,
							Source_,
							SourceFileData_,
							Permutation,
							Entries
						};

						++PendingPermutations_;

						SysKernel::pImpl()->pushFunctionJob( 
							0,
							[ this, JobParams ]()
							{
								if( buildPermutation( JobParams ) == BcFalse )
								{
									;
								}
							} );
					}
				}
			}
		}
	}
	RetVal = BcTrue;
#endif
	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// newPipeline
BcBool ScnShaderImport::newPipeline()
{
	BcBool RetVal = BcFalse;
#if PSY_IMPORT_PIPELINE
	// Read in sources.
	for( auto& SourcePair : Sources_ )
	{
		PSY_LOG( "Source: %s - %s", (*ReManager::GetEnumValueName( SourcePair.first )).c_str(), SourcePair.second.c_str() );

		BcFile SourceFile;
		if( SourceFile.open( SourcePair.second.c_str(), bcFM_READ ) )
		{
			std::vector< char > FileData( SourceFile.size() + 1 );
			BcMemZero( FileData.data(), FileData.size() );
			SourceFile.read( FileData.data(), FileData.size() );
			SourcesFileData_[ SourcePair.first ] = FileData.data();
			CodeTypes_.push_back( SourcePair.first );
			addDependency( SourcePair.second.c_str() );
		}
		else
		{
			return BcFalse;
		}
	}

	// Generate permutation.
	auto DefaultPermutation = getDefaultPermutation();

	// Generate permutations if required.
	if( UsePermutations_ )
	{
		generatePermutations( 0, GNoofPermutationGroups, GPermutationGroups, DefaultPermutation );
	}
	else
	{
		Permutations_.push_back( DefaultPermutation );
	}

	PSY_LOG( "Total permutations per backend: %u, total: %u", Permutations_.size(), Permutations_.size() * CodeTypes_.size() );

	// Sort input types from lowest to highest.
	std::sort( CodeTypes_.begin(), CodeTypes_.end(), 
		[]( RsShaderCodeType A, RsShaderCodeType B )
		{
			return A < B;
		} );

	// Kick off all permutation building j-obs.
	for( auto& Permutation : Permutations_ )
	{
		for( const auto& InputCodeType : CodeTypes_ )
		{
			std::vector< ScnShaderLevelEntry > Entries;
			if( RsShaderCodeTypeToBackendType( InputCodeType ) == RsShaderBackendType::GLSL ||
				RsShaderCodeTypeToBackendType( InputCodeType ) == RsShaderBackendType::GLSL_ES )
			{
				// Setup entries for input code type.
				std::vector< ScnShaderLevelEntry > GLSLEntries = 
				{
					{ "pixel", RsShaderType::PIXEL, InputCodeType },
					{ "hull", RsShaderType::HULL, InputCodeType },
					{ "domain", RsShaderType::DOMAIN, InputCodeType },
					{ "geometry", RsShaderType::GEOMETRY, InputCodeType },
					{ "vertex", RsShaderType::VERTEX, InputCodeType },
					{ "compute", RsShaderType::COMPUTE, InputCodeType },
				};

				for( auto& ShaderLevelEntry : GLSLEntries )
				{
					const auto& Entry = Entrypoints_[ ShaderLevelEntry.Type_ ];
					if( ShaderLevelEntry.CodeType_ == InputCodeType &&
						!Entry.empty() )
					{
						ScnShaderLevelEntry NewEntry = ShaderLevelEntry;
						Entries.push_back( NewEntry );
					}
				}
			}
			else if( RsShaderCodeTypeToBackendType( InputCodeType ) == RsShaderBackendType::D3D11 )
			{
				for( auto& ShaderLevelEntry : GShaderLevelEntries )
				{
					const auto& Entry = Entrypoints_[ ShaderLevelEntry.Type_ ];
					if( ShaderLevelEntry.CodeType_ == InputCodeType &&
						!Entry.empty() )
					{
						ScnShaderLevelEntry NewEntry = ShaderLevelEntry;
						Entries.push_back( NewEntry );
					}
				}
			}

			// If we've got valid entries, continue.
			if( Entries.size() > 0 )
			{
				// No conversion in this step.
				RsShaderCodeType OutputCodeType = InputCodeType;

				// If it isn't invalid, add and build.
				if( OutputCodeType != RsShaderCodeType::INVALID )
				{
					// Add output code type.
					if( std::find( OutputCodeTypes_.begin(), OutputCodeTypes_.end(), OutputCodeType ) == OutputCodeTypes_.end() )
					{
						OutputCodeTypes_.push_back( OutputCodeType );
					}

					// Build on a job.
					ScnShaderPermutationJobParams JobParams =
					{
						InputCodeType,
						OutputCodeType,
						Sources_[ InputCodeType ],
						SourcesFileData_[ InputCodeType ],
						Permutation,
						Entries
					};

					++PendingPermutations_;

					SysKernel::pImpl()->pushFunctionJob( 
						0,
						[ this, JobParams ]()
						{
							if( buildPermutation( JobParams ) == BcFalse )
							{
							}
						} );

					RetVal = BcTrue;
				}
			}
		}
	}
#endif
	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// getDefaultPermutation
ScnShaderPermutation ScnShaderImport::getDefaultPermutation()
{
	// Setup permutations.
	ScnShaderPermutation Permutation;
#if PSY_IMPORT_PIPELINE

	std::array< char, 64 > Buffer;
	Buffer.fill( 0 );

	// Default to alway use cbuffers.
	Permutation.Defines_[ "PSY_USE_CBUFFER" ] = "1";

	// Add code type defines.
	for( BcU32 Idx = 0; Idx < (BcU32)RsShaderCodeType::MAX; ++Idx )
	{
		auto CodeTypeString = RsShaderCodeTypeToString( (RsShaderCodeType)Idx );
		BcSPrintf( Buffer.data(), Buffer.size() - 1, "PSY_CODE_TYPE_%s", CodeTypeString.c_str() );
		Permutation.Defines_[ Buffer.data() ] = std::to_string( Idx );
	}

	// Add backend type defines.
	for( BcU32 Idx = 0; Idx < (BcU32)RsShaderBackendType::MAX; ++Idx )
	{
		auto BackendTypeString = RsShaderBackendTypeToString( (RsShaderBackendType)Idx );
		BcSPrintf( Buffer.data(), Buffer.size() - 1, "PSY_BACKEND_TYPE_%s", BackendTypeString.c_str() );
		Permutation.Defines_[ Buffer.data() ] = std::to_string( Idx );
	}
#endif
	return Permutation;
}

//////////////////////////////////////////////////////////////////////////
// generatePermutations
void ScnShaderImport::generatePermutations( 
		BcU32 GroupIdx, 
		BcU32 NoofGroups,
		ScnShaderPermutationGroup* PermutationGroups, 
		ScnShaderPermutation Permutation )
{
#if PSY_IMPORT_PIPELINE
	const auto& PermutationGroup = PermutationGroups[ GroupIdx ];

	// Check exclude & include permutations.
	BcU32 GroupExcludeMask = 0;
	BcU32 GroupIncludeMask = 0;
	BcAssert( PermutationGroup.NoofEntries_ < sizeof( GroupExcludeMask ) * 8 );
	BcAssert( PermutationGroup.NoofEntries_ < sizeof( GroupIncludeMask ) * 8 );
	for( BcU32 Idx = 0; Idx < PermutationGroup.NoofEntries_; ++Idx )
	{
		auto PermutationEntry = PermutationGroup.Entries_[ Idx ];
		for( auto ExcludePermutation : ExcludePermutations_ )
		{
			if( PermutationEntry.Flag_ == ExcludePermutation )
			{
				GroupExcludeMask |= 1 << Idx;
				break;
			}
		}

		for( auto IncludePermutation : IncludePermutations_ )
		{
			if( PermutationEntry.Flag_ == IncludePermutation )
			{
				GroupIncludeMask |= 1 << Idx;
				break;
			}
		}
	}

	// Nothing included? Include everything by default.
	if( GroupIncludeMask == 0 )
	{
		GroupIncludeMask = 0xffffffff;
	}

	// Now begin filling permutations in.
	for( BcU32 Idx = 0; Idx < PermutationGroup.NoofEntries_; ++Idx )
	{
		auto PermutationEntry = PermutationGroup.Entries_[ Idx ];

		if( ( ( 1 << Idx ) & GroupIncludeMask ) != 0 &&
			( ( 1 << Idx ) & GroupExcludeMask ) == 0 )
		{
			auto NewPermutation = Permutation; 
			NewPermutation.Flags_ |= PermutationEntry.Flag_;
			NewPermutation.Defines_[ PermutationEntry.Define_ ] = PermutationEntry.Value_;

			if( GroupIdx < ( NoofGroups - 1 ) )
			{
				generatePermutations( GroupIdx + 1, NoofGroups, PermutationGroups, NewPermutation );
			}
			else
			{
				// Add all base defines at the end.
				for( auto Define : Defines_ )
				{
					NewPermutation.Defines_.insert( Define );
				}

				Permutations_.push_back( NewPermutation );
			}
		}
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
// buildPermutation
BcBool ScnShaderImport::buildPermutation( ScnShaderPermutationJobParams Params )
{
#if PSY_IMPORT_PIPELINE
	// Early out in case any jobs failed previously.
	if( GotErrorBuilding_ != 0 )
	{
		--PendingPermutations_;
		return BcFalse;
	}

	// Add code type and backend types.
	Params.Permutation_.Defines_[ "PSY_OUTPUT_CODE_TYPE" ] = std::to_string( (BcU32)Params.OutputCodeType_ );
	Params.Permutation_.Defines_[ "PSY_INPUT_CODE_TYPE" ] = std::to_string( (BcU32)Params.InputCodeType_ );
	Params.Permutation_.Defines_[ "PSY_OUTPUT_BACKEND_TYPE" ] = std::to_string( (BcU32)RsShaderCodeTypeToBackendType( Params.OutputCodeType_ ) );
	Params.Permutation_.Defines_[ "PSY_INPUT_BACKEND_TYPE" ] = std::to_string( (BcU32)RsShaderCodeTypeToBackendType( Params.InputCodeType_ ) );

	switch( RsShaderCodeTypeToBackendType( Params.InputCodeType_ ) )
	{
	case RsShaderBackendType::D3D11:
		return buildPermutationHLSL( Params );
		break;
	case RsShaderBackendType::GLSL:
	case RsShaderBackendType::GLSL_ES:
		return buildPermutationGLSL( Params );
		break;
	default:
		BcBreakpoint;
	}
#endif // PSY_IMPORT_PIPELINE
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// buildPermutationHLSL
BcBool ScnShaderImport::buildPermutationHLSL( const ScnShaderPermutationJobParams& Params )
{
	BcBool RetVal = BcTrue;
#if PSY_IMPORT_PIPELINE
	BcAssertMsg( RsShaderCodeTypeToBackendType( Params.InputCodeType_ ) == RsShaderBackendType::D3D11, "Expecting D3D11 code input." );

	// Setup initial header.
	ScnShaderProgramHeader ProgramHeader = {};
	ProgramHeader.ProgramPermutationFlags_ = Params.Permutation_.Flags_;
	ProgramHeader.ShaderFlags_ = ScnShaderPermutationFlags::NONE;
	ProgramHeader.ShaderCodeType_ = Params.OutputCodeType_;
	
	// Vertex attributes needed by GLSL.
	std::vector< RsProgramVertexAttribute > VertexAttributes;

	for( auto& Entry : Params.Entries_ )
	{
		ScnShaderBuiltData BuiltShader;
		BcBinaryData ByteCode;
		std::vector< std::string > ErrorMessages;
		std::string Entrypoint = Entrypoints_[ Entry.Type_ ];
		BcAssert( !Entrypoint.empty() );

		std::map< std::string, std::string > Defines = Params.Permutation_.Defines_;

		const char* ShaderDefines[] = 
		{
			"VERTEX_SHADER",
			"PIXEL_SHADER",
			"HULL_SHADER",
			"DOMAIN_SHADER",
			"GEOMETRY_SHADER",
			"COMPUTE_SHADER"
		};

		Defines[ ShaderDefines[ (int)Entry.Type_ ] ] = "1";

		if( RsShaderCodeTypeToBackendType( Params.OutputCodeType_ ) == RsShaderBackendType::D3D11 )
		{
			if( compileShader( 
					Params.ShaderSource_, 
					Entrypoint, 
					Defines, 
					IncludePaths_, 
					Entry.Level_, 
					ByteCode, 
					ErrorMessages ) )
			{
				BuiltShader.ShaderType_ = Entry.Type_;
				BuiltShader.CodeType_ = Params.InputCodeType_;
				BuiltShader.Code_ = std::move( ByteCode );
				BuiltShader.Hash_ = generateShaderHash( BuiltShader );

				// Vertex shader attributes.
				if( Entry.Type_ == RsShaderType::VERTEX )
				{
					// Generate vertex attributes.
					VertexAttributes = extractShaderVertexAttributes( BuiltShader.Code_ );
				}
			}
			else
			{
				PSY_LOG( "Unable to compile D3D11 shader." );
				RetVal = BcFalse;
			}
		}
		else
		{
			PSY_LOG( "Invalid code type: %u, %u", RsShaderCodeTypeToBackendType( Params.OutputCodeType_ ), Params.OutputCodeType_ );
			RetVal = BcFalse;
			break;
		}

		if( ErrorMessages.size() > 0 )
		{
			std::lock_guard< std::mutex > Lock( BuildingMutex_ );
			ErrorMessages_.insert( ErrorMessages_.end(), ErrorMessages.begin(), ErrorMessages.end() );
		}

		// Setup program + shaders.
		if( RetVal )
		{
			ProgramHeader.NoofVertexAttributes_ = static_cast< BcU32 >( VertexAttributes.size() );
			ProgramHeader.ShaderHashes_[ (BcU32)Entry.Type_ ] = BuiltShader.Hash_;

			std::lock_guard< std::mutex > Lock( BuildingMutex_ );
			auto FoundShader = BuiltShaderData_.find( BuiltShader.Hash_ );
			if( FoundShader != BuiltShaderData_.end() )
			{
				BcAssertMsg( FoundShader->second == BuiltShader, "Hash key collision" );
			}

			BuiltShaderData_[ BuiltShader.Hash_ ] = std::move( BuiltShader );
		}
	}

	// Write out all shaders and programs.
	if( RetVal != BcFalse )
	{
		std::lock_guard< std::mutex > Lock( BuildingMutex_ );
		if( ( ProgramHeader.ShaderHashes_[ (BcU32)RsShaderType::VERTEX ] == 0 ||
			  ProgramHeader.ShaderHashes_[ (BcU32)RsShaderType::PIXEL ] == 0 ) &&
			ProgramHeader.ShaderHashes_[ (BcU32)RsShaderType::COMPUTE ] == 0 )
		{
			PSY_LOG( "No vertex and pixel shaders in program." );
			RetVal = BcFalse;
		}

		BuiltProgramData_.push_back( std::move( ProgramHeader ) );
		if( VertexAttributes.size() > 0 )
		{
			BuiltVertexAttributes_.push_back( VertexAttributes );
		}
	}

	if( RetVal == BcFalse )
	{
		GotErrorBuilding_++;
		PSY_LOG( "Error building..." );
	}

	// Write out warning/error messages.
	if( ErrorMessages_.size() > 0 )
	{
		std::lock_guard< std::mutex > Lock( BuildingMutex_ );
		std::string Errors;
		for( auto& Error : ErrorMessages_ )
		{
			Errors += Error;
		}

		PSY_LOG( "%s\n%s\n", Params.ShaderSource_.c_str(), Errors.c_str() );
		//throw CsImportException( Source_.c_str(), Errors.c_str() );
	}

	--PendingPermutations_;
#endif
	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// generateShaderHash
BcU32 ScnShaderImport::generateShaderHash( const ScnShaderBuiltData& Data )
{
	BcU32 Hash = 0;
	Hash = BcHash::GenerateCRC32( Hash, &Data.ShaderType_, sizeof( Data.ShaderType_ ) );
	Hash = BcHash::GenerateCRC32( Hash, &Data.CodeType_, sizeof( Data.CodeType_ ) );
	Hash = BcHash::GenerateCRC32( Hash, Data.Code_.getData< BcU8* >(), Data.Code_.getDataSize() );
	return Hash;
}

//////////////////////////////////////////////////////////////////////////
// logSource
void ScnShaderImport::logSource( std::string Source )
{
	int LineNum = 1;
	std::istringstream Stream( Source.c_str() );
	std::string Line;
	while( std::getline( Stream, Line ) )
	{
		if( !Line.empty() )
		{
			PSY_LOG( "%u:\t%s", LineNum++, Line.c_str() );
		}
		else
		{
			LineNum++;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// semanticToVertexChannel
RsProgramVertexAttribute ScnShaderImport::semanticToVertexAttribute( BcU32 Channel, const std::string& Name, BcU32 Index )
{
	RsProgramVertexAttribute VertexAttribute;

	VertexAttribute.Channel_ = Channel;
	VertexAttribute.Usage_ = RsVertexUsage::INVALID;
	VertexAttribute.UsageIdx_ = Index;

	if( Name == "POSITION" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::POSITION;
	}
	else if( Name == "SV_POSITION" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::POSITION;
	}
	else if( Name == "BLENDWEIGHTS" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::BLENDWEIGHTS;
	}
	else if( Name == "BLENDINDICES" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::BLENDINDICES;
	}
	else if( Name == "NORMAL" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::NORMAL;
	}
	else if( Name == "PSIZE" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::PSIZE;
	}
	else if( Name == "TEXCOORD" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::TEXCOORD;
	}
	else if( Name == "TANGENT" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::TANGENT;
	}
	else if( Name == "BINORMAL" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::BINORMAL;
	}
	else if( Name == "TESSFACTOR" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::TESSFACTOR;
	}
	else if( Name == "POSITIONT" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::POSITIONT;
	}
	else if( Name == "COLOR" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::COLOUR;
	}
	else if( Name == "COLOUR" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::COLOUR;
	}
	else if( Name == "FOG" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::FOG;
	}
	else if( Name == "DEPTH" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::DEPTH;
	}
	else if( Name == "SAMPLE" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::SAMPLE;
	}
	else if( Name == "SV_INSTANCEID" )
	{
		VertexAttribute.Usage_ = RsVertexUsage::INVALID;
	}
	else
	{
		BcAssertMsg( false, "Unknown vertex attribute \"%s\"", Name.c_str() );
	}

	return VertexAttribute;
}

//////////////////////////////////////////////////////////////////////////
// addDependency
void ScnShaderImport::addDependency( const BcChar* Dependency )
{
	CsResourceImporter::addDependency( Dependency );
}

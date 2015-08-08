/**************************************************************************
*
* File:		ScnShaderImportGLSL.cpp
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

#define EXCLUDE_PSTDINT
#include <glsl/glsl_optimizer.h>
#include <ShaderLang.h>

#if PLATFORM_WINDOWS
#pragma warning ( disable : 4512 ) // Can't generate assignment operator (for boost)
#endif

#include <boost/format.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/wave.hpp>
#include <boost/wave/cpplexer/cpp_lex_interface.hpp>
#include <boost/wave/cpplexer/cpp_lex_iterator.hpp>
#include <boost/wave/cpplexer/cpp_lex_token.hpp>
#include <boost/filesystem.hpp>
#include <bitset>
#include <regex>
#include <sstream>

#undef DOMAIN // This is defined somewhere in a core header.

namespace
{
	//////////////////////////////////////////////////////////////////////////
	// glsl_directives_hooks for boost::wave
	class glsl_directives_hooks:
		public boost::wave::context_policies::default_preprocessing_hooks
	{
	public:
		template < typename _Context, typename _Container >
		bool found_unknown_directive( const _Context& Context, _Container const& Line, _Container& Pending )
		{
			using iterator_type = typename _Container::const_iterator;
			iterator_type Iter = Line.begin();
			boost::wave::token_id ID = boost::wave::util::impl::skip_whitespace( Iter, Line.end() );

			if( ID != boost::wave::T_IDENTIFIER )
			{
				return false;
			}

			// Version.
			if( Iter->get_value() == "version" )
			{
				std::copy( Line.begin(), Line.end(), std::back_inserter( Pending ) );
				return true;
			}

			// Extensions.
			if( Iter->get_value() == "extension" )
			{
				std::copy( Line.begin(), Line.end(), std::back_inserter( Pending ) );
				return true;
			}

			// Unknown directive
			return false;
		}
	};

	TBuiltInResource GetDefaultResource()
	{
		TBuiltInResource Resource;
    	Resource.maxLights = 32;
		Resource.maxClipPlanes = 6;
		Resource.maxTextureUnits = 32;
		Resource.maxTextureCoords = 32;
		Resource.maxVertexAttribs = 64;
		Resource.maxVertexUniformComponents = 4096;
		Resource.maxVaryingFloats = 64;
		Resource.maxVertexTextureImageUnits = 32;
		Resource.maxCombinedTextureImageUnits = 80;
		Resource.maxTextureImageUnits = 32;
		Resource.maxFragmentUniformComponents = 4096;
		Resource.maxDrawBuffers = 32;
		Resource.maxVertexUniformVectors = 128;
		Resource.maxVaryingVectors = 8;
		Resource.maxFragmentUniformVectors = 16;
		Resource.maxVertexOutputVectors = 16;
		Resource.maxFragmentInputVectors = 15;
		Resource.minProgramTexelOffset = -8;
		Resource.maxProgramTexelOffset = 7;
		Resource.maxClipDistances = 8;
		Resource.maxComputeWorkGroupCountX = 65535;
		Resource.maxComputeWorkGroupCountY = 65535;
		Resource.maxComputeWorkGroupCountZ = 65535;
		Resource.maxComputeWorkGroupSizeX = 1024;
		Resource.maxComputeWorkGroupSizeY = 1024;
		Resource.maxComputeWorkGroupSizeZ = 64;
		Resource.maxComputeUniformComponents = 1024;
		Resource.maxComputeTextureImageUnits = 16;
		Resource.maxComputeImageUniforms = 8;
		Resource.maxComputeAtomicCounters = 8;
		Resource.maxComputeAtomicCounterBuffers = 1;
		Resource.maxVaryingComponents = 60;
		Resource.maxVertexOutputComponents = 64;
		Resource.maxGeometryInputComponents = 64;
		Resource.maxGeometryOutputComponents = 128;
		Resource.maxFragmentInputComponents = 128;
		Resource.maxImageUnits = 8;
		Resource.maxCombinedImageUnitsAndFragmentOutputs = 8;
		Resource.maxCombinedShaderOutputResources = 8;
		Resource.maxImageSamples = 0;
		Resource.maxVertexImageUniforms = 0;
		Resource.maxTessControlImageUniforms = 0;
		Resource.maxTessEvaluationImageUniforms = 0;
		Resource.maxGeometryImageUniforms = 0;
		Resource.maxFragmentImageUniforms = 8;
		Resource.maxCombinedImageUniforms = 8;
		Resource.maxGeometryTextureImageUnits = 16;
		Resource.maxGeometryOutputVertices = 256;
		Resource.maxGeometryTotalOutputComponents = 1024;
		Resource.maxGeometryUniformComponents = 1024;
		Resource.maxGeometryVaryingComponents = 64;
		Resource.maxTessControlInputComponents = 128;
		Resource.maxTessControlOutputComponents = 128;
		Resource.maxTessControlTextureImageUnits = 16;
		Resource.maxTessControlUniformComponents = 1024;
		Resource.maxTessControlTotalOutputComponents = 4096;
		Resource.maxTessEvaluationInputComponents = 128;
		Resource.maxTessEvaluationOutputComponents = 128;
		Resource.maxTessEvaluationTextureImageUnits = 16;
		Resource.maxTessEvaluationUniformComponents = 1024;
		Resource.maxTessPatchComponents = 120;
		Resource.maxPatchVertices = 32;
		Resource.maxTessGenLevel = 64;
		Resource.maxViewports = 16;
		Resource.maxVertexAtomicCounters = 0;
		Resource.maxTessControlAtomicCounters = 0;
		Resource.maxTessEvaluationAtomicCounters = 0;
		Resource.maxGeometryAtomicCounters = 0;
		Resource.maxFragmentAtomicCounters = 8;
		Resource.maxCombinedAtomicCounters = 8;
		Resource.maxAtomicCounterBindings = 1;
		Resource.maxVertexAtomicCounterBuffers = 0;
		Resource.maxTessControlAtomicCounterBuffers = 0;
		Resource.maxTessEvaluationAtomicCounterBuffers = 0;
		Resource.maxGeometryAtomicCounterBuffers = 0;
		Resource.maxFragmentAtomicCounterBuffers = 1;
		Resource.maxCombinedAtomicCounterBuffers = 1;
		Resource.maxAtomicCounterBufferSize = 16384;
		Resource.maxTransformFeedbackBuffers = 4;
		Resource.maxTransformFeedbackInterleavedComponents = 64;
		Resource.maxCullDistances = 8;
		Resource.maxCombinedClipAndCullDistances = 8;
		Resource.maxSamples = 4;

		Resource.limits.nonInductiveForLoops = 1;
		Resource.limits.whileLoops = 1;
		Resource.limits.doWhileLoops = 1;
		Resource.limits.generalUniformIndexing = 1;
		Resource.limits.generalAttributeMatrixVectorIndexing = 1;
		Resource.limits.generalVaryingIndexing = 1;
		Resource.limits.generalSamplerIndexing = 1;
		Resource.limits.generalVariableIndexing = 1;
		Resource.limits.generalConstantMatrixVectorIndexing = 1;

		return Resource;
	}
}

#endif // PSY_IMPORT_PIPELINE

//////////////////////////////////////////////////////////////////////////
// buildPermutationGLSL
BcBool ScnShaderImport::buildPermutationGLSL( const ScnShaderPermutationJobParams& Params )
{
	BcBool RetVal = BcFalse;
#if PSY_IMPORT_PIPELINE
	BcAssertMsg( RsShaderCodeTypeToBackendType( Params.InputCodeType_ ) == RsShaderBackendType::GLSL ||
		RsShaderCodeTypeToBackendType( Params.InputCodeType_ ) == RsShaderBackendType::GLSL_ES, "Expecting GLSL or GLSL_ES code input." );

	// Initialise GLSLang
	// TODO: Move out of here.
	static bool InitialisedGLSLang = false;
	if( !InitialisedGLSLang )
	{
		ShInitialize();
		InitialisedGLSLang = true;
	}

	// Setup initial header.
	ScnShaderProgramHeader ProgramHeader = {};
	ProgramHeader.ProgramPermutationFlags_ = Params.Permutation_.Flags_;
	ProgramHeader.ShaderFlags_ = ScnShaderPermutationFlags::NONE;
	ProgramHeader.ShaderCodeType_ = Params.OutputCodeType_;
	
	// Vertex attributes needed by GLSL.
	std::vector< RsProgramVertexAttribute > VertexAttributes;
	std::vector< std::string > VertexAttributeNames;

	std::vector< glslang::TShader* > Shaders;
	glslang::TProgram* Program = new glslang::TProgram();

	const char* ShaderDefines[] = 
	{
		"VERTEX_SHADER=1",
		"PIXEL_SHADER=1",
		"HULL_SHADER=1",
		"DOMAIN_SHADER=1",
		"GEOMETRY_SHADER=1",
		"COMPUTE_SHADER=1"
	};

	const EShLanguage Language[] =
	{
		EShLangVertex,
		EShLangFragment,
		EShLangTessControl,
		EShLangTessEvaluation,
		EShLangGeometry,
		EShLangCompute
	};

	if( Params.Entries_.size() == 0 )
	{
		PSY_LOG( "No entries." );
	}

	for( auto& Entry : Params.Entries_ )
	{

		RetVal = BcFalse;
		ScnShaderBuiltData BuiltShader;
		BcBinaryData ByteCode;
		std::vector< std::string > ErrorMessages;
		std::string Entrypoint = Entrypoints_[ Entry.Type_ ];
		BcAssert( !Entrypoint.empty() );

		if( RsShaderCodeTypeToBackendType( Params.OutputCodeType_ ) == RsShaderBackendType::GLSL ||
			RsShaderCodeTypeToBackendType( Params.OutputCodeType_ ) == RsShaderBackendType::GLSL_ES )
		{
			// Preprocess it:
			try
			{
				using lex_iterator_type = boost::wave::cpplexer::lex_iterator< boost::wave::cpplexer::lex_token<> >;
				using context_type = boost::wave::context< 
					std::string::iterator, 
					lex_iterator_type,
					boost::wave::iteration_context_policies::load_file_to_string,
					glsl_directives_hooks >;

				std::string SourceData = Params.ShaderSourceData_;

				glsl_directives_hooks GLSLDirectiveHooks;
				context_type Context( SourceData.begin(), SourceData.end(), Params.ShaderSource_.c_str(), GLSLDirectiveHooks );

				// Add defines.
				for( auto Define : Params.Permutation_.Defines_ )
				{
					Context.add_macro_definition( Define.first + "=" + Define.second );
				}
				auto ShaderDefine = ShaderDefines[ (int)Entry.Type_ ];
				Context.add_macro_definition( ShaderDefine );

				// Add entry point macro.
				Context.add_macro_definition( Entrypoint + "=main" );

				// Setup include paths.
#if PLATFORM_WINDOWS
				Context.add_sysinclude_path( ".\\" );
				Context.add_sysinclude_path( "..\\Psybrus\\Dist\\Content\\Engine\\" );
#elif PLATFORM_LINUX || PLATFORM_OSX
				Context.add_sysinclude_path( "./" );
				Context.add_sysinclude_path( "../Psybrus/Dist/Content/Engine/" );
#else
				BcBreakpoint;
#endif

				// Get preprocessed shader.
				std::string ProcessedSourceData;
				for( auto It = Context.begin(); It != Context.end(); ++It )
				{
					ProcessedSourceData += (*It).get_value().c_str();
				}

				std::regex VertexAttributePattern( 
					".?(in|attribute).*;" );

				std::regex VertexAttributeFullPattern( 
					".?(in|attribute)\\s*(float|vec2|vec2|vec4|int2|int3|int4).*;" );

				std::regex VertexAttributeExtendedPattern( 
					".?(in|attribute)\\s*(float|vec2|vec2|vec4|int2|int3|int4)\\s*([a-zA-Z_][a-zA-Z0-9_]*)\\s*:\\s*([a-zA-Z][a-zA-Z]*)([0-9])?;" );

				std::regex LineDirectivePattern(
					".?#line.*" );

				// If we're parsing a vertex shader, try grab vertex attributes.
				std::istringstream Stream( ProcessedSourceData );
				ProcessedSourceData.clear();
				std::string Line;
				while( std::getline( Stream, Line ) )
				{
					std::cmatch Match;

					if( Entry.Type_ == RsShaderType::VERTEX )
					{
						if( std::regex_match( Line.c_str(), Match, VertexAttributePattern ) )
						{
							if( std::regex_match( Line.c_str(), Match, VertexAttributeExtendedPattern ) )
							{
								const auto Keyword = Match.str( 1 );
								const auto Type = Match.str( 2 );
								const auto Name = Match.str( 3 );
								const auto Semantic = Match.str( 4 );
								const auto Index = Match.str( 5 );

								Line = Keyword + " " + Type + " " + Name + "; // " + Semantic + Index + "\n";
								VertexAttributeNames.emplace_back( Name );

								auto VertexAttr = semanticToVertexAttribute( 
									VertexAttributes.size(),
									Semantic,
									std::atoi( Index.c_str() ) );
								VertexAttributes.emplace_back( VertexAttr );
							}
							else
							{
								throw CsImportException( 
									Params.ShaderSource_.c_str(),
									"Error: Line in shader \"%s\" is missing semantic (in|attribute type name : semantic).",
									Line.c_str() );
							}
						}
					}
					
					if( std::regex_match( Line.c_str(), Match, LineDirectivePattern ) )
					{
						Line = "";
					}

					ProcessedSourceData += Line + "\n";
				}

				// Replace vertex attribute names, and semantic names with our ones.
				for( const auto& Attribute : VertexAttributes )
				{
					const auto Name = VertexAttributeNames[ Attribute.Channel_ ];
					const auto NewName = std::string( "dcl_Input" ) + std::to_string( Attribute.Channel_ );
					boost::replace_all( ProcessedSourceData, Name, NewName );	

					std::string SemanticName;
					switch( Attribute.Usage_ )
					{
					case RsVertexUsage::POSITION:
						SemanticName = "SEMANTIC_POSITION";
						break;
					case RsVertexUsage::BLENDWEIGHTS:
						SemanticName = "SEMANTIC_BLENDWEIGHTS";
						break;
					case RsVertexUsage::BLENDINDICES:
						SemanticName = "SEMANTIC_BLENDINDICES";
						break;
					case RsVertexUsage::NORMAL:
						SemanticName = "SEMANTIC_NORMAL";
						break;
					case RsVertexUsage::PSIZE:
						SemanticName = "SEMANTIC_PSIZE";
						break;
					case RsVertexUsage::TEXCOORD:
						SemanticName = "SEMANTIC_TEXCOORD";
						break;
					case RsVertexUsage::TANGENT:
						SemanticName = "SEMANTIC_TANGENT";
						break;
					case RsVertexUsage::BINORMAL:
						SemanticName = "SEMANTIC_BINORMAL";
						break;
					case RsVertexUsage::TESSFACTOR:
						SemanticName = "SEMANTIC_TESSFACTOR";
						break;
					case RsVertexUsage::POSITIONT:
						SemanticName = "SEMANTIC_POSITIONT";
						break;
					case RsVertexUsage::COLOUR:
						SemanticName = "SEMANTIC_COLOUR";
						break;
					case RsVertexUsage::FOG:
						SemanticName = "SEMANTIC_FOG";
						break;
					case RsVertexUsage::DEPTH:
						SemanticName = "SEMANTIC_DEPTH";
						break;
					case RsVertexUsage::SAMPLE:
						SemanticName = "SEMANTIC_SAMPLE";
						break;
					default:
						BcBreakpoint;
					}

					// Replaced numbered first.
					BcChar NumberedSemanticName[ 128 ] = { 0 };
					BcSPrintf( NumberedSemanticName, sizeof( NumberedSemanticName ) - 1, 
						"%s%u", SemanticName.c_str(), Attribute.UsageIdx_ );
					boost::replace_all( ProcessedSourceData, NumberedSemanticName, NewName );
					// Replace 0 case.
					if( Attribute.UsageIdx_ == 0 )
					{
						boost::replace_all( ProcessedSourceData, SemanticName, NewName );	
					}
				}

				// Run glsl-optimizer for ES (Only GLSL_ES_100)
				if( Params.InputCodeType_ == RsShaderCodeType::GLSL_ES_100 )
				{
					auto GlslOptContext = glslopt_initialize( kGlslTargetOpenGLES20 );
					glslopt_set_max_unroll_iterations( GlslOptContext, 32 );

					// Vertex shader attributes.
					glslopt_shader_type GlslOptLanguage;

					if( Entry.Type_ == RsShaderType::VERTEX )
					{
						GlslOptLanguage = kGlslOptShaderVertex;
					}
					else if( Entry.Type_ == RsShaderType::PIXEL )
					{
						GlslOptLanguage = kGlslOptShaderFragment;
					}
					else
					{
						PSY_LOG( "Error: Unsupported shader type for glsl-optimizer" );
						return BcFalse;
					}

					auto GlslOptShader = glslopt_optimize( 
						GlslOptContext, 
						GlslOptLanguage,
						ProcessedSourceData.c_str(),
						0 );

					if( glslopt_get_status( GlslOptShader ) )
					{
						// Extract shader code.
						ProcessedSourceData = glslopt_get_output( GlslOptShader );
					}
					else
					{
						PSY_LOG( "Failed to optimiser GLSL shader:\n%s\n", 
							glslopt_get_log( GlslOptShader ) );
						return BcFalse;
					}

					int ApproxMath, ApproxTex, ApproxFlow;
					glslopt_shader_get_stats(
						GlslOptShader,
						&ApproxMath, &ApproxTex, &ApproxFlow );
					PSY_LOG( "glsl-optimizer shader stats (approx): %u math, %u tex, %u flow.",
						ApproxMath, ApproxTex, ApproxFlow );

					glslopt_cleanup( GlslOptContext );
				}

				// Attempt to compile.
				TBuiltInResource Resources = GetDefaultResource();

				const char* ShaderSources[] = { ProcessedSourceData.c_str() };
				auto Shader = new glslang::TShader( Language[ (int)Entry.Type_ ] );

				Shader->setStrings( ShaderSources, 1 );

				if( !Shader->parse( &Resources, 100, false, EShMsgDefault ) )
				{
					PSY_LOG( "Compile errors:" );
					std::istringstream Stream( Shader->getInfoLog() );
					std::string Line;
					while( std::getline( Stream, Line ) )
					{
						PSY_LOG( "%s", Line.c_str() );
					}

					{
						logSource( ProcessedSourceData );
					}
					RetVal = BcFalse;
				}
				else
				{
					Shaders.push_back( Shader );
					Program->addShader( Shader );

					// TODO: glslang/glsl-optimizer
					BuiltShader.ShaderType_ = Entry.Type_;
					BuiltShader.CodeType_ = Params.OutputCodeType_;
					BuiltShader.Code_ = std::move( BcBinaryData( (void*)ProcessedSourceData.c_str(), ProcessedSourceData.size() + 1, BcTrue ) );
					BuiltShader.Hash_ = generateShaderHash( BuiltShader );
					RetVal = BcTrue;
				}
			}
			catch ( const boost::wave::preprocess_exception& Exception )
			{
				PSY_LOG( "%s: %s", Exception.what(), Exception.description() );
			}
			catch ( const boost::wave::cpplexer::lexing_exception& Exception )
			{
				PSY_LOG( "%s: %s", Exception.what(), Exception.description() );
			}
			catch( const CsImportException& Exception )
			{
				PSY_LOG( "%s", Exception.what() );
			}
		}
		else
		{
			PSY_LOG( "ERROR: Incorrect backend." );
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
		else
		{
			PSY_LOG( "ERROR: Failed to build shader." );
		}
	}

	// Test linkage if compilation was successful.
	if( RetVal != BcFalse )
	{
		if( Program->link( EShMsgDefault ) )
		{
			Program->buildReflection();
			//Program->dumpReflection();
		}
		else
		{
			PSY_LOG( "Link errors:" );
			std::istringstream Stream( Program->getInfoLog() );
			std::string Line;
			while( std::getline( Stream, Line ) )
			{
				PSY_LOG( "%s", Line.c_str() );
			}
			RetVal = BcFalse;
		}
	}

	// Write out all shaders and programs.
	if( RetVal != BcFalse )
	{
		std::lock_guard< std::mutex > Lock( BuildingMutex_ );
		if( ProgramHeader.ShaderHashes_[ (BcU32)RsShaderType::VERTEX ] == 0 ||
			ProgramHeader.ShaderHashes_[ (BcU32)RsShaderType::PIXEL ] == 0 )
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

	// Clean up.
	for( auto Shader : Shaders )
	{
		delete Shader;
	}
	delete Program;

	--PendingPermutations_;
#endif
	return RetVal;
}


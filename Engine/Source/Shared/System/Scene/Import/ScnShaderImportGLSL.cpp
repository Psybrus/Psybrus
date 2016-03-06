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

#include "System/Renderer/GL/RsProgramFileDataGL.h"

#include "System/SysKernel.h"

#include "Base/BcLinearAllocator.h"
#include "Base/BcMath.h"
#include "Base/BcStream.h"

#define EXCLUDE_PSTDINT
#include <glsl/glsl_optimizer.h>
#include <ShaderLang.h>
#include <GlslangToSpv.h>
#include <GL/glew.h>
#include <disassemble.h>
#include <SPVRemapper.h>
#include "../MachineIndependent/localintermediate.h"
#include "../MachineIndependent/gl_types.h"

extern "C"
{
	#include "cppdef.h"
	#include "fpp.h"
}

#if PLATFORM_WINDOWS
#pragma warning ( disable : 4512 ) // Can't generate assignment operator (for boost)
#endif

#include <bitset>
#include <regex>
#include <sstream>
#include <algorithm>

#include <filesystem>
namespace std { namespace filesystem { using namespace std::experimental::filesystem; } }

#undef DOMAIN // This is defined somewhere in a core header.

namespace
{
	class FCPPInterface
	{
	public:
		FCPPInterface( ScnShaderImport& Importer, const char* InputFile, const char* InputData ):
			Importer_( Importer ),
			Allocator_( 256 * 1024 )
		{
			if( strstr( InputFile, "compute.glsl" ) )
			{
				int a = 0;
				a++;
				printf("");
			}

			fppTag Tag;
			Tag.tag = FPPTAG_USERDATA;
			Tag.data = this;
			Tags_.push_back( Tag );

			Tag.tag = FPPTAG_ERROR;
			Tag.data = (void*)cbError;
			Tags_.push_back( Tag );

			Tag.tag = FPPTAG_INPUT;
			Tag.data = (void*)cbInput;
			Tags_.push_back( Tag );

			Tag.tag = FPPTAG_OUTPUT;
			Tag.data = (void*)cbOutput;
			Tags_.push_back( Tag );

			Tag.tag = FPPTAG_DEPENDENCY;
			Tag.data = (void*)cbDependency;
			Tags_.push_back( Tag );

			Tag.tag = FPPTAG_IGNOREVERSION;
			Tag.data = (void*)0;
			Tags_.push_back( Tag );

			Tag.tag = FPPTAG_LINE;
			Tag.data = (void*)0;
			Tags_.push_back( Tag );

			Tag.tag = FPPTAG_KEEPCOMMENTS;
			Tag.data = (void*)0;
			Tags_.push_back( Tag );

			Tag.tag = FPPTAG_INPUT_NAME;
			Tag.data = (void*)InputFile;
			Tags_.push_back( Tag );

			// Unix line endings.
			auto FixedInputData  = BcStrReplace( InputData, "\r\n", "\n" );
			InputOffset_ = 0;
			InputSize_ = FixedInputData.size() + 1;
			InputData_ = Allocator_.allocate< char >( InputSize_ );
			memset( InputData_, 0, InputSize_ );
			BcStrCopy( InputData_, InputSize_, FixedInputData.c_str() );
		}

		~FCPPInterface()
		{
		}

		void addInclude( const char* IncludePath ) 
		{
			auto Size = BcStrLength( IncludePath ) + 1;
			char* Data = Allocator_.allocate< char >( Size );
			BcStrCopy( Data, Size, IncludePath );

			fppTag Tag;
			Tag.tag = FPPTAG_INCLUDE_DIR;
			Tag.data = Data;
			Tags_.push_back( Tag );
		}

		void addDefine( const char* Define, const char* Value ) 
		{
			auto Size = BcStrLength( Define ) + 1;
			if( Value )
			{
				Size += BcStrLength( Value ) + 1;
			}
			char* Data = Allocator_.allocate< char >( Size );
			memset( Data, 0, Size );
			if( Value )
			{
				BcSPrintf( Data, Size, "%s=%s", Define, Value );
			}
			else
			{
				BcStrCopy( Data, Size, Define );
			}

			fppTag Tag;
			Tag.tag = FPPTAG_DEFINE;
			Tag.data = Data;
			Tags_.push_back( Tag );
		}

		int preprocess()
		{
			fppTag Tag;
			
			Tag.tag = FPPTAG_END;
			Tag.data = (void*)0;
			Tags_.push_back( Tag );

			int Result = fppPreProcess( Tags_.data() );

			// Patch up pragmas.
			Output_ = BcStrReplace( Output_, "#pragma  version", "#version" );
			Output_ = BcStrReplace( Output_, "#pragma  extension", "#extension" );

			Tags_.pop_back();

			return Result == 0;
		}

		const std::string& getOutput()
		{
			return Output_;
		}

		static void cbError( void* UserData, char* Format, va_list VarArgs )
		{
			FCPPInterface* This = static_cast< FCPPInterface* >( UserData );
			std::array< char, 4096 > ErrorBuffer;
			ErrorBuffer.fill( 0 );
			BcVSPrintf( ErrorBuffer.data(), ErrorBuffer.size() - 1, Format, VarArgs );
			PSY_LOG( "ERROR: %s", ErrorBuffer.data() );
		}

		static char* cbInput( char* Buffer, int Size, void* UserData )
		{
			FCPPInterface* This = static_cast< FCPPInterface* >( UserData );
			int OutIdx = 0;
			char InputChar = This->InputData_[ This->InputOffset_ ];
			while( This->InputOffset_ < This->InputSize_ && OutIdx < ( Size - 1 ) )
			{
				Buffer[ OutIdx ] = InputChar;
				if( InputChar == '\n' || OutIdx == ( Size - 1 ) )
				{
					Buffer[ ++OutIdx ] = '\0';
					This->InputOffset_++;
					return Buffer;
				}
				++OutIdx;
				InputChar = This->InputData_[ ++This->InputOffset_ ];
			}
			return nullptr;
		}

		static void cbOutput( int Char, void* UserData )
		{
			FCPPInterface* This = static_cast< FCPPInterface* >( UserData );
			This->Output_ += Char;
		}

		static void cbDependency( char* Dependency, void* UserData )
		{
			FCPPInterface* This = static_cast< FCPPInterface* >( UserData );
			This->Importer_.addDependency( Dependency );
		}

	private:
		ScnShaderImport& Importer_;
		std::vector< fppTag > Tags_;
		char* InputData_;
		size_t InputOffset_;
		size_t InputSize_;
		BcLinearAllocator Allocator_;
		std::string Output_;
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

	//////////////////////////////////////////////////////////////////////////
	// GLSLReflectionTraverser
	struct GLSLReflection
	{
		struct Object
		{
			std::string Name_ = "<INVALID>";
			int Size_ = -1;
			RsProgramParameterTypeValueGL ParameterType_;
			bool Enabled_ = false;

			bool operator < ( const Object& Other ) const
			{
				return 
					std::make_tuple(  
						ParameterType_.Storage_, /*ParameterType_.Binding_,*/
						Name_, Size_,
						ParameterType_.Coherent_, ParameterType_.Volatile_,
						ParameterType_.Restrict_, ParameterType_.ReadOnly_, 
						ParameterType_.WriteOnly_, ParameterType_.Type_ ) <
					std::make_tuple( 
						Other.ParameterType_.Storage_, /*Other.ParameterType_.Binding_,*/
						Other.Name_, Other.Size_,
						Other.ParameterType_.Coherent_, Other.ParameterType_.Volatile_,
						Other.ParameterType_.Restrict_, Other.ParameterType_.ReadOnly_, 
						Other.ParameterType_.WriteOnly_, Other.ParameterType_.Type_ );
			}
		};

		/**
		 * Add object. Puts in correct list.
		 * @return Binding to assign back to AST (for Vulkan)
 		 */
		BcU32 addObject( Object InObject, glslang::TQualifier* InOutQualifier )
		{	
			BcU32* FoundBinding = nullptr;
			std::vector< Object >* Container = nullptr;
			BcU32 DescriptorSet = 0;
			switch( InObject.ParameterType_.Storage_ )
			{
			case RsProgramParameterStorageGL::UNIFORM:
				Container = &Uniforms_;
				FoundBinding = &UniformBinding_;
				break;
			case RsProgramParameterStorageGL::UNIFORM_BLOCK:
				Container = &UniformBlocks_;
				FoundBinding = &UniformBlockBinding_;
				DescriptorSet = 0;
				break;
			case RsProgramParameterStorageGL::SAMPLER:
				Container = &Samplers_;
				FoundBinding = &SamplerBinding_;
				DescriptorSet = 1;
				break;
			case RsProgramParameterStorageGL::SHADER_STORAGE_BUFFER:
				Container = &Buffers_;
				FoundBinding = &BufferBinding_;
				DescriptorSet = 2;
				break;
			case RsProgramParameterStorageGL::IMAGE:
				Container = &Images_;
				FoundBinding = &ImageBinding_;
				DescriptorSet = 3;
				break;
			default:
				BcBreakpoint;
				break;
			}

			auto FoundIt = std::find_if( Container->begin(), Container->end(),
				[ &InObject ]( const Object& Object )
				{
					return Object.Name_ == InObject.Name_;
				} );
			BcU32 ReturnBinding = *FoundBinding;
			if( FoundIt == Container->end() )
			{
				if( UseFoundBindings_ && InObject.ParameterType_.Binding_ == 0xff )
				{
					InObject.ParameterType_.Binding_ = (*FoundBinding)++;
				}

				Container->emplace_back( InObject );	
			}
			else
			{
				if( UseFoundBindings_ && FoundIt->ParameterType_.Binding_ == 0xff )
				{
					InObject.ParameterType_.Binding_ = (*FoundBinding);
					FoundIt->ParameterType_.Binding_ = (*FoundBinding)++;
				}

				FoundIt->Enabled_ |= UseFoundBindings_;
				ReturnBinding = FoundIt->ParameterType_.Binding_;
			}

			// Setup qualifier descriptor set + layout.
			if( InOutQualifier )
			{
				InOutQualifier->layoutSet = DescriptorSet;
				InOutQualifier->layoutBinding = ReturnBinding;
			}

			return ReturnBinding;
		}

		bool UseFoundBindings_ = false;

		BcU32 UniformBlockBinding_ = 0;
		BcU32 UniformBinding_ = 0;
		BcU32 SamplerBinding_ = 0;
		BcU32 BufferBinding_ = 0;
		BcU32 ImageBinding_ = 0;

		std::vector< Object > UniformBlocks_;
		std::vector< Object > Uniforms_;
		std::vector< Object > Samplers_;
		std::vector< Object > Buffers_;
		std::vector< Object > Images_;
	};

	class GLSLReflectionTraverser : public glslang::TIntermTraverser
	{
	public:
		GLSLReflectionTraverser( const glslang::TIntermediate& Intermediate, GLSLReflection& Reflection ):
			Intermediate_( Intermediate ),
			Reflection_( Reflection )
		{
		}
	
		virtual bool visitAggregate(glslang::TVisit, glslang::TIntermAggregate* Node )
		{
			if ( Node->getOp() == glslang::EOpFunctionCall )
			{
				addFunctionCall( Node );
			}

			return true; // traverse this subtree
		}

		virtual bool visitBinary( glslang::TVisit, glslang::TIntermBinary* Node )
		{
			switch ( Node->getOp() )
			{
			case glslang::EOpIndexDirect:
			case glslang::EOpIndexIndirect:
			case glslang::EOpIndexDirectStruct:
				addDereferencedUniform( Node );
			break;
			default:
			break;
			}

			// still need to visit everything below, which could contain sub-expressions
			// containing different uniforms
			return true;
		}

		virtual void visitSymbol( glslang::TIntermSymbol* Base )
		{
		    if( Base->getQualifier().storage == glslang::EvqUniform )
			{
				GLSLReflection::Object Object;
				Object.ParameterType_.Storage_ = RsProgramParameterStorageGL::UNIFORM;
				Object.ParameterType_.Binding_ = Base->getQualifier().layoutBinding;
				Object.ParameterType_.Coherent_ = Base->getQualifier().coherent;
				Object.ParameterType_.Volatile_ = Base->getQualifier().volatil;
				Object.ParameterType_.Restrict_ = Base->getQualifier().restrict;
				Object.ParameterType_.ReadOnly_ = Base->getQualifier().readonly;
				Object.ParameterType_.WriteOnly_ = Base->getQualifier().writeonly;

				switch( Base->getBasicType() )
				{
				case glslang::EbtSampler:
					{
						auto Sampler = Base->getType().getSampler();
						Object.ParameterType_.Type_ = mapSamplerType( Sampler );
						Object.Name_ = Base->getName().c_str();
						if( Sampler.image )
						{
							Object.ParameterType_.Storage_ = RsProgramParameterStorageGL::IMAGE;
							Reflection_.addObject( Object, &Base->getQualifier() );
						}
						else
						{
							Object.ParameterType_.Storage_ = RsProgramParameterStorageGL::SAMPLER;
							Reflection_.addObject( Object, &Base->getQualifier() );
						}
					}
					break;
				case glslang::EbtBlock:
					Object.ParameterType_.Storage_ = RsProgramParameterStorageGL::UNIFORM_BLOCK;
					Object.ParameterType_.Type_ = 0;
					Object.Name_ = Base->getType().getTypeName().c_str();
					Object.Size_ = getBlockSize( Base->getType() );
					Reflection_.addObject( Object, &Base->getQualifier() );
					break;
				default:
					Object.ParameterType_.Storage_ = RsProgramParameterStorageGL::UNIFORM;
					Object.ParameterType_.Type_ = mapUniformType( Base->getType() );
					Object.Name_ = Base->getName().c_str();
					Intermediate_.getBaseAlignment( Base->getType(), Object.Size_, true );
					Reflection_.addObject( Object, &Base->getQualifier() );
					break;
				}
			}
		    else if( Base->getQualifier().storage == glslang::EvqBuffer )
			{
				GLSLReflection::Object Object;
				Object.ParameterType_.Storage_ = RsProgramParameterStorageGL::SHADER_STORAGE_BUFFER;
				Object.ParameterType_.Binding_ = Base->getQualifier().layoutBinding;
				Object.ParameterType_.Coherent_ = Base->getQualifier().coherent;
				Object.ParameterType_.Volatile_ = Base->getQualifier().volatil;
				Object.ParameterType_.Restrict_ = Base->getQualifier().restrict;
				Object.ParameterType_.ReadOnly_ = Base->getQualifier().readonly;
				Object.ParameterType_.WriteOnly_ = Base->getQualifier().writeonly;

				switch( Base->getBasicType() )
				{
				case glslang::EbtBlock:
					Object.ParameterType_.Type_ = 0;
					Object.Name_ = Base->getType().getTypeName().c_str();
					Object.Size_ = getBlockSize( Base->getType() );
					Reflection_.addObject( Object, &Base->getQualifier() );
					break;
				default:
					BcBreakpoint;
					break;
				}
			}
		}

		virtual bool visitSelection( glslang::TVisit, glslang::TIntermSelection* Node )
		{
			glslang::TIntermConstantUnion* Constant = Node->getCondition()->getAsConstantUnion();
			if( Constant )
			{
				// cull the path that is dead
				if( Constant->getConstArray()[0].getBConst() == true && Node->getTrueBlock() )
				{
					Node->getTrueBlock()->traverse( this );
				}
				if( Constant->getConstArray()[0].getBConst() == false && Node->getFalseBlock() )
				{
					Node->getFalseBlock()->traverse( this );
				}
				return false; // don't traverse any more, we did it all above
			}
			else
			{
				return true; // traverse the whole subtree
			}
		}

		// Lookup or calculate the offset of a block member, using the recursively
		// defined block offset rules.
		int getOffset( const glslang::TType& Type, int Index )
		{
			const glslang::TTypeList& MemberList = *Type.getStruct();

			// Don't calculate offset if one is present, it could be user supplied
			// and different than what would be calculated.  That is, this is faster,
			// but not just an optimization.
			if( MemberList[ Index ].type->getQualifier().hasOffset() )
			{
				return MemberList[ Index ].type->getQualifier().layoutOffset;
			}

			int MemberSize = 0;
			int Offset = 0;
			for( int MemberIdx = 0; MemberIdx <= Index; ++MemberIdx )
			{
				int MemberAlignment = Intermediate_.getBaseAlignment( *MemberList[ MemberIdx ].type, MemberSize, Type.getQualifier().layoutPacking == glslang::ElpStd140 );
				Offset = BcPotRoundUp( Offset, MemberAlignment );
				if( MemberIdx < Index )
				{
					Offset += MemberSize;
				}
			}

			return Offset;
		}

		// Calculate the block data size.
		// Block arrayness is not taken into account, each element is backed by a separate buffer.
		int getBlockSize( const glslang::TType& BlockType )
		{
			const glslang::TTypeList& MemberList = *BlockType.getStruct();
			int LastIndex = (int)MemberList.size() - 1;
			int LastOffset = getOffset( BlockType, LastIndex );

			int LastMemberSize = 0;
			Intermediate_.getBaseAlignment( *MemberList[ LastIndex ].type, LastMemberSize, BlockType.getQualifier().layoutPacking == glslang::ElpStd140 );

			return LastOffset + LastMemberSize;
		}

		// Convert sampler type into GL sampler type.
		int mapSamplerType( const glslang::TSampler& sampler )
		{
			if( !sampler.image )
			{
				// a sampler...
				switch( sampler.type )
				{
				case glslang::EbtFloat:
					switch( (int)sampler.dim )
					{
					case glslang::Esd1D:
						switch( (int)sampler.shadow )
						{
						case false: return sampler.arrayed ? GL_SAMPLER_1D_ARRAY : GL_SAMPLER_1D;
						case true:  return sampler.arrayed ? GL_SAMPLER_1D_ARRAY_SHADOW : GL_SAMPLER_1D_SHADOW;
						}
					case glslang::Esd2D:
						switch( (int)sampler.ms )
						{
						case false:
							switch ((int)sampler.shadow)
							{
							case false: return sampler.arrayed ? GL_SAMPLER_2D_ARRAY : GL_SAMPLER_2D;
							case true:  return sampler.arrayed ? GL_SAMPLER_2D_ARRAY_SHADOW : GL_SAMPLER_2D_SHADOW;
							}
						case true:      return sampler.arrayed ? GL_SAMPLER_2D_MULTISAMPLE_ARRAY : GL_SAMPLER_2D_MULTISAMPLE;
						}
					case glslang::Esd3D:
						return GL_SAMPLER_3D;
					case glslang::EsdCube:
						switch( (int)sampler.shadow )
						{
						case false: return sampler.arrayed ? GL_SAMPLER_CUBE_MAP_ARRAY : GL_SAMPLER_CUBE;
						case true:  return sampler.arrayed ? GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW : GL_SAMPLER_CUBE_SHADOW;
						}
					case glslang::EsdRect:
						return sampler.shadow ? GL_SAMPLER_2D_RECT_SHADOW : GL_SAMPLER_2D_RECT;
					case glslang::EsdBuffer:
						return GL_SAMPLER_BUFFER;
					}
				case glslang::EbtInt:
					switch( (int)sampler.dim )
					{
					case glslang::Esd1D:
						return sampler.arrayed ? GL_INT_SAMPLER_1D_ARRAY : GL_INT_SAMPLER_1D;
					case glslang::Esd2D:
						switch( (int)sampler.ms )
						{
						case false:  return sampler.arrayed ? GL_INT_SAMPLER_2D_ARRAY : GL_INT_SAMPLER_2D;
						case true:   return sampler.arrayed ? GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY : GL_INT_SAMPLER_2D_MULTISAMPLE;
						}
					case glslang::Esd3D:
						return GL_INT_SAMPLER_3D;
					case glslang::EsdCube:
						return sampler.arrayed ? GL_INT_SAMPLER_CUBE_MAP_ARRAY : GL_INT_SAMPLER_CUBE;
					case glslang::EsdRect:
						return GL_INT_SAMPLER_2D_RECT;
					case glslang::EsdBuffer:
						return GL_INT_SAMPLER_BUFFER;
					}
				case glslang::EbtUint:
					switch( (int)sampler.dim ) 
					{
					case glslang::Esd1D:
						return sampler.arrayed ? GL_UNSIGNED_INT_SAMPLER_1D_ARRAY : GL_UNSIGNED_INT_SAMPLER_1D;
					case glslang::Esd2D:
						switch( (int)sampler.ms )
						{
						case false:  return sampler.arrayed ? GL_UNSIGNED_INT_SAMPLER_2D_ARRAY : GL_UNSIGNED_INT_SAMPLER_2D;
						case true:   return sampler.arrayed ? GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY : GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE;
						}
					case glslang::Esd3D:
						return GL_UNSIGNED_INT_SAMPLER_3D;
					case glslang::EsdCube:
						return sampler.arrayed ? GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY : GL_UNSIGNED_INT_SAMPLER_CUBE;
					case glslang::EsdRect:
						return GL_UNSIGNED_INT_SAMPLER_2D_RECT;
					case glslang::EsdBuffer:
						return GL_UNSIGNED_INT_SAMPLER_BUFFER;
					}
				default:
					return 0;
				}
			}
			else
			{
				// an image...
				switch( sampler.type )
				{
				case glslang::EbtFloat:
					switch( (int)sampler.dim )
					{
					case glslang::Esd1D:
						return sampler.arrayed ? GL_IMAGE_1D_ARRAY : GL_IMAGE_1D;
					case glslang::Esd2D:
						switch( (int)sampler.ms )
						{
						case false:     return sampler.arrayed ? GL_IMAGE_2D_ARRAY : GL_IMAGE_2D;
						case true:      return sampler.arrayed ? GL_IMAGE_2D_MULTISAMPLE_ARRAY : GL_IMAGE_2D_MULTISAMPLE;
						}
					case glslang::Esd3D:
						return GL_IMAGE_3D;
					case glslang::EsdCube:
						return sampler.arrayed ? GL_IMAGE_CUBE_MAP_ARRAY : GL_IMAGE_CUBE;
					case glslang::EsdRect:
						return GL_IMAGE_2D_RECT;
					case glslang::EsdBuffer:
						return GL_IMAGE_BUFFER;
					}
				case glslang::EbtInt:
					switch( (int)sampler.dim)
					{
					case glslang::Esd1D:
						return sampler.arrayed ? GL_INT_IMAGE_1D_ARRAY : GL_INT_IMAGE_1D;
					case glslang::Esd2D:
						switch( (int)sampler.ms )
						{
						case false:  return sampler.arrayed ? GL_INT_IMAGE_2D_ARRAY : GL_INT_IMAGE_2D;
						case true:   return sampler.arrayed ? GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY : GL_INT_IMAGE_2D_MULTISAMPLE;
						}
					case glslang::Esd3D:
						return GL_INT_IMAGE_3D;
					case glslang::EsdCube:
						return sampler.arrayed ? GL_INT_IMAGE_CUBE_MAP_ARRAY : GL_INT_IMAGE_CUBE;
					case glslang::EsdRect:
						return GL_INT_IMAGE_2D_RECT;
					case glslang::EsdBuffer:
						return GL_INT_IMAGE_BUFFER;
					}
				case glslang::EbtUint:
					switch( (int)sampler.dim )
					{
					case glslang::Esd1D:
						return sampler.arrayed ? GL_UNSIGNED_INT_IMAGE_1D_ARRAY : GL_UNSIGNED_INT_IMAGE_1D;
					case glslang::Esd2D:
						switch( (int)sampler.ms )
						{
						case false:  return sampler.arrayed ? GL_UNSIGNED_INT_IMAGE_2D_ARRAY : GL_UNSIGNED_INT_IMAGE_2D;
						case true:   return sampler.arrayed ? GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY : GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE;
						}
					case glslang::Esd3D:
						return GL_UNSIGNED_INT_IMAGE_3D;
					case glslang::EsdCube:
						return sampler.arrayed ? GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY : GL_UNSIGNED_INT_IMAGE_CUBE;
					case glslang::EsdRect:
						return GL_UNSIGNED_INT_IMAGE_2D_RECT;
					case glslang::EsdBuffer:
						return GL_UNSIGNED_INT_IMAGE_BUFFER;
					}
				default:
					return 0;
				}
			}
			return 0;
		}

		// Convert uniform type into GL uniform type.
		int mapUniformType( const glslang::TType& Type )
		{
			if( Type.isVector() )
			{
				int Offset = Type.getVectorSize() - 2;
				switch( Type.getBasicType() )
				{
				case glslang::EbtFloat:      return GL_FLOAT_VEC2                  + Offset;
				case glslang::EbtDouble:     return GL_DOUBLE_VEC2                 + Offset;
				case glslang::EbtInt:        return GL_INT_VEC2                    + Offset;
				case glslang::EbtUint:       return GL_UNSIGNED_INT_VEC2           + Offset;
				case glslang::EbtBool:       return GL_BOOL_VEC2                   + Offset;
				case glslang::EbtAtomicUint: return GL_UNSIGNED_INT_ATOMIC_COUNTER + Offset;
				default:                     return 0;
				}
			}
			if( Type.isMatrix() )
			{
				switch( Type.getBasicType() )
				{
				case glslang::EbtFloat:
					switch( Type.getMatrixCols() )
					{
					case 2:
						switch( Type.getMatrixRows() )
						{
						case 2:    return GL_FLOAT_MAT2;
						case 3:    return GL_FLOAT_MAT2x3;
						case 4:    return GL_FLOAT_MAT2x4;
						default:   return 0;
						}
					case 3:
						switch( Type.getMatrixRows() )
						{
						case 2:    return GL_FLOAT_MAT3x2;
						case 3:    return GL_FLOAT_MAT3;
						case 4:    return GL_FLOAT_MAT3x4;
						default:   return 0;
						}
					case 4:
						switch( Type.getMatrixRows() )
						{
						case 2:    return GL_FLOAT_MAT4x2;
						case 3:    return GL_FLOAT_MAT4x3;
						case 4:    return GL_FLOAT_MAT4;
						default:   return 0;
						}
					}
				case glslang::EbtDouble:
					switch( Type.getMatrixCols() )
					{
					case 2:
						switch( Type.getMatrixRows() )
						{
						case 2:    return GL_DOUBLE_MAT2;
						case 3:    return GL_DOUBLE_MAT2x3;
						case 4:    return GL_DOUBLE_MAT2x4;
						default:   return 0;
						}
					case 3:
						switch( Type.getMatrixRows() )
						{
						case 2:    return GL_DOUBLE_MAT3x2;
						case 3:    return GL_DOUBLE_MAT3;
						case 4:    return GL_DOUBLE_MAT3x4;
						default:   return 0;
						}
					case 4:
						switch( Type.getMatrixRows() )
						{
						case 2:    return GL_DOUBLE_MAT4x2;
						case 3:    return GL_DOUBLE_MAT4x3;
						case 4:    return GL_DOUBLE_MAT4;
						default:   return 0;
						}
					}
				default:
					return 0;
				}
			}
			if( Type.getVectorSize() == 1 )
			{
				switch( Type.getBasicType() )
				{
				case glslang::EbtFloat:      return GL_FLOAT;
				case glslang::EbtDouble:     return GL_DOUBLE;
				case glslang::EbtInt:        return GL_INT;
				case glslang::EbtUint:       return GL_UNSIGNED_INT;
				case glslang::EbtBool:       return GL_BOOL;
				case glslang::EbtAtomicUint: return GL_UNSIGNED_INT_ATOMIC_COUNTER;
				default:                     return 0;
				}
			}
			return 0;
		}
	
		void addFunctionCall( glslang::TIntermAggregate* Node )
		{
			if( std::find( FunctionStack_.begin(), FunctionStack_.end(), Node ) == FunctionStack_.end() )
			{
				pushFunction( Node->getName().c_str() );
			}
		}

		void addDereferencedUniform( glslang::TIntermBinary* TopNode )
		{
		}

		void pushFunction( const char* Name )
		{
			glslang::TIntermSequence& Globals = Intermediate_.getTreeRoot()->getAsAggregate()->getSequence();
			for( unsigned int Idx = 0; Idx < Globals.size(); ++Idx )
			{
				glslang::TIntermAggregate* Candidate = Globals[ Idx ]->getAsAggregate();
				if( Candidate && Candidate->getOp() == glslang::EOpFunction && Candidate->getName() == Name )
				{
					FunctionStack_.emplace_back( Candidate );
					break;
				}
			}
		}

		void traverse()
		{
			Reflection_.UseFoundBindings_ = false;
			Intermediate_.getTreeRoot()->traverse( this );
			FunctionStack_.clear();

			Reflection_.UseFoundBindings_ = true;
			pushFunction( "main(" );

		    // process all the functions
			while( !FunctionStack_.empty() )
			{
				TIntermNode* Function = FunctionStack_.back();
				FunctionStack_.pop_back();
				Function->traverse( this );
			}
		}

	private:
		const glslang::TIntermediate& Intermediate_;
		GLSLReflection& Reflection_;

		typedef std::vector< glslang::TIntermAggregate* > FunctionStack;
		FunctionStack FunctionStack_;
	};

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
	ScnShaderProgramHeader ProgramHeaderGLSL = {};
	ProgramHeaderGLSL.ProgramPermutationFlags_ = Params.Permutation_.Flags_;
	ProgramHeaderGLSL.ShaderFlags_ = ScnShaderPermutationFlags::NONE;
	ProgramHeaderGLSL.ShaderCodeType_ = Params.OutputCodeType_;

	// Should we build SPIRV from the GLSL? Use highest GLSL version.
	RsShaderCodeType HighestGLSLVersionForSPIRV = RsShaderCodeType::GLSL_430;
	for( const auto& Source : Sources_ )
	{
		if( RsShaderCodeTypeToBackendType( Source.first ) == RsShaderBackendType::GLSL )
		{
			if( Source.first >= HighestGLSLVersionForSPIRV )
			{
				HighestGLSLVersionForSPIRV = Source.first;
			}
		}
	}
	bool BuildSPIRV = RsShaderCodeTypeToBackendType( Params.OutputCodeType_ ) == RsShaderBackendType::GLSL &&
			Params.OutputCodeType_ >= HighestGLSLVersionForSPIRV;

	ScnShaderProgramHeader ProgramHeaderSPIRV = {};

	// Vertex attributes, uniforms, and uniform blocks.
	std::vector< std::string > VertexAttributeNames;
	RsProgramVertexAttributeList VertexAttributes;
	RsProgramParameterList Parameters;

	std::vector< glslang::TShader* > Shaders;
	glslang::TProgram* Program = new glslang::TProgram();

	const char* ShaderDefines[] = 
	{
		"VERTEX_SHADER",
		"PIXEL_SHADER",
		"HULL_SHADER",
		"DOMAIN_SHADER",
		"GEOMETRY_SHADER",
		"COMPUTE_SHADER"
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
		ScnShaderBuiltData BuiltShaderGLSL;
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
				FCPPInterface Preprocessor( *this, Params.ShaderSource_.c_str(), Params.ShaderSourceData_.c_str() );
				// Add defines.
				for( auto Define : Params.Permutation_.Defines_ )
				{
					Preprocessor.addDefine( Define.first.c_str(), Define.second.c_str() );
				}
				auto ShaderDefine = ShaderDefines[ (int)Entry.Type_ ];
				Preprocessor.addDefine( ShaderDefine, "1" );

				// Add entry point macro.
				if( Entrypoint != "main" )
				{
					Preprocessor.addDefine( Entrypoint.c_str(), "main" );
				}

				// Setup include paths.
				Preprocessor.addInclude( "." );
				Preprocessor.addInclude( "../Psybrus/Dist/Content/Engine" );

				Preprocessor.preprocess();
				std::string ProcessedSourceData = Preprocessor.getOutput();

				// Parse inputs + outputs.
				std::regex VertexAttributePattern( 
					"\\s*(in|attribute)\\s{1,}.*;.*" );

				std::regex VertexAttributeFullPattern( 
					"\\s*(in|attribute)\\s{1,}(float|vec2|vec2|vec4|int|ivec2|ivec3|ivec4).*;.*" );

				std::regex VertexAttributeExtendedPattern( 
					"\\s*(in|attribute)\\s{1,}(float|vec2|vec3|vec4|int|ivec2|ivec3|ivec4)\\s{1,}([a-zA-Z_][a-zA-Z0-9_]*)\\s{1,}:\\s{1,}([a-zA-Z][a-zA-Z]*)([0-9])?.*;.*" );

				std::regex VertexAttributeLayoutPattern(
					"\\s*layout\\(location=(.*)\\)\\s{1,}(in|attribute)\\s{1,}(float|vec2|vec2|vec4|int2|int3|int4)\\s{1,}([a-zA-Z_][a-zA-Z0-9_]*)\\s{1,}:\\s{1,}([a-zA-Z][a-zA-Z]*)([0-9])?.*" );

				std::regex LineDirectivePattern(
					"\\s*#line\\s{1,}.*" );
				
				std::regex InOutAttributePattern( 
					"\\s*(in|out)\\s{1,}.*" );

				std::regex InOutAttributeFullPattern( 
					"\\s*(in|out)\\s{1,}(float|vec2|vec2|vec4|int|ivec2|ivec3|ivec4).*;.*" );

				std::regex InOutAttributeExtendedPattern( 
					"\\s*(in|out)\\s{1,}(float|vec2|vec3|vec4|int|ivec2|ivec3|ivec4)\\s{1,}([a-zA-Z_][a-zA-Z0-9_]*)\\s{1,}:\\s{1,}([a-zA-Z][a-zA-Z]*)([0-9])?.*;.*" );

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
								auto Keyword = Match.str( 1 );
								auto Type = Match.str( 2 );
								const auto Name = Match.str( 3 );
								const auto Semantic = Match.str( 4 );
								const auto Index = Match.str( 5 );

								if( Params.InputCodeType_ == RsShaderCodeType::GLSL_ES_100 )
								{
									Keyword = "attribute";
								}

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
						else if( std::regex_match( Line.c_str(), Match, VertexAttributeLayoutPattern ) )
						{
							const auto Location = Match.str( 1 );
							const auto Keyword = Match.str( 2 );
							const auto Type = Match.str( 3 );
							const auto Name = Match.str( 4 );
							const auto Semantic = Match.str( 5 );
							const auto Index = Match.str( 6 );

							Line = "layout(location=" + Location + ") " + Keyword + " " + Type + " " + Name + "; // " + Semantic + Index + "\n";
							VertexAttributeNames.emplace_back( Name );

							auto VertexAttr = semanticToVertexAttribute( 
								VertexAttributes.size(),
								Semantic,
								std::atoi( Index.c_str() ) );
							VertexAttributes.emplace_back( VertexAttr );
						}
						else if( std::regex_match( Line.c_str(), Match, InOutAttributePattern ) )
						{
							if( std::regex_match( Line.c_str(), Match, InOutAttributeExtendedPattern ) )
							{
								auto Keyword = Match.str( 1 );
								const auto Type = Match.str( 2 );
								const auto Name = Match.str( 3 );
								const auto Semantic = Match.str( 4 );
								const auto Index = Match.str( 5 );

								if( Params.InputCodeType_ == RsShaderCodeType::GLSL_ES_100 )
								{
									Keyword = "varying";
								}
								
								Line = Keyword + " " + Type + " " + Name + "; // " + Semantic + Index + "\n";
							}
						}
					}
					else
					{
						if( std::regex_match( Line.c_str(), Match, InOutAttributePattern ) )
						{
							if( std::regex_match( Line.c_str(), Match, InOutAttributeExtendedPattern ) )
							{
								auto Keyword = Match.str( 1 );
								const auto Type = Match.str( 2 );
								const auto Name = Match.str( 3 );
								const auto Semantic = Match.str( 4 );
								const auto Index = Match.str( 5 );

								if( Params.InputCodeType_ == RsShaderCodeType::GLSL_ES_100 )
								{
									Keyword = "varying";
								}

								Line = Keyword + " " + Type + " " + Name + "; // " + Semantic + Index + "\n";
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
					ProcessedSourceData = BcStrReplace( ProcessedSourceData, Name, NewName );	

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
					ProcessedSourceData = BcStrReplace( ProcessedSourceData, NumberedSemanticName, NewName );
					// Replace 0 case.
					if( Attribute.UsageIdx_ == 0 )
					{
						ProcessedSourceData = BcStrReplace( ProcessedSourceData, SemanticName, NewName );	
					}
				}

				// Run glsl-optimizer for ES (Only GLSL_ES_100)
				if( Params.InputCodeType_ == RsShaderCodeType::GLSL_ES_100 )
				{
					// HACK: glsl-optimizer is not thread safe.
					static std::mutex Mutex;
					std::lock_guard< std::mutex > Lock( Mutex );

					auto GlslOptContext = glslopt_initialize( kGlslTargetOpenGLES20 );
					glslopt_set_max_unroll_iterations( GlslOptContext, 64 );

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
						GotErrorBuilding_++;
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
						logSource( ProcessedSourceData );
						RetVal = BcFalse;
					}

#if PSY_DEBUG
					int ApproxMath, ApproxTex, ApproxFlow;
					glslopt_shader_get_stats(
						GlslOptShader,
						&ApproxMath, &ApproxTex, &ApproxFlow );
					PSY_LOG( "glsl-optimizer shader stats (approx): %u math, %u tex, %u flow.",
						ApproxMath, ApproxTex, ApproxFlow );
#endif

					glslopt_cleanup( GlslOptContext );

					// Prefix extension for draw buffers after #version.
					std::stringstream ShaderSourceStream( ProcessedSourceData );
					std::string ShaderSourceLine;
					ProcessedSourceData.clear();
					while( std::getline( ShaderSourceStream, ShaderSourceLine, '\n' ) )
					{
						ProcessedSourceData += ShaderSourceLine + "\n";
						if( ShaderSourceLine.substr( 0, 8 ) == "#version" )
						{
							ProcessedSourceData += "#extension GL_EXT_draw_buffers : enable\n";
						}
					}
				}

				{
					// HACK: glslang is not thread safe.
					static std::mutex Mutex;
					std::lock_guard< std::mutex > Lock( Mutex );

					// Attempt to compile.
					TBuiltInResource Resources = GetDefaultResource();

					const char* ShaderSources[] = { ProcessedSourceData.c_str() };
					glslang::TShader* Shader( new glslang::TShader( Language[ (int)Entry.Type_ ] ) );

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

						logSource( ProcessedSourceData );
						RetVal = BcFalse;
					}
					else
					{
						Shaders.push_back( Shader );
						Program->addShader( Shader );

						// Shader code straight in.
						BuiltShaderGLSL.ShaderType_ = Entry.Type_;
						BuiltShaderGLSL.CodeType_ = Params.OutputCodeType_;
						BuiltShaderGLSL.Code_ = std::move( BcBinaryData( (void*)ProcessedSourceData.c_str(), ProcessedSourceData.size() + 1, BcTrue ) );
						BuiltShaderGLSL.Hash_ = generateShaderHash( BuiltShaderGLSL );
						RetVal = BcTrue;
					}
				}
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
			ProgramHeaderGLSL.NoofVertexAttributes_ = static_cast< BcU32 >( VertexAttributes.size() );
			ProgramHeaderGLSL.ShaderHashes_[ (BcU32)Entry.Type_ ] = BuiltShaderGLSL.Hash_;

			std::lock_guard< std::mutex > Lock( BuildingMutex_ );
			auto FoundShader = BuiltShaderData_.find( BuiltShaderGLSL.Hash_ );
			if( FoundShader != BuiltShaderData_.end() )
			{
				BcAssertMsg( FoundShader->second == BuiltShaderGLSL, "Hash key collision" );
			}

			BuiltShaderData_[ BuiltShaderGLSL.Hash_ ] = std::move( BuiltShaderGLSL );
		}
		else
		{
			PSY_LOG( "ERROR: Failed to build shader." );
		}
	}

	// Test linkage if compilation was successful.
	if( RetVal != BcFalse )
	{
		// HACK: glslang is not thread safe.
		static std::mutex Mutex;
		std::lock_guard< std::mutex > Lock( Mutex );

		if( Program->link( BuildSPIRV ? EShMsgVulkanRules : EShMsgDefault ) )
		{
			// Build reflection info for shader.
			GLSLReflection Reflection;
			for( int Idx = 0; Idx < EShLangCount; ++Idx )
			{
				auto Intermediate = Program->getIntermediate( static_cast< EShLanguage >( Idx ) );
				if( Intermediate )
				{
					GLSLReflectionTraverser Traverser( *Intermediate, Reflection );
					Traverser.traverse();

					// Generate pseudo-uniform blocks from uniforms for GLES.
					for( auto Uniform : Reflection.Uniforms_ )
					{
						// Could be a member of a struct where we don't have uniform buffers.
						// Check the name and work out if it is. If so, add to a map so we can add all afterwards.
						auto VSTypeOffset = Uniform.Name_.find( "VS_" ); 
						auto PSTypeOffset = Uniform.Name_.find( "PS_" );
						if( VSTypeOffset != std::string::npos || PSTypeOffset != std::string::npos )
						{
							GLSLReflection::Object PseudoUniformBlock = Uniform;

							// Terminate.
							if( VSTypeOffset != std::string::npos )
							{
								PseudoUniformBlock.Name_ = Uniform.Name_.substr( 0, VSTypeOffset );
							}
							else if( PSTypeOffset != std::string::npos )
							{
								PseudoUniformBlock.Name_ = Uniform.Name_.substr( 0, PSTypeOffset );
							}

							// Add as uniform buffer object with the names stripped.
							PseudoUniformBlock.ParameterType_.Value_ = 0;
							PseudoUniformBlock.ParameterType_.Storage_ = RsProgramParameterStorageGL::UNIFORM_BLOCK;
							PseudoUniformBlock.ParameterType_.Binding_ = 0xff;
							PseudoUniformBlock.Size_ = 0;
							PseudoUniformBlock.Enabled_ = true;
							
							Reflection.addObject( PseudoUniformBlock, nullptr );
						}
					}
				}
			}

			BcU32 MaxParameters = 0;
			MaxParameters += Reflection.UniformBlocks_.size();
			MaxParameters += Reflection.Uniforms_.size();
			MaxParameters += Reflection.Samplers_.size();
			MaxParameters += Reflection.Buffers_.size();
			MaxParameters += Reflection.Images_.size();

			Parameters.clear();
			Parameters.reserve( MaxParameters );

			for( auto Object : Reflection.UniformBlocks_ )
			{
				if( Object.Enabled_ )
				{
					RsProgramParameter Parameter = {};
					memset( &Parameter, 0, sizeof( Parameter ) );
					BcStrCopy( Parameter.Name_, sizeof( Parameter.Name_ ) - 1, Object.Name_.c_str() );
					Parameter.Type_ = RsProgramParameterType::UNIFORM_BLOCK;
					Parameter.Size_ = Object.Size_;
					// GL specific internals.
					Parameter.InternalType_ = Object.ParameterType_.Value_;
					Parameters.emplace_back( Parameter );
				}
			}

			for( auto Object : Reflection.Uniforms_ )
			{
				if( Object.Enabled_ )
				{
					RsProgramParameter Parameter = {};
					memset( &Parameter, 0, sizeof( Parameter ) );
					BcStrCopy( Parameter.Name_, sizeof( Parameter.Name_ ) - 1, Object.Name_.c_str() );
					Parameter.Type_ = RsProgramParameterType::UNKNOWN;
					Parameter.Size_ = Object.Size_;
					// GL specific internals.
					Parameter.InternalType_ = Object.ParameterType_.Value_;
					Parameters.emplace_back( Parameter );
				}
			}

			for( auto Object : Reflection.Samplers_ )
			{
				if( Object.Enabled_ )
				{
					RsProgramParameter Parameter = {};
					memset( &Parameter, 0, sizeof( Parameter ) );
					BcStrCopy( Parameter.Name_, sizeof( Parameter.Name_ ) - 1, Object.Name_.c_str() );
					Parameter.Type_ = RsProgramParameterType::SAMPLER;
					Parameter.Size_ = Object.Size_;
					// GL specific internals.
					Parameter.InternalType_ = Object.ParameterType_.Value_;
					Parameters.emplace_back( Parameter );
				}
			}

			for( auto Object : Reflection.Buffers_ )
			{
				if( Object.Enabled_ )
				{
					RsProgramParameter Parameter = {};
					memset( &Parameter, 0, sizeof( Parameter ) );
					BcStrCopy( Parameter.Name_, sizeof( Parameter.Name_ ) - 1, Object.Name_.c_str() );
					Parameter.Type_ = !Object.ParameterType_.ReadOnly_ ? RsProgramParameterType::UNORDERED_ACCESS : RsProgramParameterType::SHADER_RESOURCE;
					Parameter.Size_ = Object.Size_;
					// GL specific internals.
					Parameter.InternalType_ = Object.ParameterType_.Value_;
					Parameters.emplace_back( Parameter );
				}
			}

			for( auto Object : Reflection.Images_ )
			{
				if( Object.Enabled_ )
				{
					RsProgramParameter Parameter = {};
					memset( &Parameter, 0, sizeof( Parameter ) );
					BcStrCopy( Parameter.Name_, sizeof( Parameter.Name_ ) - 1, Object.Name_.c_str() );
					Parameter.Type_ = !Object.ParameterType_.ReadOnly_ ? RsProgramParameterType::UNORDERED_ACCESS : RsProgramParameterType::SHADER_RESOURCE;
					Parameter.Size_ = Object.Size_;
					// GL specific internals.
					Parameter.InternalType_ = Object.ParameterType_.Value_;
					Parameters.emplace_back( Parameter );
				}
			}

			ProgramHeaderGLSL.NoofParameters_ = Parameters.size();

			// Should we build SPIR-V?
			if( BuildSPIRV )
			{
				ProgramHeaderSPIRV = ProgramHeaderGLSL;
				ProgramHeaderSPIRV.ShaderCodeType_ = RsShaderCodeType::SPIRV;

				RsShaderType ShaderTypes[] = {
					RsShaderType::VERTEX,
					RsShaderType::HULL,
					RsShaderType::DOMAIN,
					RsShaderType::GEOMETRY,
					RsShaderType::PIXEL,
					RsShaderType::COMPUTE,
				};

				for( size_t Idx = 0; Idx < EShLangCount; ++Idx )
				{
					auto* Intermediate = Program->getIntermediate( static_cast< EShLanguage >( Idx ) );
					if( Intermediate != nullptr )
					{
						std::vector< unsigned int > SpvOutput;
						glslang::GlslangToSpv( *Intermediate, SpvOutput );

						char OutFileName[ 1024 ] = { 0 };
						BcSPrintf( OutFileName, sizeof( OutFileName ) - 1, "%s/%s-%x-%u.spv",
							getIntermediatePath().c_str(), 
							getResourceName().c_str(),
							ProgramHeaderSPIRV.ProgramPermutationFlags_, Idx );
						glslang::OutputSpv( SpvOutput, OutFileName );

#if 0
						spv::spirvbin_t SpvRemapper;
						SpvRemapper.remap( SpvOutput, spv::spirvbin_t::OPT_ALL | spv::spirvbin_t::DCE_ALL );
						BcSPrintf( OutFileName, sizeof( OutFileName ) - 1, "%s/%s-%x-%u.remapped.spv",
							getIntermediatePath().c_str(), 
							getResourceName().c_str(),
							ProgramHeaderSPIRV.ProgramPermutationFlags_, Idx );
						glslang::OutputSpv( SpvOutput, OutFileName );
#endif
						ScnShaderBuiltData BuiltShaderSPIRV;
						BuiltShaderSPIRV.ShaderType_ = ShaderTypes[ Idx ];
						BuiltShaderSPIRV.CodeType_ = RsShaderCodeType::SPIRV;
						BuiltShaderSPIRV.Code_ = std::move( BcBinaryData( (void*)SpvOutput.data(), SpvOutput.size() * sizeof( unsigned int ), BcTrue ) );
						BuiltShaderSPIRV.Hash_ = generateShaderHash( BuiltShaderSPIRV );

						ProgramHeaderSPIRV.ShaderHashes_[ (BcU32)BuiltShaderSPIRV.ShaderType_ ] = BuiltShaderSPIRV.Hash_;

						BuiltShaderData_[ BuiltShaderSPIRV.Hash_ ] = std::move( BuiltShaderSPIRV );
					}
				}
			}					
		}
		else
		{
			PSY_LOG( "Link errors:" );
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
		if( ( ProgramHeaderGLSL.ShaderHashes_[ (BcU32)RsShaderType::VERTEX ] == 0 ||
			  ProgramHeaderGLSL.ShaderHashes_[ (BcU32)RsShaderType::PIXEL ] == 0 ) &&
			( ProgramHeaderGLSL.ShaderHashes_[ (BcU32)RsShaderType::COMPUTE ] == 0 ) )
		{
			PSY_LOG( "No vertex and pixel shaders in program, or no compute." );
			RetVal = BcFalse;
		}

		BuiltProgramData_.push_back( std::move( ProgramHeaderGLSL ) );
		if( VertexAttributes.size() > 0 )
		{
			BuiltVertexAttributes_.push_back( VertexAttributes );
		}
		if( Parameters.size() > 0 )
		{
			BuiltParameters_.push_back( Parameters );
		}

		if( BuildSPIRV )
		{
			if( ( ProgramHeaderSPIRV.ShaderHashes_[ (BcU32)RsShaderType::VERTEX ] == 0 ||
				  ProgramHeaderSPIRV.ShaderHashes_[ (BcU32)RsShaderType::PIXEL ] == 0 ) &&
				( ProgramHeaderSPIRV.ShaderHashes_[ (BcU32)RsShaderType::COMPUTE ] == 0 ) )
			{
				PSY_LOG( "No vertex and pixel shaders in program, or no compute." );
				RetVal = BcFalse;
			}

			BuiltProgramData_.push_back( std::move( ProgramHeaderSPIRV ) );
			if( VertexAttributes.size() > 0 )
			{
				BuiltVertexAttributes_.push_back( VertexAttributes );
			}
			if( Parameters.size() > 0 )
			{
				BuiltParameters_.push_back( Parameters );
			}
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
	delete Program;
	for( auto Shader : Shaders )
	{
		delete Shader;
	}

	--PendingPermutations_;
#endif
	return RetVal;
}


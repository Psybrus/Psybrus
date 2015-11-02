#include "System/Renderer/GL/RsProgramGL.h"
#include "System/Renderer/GL/RsBufferGL.h"
#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsProgram.h"
#include "System/Renderer/RsShader.h"

#include "Base/BcProfiler.h"
#include "Math/MaMat4d.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsProgramGL::RsProgramGL( class RsProgram* Parent, const RsOpenGLVersion& Version ):
	Parent_( Parent ),
	Version_( Version )
{
	const auto& Shaders = Parent_->getShaders();

	// Some checks to ensure validity.
	BcAssert( Shaders.size() > 0 );	

	// Create program.
	Handle_ = GL( CreateProgram() );
	
	// Attach shaders.
	for( auto* Shader : Shaders )
	{
		GL( AttachShader( Handle_, Shader->getHandle< GLuint >() ) );
		
	}
	
	// Bind all slots up.
	// NOTE: We shouldn't need this in later GL versions with explicit
	//       binding slots.
	BcChar ChannelNameChars[ 64 ] = { 0 };
	for( BcU32 Channel = 0; Channel < 16; ++Channel )
	{
		BcSPrintf( ChannelNameChars, sizeof( ChannelNameChars ) - 1, "dcl_Input%u", Channel );
		GL( BindAttribLocation( Handle_, Channel, ChannelNameChars ) );
		
	}
	
	// Link program.
	GL( LinkProgram( Handle_ ) );
	

	GLint ProgramLinked = 0;
	GL( GetProgramiv( Handle_, GL_LINK_STATUS, &ProgramLinked ) );
	if ( !ProgramLinked )
	{					 
		// There was an error here, first get the length of the log message.
		int i32InfoLogLength, i32CharsWritten; 
		GL( GetProgramiv( Handle_, GL_INFO_LOG_LENGTH, &i32InfoLogLength ) );

		// Allocate enough space for the message, and retrieve it.
		char* pszInfoLog = new char[i32InfoLogLength];
		GL( GetProgramInfoLog( Handle_, i32InfoLogLength, &i32CharsWritten, pszInfoLog ) );
		PSY_LOG( "RsShaderGL: Infolog:\n", pszInfoLog );
		std::stringstream LogStream( pszInfoLog );
		std::string LogLine;
		while( std::getline( LogStream, LogLine, '\n' ) )
		{
			PSY_LOG( LogLine.c_str() );
		}
		delete [] pszInfoLog;

		for( auto& Shader : Shaders )
		{
			PSY_LOG( "=======================================================\n" );
			auto ShaderData = reinterpret_cast< const GLchar* >( Shader->getData() );
			std::stringstream ShaderStream( ShaderData );
			std::string ShaderLine;
			int Line = 1;
			while( std::getline( ShaderStream, ShaderLine, '\n' ) )
			{
				auto PrintLine = Line++;
				if( ShaderLine.size() > 0 )
				{
					PSY_LOG( "%u: %s", PrintLine, ShaderLine.c_str() );
				}
			}
			PSY_LOG( "=======================================================\n" );
		}

		GL( DeleteProgram( Handle_ ) );
		Handle_ = 0;
		BcBreakpoint;
		return;
	}
	
	// Attempt to find uniform names, and uniform buffers for ES2.
	GLint ActiveUniforms = 0;
	GL( GetProgramiv( Handle_, GL_ACTIVE_UNIFORMS, &ActiveUniforms ) );
	std::set< std::string > UniformBlockSet;
	BcU32 ActiveSamplerIdx = 0;
	for( BcU32 Idx = 0; Idx < (BcU32)ActiveUniforms; ++Idx )
	{
		// Uniform information.
		GLchar UniformName[ 256 ];
		GLsizei UniformNameLength = 0;
		GLint Size = 0;
		GLenum Type = GL_INVALID_VALUE;

		// Get the uniform.
		GL( GetActiveUniform( Handle_, Idx, sizeof( UniformName ), &UniformNameLength, &Size, &Type, UniformName ) );
		
		// Add it as a parameter.
		if( UniformNameLength > 0 && Type != GL_INVALID_VALUE )
		{
			GLint UniformLocation = GL( GetUniformLocation( Handle_, UniformName ) );

			// Trim index off.
			BcChar* pIndexStart = BcStrStr( UniformName, "[0]" );
			if( pIndexStart != NULL )
			{
				*pIndexStart = '\0';
			}

			RsProgramUniformType InternalType = RsProgramUniformType::INVALID;
			switch( Type )
			{
#if !defined( RENDER_USE_GLES )
			case GL_SAMPLER_1D:
				InternalType = RsProgramUniformType::SAMPLER_1D;
				break;
#endif
			case GL_SAMPLER_2D:
				InternalType = RsProgramUniformType::SAMPLER_2D;
				break;
#if !defined( RENDER_USE_GLES )
			case GL_SAMPLER_3D:
				InternalType = RsProgramUniformType::SAMPLER_3D;
				break;
#endif
			case GL_SAMPLER_CUBE:
				InternalType = RsProgramUniformType::SAMPLER_CUBE;
				break;
#if !defined( RENDER_USE_GLES )
			case GL_SAMPLER_1D_SHADOW:
				InternalType = RsProgramUniformType::SAMPLER_1D_SHADOW;
				break;
#endif
			case GL_SAMPLER_2D_SHADOW:
				InternalType = RsProgramUniformType::SAMPLER_2D_SHADOW;
				break;
			default:
				InternalType = RsProgramUniformType::INVALID;
				break;
			}

			if( InternalType != RsProgramUniformType::INVALID )
			{
				// Add sampler. Will fail if not supported sampler type.
				Parent_->addSamplerSlot( UniformName, ActiveSamplerIdx );
				Parent_->addShaderResource( UniformName, RsShaderResourceType::TEXTURE, ActiveSamplerIdx );

				// Bind sampler to known index.
				GL( UseProgram( Handle_ ) );
				GL( Uniform1i( UniformLocation, ActiveSamplerIdx ) );
				GL( UseProgram( 0 ) );
				++ActiveSamplerIdx;
				
			}
			else
			{
				if( Version_.SupportUniformBuffers_ == BcFalse )
				{
					// Could be a member of a struct where we don't have uniform buffers.
					// Check the name and work out if it is. If so, add to a map so we can add all afterwards.
					auto VSTypePtr = BcStrStr( UniformName, "VS_" ); 
					auto PSTypePtr = BcStrStr( UniformName, "PS_" );
					if( VSTypePtr != nullptr ||
						PSTypePtr != nullptr )
					{
						// Terminate.
						if( VSTypePtr != nullptr )
						{
							VSTypePtr[ 0 ] = '\0';
						}
						else if( PSTypePtr != nullptr )
						{
							PSTypePtr[ 0 ] = '\0';
						}

						// Add to set.
						UniformBlockSet.insert( UniformName );
					}
				}
			}
		}
	}
	
	// Attempt to find uniform block names.
	if( Version_.SupportUniformBuffers_ )
	{
#if !defined( RENDER_USE_GLES )
		GLint ActiveUniformBlocks = 0;
		GL( GetProgramiv( Handle_, GL_ACTIVE_UNIFORM_BLOCKS, &ActiveUniformBlocks ) );
	
		for( BcU32 Idx = 0; Idx < (BcU32)ActiveUniformBlocks; ++Idx )
		{
			// Uniform information.
			GLchar UniformBlockName[ 256 ] = { 0 };
			GLsizei UniformBlockNameLength = 0;
			GLint Size = 0;
			GLint Binding = 0;

			// Get the uniform block size.
			GL( GetActiveUniformBlockiv( Handle_, Idx, GL_UNIFORM_BLOCK_DATA_SIZE, &Size ) );
			GL( GetActiveUniformBlockiv( Handle_, Idx, GL_UNIFORM_BLOCK_BINDING, &Binding ) );
			GL( GetActiveUniformBlockName( Handle_, Idx, sizeof( UniformBlockName ), &UniformBlockNameLength, UniformBlockName ) );

			// Add it as a parameter.
			if( UniformBlockNameLength > 0 )
			{
				auto TestIdx = GL( GetUniformBlockIndex( Handle_, UniformBlockName ) );
				BcAssert( TestIdx == Idx );
				BcUnusedVar( TestIdx );

				auto Class = ReManager::GetClass( UniformBlockName );
				BcAssert( Class->getSize() == (size_t)Size );
				Parent_->addUniformBufferSlot( 
					UniformBlockName, 
					Idx, 
					Class );

				GL( UniformBlockBinding( Handle_, Idx, Idx ) );				
			}
		}
#endif // !defined( RENDER_USE_GLES )
	}
	else
	{
		// Base uniform entry.
		UniformEntry UniformEntry;
		BcU32 UniformHandle = 0;
		for( auto UniformBlockName : UniformBlockSet )
		{
			const ReClass* Class = ReManager::GetClass( UniformBlockName );
			Parent_->addUniformBufferSlot( 
				UniformBlockName, 
				UniformHandle,
				Class );
			
			if( Class != nullptr )
			{
				// Statically cache the types.
				static auto TypeU32 = ReManager::GetClass( "BcU32" );
				static auto TypeS32 = ReManager::GetClass( "BcS32" );
				static auto TypeF32 = ReManager::GetClass( "BcF32" );
				static auto TypeVec2 = ReManager::GetClass( "MaVec2d" );
				static auto TypeVec3 = ReManager::GetClass( "MaVec3d" );
				static auto TypeVec4 = ReManager::GetClass( "MaVec4d" );
				static auto TypeMat4 = ReManager::GetClass( "MaMat4d" );			
				static auto TypeColour = ReManager::GetClass( "RsColour" );			

				// Iterate over all elements and grab the uniforms.
				auto ClassName = *Class->getName();
				auto ClassNameVS = ClassName + "VS";
				for( auto Field : Class->getFields() )
				{
					auto FieldName = *Field->getName();
					auto ValueType = Field->getType();
					auto UniformNameVS = ClassNameVS + "_X" + FieldName;

					UniformEntry.BindingPoint_ = UniformHandle;
					UniformEntry.Count_ = static_cast< GLsizei >( Field->getSize() / ValueType->getSize() );
					UniformEntry.Offset_ = Field->getOffset();

					auto UniformLocationVS = GL( GetUniformLocation( Handle_, UniformNameVS.c_str() ) );
					
					if( ValueType == TypeU32 || ValueType == TypeS32 )
					{
						UniformEntry.Type_ = UniformEntry::Type::UNIFORM_1IV;

						if( UniformLocationVS != -1 )
						{
							UniformEntry.Loc_ = UniformLocationVS;
							UniformEntry.Size_ = sizeof( BcU32 ) * UniformEntry.Count_;
							UniformEntries_.push_back( UniformEntry );
						}						
					}
					else if( ValueType == TypeF32 )
					{
						UniformEntry.Type_ = UniformEntry::Type::UNIFORM_1FV;

						if( UniformLocationVS != -1 )
						{
							UniformEntry.Loc_ = UniformLocationVS;
							UniformEntry.Size_ = sizeof( float ) * UniformEntry.Count_;
							UniformEntries_.push_back( UniformEntry );
						}
					}
					else if( ValueType == TypeVec2 )
					{
						UniformEntry.Type_ = UniformEntry::Type::UNIFORM_2FV;

						if( UniformLocationVS != -1 )
						{
							UniformEntry.Loc_ = UniformLocationVS;
							UniformEntry.Size_ = sizeof( float ) * 2 * UniformEntry.Count_;
							UniformEntries_.push_back( UniformEntry );
						}
					}
					else if( ValueType == TypeVec3 )
					{
						UniformEntry.Type_ = UniformEntry::Type::UNIFORM_3FV;

						if( UniformLocationVS != -1 )
						{
							UniformEntry.Loc_ = UniformLocationVS;
							UniformEntry.Size_ = sizeof( float ) * 3 * UniformEntry.Count_;
							UniformEntries_.push_back( UniformEntry );
						}
					}
					else if( ValueType == TypeVec4 )
					{
						UniformEntry.Type_ = UniformEntry::Type::UNIFORM_4FV;

						if( UniformLocationVS != -1 )
						{
							UniformEntry.Loc_ = UniformLocationVS;
							UniformEntry.Size_ = sizeof( float ) * 4 * UniformEntry.Count_;
							UniformEntries_.push_back( UniformEntry );
						}
					}
					else if( ValueType == TypeColour )
					{
						UniformEntry.Type_ = UniformEntry::Type::UNIFORM_4FV;

						if( UniformLocationVS != -1 )
						{
							UniformEntry.Loc_ = UniformLocationVS;
							UniformEntry.Size_ = sizeof( float ) * 4 * UniformEntry.Count_;
							UniformEntries_.push_back( UniformEntry );
						}
					}
					else if( ValueType == TypeMat4 )
					{
						UniformEntry.Type_ = UniformEntry::Type::UNIFORM_MATRIX_4FV;

						if( UniformLocationVS != -1 )
						{
							UniformEntry.Loc_ = UniformLocationVS;
							UniformEntry.Size_ = sizeof( MaMat4d ) * UniformEntry.Count_;
							UniformEntries_.push_back( UniformEntry );
						}
					}

					UniformEntry.CachedOffset_ += UniformEntry.Size_;
				}
			}

			++UniformHandle;
		}

		// Allocate a buffer to cache uniform values in.
		CachedUniforms_.reset( new BcU8[ UniformEntry.CachedOffset_ ] );
		BcMemSet( CachedUniforms_.get(), 0xff, UniformEntry.CachedOffset_ );
	}

	if( Version_.SupportProgramInterfaceQuery_ )
	{
#if !defined( RENDER_USE_GLES )
		BcAssert( Version_.SupportShaderStorageBufferObjects_ );
		GLint NumActiveShaderStorageBlocks = 0;
		GL( GetProgramInterfaceiv( Handle_, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &NumActiveShaderStorageBlocks ) );

		GLchar Name[256] = { 0 };
		GLsizei Length = 0;
		for( GLint Idx = 0; Idx < NumActiveShaderStorageBlocks; ++Idx )
		{
			GL( GetProgramResourceName( Handle_, GL_SHADER_STORAGE_BLOCK, Idx, sizeof( Name ), &Length, Name ) );
			GL( ShaderStorageBlockBinding( Handle_, Idx, Idx ) );

			Parent_->addShaderResource( Name, RsShaderResourceType::BUFFER, Idx );
		}
#endif // !defined( RENDER_USE_GLES )
	}

	// Catch error.
	

	// Validate program.
	GL( ValidateProgram( Handle_ ) );
	GLint ProgramValidated = 0;
	GL( GetProgramiv( Handle_, GL_VALIDATE_STATUS, &ProgramValidated ) );
	if ( !ProgramValidated )
	{					 
		// There was an error here, first get the length of the log message.
		int i32InfoLogLength, i32CharsWritten; 
		GL( GetProgramiv( Handle_, GL_INFO_LOG_LENGTH, &i32InfoLogLength ) );

		// Allocate enough space for the message, and retrieve it.
		char* pszInfoLog = new char[i32InfoLogLength];
		GL( GetProgramInfoLog( Handle_, i32InfoLogLength, &i32CharsWritten, pszInfoLog ) );
		PSY_LOG( "RsProgramGL: Infolog:\n %s\n", pszInfoLog );
		delete [] pszInfoLog;

		BcBreakpoint;
		GL( DeleteProgram( Handle_ ) );
		Handle_  = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
// Dtor
RsProgramGL::~RsProgramGL()
{
	GL( DeleteProgram( Handle_ ) );
	Handle_ = 0;
}

//////////////////////////////////////////////////////////////////////////
// copyUniformBuffersToUniforms
void RsProgramGL::copyUniformBuffersToUniforms( size_t NoofBuffers, class RsBuffer** Buffers )
{
	PSY_PROFILE_FUNCTION;

#if PSY_DEBUG
	if( UniformEntries_.size() > 0 )
	{
		GLint BoundHandle = 0;
		GL( GetIntegerv( GL_CURRENT_PROGRAM, &BoundHandle ) );
		BcAssert( (GLuint)BoundHandle == Handle_ );
	}
#endif

	for( auto& UniformEntry : UniformEntries_ )
	{
		RsBuffer* Buffer = Buffers[ UniformEntry.BindingPoint_ ];
		if( Buffer != nullptr )
		{
			const auto BufferGL = Buffer->getHandle< RsBufferGL* >();
			BcAssert( BufferGL );

			// Check if we have buffer data.
			if( BufferGL->BufferData_ != nullptr )
			{
				// Check version, if equal, then don't update uniform.
				if( UniformEntry.Buffer_ != Buffer ||
					UniformEntry.Version_ != BufferGL->Version_ )
				{
					// Update buffer & version.
					UniformEntry.Buffer_ = Buffer;
					UniformEntry.Version_ = BufferGL->Version_;

					// Setup uniforms.
					const auto* UniformData = BufferGL->BufferData_.get() + UniformEntry.Offset_;
					auto* CachedUniformData = CachedUniforms_.get() + UniformEntry.CachedOffset_;

					// Check if value has changed.
					if( memcmp( CachedUniformData, UniformData, UniformEntry.Size_ ) != 0 )
					{
						memcpy( CachedUniformData, UniformData, UniformEntry.Size_ );
						switch( UniformEntry.Type_ )
						{
						case RsProgramGL::UniformEntry::Type::UNIFORM_1IV:
							GL( Uniform1iv( UniformEntry.Loc_, UniformEntry.Count_, reinterpret_cast< const BcS32* >( UniformData ) ) );
							break;
						case RsProgramGL::UniformEntry::Type::UNIFORM_1FV:
							GL( Uniform1fv( UniformEntry.Loc_, UniformEntry.Count_, reinterpret_cast< const BcF32* >( UniformData ) ) );
							break;
						case RsProgramGL::UniformEntry::Type::UNIFORM_2FV:
							GL( Uniform2fv( UniformEntry.Loc_, UniformEntry.Count_, reinterpret_cast< const BcF32* >( UniformData ) ) );
							break;
						case RsProgramGL::UniformEntry::Type::UNIFORM_3FV:
							GL( Uniform3fv( UniformEntry.Loc_, UniformEntry.Count_, reinterpret_cast< const BcF32* >( UniformData ) ) );
							break;
						case RsProgramGL::UniformEntry::Type::UNIFORM_4FV:
							GL( Uniform4fv( UniformEntry.Loc_, UniformEntry.Count_, reinterpret_cast< const BcF32* >( UniformData ) ) );
							break;
						case RsProgramGL::UniformEntry::Type::UNIFORM_MATRIX_4FV:
							GL( UniformMatrix4fv( UniformEntry.Loc_, UniformEntry.Count_, GL_FALSE, reinterpret_cast< const BcF32* >( UniformData ) ) );
							break;
						default:
							BcBreakpoint;
							break;
						}
							
					}
				}
			}
		}
		else
		{
			BcBreakpoint;
		}
	}
}

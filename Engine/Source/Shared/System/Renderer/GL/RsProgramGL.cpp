#include "System/Renderer/GL/RsProgramGL.h"
#include "System/Renderer/GL/RsBufferGL.h"
#include "System/Renderer/GL/RsContextGL.h"
#include "System/Renderer/GL/RsProgramFileDataGL.h"
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
	Parent_->setHandle( this );
	const auto& Shaders = Parent_->getShaders();
	auto ContextGL = static_cast< RsContextGL* >( Parent_->getContext() );

	// Some checks to ensure validity.
	BcAssert( Shaders.size() > 0 );	

	// Create program.
	Handle_ = GL( CreateProgram() );

#if !defined( RENDER_USE_GLES ) && !PSY_PRODUCTION
	if( GLEW_KHR_debug ) 
	{
		glObjectLabel( GL_PROGRAM, Handle_, BcStrLength( Parent->getDebugName() ), Parent->getDebugName() );
	}
#endif

	// Attach shaders.
	for( auto* Shader : Shaders )
	{
		BcAssertMsg( Shader->getHandle< GLuint >() != 0, 
			"RsShader \"%s\" invalid when attaching to RsProgram \"%s\"",
			Shader->getDebugName(),
			Parent->getDebugName() );
		glAttachShader( Handle_, Shader->getHandle< GLuint >() );
		if( glGetError() != 0 )
		{
			return;
		}
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
		PSY_LOG( "RsProgramGL: Infolog (%s):\n", Parent_->getDebugName(), pszInfoLog );
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

	// Use program.
	ContextGL->bindProgram( Parent_ );

	// Iterate over all parameters and setup program.
	UniformEntry UniformEntry;
	for( const auto& Parameter : Parent->getParameterList() )
	{
		RsProgramParameterTypeValueGL InternalType;
		InternalType.Value_ = Parameter.InternalType_;
		switch( (RsProgramParameterStorageGL)InternalType.Storage_ )
		{
		case RsProgramParameterStorageGL::UNIFORM:
			{
#if 0 // TODO: Nvidia drivers optimise out unused stuff. Should either not assert, or fix import pipeline.
				auto Location = GL( GetUniformLocation( Handle_, Parameter.Name_ ) );
				BcAssertMsg( Location != -1,
					"Invalid uniform in RsProgram \"%s\". Unable to find \"%s\"",
					Parent_->getDebugName(),
					Parameter.Name_ );
#endif
			}
			break;
		case RsProgramParameterStorageGL::UNIFORM_BLOCK:
			{
				if( Version_.SupportUniformBuffers_ )
				{
#if !defined( RENDER_USE_GLES ) || defined( RENDER_USE_GLES3 )
					auto Index = GL( GetUniformBlockIndex( Handle_, Parameter.Name_ ) );
#if 0 // TODO: Nvidia drivers optimise out unused stuff. Should either not assert, or fix import pipeline.
					BcAssertMsg( Index != -1,
						"Invalid uniform block in RsProgram \"%s\". Unable to find \"%s\"",
						Parent_->getDebugName(),
						Parameter.Name_ );
#endif
					if( Index != -1 )
					{
						BcU32 UBSlot = BcU32( UniformBufferBindInfo_.size() );
						Parent_->addUniformBufferSlot( Parameter.Name_, UBSlot, Parameter.Size_ );
						UniformBufferBindInfo_.emplace_back( RsProgramBindInfoGL( RsProgramBindTypeGL::UNIFORM_BLOCK, InternalType.Binding_ ) );
						GL( UniformBlockBinding( Handle_, Index, InternalType.Binding_ ) );				
					}
#endif // !defined( RENDER_USE_GLES ) || defined( RENDER_USE_GLES3 )
				}

				{
					BcU32 UBSlot = BcU32( UniformBufferBindInfo_.size() );
					Parent_->addUniformBufferSlot( Parameter.Name_, UBSlot, Parameter.Size_ );
					UniformBufferBindInfo_.emplace_back( RsProgramBindInfoGL( RsProgramBindTypeGL::UNIFORM_BLOCK, InternalType.Binding_ ) );

					// TODO: Package uniform info straight into the shader data instead of using reflection.
					const ReClass* Class = ReManager::GetClass( Parameter.Name_ );
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

							UniformEntry.BindingPoint_ = InternalType.Binding_;
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
				}
			}
			break;
		case RsProgramParameterStorageGL::SAMPLER:
			{
				auto Location = GL( GetUniformLocation( Handle_, Parameter.Name_ ) );
#if 0 // TODO: Nvidia drivers optimise out unused stuff. Should either not assert, or fix import pipeline.
					BcAssertMsg( Location != -1,
					"Invalid sampler in RsProgram \"%s\". Unable to find \"%s\"",
					Parent_->getDebugName(),
					Parameter.Name_ );
#endif
				if( Location != -1 )
				{
					RsTextureType TextureType = RsTextureType::UNKNOWN;
					switch( InternalType.Type_ )
					{
					case GL_SAMPLER_1D:
						TextureType = RsTextureType::TEX1D;
						break;
					case GL_SAMPLER_2D:
						TextureType = RsTextureType::TEX2D;
						break;
					case GL_SAMPLER_3D:
						TextureType = RsTextureType::TEX3D;
						break;
					case GL_SAMPLER_CUBE:
						TextureType = RsTextureType::TEXCUBE;
						break;
					case GL_SAMPLER_1D_SHADOW:
						TextureType = RsTextureType::TEX1D;
						break;
					case GL_SAMPLER_2D_SHADOW:
						TextureType = RsTextureType::TEX2D;
						break;
					default:
						BcAssertMsg( BcFalse, "Unsupported sampler type in program \"%s\"", 
							Parent->getDebugName() );
					}

					BcU32 SamplerSlot = BcU32( SamplerBindInfo_.size() );
					Parent_->addSamplerSlot( Parameter.Name_, SamplerSlot );
					SamplerBindInfo_.emplace_back( RsProgramBindInfoGL( RsProgramBindTypeGL::SAMPLER, TextureType, InternalType.Binding_ ) );

					BcU32 SRVSlot = BcU32( SRVBindInfo_.size() );
					Parent_->addShaderResource( Parameter.Name_, RsShaderResourceType::TEXTURE, SRVSlot );
					SRVBindInfo_.emplace_back( RsProgramBindInfoGL( RsProgramBindTypeGL::TEXTURE, TextureType, InternalType.Binding_ ) );

					GL( Uniform1i( Location, InternalType.Binding_ ) );
				}
			}
			break;
		case RsProgramParameterStorageGL::SHADER_STORAGE_BUFFER:
			{
#if !defined( RENDER_USE_GLES )
				if( Version_.SupportShaderStorageBufferObjects_ )
				{
					BcAssert( Version_.SupportProgramInterfaceQuery_ );
					auto Index = GL( GetProgramResourceIndex( Handle_, GL_SHADER_STORAGE_BLOCK, Parameter.Name_ ) );
#if 0 // TODO: Nvidia drivers optimise out unused stuff. Should either not assert, or fix import pipeline.
					BcAssertMsg( Index != -1,
						"Invalid shader storage buffer in RsProgram \"%s\". Unable to find \"%s\"",
						Parent_->getDebugName(),
						Parameter.Name_ );
#endif
					if( Index != -1 )
					{
						if( InternalType.ReadOnly_ )
						{
							BcU32 SRVSlot = BcU32( SRVBindInfo_.size() );
							Parent_->addShaderResource( Parameter.Name_, RsShaderResourceType::BUFFER, SRVSlot );
							SRVBindInfo_.emplace_back( RsProgramBindInfoGL( RsProgramBindTypeGL::SHADER_STORAGE_BUFFER_OBJECT, InternalType.Binding_ ) );
						}
						else
						{
							BcU32 UAVSlot = BcU32( UAVBindInfo_.size() );
							Parent_->addUnorderedAccess( Parameter.Name_, RsUnorderedAccessType::BUFFER, UAVSlot );
							UAVBindInfo_.emplace_back( RsProgramBindInfoGL( RsProgramBindTypeGL::SHADER_STORAGE_BUFFER_OBJECT, InternalType.Binding_ ) );
						}
						GL( ShaderStorageBlockBinding( Handle_, Index, InternalType.Binding_ ) );
					}
				}
#endif // !defined( RENDER_USE_GLES )
		}
			break;
		case RsProgramParameterStorageGL::IMAGE:
			{
#if !defined( RENDER_USE_GLES )
				if( Version_.SupportImageLoadStore_ )
				{
					auto Location = GL( GetUniformLocation( Handle_, Parameter.Name_ ) );
#if 0 // TODO: Nvidia drivers optimise out unused stuff. Should either not assert, or fix import pipeline.
					BcAssertMsg( Location != -1,
						"Invalid image in RsProgram \"%s\". Unable to find \"%s\"",
						Parent_->getDebugName(),
						Parameter.Name_ );
#endif
					if( Location != -1 )
					{
						RsTextureType TextureType = RsTextureType::UNKNOWN;
						switch( InternalType.Type_ )
						{
						case GL_IMAGE_1D:
							TextureType = RsTextureType::TEX1D;
							break;
						case GL_IMAGE_2D:
							TextureType = RsTextureType::TEX2D;
							break;
						case GL_IMAGE_3D:
							TextureType = RsTextureType::TEX3D;
							break;
						case GL_IMAGE_2D_RECT:
							TextureType = RsTextureType::TEX2D;
							break;
						case GL_IMAGE_CUBE:
							TextureType = RsTextureType::TEXCUBE;
							break;
						case GL_IMAGE_1D_ARRAY:
							TextureType = RsTextureType::TEX1D;
							break;
						case GL_IMAGE_2D_ARRAY:
							TextureType = RsTextureType::TEX2D;
							break;
						case GL_IMAGE_CUBE_MAP_ARRAY:
							TextureType = RsTextureType::TEXCUBE;
							break;
						case GL_IMAGE_2D_MULTISAMPLE:
							TextureType = RsTextureType::TEX2D;
							break;
						case GL_IMAGE_2D_MULTISAMPLE_ARRAY:
							TextureType = RsTextureType::TEX2D;
							break;
						default:
							BcAssertMsg( BcFalse, "Unsupported image type in program \"%s\"", 
								Parent->getDebugName() );
						}

						if( InternalType.ReadOnly_ )
						{
							BcU32 SRVSlot = BcU32( SRVBindInfo_.size() );
							Parent->addShaderResource( Parameter.Name_, RsShaderResourceType::TEXTURE, SRVSlot );
							SRVBindInfo_.emplace_back( RsProgramBindInfoGL( RsProgramBindTypeGL::IMAGE, TextureType, InternalType.Binding_ ) );
						}
						else
						{
							BcU32 UAVSlot = BcU32( UAVBindInfo_.size() );
							Parent->addUnorderedAccess( Parameter.Name_, RsUnorderedAccessType::TEXTURE, UAVSlot );
							UAVBindInfo_.emplace_back( RsProgramBindInfoGL( RsProgramBindTypeGL::IMAGE, TextureType, InternalType.Binding_ ) );
						}
						GL( Uniform1i( Location, InternalType.Binding_ ) );
					}
				}
#endif // !defined( RENDER_USE_GLES )
			}
			break;
		}
	}
	
	// If we don't support uniform buffers, setup uniform cache.
	if( UniformEntry.CachedOffset_ > 0 )
	{
		// Allocate a buffer to cache uniform values in.
		CachedUniforms_.reset( new BcU8[ UniformEntry.CachedOffset_ ] );
		BcMemSet( CachedUniforms_.get(), 0xff, UniformEntry.CachedOffset_ );
	}
	
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
		PSY_LOG( "RsProgramGL: Infolog (%s):\n %s\n", Parent_->getDebugName(), pszInfoLog );
		delete [] pszInfoLog;

		//BcBreakpoint;
		//GL( DeleteProgram( Handle_ ) );
		//Handle_  = 0;
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
void RsProgramGL::copyUniformBuffersToUniforms( size_t NoofBuffers, const RsUBSlot* UBSlots )
{
	if( UniformEntries_.size() == 0 )
	{
		return;
	}

	PSY_PROFILER_SECTION( "RsProgramGL::copyUniformBuffersToUniforms( %u )", NoofBuffers );
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
		const auto UBSlot = UBSlots[ UniformEntry.BindingPoint_ ];
		if( UBSlot.Buffer_ != nullptr )
		{
			const auto BufferGL = UBSlot.Buffer_->getHandle< RsBufferGL* >();
			BcAssert( BufferGL );

			// Check if we have buffer data.
			if( BufferGL->getBufferData() != nullptr )
			{
				// Check version, if equal, then don't update uniform.
				if( UniformEntry.Buffer_ != UBSlot.Buffer_ ||
					UniformEntry.SlotOffset_ != UBSlot.Offset_ ||
					UniformEntry.SlotSize_ != UBSlot.Size_ ||
					UniformEntry.Version_ != BufferGL->getVersion() )
				{
					// Update buffer & version.
					UniformEntry.Buffer_ = UBSlot.Buffer_;
					UniformEntry.SlotOffset_ = UBSlot.Offset_;
					UniformEntry.SlotSize_ = UBSlot.Size_;
					UniformEntry.Version_ = BufferGL->getVersion();

					// Setup uniforms.
					const auto* UniformData = BufferGL->getBufferData() + UniformEntry.Offset_ + UBSlot.Offset_;
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
			//BcBreakpoint;
		}
	}
}

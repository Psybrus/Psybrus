#pragma once

#include "System/Renderer/GL/RsGL.h"
#include "System/Renderer/GL/RsProgramFileDataGL.h"

//////////////////////////////////////////////////////////////////////////
// RsProgramBindTypeGL
enum class RsProgramBindTypeGL
{
	NONE,
	TEXTURE,
	IMAGE,
	SHADER_STORAGE_BUFFER_OBJECT,
	UNIFORM_BLOCK,
	SAMPLER,
};

//////////////////////////////////////////////////////////////////////////
// RsProgramBindInfoGL
struct RsProgramBindInfoGL
{
	RsProgramBindInfoGL()
	{}
		
	RsProgramBindInfoGL( RsProgramBindTypeGL BindType, BcU32 Binding ):
		BindType_( BindType ),
		Binding_( Binding )
	{}

	RsProgramBindInfoGL( RsProgramBindTypeGL BindType, RsTextureType TextureType, BcU32 Binding ):
		BindType_( BindType ),
		TextureType_( TextureType ),
		Binding_( Binding )
	{}

	RsProgramBindTypeGL BindType_ = RsProgramBindTypeGL::NONE;
	RsTextureType TextureType_ = RsTextureType::UNKNOWN;
	BcU32 Binding_ = BcErrorCode;
};

//////////////////////////////////////////////////////////////////////////
// RsProgramGL
class RsProgramGL
{
public:
	RsProgramGL( class RsProgram* Parent, const RsOpenGLVersion& Version );
	~RsProgramGL();

	/**
	 * Copy data from uniform buffers into the uniforms.
	 * @pre Currently bound program is this one.
	 */
	void copyUniformBuffersToUniforms( size_t NoofBuffers, const class RsBuffer* const * Buffers );


	GLuint getHandle() const { return Handle_; }

	size_t getSRVBindCount() { return SRVBindInfo_.size(); };
	size_t getUAVBindCount() { return UAVBindInfo_.size(); };
	size_t getSamplerBindCount() { return SamplerBindInfo_.size(); };
	size_t getUniformBufferBindCount() { return UniformBufferBindInfo_.size(); };

	RsProgramBindInfoGL getSRVBindInfo( BcU32 Idx ) const { return SRVBindInfo_[ Idx ]; };
	RsProgramBindInfoGL getUAVBindInfo( BcU32 Idx ) const { return UAVBindInfo_[ Idx ]; };
	RsProgramBindInfoGL getSamplerBindInfo( BcU32 Idx ) const { return SamplerBindInfo_[ Idx ]; };
	RsProgramBindInfoGL getUniformBufferBindInfo( BcU32 Idx ) const { return UniformBufferBindInfo_[ Idx ]; };

private:
	class RsProgram* Parent_ = nullptr;
	const RsOpenGLVersion& Version_;

	struct UniformEntry
	{
		enum class Type
		{
			UNKNOWN,
			UNIFORM_1IV,
			UNIFORM_1FV,
			UNIFORM_2FV,
			UNIFORM_3FV,
			UNIFORM_4FV,
			UNIFORM_MATRIX_4FV,
		};

		BcU32 BindingPoint_ = 0;
		Type Type_ = UniformEntry::Type::UNKNOWN;
		GLint Loc_ = 0;
		GLsizei Count_ = 0;
		size_t Offset_ = 0;
		size_t CachedOffset_ = 0;
		size_t Size_ = 0;

		// Used for redundancy checks.
		const class RsBuffer* Buffer_ = nullptr;
		BcU32 Version_ = 0;
	};

	GLuint Handle_ = 0;
	std::vector< UniformEntry > UniformEntries_;
	std::unique_ptr< BcU8[] > CachedUniforms_;

	std::vector< RsProgramBindInfoGL > SRVBindInfo_;
	std::vector< RsProgramBindInfoGL > UAVBindInfo_;
	std::vector< RsProgramBindInfoGL > SamplerBindInfo_;
	std::vector< RsProgramBindInfoGL > UniformBufferBindInfo_;

	BcU32 NoofImages_ = 0;
	BcU32 NoofSSBOs_ = 0;
};


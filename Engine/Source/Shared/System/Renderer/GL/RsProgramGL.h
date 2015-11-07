#pragma once

#include "System/Renderer/GL/RsGL.h"

//////////////////////////////////////////////////////////////////////////
// RsProgramBindTypeGL
enum class RsProgramBindTypeGL
{
	NONE,
	TEXTURE,
	IMAGE,
	SHADER_STORAGE_BUFFER_OBJECT,
	SAMPLER,
	UNIFORM_BLOCK
};

//////////////////////////////////////////////////////////////////////////
// RsProgramBindInfoGL
struct RsProgramBindInfoGL
{
	RsProgramBindInfoGL()
	{}
		
	RsProgramBindInfoGL( RsProgramBindTypeGL BindType, BcU32 Slot ):
		BindType_( BindType ),
		Slot_( Slot )
	{}

	RsProgramBindInfoGL( RsProgramBindTypeGL BindType, RsTextureType TextureType, BcU32 Slot ):
		BindType_( BindType ),
		TextureType_( TextureType ),
		Slot_( Slot )
	{}

	RsProgramBindTypeGL BindType_ = RsProgramBindTypeGL::NONE;
	RsTextureType TextureType_ = RsTextureType::UNKNOWN;
	BcU32 Slot_ = -1;
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
	void copyUniformBuffersToUniforms( size_t NoofBuffers, class RsBuffer** Buffers );


	GLuint getHandle() const { return Handle_; }

	RsProgramBindInfoGL getSRVBindInfo( BcU32 Idx ) { return SRVBindInfo_[ Idx ]; };
	RsProgramBindInfoGL getUAVBindInfo( BcU32 Idx ) { return UAVBindInfo_[ Idx ]; };
	RsProgramBindInfoGL getSamplerBindInfo( BcU32 Idx ) { return SamplerBindInfo_[ Idx ]; };
	RsProgramBindInfoGL getUniformBufferBindInfo( BcU32 Idx ) { return UniformBufferBindInfo_[ Idx ]; };

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
		class RsBuffer* Buffer_ = nullptr;
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


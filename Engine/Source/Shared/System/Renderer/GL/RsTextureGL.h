#pragma once

#include "System/Renderer/GL/RsGL.h"

//////////////////////////////////////////////////////////////////////////
// RsTextureGL
class RsTextureGL
{
public:
	enum class ResourceType
	{
		TEXTURE,
		BACKBUFFER_RT,
		BACKBUFFER_DS
	};

public:
	RsTextureGL( class RsTexture* Parent, ResourceType ResourceType );
	~RsTextureGL();

	void loadTexture( 
		const struct RsTextureSlice& Slice,
		BcU32 DataSize,
		void* Data );

	ResourceType getResourceType() const { return ResourceType_; }
	GLuint getHandle() const { return Handle_; }

private:
	class RsTexture* Parent_ = nullptr;
	ResourceType ResourceType_ = ResourceType::TEXTURE;
	GLuint Handle_ = 0;
};

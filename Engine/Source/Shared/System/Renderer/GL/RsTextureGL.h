#pragma once

#include "System/Renderer/GL/RsGL.h"

//////////////////////////////////////////////////////////////////////////
// RsTextureGL
class RsTextureGL
{
public:
	RsTextureGL( class RsTexture* Parent );
	~RsTextureGL();

	void loadTexture( 
		const struct RsTextureSlice& Slice,
		BcU32 DataSize,
		void* Data );

	GLuint getHandle() const { return Handle_; }

private:
	class RsTexture* Parent_ = nullptr;
	GLuint Handle_ = 0;
};

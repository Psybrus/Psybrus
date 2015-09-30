#pragma once

#include "System/Renderer/GL/RsGL.h"

//////////////////////////////////////////////////////////////////////////
// RsTextureGL
class RsTextureGL
{
public:
	RsTextureGL( RsTexture* Parent );
	~RsTextureGL();

	RsTexture* Parent_ = nullptr;
	GLuint Handle_ = 0;
};

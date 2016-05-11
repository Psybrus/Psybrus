#pragma once

#include "System/Renderer/GL/RsGL.h"

//////////////////////////////////////////////////////////////////////////
// RsQueryHeapGL
class RsQueryHeapGL
{
public:
	RsQueryHeapGL( class RsQueryHeap* Parent, const RsOpenGLVersion& Version );
	~RsQueryHeapGL();

	GLuint getHandle( size_t Idx ) const { return Handles_[ Idx ]; }
	
private:
	class RsQueryHeap* Parent_ = nullptr;
	std::vector< GLuint > Handles_;
};

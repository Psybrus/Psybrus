#pragma once

#include "System/Renderer/GL/RsGL.h"

//////////////////////////////////////////////////////////////////////////
// RsBufferGL
class RsBufferGL
{
public:
	RsBufferGL( RsBuffer* Parent, const RsOpenGLVersion& Version );
	~RsBufferGL();

	BcU8* getBufferData() const { return BufferData_.get(); }
	GLuint getHandle() const { return Handle_; }
	BcU32 getVersion() const { return Version_; }
	void incVersion(){ Version_++; }
	
private:
	RsBuffer* Parent_ = nullptr;
	GLuint Handle_ = 0;
	std::unique_ptr< BcU8[] > BufferData_;
	BcU32 Version_ = 0;
};

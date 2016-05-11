#pragma once

#include <memory>

//////////////////////////////////////////////////////////////////////////
// Resource deletion.
class RsResourceDeleters
{
public:
	void operator()( class RsBuffer* Resource );
	void operator()( class RsContext* Resource );
	void operator()( class RsFrameBuffer* Resource );
	void operator()( class RsGeometryBinding* Resource );
	void operator()( class RsProgram* Resource );
	void operator()( class RsProgramBinding* Resource );
	void operator()( class RsQueryHeap* Resource );
	void operator()( class RsRenderState* Resource );
	void operator()( class RsSamplerState* Resource );
	void operator()( class RsShader* Resource );
	void operator()( class RsTexture* Resource );
	void operator()( class RsVertexDeclaration* Resource );
};

typedef std::unique_ptr< class RsBuffer, RsResourceDeleters > RsBufferUPtr;
typedef std::unique_ptr< class RsContext, RsResourceDeleters > RsContextUPtr;
typedef std::unique_ptr< class RsFrameBuffer, RsResourceDeleters > RsFrameBufferUPtr;
typedef std::unique_ptr< class RsGeometryBinding, RsResourceDeleters > RsGeometryBindingUPtr;
typedef std::unique_ptr< class RsProgram, RsResourceDeleters > RsProgramUPtr;
typedef std::unique_ptr< class RsProgramBinding, RsResourceDeleters > RsProgramBindingUPtr;
typedef std::unique_ptr< class RsQueryHeap, RsResourceDeleters > RsQueryHeapUPtr;
typedef std::unique_ptr< class RsRenderState, RsResourceDeleters > RsRenderStateUPtr;
typedef std::unique_ptr< class RsSamplerState, RsResourceDeleters > RsSamplerStateUPtr;
typedef std::unique_ptr< class RsShader, RsResourceDeleters > RsShaderUPtr;
typedef std::unique_ptr< class RsTexture, RsResourceDeleters > RsTextureUPtr;
typedef std::unique_ptr< class RsVertexDeclaration, RsResourceDeleters > RsVertexDeclarationUPtr;

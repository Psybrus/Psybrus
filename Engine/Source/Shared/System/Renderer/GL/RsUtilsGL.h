#pragma once

#include "System/Renderer/GL/RsGL.h"

////////////////////////////////////////////////////////////////////////////////
// RsUtilsGL
class RsUtilsGL
{
public:
	struct ResourceFormat
	{
		ResourceFormat(
				BcBool Compressed,
				BcBool DepthStencil,
				GLenum InternalFormat,
				GLenum Format,
				GLenum Type ):
			Compressed_( Compressed ),
			DepthStencil_( DepthStencil ),
			InternalFormat_( InternalFormat ),
			Format_( Format ),
			Type_( Type )
		{}

		BcBool Compressed_;
		BcBool DepthStencil_;
		GLenum InternalFormat_;
		GLenum Format_;
		GLenum Type_;
	};

	static GLenum GetCompareMode( RsCompareMode V );
	static GLenum GetBlendOp( RsBlendOp V );
	static GLenum GetBlendType( RsBlendType V );
	static GLenum GetStencilOp( RsStencilOp V );
	static GLenum GetTextureFiltering( RsTextureFilteringMode V );
	static GLenum GetTextureSampling( RsTextureSamplingMode V );
	static GLenum GetTextureType( RsTextureType V );
	static GLenum GetTextureFace( RsTextureFace V );
	static GLenum GetVertexDataType( RsVertexDataType V );
	static GLboolean GetVertexDataNormalised( RsVertexDataType V );
	static GLint GetVertexDataSize( RsVertexDataType V );
	static GLenum GetTopologyType( RsTopologyType V );
	static GLenum GetBufferType( RsBindFlags V );
	static ResourceFormat GetResourceFormat( RsResourceFormat V );
	static GLenum GetShaderType( RsShaderType V );
	static GLenum GetQueryType( RsQueryType Query );
};

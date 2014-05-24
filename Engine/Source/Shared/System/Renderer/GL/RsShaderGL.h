/**************************************************************************
 *
 * File:	RsShaderGL.h
 * Author: 	Neil Richardson 
 * Ver/Date:	
 * Description:
 *		GL Shader.
 *		
 *
 *
 * 
 **************************************************************************/

#ifndef __RSSHADERGL_H__
#define __RSSHADERGL_H__

#include "System/Renderer/RsShader.h"
#include "System/Renderer/GL/RsGL.h"

////////////////////////////////////////////////////////////////////////////////
// RsShaderGL
class RsShaderGL:
	public RsShader
{
public:
	/**
	 * Create shader.
	 * @param ShaderType Shader type.
	 * @param ShaderDataType Shader data type.
	 * @param pShaderData Shader data.
	 * @param ShaderDataSize Shader data size.
	 */
	RsShaderGL( RsContext* pContext, RsShaderType ShaderType, RsShaderDataType ShaderDataType, void* pShaderData, BcU32 ShaderDataSize );
	virtual ~RsShaderGL();
	
	void								create();
	virtual void						update();
	virtual void						destroy();	

	void								logShader();
	
private:
	GLenum								Type_;
	RsShaderDataType					DataType_;
	BcU32								Format_;
};

#endif

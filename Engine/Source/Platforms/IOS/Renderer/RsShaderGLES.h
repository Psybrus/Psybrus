/**************************************************************************
 *
 * File:	RsShaderGLES.h
 * Author: 	Neil Richardson 
 * Ver/Date:	
 * Description:
 *		GLES Shader.
 *		
 *
 *
 * 
 **************************************************************************/

#ifndef __RSSHADERGLES_H__
#define __RSSHADERGLES_H__

#include "RsGPUResourceGLES.h"
#include "RsShader.h"

////////////////////////////////////////////////////////////////////////////////
// RsShaderGLES
class RsShaderGLES:
	public RsShader,
	public RsGPUResourceGLES
{
public:
	/**
	 * Create shader.
	 * @param ShaderType Shader type.
	 * @param ShaderDataType Shader data type.
	 * @param pShaderData Shader data.
	 * @param ShaderDataSize Shader data size.
	 */
	RsShaderGLES( eRsShaderType ShaderType, eRsShaderDataType ShaderDataType, void* pShaderData, BcU32 ShaderDataSize );
	virtual ~RsShaderGLES();
	
	void								create();
	virtual void						update();
	virtual void						destroy();	
	
private:
	GLenum								Type_;
	eRsShaderDataType					DataType_;
	BcU32								Format_;
};

#endif

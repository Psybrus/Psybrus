/**************************************************************************
*
* File:		RsShader.h
* Author:	Neil Richardson 
* Ver/Date:	4/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSSHADER_H__
#define __RSSHADER_H__

#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsResource.h"

//////////////////////////////////////////////////////////////////////////
// RsShaderDesc
struct RsShaderDesc
{
	RsShaderDesc();
	RsShaderDesc( 
		RsShaderType ShaderType, 
		RsShaderCodeType ShaderCodeType );

	RsShaderType ShaderType_;
	RsShaderCodeType ShaderCodeType_;
};

//////////////////////////////////////////////////////////////////////////
// RsShader
class RsShader:
	public RsResource
{
public:
	RsShader( 
		class RsContext* pContext,
		const RsShaderDesc& Desc,
		void* Data,
		BcU32 DataSize );
	~RsShader();

	/**
	 * Get desc.
	 */
	const RsShaderDesc& getDesc() const;

	/**
	 * Get data.
	 */
	void* getData() const;

	/**
 	 * Get data size.
	 */
	BcU32 getDataSize() const;

private:
	RsShaderDesc Desc_;
	void* Data_;
	BcU32 DataSize_;
};

#endif

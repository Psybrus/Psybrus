/**************************************************************************
*
* File:		RsShader.cpp
* Author:	Neil Richardson 
* Ver/Date:	4/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/RsShader.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsShaderDesc::RsShaderDesc():
	ShaderType_( RsShaderType::INVALID ),
	ShaderCodeType_( RsShaderCodeType::INVALID )
{
	
}

//////////////////////////////////////////////////////////////////////////
// Ctor
RsShaderDesc::RsShaderDesc( 
	RsShaderType ShaderType, 
	RsShaderCodeType ShaderCodeType ):
	ShaderType_( ShaderType ),
	ShaderCodeType_( ShaderCodeType )
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
RsShader::RsShader( 
	class RsContext* pContext,
	const RsShaderDesc& Desc,
	void* Data,
	BcU32 DataSize ):
	RsResource( RsResourceType::SHADER, pContext ),
	Desc_( Desc ),
	Data_( Data ),
	DataSize_( DataSize )
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
RsShader::~RsShader()
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
const RsShaderDesc& RsShader::getDesc() const
{
	return Desc_;
}

//////////////////////////////////////////////////////////////////////////
// Ctor
void* RsShader::getData() const
{
	return Data_;
}

//////////////////////////////////////////////////////////////////////////
// Ctor
BcU32 RsShader::getDataSize() const
{
	return DataSize_;
}

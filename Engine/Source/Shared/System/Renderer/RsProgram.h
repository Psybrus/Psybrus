/**************************************************************************
*
* File:		RsProgram.h
* Author:	Neil Richardson 
* Ver/Date:	28/02/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSPROGRAM_H__
#define __RSPROGRAM_H__

#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsUniformBuffer.h"

////////////////////////////////////////////////////////////////////////////////
// RsProgram
class RsProgram:
	public RsResource
{
public:
	RsProgram( class RsContext* pContext );
	virtual ~RsProgram();
	
	/**
	 * Get parameter buffer size.
	 * @return Size of parameter buffer in bytes.
	 */
	virtual BcU32						getParameterBufferSize() const = 0;

	/**
	 * Find offset of parameter in the buffer.
	 * @param Type Type of parameter (out.)
	 * @param Offset Offset of parameter (out.)
	 * @param TypeBytes Size of type in bytes (out.)
	 * @return Success.
	 */
	virtual BcU32						findParameterOffset( const BcChar* Name, eRsShaderParameterType& Type, BcU32& Offset, BcU32& TypeBytes ) const = 0;
	
	/**
	 * Bind program.
	 * @param pParameterBuffer Pointer to parameter buffer for binding.
	 */
	virtual void						bind( void* pParameterBuffer ) = 0;
	
	/**
	 * Find uniform block by name.
	 * @param Name
	 * @return Index.
	 */
	virtual BcU32						findUniformBlockIndex( const BcChar* Name ) = 0;

	/**
	 * Set uniform block.
	 * @param Index
	 * @param Uniform buffer to set to.
	 */
	virtual void						setUniformBlock( BcU32 Index, RsUniformBuffer* Buffer ) = 0;


	/**
	 * Get vertex attribute list.
	 */
	virtual const RsProgramVertexAttributeList& getVertexAttributeList() const = 0;

};

#endif

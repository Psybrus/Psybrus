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
	 * Find sampler.
	 */
	virtual BcU32						findSampler( const BcChar* Name ) = 0;

	/**
	 * Set sampler.
	 */
	virtual void						setSampler( BcU32 Handle, BcU32 SamplerSlotIdx ) = 0;
	
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
	 * Bind program.
	 * @param pParameterBuffer Pointer to parameter buffer for binding.
	 */
	virtual void						bind() = 0;
	

	/**
	 * Get vertex attribute list.
	 */
	virtual const RsProgramVertexAttributeList& getVertexAttributeList() const = 0;

	/**
	 * Log out shaders.
	 */
	virtual void logShaders() const = 0;

};

#endif

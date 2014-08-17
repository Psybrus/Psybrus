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
#include "System/Renderer/RsBuffer.h"

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
	virtual BcU32 findSamplerSlot( const BcChar* Name ) = 0;

	/**
	 * Find uniform block by name.
	 * @param Name
	 * @return Index.
	 */
	virtual BcU32 findUniformBufferSlot( const BcChar* Name ) = 0;
	
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

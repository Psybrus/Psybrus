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

#include "RsTypes.h"
#include "RsTexture.h"

////////////////////////////////////////////////////////////////////////////////
// RsProgram
class RsProgram:
	public RsResource
{
public:
	RsProgram();
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
	 * @return Success.
	 */
	virtual BcU32						findParameterOffset( const std::string& Name, eRsShaderParameterType& Type, BcU32& Offset ) const = 0;
	
	/**
	 * Bind program.
	 * @param pParameterBuffer Pointer to parameter buffer for binding.
	 */
	virtual void						bind( void* pParameterBuffer ) = 0;
};

#endif

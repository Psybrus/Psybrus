/**************************************************************************
*
* File:		RsResourceInterface.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSRESOURCEINTERFACE_H__
#define __RSRESOURCEINTERFACE_H__

#include "System/Renderer/RsTypes.h"

//////////////////////////////////////////////////////////////////////////
// RsResourceInterface
class RsResourceInterface
{
public:
	virtual ~RsResourceInterface(){};

	/**
	 * Create buffer.
	 * @param Buffer Buffer to create from.
	 */
	virtual bool createBuffer( 
		class RsBuffer* Buffer ) = 0;

	/**
	 * Destroy buffer.
	 * @param Buffer Buffer to destroy from.
	 */
	virtual bool destroyBuffer( 
		class RsBuffer* Buffer ) = 0;

	/**
	 * Update buffer.
	 * @param Buffer to update.
	 * @param Offset to update.
	 * @param Size to update.
	 * @param Flags for update.
	 * @param UpdateFunc to call.
	 */
	virtual bool updateBuffer( 
		class RsBuffer* Buffer,
		BcSize Offset,
		BcSize Size,
		RsResourceUpdateFlags Flags,
		RsBufferUpdateFunc UpdateFunc ) = 0;

	/**
	 * Create texture.
	 * @param Texture Texture to create from.
	 */
	virtual bool createTexture( 
		class RsTexture* Texture ) = 0;

	/**
	 * Destroy texture.
	 * @param Texture Texture to destroy.
	 */
	virtual bool destroyTexture( 
		class RsTexture* Texture ) = 0;

	/**
	 * Update texture
	 * @param Texture to update.
	 */
	virtual bool updateTexture( 
		class RsTexture* Texture,
		const struct RsTextureSlice& Slice,
		RsResourceUpdateFlags Flags,
		RsTextureUpdateFunc UpdateFunc ) = 0;

	/**
	 * Create shader.
	 * @param Shader Shader to create from.
	 */
	virtual bool createShader(
		class RsShader* Shader ) = 0;

};

#endif

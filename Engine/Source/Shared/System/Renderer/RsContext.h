/**************************************************************************
*
* File:		RsContext.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSCONTEXT_H__
#define __RSCONTEXT_H__

#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsFeatures.h"
#include "System/Renderer/RsResource.h"
#include "System/Renderer/RsResourceInterface.h"
#include "System/Renderer/RsDrawInterface.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class OsClient;

//////////////////////////////////////////////////////////////////////////
// RsContext
class RsContext:
	public RsResource,
	public RsResourceInterface,
	public RsDrawInterface
{
public:
	enum
	{
		MAX_RENDER_TARGETS = 8,
		MAX_VERTEX_STREAMS = 16,
		MAX_UNIFORM_SLOTS = 16,
		MAX_TEXTURE_SLOTS = 16,
		MAX_SAMPLER_SLOTS = 16,
	};

public:
	RsContext( RsContext* pParentContext ):
		RsResource( pParentContext )
	{}
	virtual ~RsContext();

	/**
 	 * Get width.
	 * Thread safe.
	 */
	virtual BcU32 getWidth() const = 0;

	/**
 	 * Get height.
	 * Thread safe.
	 */
	virtual BcU32 getHeight() const = 0;

	/**
	 * Get client.
	 * Thread safe.
	 */
	virtual OsClient* getClient() const = 0;

	/**
	 * Get features.
	 * Thread safe.
	 */
	virtual const RsFeatures& getFeatures() const = 0;

	/** 
	 * Is shader code type supported?
	 * Thread safe.
	 * TODO: Deprecate, use getFeatures.
	 */
	virtual BcBool isShaderCodeTypeSupported( RsShaderCodeType CodeType ) const = 0;

	/** 
	 * Get max shader type supported.
	 * Thread safe.
	 * TODO: Deprecate, use getFeatures.
	 */
	virtual RsShaderCodeType maxShaderCodeType( RsShaderCodeType CodeType ) const = 0;

	/** 
	 * Present back buffer.
	 */
	virtual void presentBackBuffer() = 0;

	/**
	 * Requests a screenshot from the context.
	 */
	virtual void takeScreenshot() = 0;
};

#endif

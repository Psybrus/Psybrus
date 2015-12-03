/**************************************************************************
*
* File:		RsResource.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Renderer resource.
*		
*
*
* 
**************************************************************************/

#ifndef __RSRESOURCE_H__
#define __RSRESOURCE_H__

#include "System/Renderer/RsTypes.h"
#include "System/SysResource.h"

/**
* Render data input descriptor.
*/
struct RsResourceDataInputDesc
{
	RsResourceDataInputDesc( void* pData, BcU32 DataSize );
	RsResourceDataInputDesc( void* pData, BcU32 DataSize, BcU32 TargetOffset );

	void* pData_;
	BcU32 DataSize_;
	BcU32 TargetOffset_;
};

////////////////////////////////////////////////////////////////////////////////
// RsResourceType
enum class RsResourceType
{
	BUFFER,
	CONTEXT,
	FRAMEBUFFER,
	GEOMETRY_BINDING,
	RENDER_STATE,
	SAMPLER_STATE,
	SHADER,
	TEXTURE,
	PROGRAM,
	PROGRAM_BINDING,
	VERTEX_DECLARATION,
};

////////////////////////////////////////////////////////////////////////////////
// RsResource
class RsResource:
	public SysResource
{
public:
	RsResource( RsResourceType ResourceType, class RsContext* pContext );
	virtual ~RsResource();

	/**
	 * Gt resource type.
	 */
	RsResourceType getResourceType() const { return ResourceType_; }

	/**
	 * Get context resource belongs to.
	 */
	class RsContext* getContext();

	/**
	 * Set debug name.
	 */
	void setDebugName( const BcChar* DebugName );

	/**
	 * Get debug name.
	 */
	const BcChar* getDebugName() const;

private:
	friend class RsCoreImpl;

	RsResourceType ResourceType_;
	class RsContext* pContext_;		///!< Owning/parent context.
#if PSY_DEBUG
	std::string DebugName_;
#endif
};

#endif

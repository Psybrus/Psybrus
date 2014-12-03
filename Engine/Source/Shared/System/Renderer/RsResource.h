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
// RsResource
class RsResource:
	public SysResource
{
public:
	RsResource( class RsContext* pContext );
	virtual ~RsResource();

	/**
	 * Get context resource belongs to.
	 */
	class RsContext*				getContext();

private:
	friend class RsCoreImpl;

	class RsContext*				pContext_;		///!< Owning/parent context.
	std::atomic< int >				Refs_;			///!< Refs.
};

#endif

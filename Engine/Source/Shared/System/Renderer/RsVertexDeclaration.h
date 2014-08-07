/**************************************************************************
*
* File:		RsVertexDeclaration.h
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSVERTEXDECLARATION_H__
#define __RSVERTEXDECLARATION_H__

#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsResource.h"
#include "System/Renderer/RsVertex.h"

#include <vector>

/**
 * RsVertexDeclarationDesc
 */
struct RsVertexDeclarationDesc
{
	RsVertexDeclarationDesc( BcU32 NoofElements = 0 );

	RsVertexDeclarationDesc& addElement( const RsVertexElement& Element );

	BcU32 getHash() const;
	BcU32 getMinimumStride() const;

	std::vector< RsVertexElement > Elements_;
};

//////////////////////////////////////////////////////////////////////////
// RsVertexDeclaration
class RsVertexDeclaration:
	public RsResource
{
public:
	RsVertexDeclaration( class RsContext* pContext, const RsVertexDeclarationDesc& Desc );
	virtual ~RsVertexDeclaration();

	const RsVertexDeclarationDesc& getDesc() const;

protected:
	RsVertexDeclarationDesc Desc_;
};

#endif

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

#include <vector>

/**
 * RsVertexElement
 */
struct RsVertexElement
{
	RsVertexElement();

	RsVertexElement( BcU32 StreamIdx,
	                 BcU32 Offset,
	                 BcU32 Components,
	                 RsVertexDataType DataType,
	                 RsVertexUsage Usage,
	                 BcU32 UsageIdx );

	bool operator==( const RsVertexElement& Other ) const;
	bool operator!=( const RsVertexElement& Other ) const;

	BcU32 getElementSize() const;


	BcU32				StreamIdx_;
	BcU32				Offset_;
	BcU32				Components_;
	RsVertexDataType	DataType_;
	RsVertexUsage		Usage_;
	BcU32				UsageIdx_;
};

/**
 * RsVertexDeclarationDesc
 */
struct RsVertexDeclarationDesc
{
	RsVertexDeclarationDesc( size_t NoofElements = 0 );
	RsVertexDeclarationDesc( RsVertexElement* Elements, size_t NoofElements );

	RsVertexDeclarationDesc& addElement( const RsVertexElement& Element );

	bool operator ==( const RsVertexDeclarationDesc& Other ) const;
	bool operator !=( const RsVertexDeclarationDesc& Other ) const;

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
	BcU32 getInputLayoutHash() const;

protected:
	RsVertexDeclarationDesc Desc_;
	BcU32 InputLayoutHash_;
};

#endif

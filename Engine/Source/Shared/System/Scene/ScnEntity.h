/**************************************************************************
*
* File:		ScnEntity.h
* Author:	Neil Richardson 
* Ver/Date:	26/11/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnEntity_H__
#define __ScnEntity_H__

#include "RsCore.h"
#include "CsResource.h"

#include "ScnTypes.h"

#include "ScnComponent.h"
#include "ScnRenderableComponent.h"

#include "ScnTransform.h"
#include "ScnSpacialTree.h"
#include "ScnVisitor.h"

//////////////////////////////////////////////////////////////////////////
// ScnEntity
class ScnEntity:
	public CsResource
{
public:
	DECLARE_RESOURCE( CsResource, ScnEntity );
	DECLARE_VISITABLE( ScnEntity );
	
#if PSY_SERVER
	virtual BcBool						import( const Json::Value& Object, CsDependancyList& DependancyList );
#endif	
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();
	
public:
	virtual void						update( BcReal Tick );
	virtual void						render( RsFrame* pFrame, RsRenderSort Sort ); // NEILO TODO: Don't implement here. Test code only.
	virtual void						attach( ScnComponent* Component );
	virtual void						detach( ScnComponent* Component );

	virtual void						onAttachScene();
	virtual void						onDetachScene();

	/**
 	 * Get AABB which encompasses this entity.
	 */
	const BcAABB&						getAABB() const;

	/**
	 * Set the spacial tree node we belong in.
	 */
	void								setSpacialTreeNode( ScnSpacialTreeNode* pNode );

	/**
	 * Get the spacial tree node we are in.
	 */
	ScnSpacialTreeNode*					getSpacialTreeNode();

protected:
	virtual void						fileReady();
	virtual void						fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData );
	
protected:

	struct THeader
	{
	};
	
	THeader*							pHeader_;

	ScnComponentList					Components_;
	ScnTransform						Transform_;

private:
	ScnSpacialTreeNode*					pSpacialTreeNode_;
	BcBool								IsAttached_;
};

#endif

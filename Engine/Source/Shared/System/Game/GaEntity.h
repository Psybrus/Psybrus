/**************************************************************************
*
* File:		GaEntity.h
* Author:	Neil Richardson 
* Ver/Date:	24/07/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __GaEntity_H__
#define __GaEntity_H__

#include "CsResourceRef.h"

#include "GaTransform.h"
#include "GaComponent.h"

//////////////////////////////////////////////////////////////////////////
// GaEntityRef
typedef CsResourceRef< class GaEntity > GaEntityRef;
typedef std::vector< GaEntityRef > GaEntityList;
typedef GaEntityList::iterator GaEntityListIterator;

//////////////////////////////////////////////////////////////////////////
// GaEntity
class GaEntity:
	public CsResource
{
public:
	DECLARE_RESOURCE( CsResource, GaEntity );

	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();
	
public:
	/**
	 * Attach component.
	 */
	void								attachComponent( GaComponent* pComponent );
	
	/**
	 * Detach component.
	 */
	void								detachComponent( GaComponent* pComponent );
	
	/**
	 * Detach all components.
	 */ 
	void								detachAllComponents();
	
	/**
	 * Set transform.
	 */
	void								setTransform( const GaTransform& Transform );
	
	/**
	 * Get transform
	 */
	const GaTransform&					getTransform() const;
	
	/**
	 * Update entity.
	 */
	virtual void						update( BcReal Tick );
	
private:
	GaTransform							Transform_;
	
	GaComponentList						ComponentList_;
};

#endif



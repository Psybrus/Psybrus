/**************************************************************************
*
* File:		GaComponent.h
* Author:	Neil Richardson 
* Ver/Date:	24/07/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __GaComponent_H__
#define __GaComponent_H__

#include "CsResourceRef.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class GaEntity;

//////////////////////////////////////////////////////////////////////////
// GaComponentRef
typedef CsResourceRef< class GaComponent > GaComponentRef;
typedef std::vector< GaComponentRef > GaComponentList;
typedef GaComponentList::iterator GaComponentListIterator;

//////////////////////////////////////////////////////////////////////////
// GaComponent
class GaComponent:
	public CsResource
{
public:
	DECLARE_RESOURCE( CsResource, GaComponent );
	
	/**
	 * Reattach component to owner.
	 */
	void					reAttach();
	
public:
	/**
	 *	Pre-attach call.
	 */
	virtual void			preAttach( GaEntity* pOwner );
	
	/**
	 *	Post-attach call.
	 */
	virtual void			postAttach();
	
	/**
	 *	Pre-detach call.
	 */
	virtual void			preDetach();
	
	/**
	 *	Post-detach call.
	 */
	virtual void			postDetach();

	/**
	 * Update component.
	 */
	virtual void			update( BcReal Tick );
	
private:
	GaEntity*				pOwner_;
	
};

#endif

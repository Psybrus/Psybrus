/**************************************************************************
*
* File:		ScnComponent.h
* Author:	Neil Richardson 
* Ver/Date:	26/11/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnComponent_H__
#define __ScnComponent_H__

#include "RsCore.h"
#include "CsResource.h"

#include "ScnTypes.h"

//////////////////////////////////////////////////////////////////////////
// ScnComponent
class ScnComponent:
	public CsResource
{
public:
	DECLARE_RESOURCE( CsResource, ScnComponent );
	
public:
	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

	BcBool								isAttached() const;
	BcBool								isAttached( ScnEntityWeakRef Parent ) const;
	ScnEntityWeakRef					getParentEntity();

protected:

	struct THeader
	{
	};
	
	THeader*							pHeader_;

	ScnEntityWeakRef					ParentEntity_;
};

#endif

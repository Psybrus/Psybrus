/**************************************************************************
*
* File:		ScnRenderableComponent.h
* Author:	Neil Richardson 
* Ver/Date:	28/12/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnRenderableComponent_H__
#define __ScnRenderableComponent_H__

#include "RsCore.h"
#include "ScnTypes.h"
#include "ScnComponent.h"

//////////////////////////////////////////////////////////////////////////
// Typedefs
typedef CsResourceRef< class ScnRenderableComponent > ScnRenderableComponentRef;
typedef CsResourceRef< class ScnRenderableComponent, false > ScnRenderableComponentWeakRef;

//////////////////////////////////////////////////////////////////////////
// ScnRenderableComponent
class ScnRenderableComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, ScnRenderableComponent );
	DECLARE_VISITABLE( ScnRenderableComponent );

public:
	void								initialise();
	virtual void						update( BcReal Tick );
	virtual void						render( RsFrame* pFrame, RsRenderSort Sort );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );
};

#endif

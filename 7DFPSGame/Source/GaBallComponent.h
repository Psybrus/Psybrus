/**************************************************************************
*
* File:		GaBallComponent.h
* Author:	Neil Richardson 
* Ver/Date:	29/12/11	
* Description:
*		Example user component.
*		
*
*
* 
**************************************************************************/

#ifndef __GABALLCOMPONENT_H__
#define __GABALLCOMPONENT_H__

#include "Psybrus.h"

//////////////////////////////////////////////////////////////////////////
// GaExampleComponentRef
typedef CsResourceRef< class GaBallComponent > GaBallComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaBallComponent
class GaBallComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaBallComponent );

	void								initialise( const Json::Value& Object );

	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

private:
	BcReal								Rotation_;
	BcVec3d								Velocity_;
};

#endif

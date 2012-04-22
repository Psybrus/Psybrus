/**************************************************************************
*
* File:		GaSunComponent.h
* Author:	Neil Richardson 
* Ver/Date:	22/04/12
* Description:
*	
*		
*
*
* 
**************************************************************************/

#ifndef __GaSunComponent_H__
#define __GaSunComponent_H__

#include "Psybrus.h"

//////////////////////////////////////////////////////////////////////////
// GaExampleComponentRef
typedef CsResourceRef< class GaSunComponent > GaSunComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaSunComponent
class GaSunComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaSunComponent );

	void								initialise( const Json::Value& Object );

	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );
	
private:
	friend class GaGameComponent;
	
	RsColour							Colour_;
	BcReal								Radius_;
	BcReal								RadiusMult_;
	BcReal								RotationMult_;
	BcReal								RotationSpeed_;

	BcReal								Rotation_;

	std::vector< ScnModelComponentRef >		Models_;
	std::vector< ScnMaterialComponentRef >	Materials_;
	std::vector< BcU32 >					MaterialColourParams_;
};

#endif

/**************************************************************************
*
* File:		Rendering/ScnSpriteComponent.h
* Author:	Neil Richardson 
* Ver/Date:	23/04/12
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnSpriteComponent_H__
#define __ScnSpriteComponent_H__

#include "System/Scene/ScnComponent.h"
#include "System/Scene/Rendering/ScnCanvasComponent.h"

//////////////////////////////////////////////////////////////////////////
// ScnSpriteComponentRef
typedef ReObjectRef< class ScnSpriteComponent > ScnSpriteComponentRef;

//////////////////////////////////////////////////////////////////////////
// ScnSpriteComponent
class ScnSpriteComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnSpriteComponent, ScnComponent );
	
	virtual void						initialise();
	virtual void						initialise( const Json::Value& Object );
	virtual void						postUpdate( BcF32 Tick );
	
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

private:
	ScnCanvasComponentRef				Canvas_;
	ScnMaterialComponentRef				Material_;
	std::string							MaterialName_;

	MaVec2d								Position_;
	MaVec2d								Size_;
	RsColour							Colour_;
	BcU32								Index_;
	BcU32								Layer_;
	BcBool								Center_;
};


#endif



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
// ScnSpriteAnimation
struct ScnSpriteAnimation
{
	REFLECTION_DECLARE_BASIC( ScnSpriteAnimation );
	ScnSpriteAnimation(){};
	std::string Next_;
	std::vector< BcS32 > Keys_;
};

//////////////////////////////////////////////////////////////////////////
// ScnSpriteComponent
class ScnSpriteComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( ScnSpriteComponent, ScnComponent );
	
	ScnSpriteComponent();
	virtual ~ScnSpriteComponent();
	
	virtual void onAttach( ScnEntityWeakRef Parent );
	virtual void onDetach( ScnEntityWeakRef Parent );

	const MaVec2d& getPosition() const;
	void setPosition( const MaVec2d& Position );

	const MaVec2d& getSize() const;
	void setSize( const MaVec2d& Size );

	const RsColour& getColour() const;
	void setColour( const RsColour& Colour );

	BcF32 getRotation() const;
	void setRotation( BcF32 Rotation );

	void setAnimation( std::string Animation );
	void setMaterial( ScnMaterialComponentRef Material );

	void setSpriteIndex( BcU32 Index );

private:
	void updateAnimation( BcF32 Tick );
	void draw();

	static void update( const ScnComponentList& Components );

private:
	ScnCanvasComponentRef Canvas_;
	ScnMaterialComponentRef Material_;
	std::string MaterialName_;

	MaVec2d Position_;
	MaVec2d Size_;
	RsColour Colour_;
	BcS32 Index_;
	BcU32 Layer_;
	BcBool Center_;
	BcBool IsScreenSpace_;
	BcF32 Rotation_;

	BcU32 CurrKey_;
	BcF32 AnimationTimer_;
	BcF32 AnimationRate_;

	std::string Animation_;
	std::map< std::string, ScnSpriteAnimation > Animations_;
};


#endif



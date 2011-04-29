/**************************************************************************
*
* File:		SgVisitor.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Scenegraph Visitor
*			
*
*
* 
**************************************************************************/

#ifndef __SgVisitor_H__
#define __SgVisitor_H__

#include "SgTypes.h"

//////////////////////////////////////////////////////////////////////////
// Macro
#define DECLARE_VISITABLE( _ClassName ) \
	virtual void visit_accept( SgVisitor* pVisitor ) \
	{ \
		pVisitor->visit( (_ClassName*)this ); \
	}

//////////////////////////////////////////////////////////////////////////
// ScnVisitor
class ScnVisitor
{
public:
	ScnVisitor();
	virtual ~ScnVisitor();

	/**
	*	Called before recursing into the scene.
	*/
	virtual void preScene();

	/**
	*	Called after recursing into the scene.
	*/
	virtual void postScene();

	/**
	*	Called when an entity is hit.
	*/
	virtual void visit( ScnEntity* pEntity );

private:

};


#endif

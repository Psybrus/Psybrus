/**************************************************************************
*
* File:		SgVisitor.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Scenegraph Visitor
*			
*
*
* 
**************************************************************************/

#include "System/Scene/ScnVisitor.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnVisitor::ScnVisitor()
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnVisitor::~ScnVisitor()
{

}

//////////////////////////////////////////////////////////////////////////
// preVisit
//virtual
void ScnVisitor::preScene()
{

}

//////////////////////////////////////////////////////////////////////////
// postVisit
//virtual
void ScnVisitor::postScene()
{

}

//////////////////////////////////////////////////////////////////////////
// visit
//virtual
void ScnVisitor::visit( class ScnSpatialComponent* pComponent )
{

}

//////////////////////////////////////////////////////////////////////////
// visit
//virtual
void ScnVisitor::visit( class ScnRenderableComponent* pComponent )
{

}

//////////////////////////////////////////////////////////////////////////
// visit
//virtual
void ScnVisitor::visit( class ScnLightComponent* pComponent )
{

}


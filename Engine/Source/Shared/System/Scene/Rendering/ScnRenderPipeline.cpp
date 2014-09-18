/**************************************************************************
*
* File:		ScnRenderPipeline.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Rendering/ScnRenderPipeline.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnRenderPipeline );

void ScnRenderPipeline::StaticRegisterClass()
{
	/*
	ReField* Fields[] = 
	{

	};
	*/
	
	ReRegisterClass< ScnRenderPipeline, Super >();
}

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnRenderPipeline::initialise()
{
	Super::initialise();
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual 
void ScnRenderPipeline::destroy()
{
	Super::destroy();
}

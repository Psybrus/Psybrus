/**************************************************************************
*
* File:		ScnComponentImport.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Import/ScnComponentImport.h"


#ifdef PSY_SERVER
#include "Base/BcFile.h"
#include "Base/BcStream.h"

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( ScnComponentImport )
	
void ScnComponentImport::StaticRegisterClass()
{
	/*
	ReField* Fields[] = 
	{
		new ReField( "Source_", &ScnComponentImport::Source_ ),
	};
	*/
		
	ReRegisterClass< ScnComponentImport, Super >();
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnComponentImport::ScnComponentImport()
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnComponentImport::ScnComponentImport( ReNoInit )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnComponentImport::~ScnComponentImport()
{

}

//////////////////////////////////////////////////////////////////////////
// import
BcBool ScnComponentImport::import(
		const Json::Value& Object )
{
	// Write out object to be used later.
	Json::FastWriter Writer;
	std::string JsonData = Writer.write( Object );

	//
	CsResourceImporter::addChunk( BcHash( "object" ), JsonData.c_str(), JsonData.size() + 1 );
	return BcTrue;
}

#endif

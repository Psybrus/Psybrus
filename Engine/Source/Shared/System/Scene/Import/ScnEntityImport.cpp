/**************************************************************************
*
* File:		ScnEntityImport.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Import/ScnEntityImport.h"

#ifdef PSY_SERVER
#include "Base/BcFile.h"
#include "Base/BcStream.h"

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( ScnEntityImport )
	
void ScnEntityImport::StaticRegisterClass()
{
	/*
	ReField* Fields[] = 
	{
		new ReField( "Source_", &ScnEntityImport::Source_ ),
	};
	*/
		
	ReRegisterClass< ScnEntityImport, Super >();
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnEntityImport::ScnEntityImport()
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnEntityImport::~ScnEntityImport()
{

}

//////////////////////////////////////////////////////////////////////////
// import
BcBool ScnEntityImport::import(
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

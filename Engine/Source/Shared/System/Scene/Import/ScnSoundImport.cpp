/**************************************************************************
*
* File:		ScnSoundImport.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Import/ScnSoundImport.h"

#include "Serialisation/SeJsonReader.h"
#include "Serialisation/SeJsonWriter.h"
#include "System/Scene/ScnComponent.h"
#include "System/Content/CsSerialiserPackageObjectCodec.h"

#ifdef PSY_SERVER
#include "Base/BcFile.h"
#include "Base/BcStream.h"

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( ScnSoundImport )
	
void ScnSoundImport::StaticRegisterClass()
{
	/*
	ReField* Fields[] = 
	{
		new ReField( "Source_", &ScnSoundImport::Source_ ),
	};
	*/
		
	ReRegisterClass< ScnSoundImport, Super >();
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnSoundImport::ScnSoundImport()
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnSoundImport::ScnSoundImport( ReNoInit )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnSoundImport::~ScnSoundImport()
{

}

//////////////////////////////////////////////////////////////////////////
// import
BcBool ScnSoundImport::import(
		const Json::Value& Object )
{
	Header_.SampleRate_ = 0;
	Header_.Channels_ = 1;
	Header_.Looping_ = BcFalse;
	Header_.IsOgg_ = BcFalse;
	CsResourceImporter::addChunk( BcHash( "header" ), &Header_, sizeof( Header_ ) );
	CsResourceImporter::addChunk( BcHash( "sample" ), &Header_, sizeof( Header_ ) );
	return BcTrue;
}

#endif

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
	ReField* Fields[] = 
	{
		new ReField( "Source_", &ScnSoundImport::Source_, bcRFF_IMPORTER ),
		new ReField( "IsStream_", &ScnSoundImport::IsStream_, bcRFF_IMPORTER ),
	};

	ReRegisterClass< ScnSoundImport, Super >( Fields );
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
	// Check file type.
	if( Source_.rfind( ".ogg" ) != std::string::npos ||
		Source_.rfind( ".wav" ) != std::string::npos )
	{
		if( IsStream_ )
		{
			FileData_.Type_ = SsSourceFileData::WAVSTREAM;
		}
		else
		{
			FileData_.Type_ = SsSourceFileData::WAV;
		}
	}
	else if( Source_.rfind( ".sfxr" ) != std::string::npos )
	{
		FileData_.Type_ = SsSourceFileData::SFXR;
	}
	else if( Source_.rfind( ".mod" ) != std::string::npos ||
		Source_.rfind( ".s3m" ) != std::string::npos ||
		Source_.rfind( ".it" ) != std::string::npos ||
		Source_.rfind( ".xm" ) != std::string::npos )
	{
		FileData_.Type_ = SsSourceFileData::MODPLUG;
	}

	FileData_.FileHash_= addFile( Source_ );

	CsResourceImporter::addChunk( BcHash( "filedata" ), &FileData_, sizeof( FileData_ ) );
	return BcTrue;
}

#endif

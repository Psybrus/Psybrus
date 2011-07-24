/**************************************************************************
*
* File:		ScnScript.cpp
* Author:	Neil Richardson 
* Ver/Date:	5/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "gmMachine.h"
#include "gmStream.h"

#include "ScnScript.h"
#include "GaCore.h"

#ifdef PSY_SERVER
#include "BcStream.h"
#include "BcFile.h"
#endif

#ifdef PSY_SERVER

//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnScript::import( const Json::Value& Object, CsDependancyList& DependancyList )
{
	const std::string& FileName = Object[ "source" ].asString();
	BcFile File;
	
	// Add root dependancy.
	DependancyList.push_back( FileName );

	if( File.open( FileName.c_str(), bcFM_READ ) )
	{
		BcBool Success = BcTrue;
		
		// Read entire script in.
		BcChar* pScript = new BcChar[ File.size() + 1 ];
		BcMemSet( pScript, 0, File.size() + 1 );
		File.read( pScript, File.size() );
		
		
		// Construct a machine to check the script.
		gmMachine GmMachine;
		int RetVal = GmMachine.CheckSyntax( pScript );
		if( RetVal == 0 )
		{
			// No errors, pack it.
			BcStream ScriptStream;
			
			ScriptStream.push( pScript, File.size() );
			ScriptStream << BcU8( 0 );
			pFile_->addChunk( BcHash( "script" ), ScriptStream.pData(), ScriptStream.dataSize() );
			delete [] pScript;
			return BcTrue;
		}
		delete [] pScript;

		// Log errors found.
		gmLog& GmLog = GmMachine.GetLog();
		bool First = true;
		const char* Entry = NULL;
		while( Entry = GmLog.GetEntry( First ) ) // Yes this is an assign. It's intended!
		{
			BcPrintf( "GmLog: %s\n", Entry );
		}
	}
	return BcFalse;
}
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnScript );

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnScript::initialise()
{
	pScript_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnScript::create()
{

}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnScript::destroy()
{
	
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool ScnScript::isReady()
{
	return pScript_ != NULL;
}

//////////////////////////////////////////////////////////////////////////
// execute
int ScnScript::execute( BcBool Now )
{
	return GaCore::pImpl()->executeScript( pScript_, getName().c_str(), Now );
}

//////////////////////////////////////////////////////////////////////////
// fileReady
void ScnScript::fileReady()
{
	// File is ready, get the header chunk.
	getChunk( 0 );
}

//////////////////////////////////////////////////////////////////////////
//fileChunkReady
void ScnScript::fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData )
{
	if( pChunk->ID_ == BcHash( "script" ) )
	{
		// TODO: Compile the script.
		pScript_ = (const char*)pData;
	}
}


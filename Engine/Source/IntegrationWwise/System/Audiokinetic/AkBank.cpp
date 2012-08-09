/**************************************************************************
*
* File:		AkBank.h
* Author:	Neil Richardson 
* Ver/Date:		
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Audiokinetic/AkBank.h"

#if PSY_SERVER
//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool AkBank::import( class CsPackageImporter& Importer, const Json::Value& Object )
{
	return BcFalse;
}
#endif	

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( AkBank );

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void AkBank::initialise()
{
	pBankName_ = NULL;
	BankID_ = BcErrorCode;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void AkBank::create()
{
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void AkBank::destroy()
{
	if( pBankName_ != NULL )
	{
		AK::SoundEngine::UnloadBank( pBankName_ );
	}
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool AkBank::isReady()
{
	return pBankName_ != NULL;
}

//////////////////////////////////////////////////////////////////////////
// fileReady
//virtual
void AkBank::fileReady()
{
	requestChunk( 0, &Header_ );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
//virtual
void AkBank::fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData )
{
	if( ChunkID == BcHash( "header" ) )
	{
		pBankName_ = getString( Header_.BankNameID_ );
		if ( AK::SoundEngine::LoadBank( pBankName_, AK_DEFAULT_POOL_ID, BankID_ ) != AK_Success )
		{
			BcBreakpoint;
		}
	};
}

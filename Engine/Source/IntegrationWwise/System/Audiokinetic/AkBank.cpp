/**************************************************************************
*
* File:		AkBank.cpp
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
#include "System/Audiokinetic/AkCore.h"

#include "System/Content/CsCore.h"

#include "System/Scene/ScnEntity.h"

#if PSY_SERVER
#include "System/Content/CsPackageImporter.h"
#include "Base/BcStream.h"

//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool AkBank::import( class CsPackageImporter& Importer, const Json::Value& Object )
{
	// Temporary hack until post-LD.
	std::string BankName = (*getName());

	BcStream HeaderStream;
			
	THeader Header = { Importer.addString( BankName.c_str() ) };
	HeaderStream << Header;
			
	// Add chunks and finish up.
	Importer.addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize(), 16, csPCF_IN_PLACE );

	return BcTrue;
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
	if( AkCore::pImpl() && pBankName_ )
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
		if( AkCore::pImpl() )
		{
			if ( AK::SoundEngine::LoadBank( pBankName_, AK_DEFAULT_POOL_ID, BankID_ ) != AK_Success )
			{
				BcVerifyMsg( BcFalse, "Unable to load Wwise sound bank \"%s\". Have they been generated from the Wwise tool?", pBankName_ );
			}
		}
	};
}

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( AkBankComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void AkBankComponent::initialise( const Json::Value& Object )
{
	Bank_ = CsCore::pImpl()->getResource( Object["name"].asCString() );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void AkBankComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void AkBankComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );
}

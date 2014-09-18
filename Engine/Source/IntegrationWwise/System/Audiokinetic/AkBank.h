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

#ifndef __AKBANK_H__
#define __AKBANK_H__

#include "System/Scene/ScnComponent.h"

#include "System/Audiokinetic/AkEvents.h"

//////////////////////////////////////////////////////////////////////////
// AkBankRef
typedef CsResourceRef< class AkBank > AkBankRef;

//////////////////////////////////////////////////////////////////////////
// AkBank
class AkBank:
	public CsResource
{
public:
	DECLARE_RESOURCE( CsResource, AkBank );

#if PSY_IMPORT_PIPELINE
	virtual BcBool						import( class CsPackageImporter& Importer, const Json::Value& Object );
#endif	
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();

	virtual void						fileReady();
	virtual void						fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData );

private:
	struct THeader
	{
		BcU32							BankNameID_;
	};

	THeader								Header_;
	const BcChar*						pBankName_;
	AkBankID							BankID_;
};

//////////////////////////////////////////////////////////////////////////
// AkBankComponentRef
typedef CsResourceRef< class AkBankComponent > AkBankComponentRef;

//////////////////////////////////////////////////////////////////////////
// AkBankComponent
class AkBankComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, AkBankComponent );

	virtual void						initialise( const Json::Value& Object );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

private:
	AkBankRef							Bank_;
};

#endif // __AKBANK_H__

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

#include "System/Content/CsResource.h"

#include "AkIncludes.h"

//////////////////////////////////////////////////////////////////////////
// AkBank
class AkBank:
	public CsResource
{
public:
	DECLARE_RESOURCE( CsResource, AkBank );

#if PSY_SERVER
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

#endif // __AKBANK_H__

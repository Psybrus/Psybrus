/**************************************************************************
*
* File:		ScnTexture.h
* Author:	Neil Richardson 
* Ver/Date:	5/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNTEXTURE_H__
#define __SCNTEXTURE_H__

#include "RsCore.h"
#include "CsResourceRef.h"

//////////////////////////////////////////////////////////////////////////
// ScnTextureRef
typedef CsResourceRef< class ScnTexture > ScnTextureRef;

//////////////////////////////////////////////////////////////////////////
// ScnTexture
class ScnTexture:
	public CsResource
{
public:
	DECLARE_RESOURCE( ScnTexture );
	
#if PSY_SERVER
	virtual BcBool						import( const Json::Value& Object );
#endif	
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();
	
	void								setup();
	
private:
	void								fileReady();
	void								fileChunkReady( const CsFileChunk* pChunk, void* pData );

private:
	RsTexture*							pTexture_;
	
	struct THeader
	{
		BcU32							Width_;
		BcU32							Height_;
		BcU32							Levels_;
		eRsTextureFormat				Format_;
	};
	
	THeader*							pHeader_;
	void*								pTextureData_;
	BcBool								CreateNewTexture_;
};

#endif

/**************************************************************************
*
* File:		ScnFont.h
* Author:	Neil Richardson 
* Ver/Date:	16/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnFont_H__
#define __ScnFont_H__

#include "CsResourceRef.h"

//////////////////////////////////////////////////////////////////////////
// ScnFontRef
typedef CsResourceRef< class ScnFont > ScnFontRef;

//////////////////////////////////////////////////////////////////////////
// ScnFont
class ScnFont:
	public CsResource
{
public:
	DECLARE_RESOURCE( ScnFont );
	
#ifdef PSY_SERVER
	virtual BcBool						import( const Json::Value& Object );
#endif
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();
	
private:
	void								fileReady();
	void								fileChunkReady( const CsFileChunk* pChunk, void* pData );
	
private:
	
};


#endif



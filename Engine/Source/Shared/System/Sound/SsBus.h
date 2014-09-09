/**************************************************************************
 *
 * File:		SsBus.h
 * Author:		Neil Richardson
 * Ver/Date:	
 * Description:
 *		
 *
 *
 *
 * 
 **************************************************************************/

#ifndef __SSBUS_H__
#define __SSBUS_H__

#include "System/Sound/SsResource.h"

//////////////////////////////////////////////////////////////////////////
// SsBusParams
class SsBusParams
{
public:
	SsBusParams( BcF32 Gain = 1.0f );

	BcF32 Gain_;
};

//////////////////////////////////////////////////////////////////////////
// SsBus
class SsBus:
	public SsResource
{
public:
	SsBus( const SsBusParams& Params );
	virtual ~SsBus();

private:
	SsBusParams Params_;
};

#endif

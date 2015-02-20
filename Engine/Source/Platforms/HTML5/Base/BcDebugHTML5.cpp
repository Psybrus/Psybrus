/**************************************************************************
*
* File:		BcDebugLinux.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Debug stuff
*		
*
*
* 
**************************************************************************/

#include "Base/BcDebug.h"

#include <malloc.h>

//////////////////////////////////////////////////////////////////////////
// BcMessageBox
BcBacktraceResult BcBacktrace()
{
	BcBacktraceResult Result;

	return Result;
}

//////////////////////////////////////////////////////////////////////////
// BcMessageBox
BcMessageBoxReturn BcMessageBox( const BcChar* pTitle, const BcChar* pMessage, BcMessageBoxType Type, BcMessageBoxIcon Icon )
{
	// Log.
	PSY_LOG( "%s: %s\n", pTitle, pMessage );

	return bcMBR_OK; // bcMBR_CANCEL
}

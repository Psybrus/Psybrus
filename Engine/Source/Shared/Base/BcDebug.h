/**************************************************************************
*
* File:		BcDebug.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __BCDEBUG_H__
#define __BCDEBUG_H__

#include "BcTypes.h"

//////////////////////////////////////////////////////////////////////////
// BcPrintf
extern void BcPrintf( const BcChar* pString, ... );

//////////////////////////////////////////////////////////////////////////
// BcMessageBox
enum BcMessageBoxType
{
	bcMBT_OK = 0,
	bcMBT_OKCANCEL,
	bcMBT_YESNO,
	bcMBT_YESNOCANCEL
};

enum BcMessageBoxIcon
{
	bcMBI_WARNING = 0,
	bcMBI_ERROR,
	bcMBI_QUESTION
};

enum BcMessageBoxReturn
{
	bcMBR_OK = 0,
	bcMBR_YES = 0,
	bcMBR_NO = 1,
	bcMBR_CANCEL = 2,
};

extern BcMessageBoxReturn BcMessageBox( const BcChar* pTitle, const BcChar* pMessage, BcMessageBoxType Type = bcMBT_OK, BcMessageBoxIcon Icon = bcMBI_WARNING );

//////////////////////////////////////////////////////////////////////////
// BcAssert
extern BcBool BcAssertInternal( const BcChar* pMessage, const BcChar* pFile, int Line );

#if defined( PSY_DEBUG ) || defined( PSY_RELEASE )
#  define BcAssertMsg( Condition, Message )	\
	if( !( Condition ) ) \
	{ \
		if( !BcAssertInternal( Message, __FILE__, __LINE__ ) ) \
			BcBreakpoint; \
	}
#  define BcAssert( Condition )			BcAssertMsg( Condition, #Condition )
#  define BcPreCondition( Condition )	BcAssert( Condition )
#  define BcPostCondition( Condition )	BcAssert( Condition )
#  define BcAssertException( Condition, Exception )	\
	if( !( Condition ) ) \
	{ \
		throw Exception; \
	}
#else
#  define BcAssertMsg( Condition, Message )
#  define BcAssert( Condition )
#  define BcPreCondition( Condition )
#  define BcPostCondition( Condition )
#  define BcAssertException( Condition, Exception )	\
	if( !( Condition ) ) \
	{ \
		throw Exception; \
	}
#endif

//////////////////////////////////////////////////////////////////////////
// BcVerify
extern BcBool BcVerifyInternal( const BcChar* pMessage, const BcChar* pFile, int Line );

#if defined( PSY_DEBUG ) || defined( PSY_RELEASE )
#  define BcVerify( Condition, Message )	\
	{ \
		static BcBool ShouldNotify = BcTrue; \
		if( ShouldNotify && !( Condition ) ) \
		{ \
			if( BcVerifyInternal( Message, __FILE__, __LINE__ ) ) \
				ShouldNotify = BcFalse; \
		} \
	}
#else
#  define BcVerify( Condition, Message )
#endif

#endif


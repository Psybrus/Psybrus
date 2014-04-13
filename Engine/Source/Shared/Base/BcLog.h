/**************************************************************************
*
* File:		BcLog.h
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*		Logging system.
*		
*		
*		
* 
**************************************************************************/

#ifndef __BCLOG_H__
#define __BCLOG_H__

#include "Base/BcTypes.h"
#include "Base/BcGlobal.h"

//////////////////////////////////////////////////////////////////////////
// BcLog
class BcLog:
	public BcGlobal< BcLog >
{
public:
	BcLog(){};
	virtual ~BcLog(){};

	/**
	 * Write to log.
	 */
	virtual void write( const BcChar* pText, ... ) = 0;
	
	/**
	 * Write to log.
	 */
	virtual void write( BcU32 Catagory, const BcChar* pText, ... ) = 0;

	/**
	 * Flush log.
	 */
	virtual void flush() = 0;

	/**
	 * Set catagory suppression.
	 */
	virtual void setCatagorySuppression( BcU32 Catagory, BcBool IsSuppressed ) = 0;

	/**
	 * Get catagory suppression.
	 */
	virtual BcBool getCatagorySuppression( BcU32 Catagory ) const = 0;
};


#endif

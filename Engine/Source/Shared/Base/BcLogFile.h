/**************************************************************************
*
* File:		BcLogFile.h
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*		Logging system.
*		
*		
*		
* 
**************************************************************************/

#ifndef __BCLOGFILE_H__
#define __BCLOGFILE_H__

#include "BcLog.h"
#include "BcFile.h"

//////////////////////////////////////////////////////////////////////////
// BcLogFile
class BcLogFile:
	public BcLog
{
public:
	BcLogFile( const BcChar* pFileName );
	virtual ~BcLogFile();
	
protected:
	/**
	 * Overridable internal for all writes.
	 */
	virtual void internalWrite( const BcChar* pText );

	/**
	 * Overridable internal for flush.
	 */
	virtual void internalFlush();

private:
	BcFile OutputFile_;
};


#endif

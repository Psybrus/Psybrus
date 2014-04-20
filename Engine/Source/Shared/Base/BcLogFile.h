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

#include "Base/BcLogImpl.h"
#include "Base/BcFile.h"

//////////////////////////////////////////////////////////////////////////
// BcLogFile
class BcLogFile:
	public BcLogImpl
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

	/*
	* Get log data
	*/
	std::vector<std::string> getLogData();
private:
	BcFile OutputFile_;
};


#endif

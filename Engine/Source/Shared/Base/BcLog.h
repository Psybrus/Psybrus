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
#include "Base/BcMisc.h"
#include "Base/BcName.h"
#include <vector>

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
	 * Flush log.
	 */
	virtual void flush() = 0;

	/**
	 * Set catagory suppression.
	 */
	virtual void setCatagorySuppression( BcName Catagory, BcBool IsSuppressed ) = 0;

	/**
	 * Get catagory suppression.
	 */
	virtual BcBool getCatagorySuppression( BcName Catagory ) const = 0;

	/*
	* Get log data
	*/
	virtual std::vector<std::string> getLogData() = 0;

protected:
	friend class BcLogScopedCatagory;
	friend class BcLogScopedIndent;

	/**
	 * Set catagory.
	 */
	virtual void setCatagory( BcName Catagory ) = 0;

	/**
	 * Get catagory.
	 */
	virtual BcName getCatagory() = 0;

	/**
	 * Increase indent.
	 */
	virtual void increaseIndent() = 0;

	/**
	 * Decreate indent.
	 */
	virtual void decreaseIndent() = 0;
};

//////////////////////////////////////////////////////////////////////////
// BcLogScopedCatagory
class BcLogScopedCatagory
{
public:
	BcLogScopedCatagory( BcName Catagory )
	{
		if( BcLog::pImpl() )
		{
			OldCatagory_ = BcLog::pImpl()->getCatagory();
			BcLog::pImpl()->setCatagory( Catagory );
		}
	}

	~BcLogScopedCatagory()
	{
		if( BcLog::pImpl() )
		{
			BcLog::pImpl()->setCatagory( OldCatagory_ );
		}
	}

private:
	BcName OldCatagory_;
};

//////////////////////////////////////////////////////////////////////////
// BcLogScopedIndent
class BcLogScopedIndent
{
public:
	BcLogScopedIndent()
	{
		if( BcLog::pImpl() )
		{
			BcLog::pImpl()->increaseIndent();
		}
	}

	~BcLogScopedIndent()
	{
		if( BcLog::pImpl() )
		{
			BcLog::pImpl()->decreaseIndent();
		}
	}
};

#endif

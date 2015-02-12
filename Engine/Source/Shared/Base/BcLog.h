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
	 * Set category suppression.
	 */
	virtual void setCategorySuppression( BcName Category, BcBool IsSuppressed ) = 0;

	/**
	 * Get category suppression.
	 */
	virtual BcBool getCategorySuppression( BcName Category ) const = 0;

	/*
	* Get log data
	*/
	virtual std::vector<std::string> getLogData() = 0;

protected:
	friend class BcLogScopedCategory;
	friend class BcLogScopedIndent;

	/**
	 * Set category.
	 */
	virtual void setCategory( BcName Category ) = 0;

	/**
	 * Get category.
	 */
	virtual BcName getCategory() = 0;

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
// BcLogScopedCategory
class BcLogScopedCategory
{
public:
	BcLogScopedCategory( BcName Category )
	{
		if( BcLog::pImpl() )
		{
			OldCategory_ = BcLog::pImpl()->getCategory();
			BcLog::pImpl()->setCategory( Category );
		}
	}

	~BcLogScopedCategory()
	{
		if( BcLog::pImpl() )
		{
			BcLog::pImpl()->setCategory( OldCategory_ );
		}
	}

private:
	BcName OldCategory_;
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

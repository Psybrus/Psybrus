/**************************************************************************
*
* File:		BcLogImpl.h
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*		Logging system.
*		
*		
*		
* 
**************************************************************************/

#ifndef __BcLogImpl_H__
#define __BcLogImpl_H__

#include "Base/BcLog.h"
#include "Base/BcGlobal.h"
#include <mutex>
#include <string>
#include <list>
#include <vector>
#include <map>

//////////////////////////////////////////////////////////////////////////
// BcLogImpl
class BcLogImpl:
	public BcLog
{
public:
	BcLogImpl();
	virtual ~BcLogImpl();

	/**
	 * Write to log.
	 */
	void write( const BcChar* pText, ... ) override;
	
	/**
	 * Flush log.
	 */
	void flush() override;

	/**
	 * Set catagory suppression.
	 */
	void setCatagorySuppression( BcName Catagory, BcBool IsSuppressed ) override;

	/**
	 * Get catagory suppression.
	 */
	BcBool getCatagorySuppression( BcName Catagory ) const override;
	
	/**
	 * Set default catagory.
	 */
	void setCatagory( BcName Catagory ) override;

	/**
	 * Get default catagory.
	 */
	BcName getCatagory() override;

	/**
	 * Increase indent.
	 */
	void increaseIndent() override;

	/**
	 * Decreate indent.
	 */
	void decreaseIndent() override;

	/*
	 * Get log data
	 */
	std::vector<std::string> getLogData();

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
	/**
	 * Private write using va_list.
	 */
	void privateWrite( const BcChar* pText, va_list Args );

private:
	typedef std::map< BcName, BcBool > TSuppressionMap;
	typedef std::map< BcThreadId, int > TIndentLevels;
	typedef std::map< BcThreadId, BcName > TCatagories;

	mutable std::recursive_mutex Lock_;
	TIndentLevels IndentLevel_;
	TCatagories Catagories_;
	BcBool SuppressionDefault_;
	TSuppressionMap SuppressedMap_;

	std::list<std::string> LogBuffer;

};


#endif

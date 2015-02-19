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
#include "Base/BcTimer.h"
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

	void write( const BcChar* pText, ... ) override;
	void flush() override;
	void setCategorySuppression( BcName Category, BcBool IsSuppressed ) override;
	BcBool getCategorySuppression( BcName Category ) const override;	
	void registerListener( class BcLogListener* Listener ) override;
	void deregisterListener( class BcLogListener* Listener ) override;
	void setCategory( BcName Category ) override;
	BcName getCategory() override;
	void increaseIndent() override;
	void decreaseIndent() override;
	std::vector<std::string> getLogData();

private:
	/**
	 * Private write using va_list.
	 */
	void privateWrite( const BcChar* pText, va_list Args );

private:
	typedef std::map< BcName, BcBool > TSuppressionMap;
	typedef std::map< BcThreadId, int > TIndentLevels;
	typedef std::map< BcThreadId, BcName > TCatagories;
	typedef std::vector< BcLogListener* > TLogListeners;

	mutable std::recursive_mutex Lock_;
	TIndentLevels IndentLevel_;
	TCatagories Catagories_;
	BcBool SuppressionDefault_;
	TSuppressionMap SuppressedMap_;
	BcTimer Timer_;
	TLogListeners Listeners_;
	std::list<std::string> LogBuffer_;

	std::unique_ptr< BcLogListener > DefaultListener_;
};


#endif

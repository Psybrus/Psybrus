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
#include <vector>
#include <map>
#include <memory>

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
	void setCategorySuppression( const BcName Category, BcBool IsSuppressed ) override;
	BcBool getCategorySuppression( const BcName Category ) const override;	
	void registerListener( class BcLogListener* Listener ) override;
	void deregisterListener( class BcLogListener* Listener ) override;
	void setCategory( const BcName Category ) override;
	BcName getCategory() override;
	void increaseIndent() override;
	void decreaseIndent() override;

private:
	/**
	 * Private write using va_list.
	 */
	void privateWrite( const BcChar* pText, va_list Args );

private:
	typedef std::map< BcName, BcBool > TSuppressionMap;
	typedef std::map< BcThreadId, int > TIndentLevels;
	typedef std::map< BcThreadId, BcName > TCategories;
	typedef std::vector< BcLogListener* > TLogListeners;

	mutable std::recursive_mutex Lock_;
	TIndentLevels IndentLevel_;
	TCategories Categories_;
	BcBool SuppressionDefault_;
	TSuppressionMap SuppressedMap_;
	BcTimer Timer_;
	TLogListeners Listeners_;

	std::unique_ptr< BcLogListener > DefaultListener_;
};


#endif

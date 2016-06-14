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
/* @class BcLog
 * @brief Abstract interface and global access for logging.
 */
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
	virtual void setCategorySuppression( const BcName Category, BcBool IsSuppressed ) = 0;

	/**
	 * Get category suppression.
	 */
	virtual BcBool getCategorySuppression( const BcName Category ) const = 0;

protected:
	friend class BcLogScopedCategory;
	friend class BcLogScopedIndent;
	friend class BcLogListener;

	/**
	 * Register listener.
	 */
	virtual void registerListener( class BcLogListener* Listener ) = 0;

	/**
	 * Deregister listener.
	 */
	virtual void deregisterListener( class BcLogListener* Listener ) = 0;

	/**
	 * Set category.
	 */
	virtual void setCategory( const BcName Category ) = 0;

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
/* @class BcLogEntry
 * @brief Log entry containing all useful, loggable information per
 *        user log entry.
 */
struct BcLogEntry
{
	BcF64 Time_;
	BcThreadId ThreadId_;
	BcName Category_;
	int Indent_;
	std::string Text_;
};

//////////////////////////////////////////////////////////////////////////
/**
 * @class BcLogListener
 * @brief Interface for receiving callbacks from the logging system.
 */
class BcLogListener
{
public:
	BcLogListener();
	BcLogListener( const BcLogListener& Other );
	virtual ~BcLogListener();

	/**
	 * Called when there has been any logging occur.
	 */
	virtual void onLog( const BcLogEntry& Entry ) = 0;
};

//////////////////////////////////////////////////////////////////////////
/* @class BcLogScopedCategory
 * @brief Scoped log Category setting to set/unset current Category.
 */
class BcLogScopedCategory
{
public:
	BcLogScopedCategory( const BcName Category );
	~BcLogScopedCategory();

private:
	BcName OldCategory_;
};

//////////////////////////////////////////////////////////////////////////
/* @class BcLogScopedIndent
 * @brief Scoped log indent setting to indent/unindent.
 */
class BcLogScopedIndent
{
public:
	BcLogScopedIndent();
	~BcLogScopedIndent();
};

//////////////////////////////////////////////////////////////////////////
// Macros
#if !PSY_PRODUCTION
#  define PSY_LOGSCOPEDCATEGORY( _NAME ) \
	static const BcName LOG_CATEGORY_##_NAME( #_NAME ); \
	BcLogScopedCategory _LogScopedCategory_##__LINE__( LOG_CATEGORY_##_NAME );

#  define PSY_LOGSCOPEDINDENT \
	BcLogScopedIndent _LogScopedIndent_##__LINE__;

#  define PSY_LOG \
	if( BcLog::pImpl() ) BcLog::pImpl()->write

#else
#  define PSY_LOGSCOPEDCATEGORY( _NAME )
#  define PSY_LOGSCOPEDINDENT
#  define PSY_LOG
#endif

#endif

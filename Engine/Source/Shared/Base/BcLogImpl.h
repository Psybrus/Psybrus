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
#include <mutex>
#include "Base/BcGlobal.h"

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
	void write( const BcChar* pText, ... );
	
	/**
	 * Write to log.
	 */
	void write( BcU32 Catagory, const BcChar* pText, ... );

	/**
	 * Flush log.
	 */
	void flush();

	/**
	 * Set catagory suppression.
	 */
	void setCatagorySuppression( BcU32 Catagory, BcBool IsSuppressed );

	/**
	 * Get catagory suppression.
	 */
	BcBool getCatagorySuppression( BcU32 Catagory ) const;
	
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
	typedef std::map< BcU32, BcBool > TSuppressionMap;

	mutable std::mutex Lock_;
	BcBool SuppressionDefault_;
	TSuppressionMap SuppressedMap_;
	BcChar TextBuffer_[ 1024 * 64 ];

};


#endif

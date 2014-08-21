/**************************************************************************
*
* File:		CsTypes.h
* Author:	Neil Richardson 
* Ver/Date:	7/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __CSTYPES_H__
#define __CSTYPES_H__

#include "Base/BcTypes.h"
#include "Base/BcDebug.h"
#include "Base/BcName.h"
#include "Base/BcPath.h"

#include "System/File/FsFile.h"
#include <atomic>
#include "Base/BcEndian.h"
#include "Base/BcHash.h"
#include <mutex>


//////////////////////////////////////////////////////////////////////////
// Callbacks
typedef BcDelegate< void(*)( class CsPackage*, BcU32 ) > CsPackageReadyCallback;

//////////////////////////////////////////////////////////////////////////
// CsDependencyList
typedef std::list< class CsDependency > CsDependencyList;
typedef CsDependencyList::iterator CsDependencyListIterator;

//////////////////////////////////////////////////////////////////////////
// CsDependency
class CsDependency
{
public:
	CsDependency( const std::string& FileName );
	CsDependency( const std::string& FileName, const FsStats& Stats );
	CsDependency( const CsDependency& Other );
	~CsDependency();

	/**
	 * Get file name.
	 */
	const std::string& getFileName() const;

	/**
	 * Get stats.
	 */
	const FsStats& getStats() const;

	/**
	 * Had dependancy changed?
	 */
	BcBool hasChanged() const;

	/**
	 * Update stats.
	 */
	void updateStats();

	/**
	 * Comparison.
	 */
	bool operator < ( const CsDependency& Dep ) const;

private:
	std::string FileName_;
	FsStats Stats_;
};

//////////////////////////////////////////////////////////////////////////
// CsImportException
class CsImportException:
	public std::exception
{
public:
	CsImportException( const std::string& Error,
		const std::string& File ):
		std::exception( Error.c_str() ),
		File_( File )
	{}
	virtual ~CsImportException(){}

	const std::string& file() const
	{
		return File_;
	}

private:
	std::string File_;
};

#endif

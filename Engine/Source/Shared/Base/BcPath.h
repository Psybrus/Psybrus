/**************************************************************************
*
* File:		BcPath.h
* Author: 	Neil Richardson
* Ver/Date:
* Description:
*		File system path
*
*
*
*
**************************************************************************/

#ifndef __BCPATH_H__
#define __BCPATH_H__

#include "Base/BcTypes.h"
#include "Base/BcName.h"

//////////////////////////////////////////////////////////////////////////
// BcPath
class BcPath
{
public:
	static const BcChar Seperator;

public:
	BcPath();
	BcPath( const BcPath& Value );
	BcPath( const std::string& Value );
	BcPath( const BcChar* pValue );
	virtual ~BcPath();

	std::string operator * () const;
	std::string getFileName() const;
	std::string getExtension() const;
	std::string getFileNameNoExtension() const;

	void append( const BcPath& PathA );
	void join( const BcPath& PathA );
	void join( const BcPath& PathA, const BcPath& PathB );
	void join( const BcPath& PathA, const BcPath& PathB, const BcPath& PathC );


protected:
	void fixSeperators();
	static std::string stripLeadingSeperator( const std::string& Path );
	static std::string appendTrailingSeperator( const std::string& Path );

private:
	std::string InternalValue_;

};


#endif

/**************************************************************************
*
* File:		BcRegex.h
* Author:	Neil Richardson 
* Ver/Date:	12/12/12	
* Description:
*		Regular Expression support with PCRE.
*		
*
*
* 
**************************************************************************/

#ifndef __BCREGEX_H__
#define __BCREGEX_H__

#include "Base/BcTypes.h"
#include "Base/BcDebug.h"
#include "Base/BcString.h"

#include <string>

//////////////////////////////////////////////////////////////////////////
// BcRegexMatch
class BcRegexMatch
{
public:
	BcRegexMatch( BcU32 SubstringLines = 4 );
	~BcRegexMatch();

private:
	BcRegexMatch( const BcRegexMatch& Other ) {};

public:
	BcU32 noofMatches() const;
	BcBool getMatch( BcU32 Idx, std::string& OutString ) const;

private:
	friend class BcRegex;

	BcU32 SubstringLines_;
	BcU32 MatchedSubstringLines_;
	BcChar* pMatchString_;
	int* pSubstringVec_;
};

//////////////////////////////////////////////////////////////////////////
// BcRegex
class BcRegex
{
public:
	BcRegex();
	BcRegex( const BcChar* pPattern );
	BcRegex( const BcRegex& Other );
	~BcRegex();

	/**
	 * Match.
	 * @return How many matches there were.
	 */
	BcU32 match( const BcChar* pText, BcRegexMatch& OutMatch );

private:
	struct BcRegexPrivate* pPrivate_;
};

#endif

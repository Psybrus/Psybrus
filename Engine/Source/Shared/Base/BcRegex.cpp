/**************************************************************************
*
* File:		BcRegex.cpp
* Author:	Neil Richardson 
* Ver/Date:	12/12/12	
* Description:
*		Regular Expression support with PCRE.
*		
*
*
* 
**************************************************************************/

#include "Base/BcRegex.h"

//////////////////////////////////////////////////////////////////////////
// Include PCRE
#define PCRE_STATIC 1
#include <pcre.h>

//////////////////////////////////////////////////////////////////////////
// Ctor
BcRegexMatch::BcRegexMatch( BcU32 SubstringLines /* = 8 */ ):
	SubstringLines_( SubstringLines ),
	MatchedSubstringLines_( 0 ),
	pMatchString_( NULL ),
	pSubstringVec_( NULL )
{
	pSubstringVec_ = new int[ SubstringLines_ * 3 ];
}

//////////////////////////////////////////////////////////////////////////
// Dtor
BcRegexMatch::~BcRegexMatch()
{
	// If we have a match, free it.
	if( pMatchString_ != NULL )
	{
		delete [] pMatchString_;
		pMatchString_ = NULL;
	}
	
	delete [] pSubstringVec_;
	pSubstringVec_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// noofMatches
BcU32 BcRegexMatch::noofMatches() const
{
	return MatchedSubstringLines_;
}

//////////////////////////////////////////////////////////////////////////
// getMatch
BcBool BcRegexMatch::getMatch( BcU32 Idx, std::string& OutString ) const
{
	if( Idx >= MatchedSubstringLines_ )
	{
		return BcFalse;
	}

	if( MatchedSubstringLines_ == BcErrorCode )
	{
		return BcFalse;
	}

	const char* pSubstringMatch = NULL;

	pcre_get_substring( pMatchString_, pSubstringVec_, MatchedSubstringLines_, Idx, &pSubstringMatch );

	OutString = pSubstringMatch;
	
	pcre_free_substring( pSubstringMatch );

	return BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// BcRegexPrivate: This means we don't include pcre elsewhere.
struct BcRegexPrivate
{
	pcre* pCode_;
	pcre_extra* pExtra_;
};

//////////////////////////////////////////////////////////////////////////
// Ctor
BcRegex::BcRegex()
{
	pPrivate_ = new BcRegexPrivate();
	pPrivate_->pCode_ = NULL;
	pPrivate_->pExtra_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Ctor
BcRegex::BcRegex( const BcChar* pPattern )
{
	int Options = 0;
	const char* pPCREErrorString = NULL;
	int PCREErrorOffset = 0;

	// Create private data.
	pPrivate_ = new BcRegexPrivate();
	pPrivate_->pCode_ = NULL;
	pPrivate_->pExtra_ = NULL;

	// Compile.
	pPrivate_->pCode_ = pcre_compile( pPattern, Options, &pPCREErrorString, &PCREErrorOffset, NULL );

	// If it's compiled, optimise it.
	if( pPrivate_->pCode_ != NULL )
	{
		// Inc ref count.
		pcre_refcount( pPrivate_->pCode_, 1 );

		// Optimise
		pPrivate_->pExtra_ = pcre_study( pPrivate_->pCode_, 0, &pPCREErrorString );
	}
}

//////////////////////////////////////////////////////////////////////////
// Copy Ctor
BcRegex::BcRegex( const BcRegex& Other )
{
	pPrivate_ = new BcRegexPrivate();
	pPrivate_->pCode_ = Other.pPrivate_->pCode_;
	pPrivate_->pExtra_ = Other.pPrivate_->pExtra_;

	// Copy across and inc ref count.
	if( pPrivate_->pCode_ != NULL )
	{
		// Inc ref count.
		pcre_refcount( pPrivate_->pCode_, 1 );
	}
}

//////////////////////////////////////////////////////////////////////////
// Dtor
BcRegex::~BcRegex()
{
	if( pPrivate_ != NULL )
	{
		if( pPrivate_->pCode_ != NULL )
		{
			int refCount = pcre_refcount( pPrivate_->pCode_, -1 );
			if( refCount == 0 )
			{
				pcre_free( pPrivate_->pCode_ );

				if( pPrivate_->pExtra_ != NULL )
				{
					pcre_free( pPrivate_->pExtra_ );
				}
			}
		}
	
		delete pPrivate_;
		pPrivate_ = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
// match
BcU32 BcRegex::match( const BcChar* pText, BcRegexMatch& OutMatch )
{
	BcU32 TextLength( BcStrLength( pText ) );
	BcU32 MatchesTotal = 0;
	BcU32 NextMatchOffset = 0;
	int PCREReturn = 0;

	do
	{
		PCREReturn = pcre_exec( pPrivate_->pCode_,
								pPrivate_->pExtra_,
								pText,
								TextLength,
								NextMatchOffset,
								0,
								OutMatch.pSubstringVec_ + ( MatchesTotal * 2 ),
								( OutMatch.SubstringLines_ - MatchesTotal ) * 3 );

		if( PCREReturn < 0 )
		{
			/*
			switch( PCREReturn )
			{
				case PCRE_ERROR_NOMATCH      : BcPrintf("String did not match the pattern\n");        break;
				case PCRE_ERROR_NULL         : BcPrintf("Something was null\n");                      break;
				case PCRE_ERROR_BADOPTION    : BcPrintf("A bad option was passed\n");                 break;
				case PCRE_ERROR_BADMAGIC     : BcPrintf("Magic number bad (compiled re corrupt?)\n"); break;
				case PCRE_ERROR_UNKNOWN_NODE : BcPrintf("Something kooky in the compiled re\n");      break;
				case PCRE_ERROR_NOMEMORY     : BcPrintf("Ran out of memory\n");                       break;
				default                      : BcPrintf("Unknown error\n");                           break;
			}
			*/
		}
		else if ( PCREReturn > 0 )
		{
			MatchesTotal += PCREReturn;

			NextMatchOffset = OutMatch.pSubstringVec_[ ( MatchesTotal - 1 ) * 2 + 1 ];
		}
	}
	while( PCREReturn > 0 && MatchesTotal < OutMatch.SubstringLines_ && NextMatchOffset < TextLength );
	
	OutMatch.MatchedSubstringLines_ = MatchesTotal;
	OutMatch.pMatchString_ = new BcChar[ TextLength + 1 ];
	BcStrCopy( OutMatch.pMatchString_, pText );

	return MatchesTotal;
}

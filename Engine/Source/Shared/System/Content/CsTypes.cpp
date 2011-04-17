/**************************************************************************
*
* File:		CsTypes.cpp
* Author:	Neil Richardson 
* Ver/Date:	7/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "CsTypes.h"

//////////////////////////////////////////////////////////////////////////
// CsDependancy
CsDependancy::CsDependancy( const std::string& FileName )
{
	FileName_ = FileName;
}

//////////////////////////////////////////////////////////////////////////
// CsDependancy
CsDependancy::CsDependancy( const CsDependancy& Other )
{
	FileName_ = Other.FileName_;
}

//////////////////////////////////////////////////////////////////////////
// CsDependancy
CsDependancy::~CsDependancy()
{
	
}

//////////////////////////////////////////////////////////////////////////
// CsDependancy
const std::string& CsDependancy::getFileName() const
{
	return FileName_;
}


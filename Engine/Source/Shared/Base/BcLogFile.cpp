/**************************************************************************
*
* File:		BcLogFile.cpp
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*		Logging system.
*		
*		
*		
* 
**************************************************************************/

#include "Base/BcLogFile.h"
#include "Base/BcString.h"
#include "Base/BcFile.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
BcLogFile::BcLogFile( const BcChar* pFileName )
{
	// Open log file for output.
	OutputFile_.open( pFileName, bcFM_WRITE );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
BcLogFile::~BcLogFile()
{
	
}

//////////////////////////////////////////////////////////////////////////
// internalWrite
//virtual
void BcLogFile::internalWrite( const BcChar* pText )
{
	BcLogImpl::internalWrite( pText );

	if( OutputFile_.isOpen() )
	{
		OutputFile_.write( pText, BcStrLength( pText ) );
		internalFlush();
	}
}

//////////////////////////////////////////////////////////////////////////
// internalFlush
//virtual
void BcLogFile::internalFlush()
{
	BcLogImpl::internalFlush();

	if( OutputFile_.isOpen() )
	{
		OutputFile_.flush();
	}
}

//////////////////////////////////////////////////////////////////////////
// getLogData
std::vector<std::string> BcLogFile::getLogData()
{
	std::vector<std::string> ret;
	ret.push_back("Not available during file output");
	return ret;
}
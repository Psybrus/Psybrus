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

#include "System/Content/CsTypes.h"
#include "System/File/FsCore.h"

#include "Base/BcString.h"

#include <cstdarg>
#include <cstdio>

//////////////////////////////////////////////////////////////////////////
// Paths
#if PLATFORM_LINUX | PLATFORM_WINDOWS | PLATFORM_OSX
const BcPath CsPaths::PACKED_CONTENT( "PackedContent/pc" );
const BcPath CsPaths::INTERMEDIATE( "Intermediate/pc" );
const BcPath CsPaths::CONTENT( "Content" );
#else
const BcPath CsPaths::PACKED_CONTENT( "PackedContent" );
const BcPath CsPaths::INTERMEDIATE( "Intermediate" );
const BcPath CsPaths::CONTENT( "Content" );
#endif

//////////////////////////////////////////////////////////////////////////
// CsPackageDependencies
REFLECTION_DEFINE_BASIC( CsDependency );

void CsDependency::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "FileName_",	&CsDependency::FileName_ ),
		new ReField( "Stats_",		&CsDependency::Stats_ ),
	};

	ReRegisterClass< CsDependency >( Fields );
};

//////////////////////////////////////////////////////////////////////////
// Ctor
CsDependency::CsDependency()
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
CsDependency::CsDependency( const std::string& FileName )
{
	FileName_ = *BcPath( FileName ); // TODO: Use Boost Filesystem
	updateStats();
}

//////////////////////////////////////////////////////////////////////////
// Ctor
CsDependency::CsDependency( const std::string& FileName, const FsStats& Stats )
{
	FileName_ = *BcPath( FileName ); // TODO: Use Boost Filesystem
	Stats_ = Stats;
}

//////////////////////////////////////////////////////////////////////////
// Ctor
CsDependency::CsDependency( const CsDependency& Other )
{
	FileName_ = Other.FileName_;
	Stats_ = Other.Stats_;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
CsDependency::~CsDependency()
{
	
}

//////////////////////////////////////////////////////////////////////////
// getFileName
const std::string& CsDependency::getFileName() const
{
	return FileName_;
}

//////////////////////////////////////////////////////////////////////////
// getStats
const FsStats& CsDependency::getStats() const
{
	return Stats_;
}

//////////////////////////////////////////////////////////////////////////
// hasChanged
BcBool CsDependency::hasChanged() const
{
	FsStats Stats;
	if( FsCore::pImpl()->fileStats( FileName_.c_str(), Stats ) )
	{
		if( Stats.ModifiedTime_ != Stats_.ModifiedTime_ )
		{
			return BcTrue;
		}
	}

	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// updateStats
void CsDependency::updateStats()
{
	// TODO: Use Boost Filesystem
	FsCore::pImpl()->fileStats( FileName_.c_str(), Stats_ );
}

//////////////////////////////////////////////////////////////////////////
// operator < 
bool CsDependency::operator < ( const CsDependency& Dep ) const
{
	return FileName_ < Dep.FileName_;
}


//////////////////////////////////////////////////////////////////////////
// getName
std::string CsFileHash::getName() const
{
	BcChar OutChars[ 128 ] = { 0 };
	BcSPrintf( OutChars, sizeof( OutChars ) - 1, "%08X%08X%08X%08X%08X",
		Hash_[ 0 ],
		Hash_[ 1 ],
		Hash_[ 2 ],
		Hash_[ 3 ],
		Hash_[ 4 ] );
	return OutChars;
}

//////////////////////////////////////////////////////////////////////////
// Ctor
CsImportException::CsImportException( 
		const char* File,
		const char* Error,
		... ) NOEXCEPT
{
	BcAssert( File );
	BcAssert( Error );

	BcMemSet( File_, 0, sizeof( File_ ) );
	BcStrCopy( File_, sizeof( File_ ) - 1, File );

	BcMemSet( Error_, 0, sizeof( Error_ ) );

	va_list Args;
	va_start( Args, Error );
	vsnprintf( Error_, sizeof( Error_ ), Error, Args );
	va_end( Args );
}

//////////////////////////////////////////////////////////////////////////
// what
const char* CsImportException::what() const NOEXCEPT
{
	return Error_;
}

//////////////////////////////////////////////////////////////////////////
// file
const char* CsImportException::file() const NOEXCEPT
{
	return File_;
}

//////////////////////////////////////////////////////////////////////////
// error
const char* CsImportException::error() const NOEXCEPT
{
	return Error_;
}

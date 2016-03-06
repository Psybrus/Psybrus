/**************************************************************************
*
* File:		BcFile.h
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*		
*		
*		
*		
* 
**************************************************************************/

#include "Base/BcFile.h"
#include "Base/BcDebug.h"
#include "Base/BcString.h"
#include "Base/BcMemory.h"

#include <fcntl.h>
#include <sys/stat.h>

#if PLATFORM_LINUX || PLATFORM_OSX
#include <dirent.h>
#elif PLATFORM_WINDOWS
#include <filesystem>
#endif

#if COMPILER_MSVC

//////////////////////////////////////////////////////////////////////////
// Windows Includes
#include <io.h>

//////////////////////////////////////////////////////////////////////////
// Windows Defines
#define lowLevelOpen		::open
#define lowLevelClose		::close
#define lowLevelReadFlags	(_O_RDONLY | _O_BINARY)

#define streamOpen			fopen
#define streamClose			fclose


#elif COMPILER_GCC || COMPILER_LLVM
//////////////////////////////////////////////////////////////////////////
// *nix Includes
#include <unistd.h>

//////////////////////////////////////////////////////////////////////////
// *nix Defines
#define lowLevelOpen		::open
#define lowLevelClose		::close
#define lowLevelReadFlags	(O_RDONLY)	//Binary flag does not exist on posix

#define streamOpen			fdopen
#define streamClose			fclose

#endif

//////////////////////////////////////////////////////////////////////////
// BcFileSystemExists
bool BcFileSystemExists( const char* Path )
{
#if PLATFORM_LINUX || PLATFORM_OSX
	struct stat Stat;
	if (stat(Path, &Stat) == 0)
	{
		return true;
	}
#elif PLATFORM_WINDOWS
	return std::experimental::filesystem::exists( Path );
#endif
    return false;
}

//////////////////////////////////////////////////////////////////////////
// BcFileSystemRemove
bool BcFileSystemRemove( const char* Path )
{
#if PLATFORM_LINUX || PLATFORM_OSX
	return remove( Path ) == 0;
#elif PLATFORM_WINDOWS
	return std::experimental::filesystem::remove( Path );
#endif
}

//////////////////////////////////////////////////////////////////////////
// BcFileSystemRename
bool BcFileSystemRename( const char* SrcPath, const char* DestPath )
{
#if PLATFORM_LINUX || PLATFORM_OSX
	return rename( SrcPath, DestPath ) == 0;
#elif PLATFORM_WINDOWS
	std::experimental::filesystem::rename( SrcPath, DestPath );
	return true;
#endif
}

//////////////////////////////////////////////////////////////////////////
// BcFileSystemCreateDirectories
bool BcFileSystemCreateDirectories( const char* Path )
{
#if PLATFORM_LINUX || PLATFORM_OSX
	std::array< char, 256 > TempPath;

	// Copy into temp path.
	BcStrCopy( TempPath.data(), TempPath.size(), Path );
	char* FoundSeparator = nullptr;
	char* NextSearchPosition = TempPath.data();
	do
	{
		// Find separator.
		FoundSeparator = strstr( NextSearchPosition, "/" );
		if( FoundSeparator == nullptr )
		{
			FoundSeparator = strstr( NextSearchPosition, "\\" );
		}

		// If found, null terminate.
		char SeparatorChar = 0;
		if( FoundSeparator != nullptr )
		{
			SeparatorChar = *FoundSeparator;
			*FoundSeparator = '\0';
		}

		// Attempt to create the path if it doesn't exist.
		if( BcFileSystemExists( TempPath.data() ) == false )
		{
			int RetVal = mkdir( TempPath.data(), 0755 );
			if( RetVal )
			{
				return false;
			}
		}

		// Put separator back.
		if( SeparatorChar )
		{
			*FoundSeparator = SeparatorChar;
			NextSearchPosition = FoundSeparator + 1;
		}
	}
	while( FoundSeparator != nullptr );

	return true;
#elif PLATFORM_WINDOWS
	std::experimental::filesystem::create_directories( Path );
#endif
}

//////////////////////////////////////////////////////////////////////////
// BcFileSystemChangeDirectory
bool BcFileSystemChangeDirectory( const char* Path )
{
#if PLATFORM_LINUX || PLATFORM_OSX
	return chdir( Path ) == 0;
#elif PLATFORM_WINDOWS
	return Path != std::experimental::filesystem::current_path( Path );
#endif
}

//////////////////////////////////////////////////////////////////////////
// Ctor & Dtor
BcFile::BcFile()
{
	FileDescriptor_ = -1;
	FileHandle_ = NULL;
}

BcFile::~BcFile()
{
	if ( FileHandle_ != NULL )
	{
		close();
	}
}

//////////////////////////////////////////////////////////////////////////
// calcFileSize
void BcFile::calcFileSize( void )
{
#if COMPILER_MSVC

	if( FileHandle_ != NULL )
	{
		FileSize_ = _filelength( _fileno( FileHandle_ ) );
	}

#elif COMPILER_GCC || COMPILER_LLVM
	//Early out if no file open
	if(FileDescriptor_ == -1)
	{
		return;
	}

	struct stat _stat;
	if ( 0 == fstat(FileDescriptor_, &_stat) )
	{
		FileSize_ = _stat.st_size;
	}

#endif 
}

//////////////////////////////////////////////////////////////////////////
// open
BcBool BcFile::open( const BcChar* FileName, eBcFileMode AccessMode )
{
	switch ( AccessMode )
	{
	case bcFM_READ:
		{
#if COMPILER_MSVC
			// Open the stream
			FileHandle_ = streamOpen(FileName, "rb");

			// Grab the size of the file
			calcFileSize();
#elif COMPILER_GCC || COMPILER_LLVM
			// Open the file
			FileDescriptor_ = lowLevelOpen(FileName, lowLevelReadFlags);

			//Did the file open successfully?
			if(FileDescriptor_ != -1)
			{
				//Yep - grab the size of the file
				calcFileSize();

				// Open the stream
				FileHandle_ = streamOpen(FileDescriptor_, "rb");
			}
#endif
			AccessMode_ = AccessMode;
		}
		break;

	case bcFM_WRITE:
		{
			FileHandle_ = fopen( FileName, "wb+" );
		}
		break;

	case bcFM_WRITE_TEXT:
		{
			FileHandle_ = fopen( FileName, "w+" );
		}
		break;

	default:
		return BcFalse;
		break;
	};

	return ( FileHandle_ != 0 );
}

//////////////////////////////////////////////////////////////////////////
// close
void BcFile::close()
{
	if( FileHandle_ != 0 )
	{
		fclose( FileHandle_ );
		FileHandle_ = 0;
	}

#if COMPILER_GCC || CONMPILER_LLVM
	if( FileDescriptor_ != -1)
	{
		lowLevelClose(FileDescriptor_);
		FileDescriptor_ = -1;
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
// eof
BcBool BcFile::eof()
{
	return feof( FileHandle_ );
}

//////////////////////////////////////////////////////////////////////////
// flush
void BcFile::flush()
{
	fflush( FileHandle_ );
}

//////////////////////////////////////////////////////////////////////////
// tell
size_t BcFile::tell()
{
	return ftell( FileHandle_ );
}

//////////////////////////////////////////////////////////////////////////
// seek
void BcFile::seek( size_t Position )
{
	fseek( FileHandle_, (long)Position, 0 );
}

//////////////////////////////////////////////////////////////////////////
// read
void BcFile::read( void* pDest, size_t nBytes )
{
	BcAssert( FileHandle_ != NULL );
	
	fread( pDest, (long)nBytes, 1, FileHandle_ );
}

//////////////////////////////////////////////////////////////////////////
// readLine
void BcFile::readLine( BcChar* pBuffer, size_t Size )
{
	BcU32 BytesRead = 0;
	BcMemZero( pBuffer, Size );
	while( BytesRead < Size && !eof() )
	{
		read( &pBuffer[ BytesRead++ ], 1 );
		if( pBuffer[ BytesRead - 1 ] == '\n' )
		{
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// readAllBytes
std::unique_ptr< BcU8[] > BcFile::readAllBytes()
{
	std::unique_ptr< BcU8[] > Bytes( new BcU8[ size() + 1 ] );
	memset( Bytes.get(), 0, size() + 1 );
	read( Bytes.get(), size() );
	return Bytes;
}

//////////////////////////////////////////////////////////////////////////
// write
void BcFile::write( const void* pSrc, size_t nBytes )
{
	BcAssert( FileHandle_ != NULL );

	fwrite( pSrc, (long)nBytes, 1, FileHandle_ );
}

//////////////////////////////////////////////////////////////////////////
// writeLine
void BcFile::writeLine( const BcChar* pTest )
{
	BcAssert( FileHandle_ != NULL );

	fwrite( pTest, BcStrLength( pTest ), 1, FileHandle_ );
	fwrite( "\n", 2, 1, FileHandle_ );
}

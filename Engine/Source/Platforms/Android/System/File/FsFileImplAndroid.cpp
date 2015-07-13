/**************************************************************************
*
* File:		FsFileImplAndroid.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/File/FsFileImplAndroid.h"

#include "System/File/FsCore.h"
#include "System/File/FsCoreImplAndroid.h"

#include <android_native_app_glue.h>
#include <android/asset_manager.h>

//////////////////////////////////////////////////////////////////////////
// Ctor
FsFileImplAndroid::FsFileImplAndroid():
	Asset_( nullptr ),
	FileSize_( 0 )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
FsFileImplAndroid::~FsFileImplAndroid()
{
	BcAssert( Asset_ == nullptr );
}

//////////////////////////////////////////////////////////////////////////
// open
//virtual
BcBool FsFileImplAndroid::open( const BcChar* FileName, eFsFileMode FileMode )
{
	BcBool RetVal = BcFalse;

	if( Asset_ == nullptr )
	{
		// Read access.
		if( FileMode == fsFM_READ )
		{
			// TODO: Proper remapping.
			FileName_ = std::string( FileName ) + ".mp3";

			// Attempt to open asset.
			extern android_app* GAndroidApp;
			AAssetManager* AssetManager = GAndroidApp->activity->assetManager;
			Asset_ = AAssetManager_open( AssetManager, FileName_.c_str(), AASSET_MODE_RANDOM );

			if( Asset_ != nullptr )
			{
				FileSize_ = AAsset_getLength( Asset_ );
				RetVal = BcTrue;
			}
			else
			{
				PSY_LOG( "Unable to load asset %s", FileName );
			}
		}
		else if( FileMode == fsFM_WRITE )
		{
			BcBreakpoint;
		}
	}

	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
BcBool FsFileImplAndroid::close()
{
	BcBool RetVal = BcFalse;

	if( Asset_ )
	{
		AAsset_close( Asset_ );
		Asset_ = nullptr;
		RetVal = BcTrue;
	}

	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// fileName
//virtual
const BcChar* FsFileImplAndroid::fileName() const
{
	return FileName_.c_str();
}

//////////////////////////////////////////////////////////////////////////
// size
//virtual
BcSize FsFileImplAndroid::size() const
{
	return (BcSize)FileSize_;
}

//////////////////////////////////////////////////////////////////////////
// tell
//virtual
BcSize FsFileImplAndroid::tell() const
{
	BcBreakpoint;
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// seek
//virtual
void FsFileImplAndroid::seek( BcSize Position )
{
	AAsset_seek( Asset_, Position, SEEK_SET );
}

//////////////////////////////////////////////////////////////////////////
// eof
//virtual
BcBool FsFileImplAndroid::eof() const
{
	BcBreakpoint;
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// read
void FsFileImplAndroid::read( void* pDest, BcSize Bytes )
{
	AAsset_read( Asset_, pDest, Bytes );
}

//////////////////////////////////////////////////////////////////////////
// write
void FsFileImplAndroid::write( void* pSrc, BcSize Bytes )
{
	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// readAsync
//virtual
void FsFileImplAndroid::readAsync( BcSize Position, void* pData, BcSize Bytes, FsFileOpCallback DoneCallback )
{
	FsCore::pImpl()->addReadOp( this, Position, pData, Bytes, DoneCallback );
}

//////////////////////////////////////////////////////////////////////////
// writeAsync
//virtual
void FsFileImplAndroid::writeAsync( BcSize Position, void* pData, BcSize Bytes, FsFileOpCallback DoneCallback )
{
	FsCore::pImpl()->addWriteOp( this, Position, pData, Bytes, DoneCallback );
}



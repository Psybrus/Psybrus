/**************************************************************************
 *
 * File:		SsSource.h
 * Author:		Neil Richardson
 * Ver/Date:	
 * Description:
 *		
 *
 *
 *
 * 
 **************************************************************************/

#ifndef __SSSOURCE_H__
#define __SSSOURCE_H__

#include "System/Sound/SsResource.h"
#include "System/Content/CsTypes.h"

//////////////////////////////////////////////////////////////////////////
// SsSourceFileData
struct SsSourceFileData
{
	enum Type
	{
		SFXR = 0,
		WAV,
		WAVSTREAM,
		MODPLUG
	};

	BcU32 Type_;
	CsFileHash FileHash_;
};

//////////////////////////////////////////////////////////////////////////
// SsSourceParams
class SsSourceParams
{
public:
	SsSourceParams();

};

//////////////////////////////////////////////////////////////////////////
// SsSourceUPtr
typedef std::unique_ptr< class SsSource > SsSourceUPtr;

//////////////////////////////////////////////////////////////////////////
// SsSource
class SsSource:
	public SsResource
{
public:
	SsSource( const SsSourceParams& Params );
	virtual ~SsSource();

private:
	SsSourceParams Params_;
};



#endif


/**************************************************************************
 *
 * File:		SsFileData.h
 * Author:		Neil Richardson
 * Ver/Date:	
 * Description:
 *		
 *
 *
 *
 * 
 **************************************************************************/

#ifndef __SSFILEDATA_H__
#define __SSFILEDATA_H__

#include "exSsTypes.h"

//////////////////////////////////////////////////////////////////////////
// SsSoundSampleType
enum SsSoundSampleType
{
	ssST_SAMPLE = 0,
	ssST_STREAM,
};

//////////////////////////////////////////////////////////////////////////
// SsSoundSample
struct SsSoundSample
{
public:
	SsSoundSampleType		SampleType_;

	BcU32 SampleRate_;
	BcU32 Channels_;
	BcU32 Samples_;
	void* pData_;
	BcBool Looped_;

	// User
	BcU32 Handle_;
};

#endif


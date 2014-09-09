/**************************************************************************
 *
 * File:		SsFilter.h
 * Author:		Neil Richardson
 * Ver/Date:	
 * Description:
 *		
 *
 *
 *
 * 
 **************************************************************************/

#ifndef __SSFILTER_H__
#define __SSFILTER_H__

#include "System/Sound/SsResource.h"

//////////////////////////////////////////////////////////////////////////
// SsFilterParams
class SsFilterParams
{
public:
	SsFilterParams();

};

//////////////////////////////////////////////////////////////////////////
// SsFilterUPtr
typedef std::unique_ptr< class SsFilter > SsFilterUPtr;

//////////////////////////////////////////////////////////////////////////
// SsFilter
class SsFilter:
	public SsResource
{
public:
	SsFilter( const SsFilterParams& Params );
	virtual ~SsFilter();

private:
	SsFilterParams Params_;
};

#endif

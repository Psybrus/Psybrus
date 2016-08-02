/**************************************************************************
*
* File:		DsProfilerRemotery.h
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*			Remotery profiler.
*			
*
*
* 
**************************************************************************/

#ifndef __DsProfilerRemotery_H__
#define __DsProfilerRemotery_H__

#include "Base/BcProfiler.h"

#if PSY_USE_PROFILER

//////////////////////////////////////////////////////////////////////////
// BcProfiler
class DsProfilerRemotery:
	public BcProfiler
{
public:
	DsProfilerRemotery();
	virtual ~DsProfilerRemotery();

	void setCurrentThreadName( const char* Name ) override;
	void initialiseGraphics( const char* API, void* Context, void* Device ) override;
	void shutdownGraphics() override;
	void beginProfiling() override;
	void endProfiling() override;
	void enterSection( const char* Tag ) override;
	void exitSection( const char* Tag ) override;
	void enterGPUSection( const char* Tag ) override;
	void exitGPUSection( const char* Tag ) override;
	void startAsync( const char* Tag, void* Data ) override;
	void stepAsync( const char* Tag, void* Data ) override;
	void endAsync( const char* Tag, void* Data ) override;
	void instantEvent( const char* Tag ) override;

private:
	struct rmtSettings* Settings_ = nullptr;
	struct Remotery* Remotery_ = nullptr;

	bool UseGL_ = false;
	bool UseD3D11_ = false;
};

//////////////////////////////////////////////////////////////////////////
// Inlines

#endif // PSY_USE_PROFILER



#endif // __DsProfilerRemotery_H__

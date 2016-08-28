/**************************************************************************
*
* File:		DsProfilerRemotery.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*			Remotery profiler.
*			
*
*
* 
**************************************************************************/

#include "System/Debug/DsProfilerRemotery.h"

#include "Remotery.h"

#if PSY_USE_PROFILER

//////////////////////////////////////////////////////////////////////////
// Ctor
DsProfilerRemotery::DsProfilerRemotery()
{
	Settings_ = rmt_Settings();
    rmt_CreateGlobalInstance( &Remotery_ );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
DsProfilerRemotery::~DsProfilerRemotery()
{
	rmt_DestroyGlobalInstance( Remotery_ );
}

//////////////////////////////////////////////////////////////////////////
// setCurrentThreadName
//virtual
void DsProfilerRemotery::setCurrentThreadName( const char* Name )
{
	rmt_SetCurrentThreadName( Name );
}

//////////////////////////////////////////////////////////////////////////
// initialiseGraphics
void DsProfilerRemotery::initialiseGraphics( const char* API, void* Context, void* Device )
{
	if( BcStrCompare( API, "GL" ) )
	{
		UseGL_ = true;
		_rmt_BindOpenGL();
	}

	if( BcStrCompare( API, "D3D11" ) )
	{
		UseD3D11_ = true;
		rmt_BindD3D11( Device, Context );
	}
}

//////////////////////////////////////////////////////////////////////////
// shutdownGraphics
void DsProfilerRemotery::shutdownGraphics()
{
	if( UseGL_ )
	{
		UseGL_ = false;
		rmt_UnbindOpenGL();
	}

	if( UseD3D11_ )
	{
		UseD3D11_ = false;
		rmt_UnbindD3D11();
	}
}

//////////////////////////////////////////////////////////////////////////
// beginProfiling
//virtual
void DsProfilerRemotery::beginProfiling()
{
}

//////////////////////////////////////////////////////////////////////////
// endProfiling
//virtual
void DsProfilerRemotery::endProfiling()
{
}

//////////////////////////////////////////////////////////////////////////
// enterSection
//virtual
void DsProfilerRemotery::enterSection( const char* Tag )
{
	_rmt_BeginCPUSample( Tag, 0, nullptr );
}

//////////////////////////////////////////////////////////////////////////
// exitSection
//virtual
void DsProfilerRemotery::exitSection( const char* Tag )
{
	_rmt_EndCPUSample();
}

//////////////////////////////////////////////////////////////////////////
// enterGPUSection
//virtual
void DsProfilerRemotery::enterGPUSection( const char* Tag )
{
	if( UseGL_ )
	{
#if RMT_USE_OPENGL
		_rmt_BeginOpenGLSample( Tag, nullptr );
#endif
	}

	if( UseD3D11_ )
	{
#if RMT_USE_D3D11
		_rmt_BeginD3D11Sample( Tag );
#endif
	}
}

//////////////////////////////////////////////////////////////////////////
// exitGPUSection
//virtual
void DsProfilerRemotery::exitGPUSection( const char* Tag )
{
	if( UseGL_ )
	{
#if RMT_USE_OPENGL
		_rmt_EndOpenGLSample();
#endif
	}

	if( UseD3D11_ )
	{
#if RMT_USE_D3D11
		_rmt_EndD3D11Sample();
#endif
	}
}

//////////////////////////////////////////////////////////////////////////
// startAsync
void DsProfilerRemotery::startAsync( const char* Tag, void* Data )
{
}

//////////////////////////////////////////////////////////////////////////
// stepAsync
void DsProfilerRemotery::stepAsync( const char* Tag, void* Data )
{
}

//////////////////////////////////////////////////////////////////////////
// endAsync
void DsProfilerRemotery::endAsync( const char* Tag, void* Data )
{

}

//////////////////////////////////////////////////////////////////////////
// instantEvent
//virtual
void DsProfilerRemotery::instantEvent( const char* Tag )
{

}


#endif // PSY_USE_PROFILER

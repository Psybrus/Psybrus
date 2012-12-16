/**************************************************************************
*
* File:		GaCameraComponent.h
* Author:	Neil Richardson 
* Ver/Date:	15/12/12	
* Description:
*		Portaudio component.
*		
*
*
* 
**************************************************************************/

#ifndef __GaPortaudioComponent_H__
#define __GaPortaudioComponent_H__

#include "Psybrus.h"

#include <portaudio.h>

//////////////////////////////////////////////////////////////////////////
// GaPortaudioComponent
typedef CsResourceRef< class GaPortaudioComponent > GaPortaudioComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaPortaudioComponent
class GaPortaudioComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaPortaudioComponent );

	void								initialise( const Json::Value& Object );

	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

	BcU32								noofInputFrames() const;
	void								popInputFrames( BcU32 NoofFrames, std::vector< BcF32 >& OutputBuffer );
	
private:
	static int StaticStreamCallback(
		const void *input, void *output,
		unsigned long frameCount,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void *userData );

	int streamCallback(
		const void *input, void *output,
		unsigned long frameCount,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags );

private:
	PaStream*							pPaStream_;

	static BcAtomicU32					GlobalPaRefCount_;

	mutable BcMutex						InputBufferLock_;
	std::vector< BcF32 >				InputBuffer_;

};

#endif


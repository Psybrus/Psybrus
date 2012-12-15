/**************************************************************************
*
* File:		GaCameraComponent.h
* Author:	Neil Richardson 
* Ver/Date:	15/12/12	
* Description:
*		Player component.
*		
*
*
* 
**************************************************************************/

#ifndef __GaPlayerComponent_H__
#define __GaPlayerComponent_H__

#include "Psybrus.h"

#include "GaPortaudioComponent.h"

//////////////////////////////////////////////////////////////////////////
// GaPlayerComponent
typedef CsResourceRef< class GaPlayerComponent > GaPlayerComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaPlayerComponent
class GaPlayerComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaPlayerComponent );

	void								initialise( const Json::Value& Object );

	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

	void								autoCorrelateInputBuffer();
	void								findAutocorrelationPeaks();

	void								analyzeAudio( BcReal& Pitch, BcReal& PeriodSD, BcReal& RMS );
	
private:
	GaPortaudioComponentRef				PortaudioComponent_;
	ScnCanvasComponentRef				Canvas_;

	ScnFontComponentRef					Font_;

	struct TPeak
	{
		BcF32							Index_;
		BcF32							Value_;
	};

	class GaPlayerComponentAutocorrelationPeakSorter
	{
	public:
		bool operator()( const TPeak& A, const TPeak& B )
		{
			return A.Value_ > B.Value_;
		}
	};
	
	std::vector< BcF32 >				InputBuffer_;
	std::vector< BcF32 >				AutocorrelationBuffer_;
	std::vector< TPeak >				AutocorrelationPeaks_;

	BcVec3d								Position_;
	BcVec3d								TargetPosition_;

	BcReal								SmoothedEstimatedPitch_;

	BcReal								LockEstimatedPitch_;
	BcReal								LockTimer_;
	BcReal								MaxLockTime_;
	BcReal								LockCooldownTimer_;
	BcReal								MaxCooldownTime_;
	
	BcReal								LastDecidingPitch_;
};

#endif


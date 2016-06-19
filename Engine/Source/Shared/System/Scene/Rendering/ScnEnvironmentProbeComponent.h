#pragma once

#include "System/Scene/ScnComponent.h"
#include "System/Scene/ScnComponentProcessor.h"
#include "System/Scene/ScnCoreCallback.h"

#include <deque>

//////////////////////////////////////////////////////////////////////////
// ScnEnvironmentProbeProcessor
class ScnEnvironmentProbeProcessor:
	public ScnComponentProcessor,
	public ScnCoreCallback
{
public:
	ScnEnvironmentProbeProcessor();
	virtual ~ScnEnvironmentProbeProcessor();

private:
	void initialise() override;
	void shutdown() override;
	void updateProbes( const ScnComponentList& Components );

	// ScnCoreCallback
	void onAttachComponent( ScnComponent* Component ) override;
	void onDetachComponent( ScnComponent* Component ) override;

private:
	std::deque< class ScnEnvironmentProbeComponent* > ProbeUpdateQueue_;
		
};

//////////////////////////////////////////////////////////////////////////
// ScnEnvironmentProbeComponent
class ScnEnvironmentProbeComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( ScnEnvironmentProbeComponent, ScnComponent );

	ScnEnvironmentProbeComponent();
	virtual ~ScnEnvironmentProbeComponent();

	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;
	
private:
	friend class ScnEnvironmentProbeProcessor;

	ScnEntity* TemplateRenderer_ = nullptr;
	ScnEntity* Renderer_ = nullptr;

	class ScnTexture* Texture_ = nullptr;

};

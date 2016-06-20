#pragma once

#include "System/Scene/ScnComponent.h"
#include "System/Scene/ScnComponentProcessor.h"
#include "System/Scene/ScnCoreCallback.h"

#include <deque>

//////////////////////////////////////////////////////////////////////////
// ScnEnvironmentProbeProcessor
class ScnEnvironmentProbeProcessor:
	public BcGlobal< ScnEnvironmentProbeProcessor >,
	public ScnComponentProcessor,
	public ScnCoreCallback
{
public:
	ScnEnvironmentProbeProcessor();
	virtual ~ScnEnvironmentProbeProcessor();

	class ScnTexture* getProbeEnvironmentMap( const MaVec3d& Position ) const;

private:
	void initialise() override;
	void shutdown() override;
	void updateProbes( const ScnComponentList& Components );

	// ScnCoreCallback
	void onAttachComponent( ScnComponent* Component ) override;
	void onDetachComponent( ScnComponent* Component ) override;

private:
	std::vector< class ScnEnvironmentProbeComponent* > EnvironmentProbes_;
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

	std::array< RsFrameBufferUPtr, 6 > CubemapFaceTargets_;
	MaVec3d GeneratedWorldPosition_ = MaVec3d( FLT_MAX, FLT_MAX, FLT_MAX );

};

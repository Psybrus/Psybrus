#pragma once

#include "System/Scene/ScnComponentProcessor.h"
#include "System/Scene/ScnCoreCallback.h"

//////////////////////////////////////////////////////////////////////////
// ScnViewProcessor
class ScnViewProcessor:
	public ScnComponentProcessor,
	public ScnCoreCallback
{
public:
	REFLECTION_DECLARE_DERIVED( ScnViewProcessor, ScnComponentProcessor );

	ScnViewProcessor();
	virtual ~ScnViewProcessor();

	void initialise() override;
	void shutdown() override;

private:
	void renderViews( const ScnComponentList& InComponents );

private:
	void onAttachComponent( class ScnComponent* Component ) override;
	void onDetachComponent( class ScnComponent* Component ) override;

private:
	std::set< class ScnRenderableComponent* > RenderableComponents_;
	ScnComponentList GatheredComponents_;
};

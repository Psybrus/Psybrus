#pragma once

#include "Editor/EdSceneContext.h"
#include "System/Debug/DsImGui.h"

class EdPanelSceneHierarchy
{
public:
	static void Register( EdSceneContext& SceneContext );

	EdPanelSceneHierarchy( EdSceneContext& SceneContext );
	EdPanelSceneHierarchy( const EdPanelSceneHierarchy& Other );
	void operator()( BcU32 );

private:
	void recurseNode( class ScnComponent* Component );

	EdSceneContext& SceneContext_;
};

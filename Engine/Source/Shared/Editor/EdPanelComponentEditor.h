#pragma once

#include "Editor/EdSceneContext.h"
#include "System/Debug/DsImGui.h"

class EdPanelComponentEditor
{
public:
	static void Register( EdSceneContext& SceneContext );

	EdPanelComponentEditor( EdSceneContext& SceneContext );
	EdPanelComponentEditor( const EdPanelComponentEditor& Other );
	void operator()( BcU32 );

private:
	EdSceneContext& SceneContext_;
};

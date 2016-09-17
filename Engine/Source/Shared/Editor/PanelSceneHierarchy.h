#pragma once

#include "Editor/SceneContext.h"
#include "System/Debug/DsImGui.h"

namespace Editor
{
	class PanelSceneHierarchy
	{
	public:
		static void Register( SceneContext& SceneContext );

		PanelSceneHierarchy( SceneContext& SceneContext );
		PanelSceneHierarchy( const PanelSceneHierarchy& Other );
		void operator()( BcU32 );

	private:
		void recurseNode( class ScnComponent* Component );

		SceneContext& SceneContext_;
	};
}
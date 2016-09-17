#pragma once

#include "Editor/SceneContext.h"
#include "System/Debug/DsImGui.h"

namespace Editor
{
	class PanelComponentEditor
	{
	public:
		static void Register( SceneContext& SceneContext );

		PanelComponentEditor( SceneContext& SceneContext );
		PanelComponentEditor( const PanelComponentEditor& Other );
		void operator()( BcU32 );

	private:
		SceneContext& SceneContext_;
	};
}

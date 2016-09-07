#include "Editor/Editor.h"

#include "Editor/EdPanelComponentEditor.h"
#include "Editor/EdPanelSceneHierarchy.h"

#include "System/Scene/ScnCore.h"

namespace Editor
{	
	EdSceneContext SceneContext_;

	void Init()
	{
		if( ScnCore::pImpl() )
		{
			// Register scene context for callbacks.
			ScnCore::pImpl()->addCallback( &SceneContext_ );

			// Register panels.
			EdPanelComponentEditor::Register( SceneContext_ );
			EdPanelSceneHierarchy::Register( SceneContext_ );
		}
	}
}
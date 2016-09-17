#include "Editor/SceneContext.h"

namespace Editor
{
	//////////////////////////////////////////////////////////////////////////
	// onAttachComponent
	void SceneContext::onAttachComponent( class ScnComponent* Component )
	{

	}

	//////////////////////////////////////////////////////////////////////////
	// onDetachComponent
	void SceneContext::onDetachComponent( class ScnComponent* Component )
	{
		auto It = DebugComponents_.find( Component );
		if( It != DebugComponents_.end() )
		{
			DebugComponents_.erase( It );
		}
	}
}

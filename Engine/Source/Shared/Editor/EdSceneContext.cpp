#include "Editor/EdSceneContext.h"

//////////////////////////////////////////////////////////////////////////
// onAttachComponent
void EdSceneContext::onAttachComponent( class ScnComponent* Component )
{

}

//////////////////////////////////////////////////////////////////////////
// onDetachComponent
void EdSceneContext::onDetachComponent( class ScnComponent* Component )
{
	auto It = DebugComponents_.find( Component );
	if( It != DebugComponents_.end() )
	{
		DebugComponents_.erase( It );
	}
}

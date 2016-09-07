#include "Editor/EdPanelSceneHierarchy.h"

#include "System/Debug/DsCore.h"
#include "System/Scene/ScnCore.h"

//////////////////////////////////////////////////////////////////////////
// Register
void EdPanelSceneHierarchy::Register( EdSceneContext& SceneContext )
{
	DsCore::pImpl()->registerPanel( "Scene", "Hierarchy", "Ctrl+Shift+H", 
		EdPanelSceneHierarchy( SceneContext ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
EdPanelSceneHierarchy::EdPanelSceneHierarchy( EdSceneContext& SceneContext ):
	SceneContext_( SceneContext )
{
}

//////////////////////////////////////////////////////////////////////////
// Copy Ctor
EdPanelSceneHierarchy::EdPanelSceneHierarchy( const EdPanelSceneHierarchy& Other ):
	SceneContext_( Other.SceneContext_ )
{
}

//////////////////////////////////////////////////////////////////////////
// Callable
void EdPanelSceneHierarchy::operator()( BcU32 )
{
	if( ImGui::TreeNode( "Scene Hierarchy" ) )
	{
		BcU32 Idx = 0;
		while( ScnEntity* Entity = ScnCore::pImpl()->getEntity( Idx++ ) )
		{
			if( Entity->getParentEntity() == nullptr )
			{
				recurseNode( Entity );
			}
		}
		ImGui::TreePop();
	}
}

//////////////////////////////////////////////////////////////////////////
// recurseNode
void EdPanelSceneHierarchy::recurseNode( class ScnComponent* Component )
{
	ImGui::PushID( Component );
	if( Component->isTypeOf< ScnEntity >() )
	{
		auto TreeNodeOpen = ImGui::TreeNode( Component, "" );

		ImGui::SameLine();
		auto IsSelected = SceneContext_.DebugComponents_.find( Component ) != SceneContext_.DebugComponents_.end();
		auto Colour = IsSelected ? RsColour::WHITE : RsColour::GRAY;
		if( ImGui::SmallButton( (*Component->getName()).c_str() ) )
		{
			SceneContext_.DebugComponents_.clear();
			SceneContext_.DebugComponents_.insert( Component );
		}	

		if( TreeNodeOpen )
		{
			BcU32 ChildIdx = 0;
			while( auto Child = Component->getComponent( ChildIdx++ ) )
			{
				recurseNode( Child );
			}
			ImGui::TreePop();
		}
	}
	ImGui::PopID();
}

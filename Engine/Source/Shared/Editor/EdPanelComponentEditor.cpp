#include "Editor/EdPanelComponentEditor.h"

#include "System/Debug/DsCore.h"
#include "System/Debug/DsImGuiFieldEditor.h"
#include "System/Scene/ScnCore.h"

//////////////////////////////////////////////////////////////////////////
// Register
void EdPanelComponentEditor::Register( EdSceneContext& SceneContext )
{
	DsCore::pImpl()->registerPanel( "Scene", "Component Editor", "Ctrl+Shift+E", 
		EdPanelComponentEditor( SceneContext ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
EdPanelComponentEditor::EdPanelComponentEditor( EdSceneContext& SceneContext ):
	SceneContext_( SceneContext )
{
}

//////////////////////////////////////////////////////////////////////////
// Copy Ctor
EdPanelComponentEditor::EdPanelComponentEditor( const EdPanelComponentEditor& Other ):
	SceneContext_( Other.SceneContext_ )
{
}

//////////////////////////////////////////////////////////////////////////
// Callable
void EdPanelComponentEditor::operator()( BcU32 )
{
	ImGui::Text( "Components editing: %u", SceneContext_.DebugComponents_.size() );
	ImGui::Separator();

	for( auto Component : SceneContext_.DebugComponents_ )
	{
		auto UpperClass = Component->getClass();
		auto Class = UpperClass;

		if( Component->isTypeOf< ScnEntity >() )
		{
			auto Entity = static_cast< ScnEntity* >( Component );
			auto OnEdit = []( ScnComponent* Component )
			{
				auto UpperClass = Component->getClass();
				auto Class = UpperClass;

				// Find editor.
				DsImGuiFieldEditor* FieldEditor = nullptr;
				while( FieldEditor == nullptr && Class != nullptr )
				{
					FieldEditor = Class->getAttribute< DsImGuiFieldEditor >();
					Class = Class->getSuper();

				}
				if( FieldEditor )
				{
					FieldEditor->onEdit( "", Component, UpperClass, bcRFF_NONE );
				}
			};

			auto Mat4dClass = ReManager::GetClass< MaMat4d >();
			auto Mat4dEditor = Mat4dClass->getAttribute< DsImGuiFieldEditor >();

			MaMat4d LocalMatrix = Entity->getLocalMatrix();
			Mat4dEditor->onEdit( "Local:", &LocalMatrix, Mat4dClass, bcRFF_NONE );
			Entity->setLocalMatrix( LocalMatrix );

			ImGui::Separator();

			for( auto Idx = 0; Idx < Entity->getNoofComponents(); ++Idx )
			{
				auto EntityComponent = Entity->getComponent( Idx );
				if( !EntityComponent->isTypeOf< ScnEntity >() )
				{
					OnEdit( EntityComponent );
					ImGui::Separator();
				}
			}
		}
		else
		{
			// Ignore for now.
		}
	}

	if( SceneContext_.DebugComponents_.size() == 1 )
	{
		auto Component = *SceneContext_.DebugComponents_.begin();
		auto Entity = ScnEntityRef( Component );

		auto getParent = []( ScnEntity* Entity )
		{
			if( Entity->getParentEntity() )
			{
				return Entity->getParentEntity()->getWorldMatrix();
			}
			return MaMat4d();
		};
	}
}

#include "Editor/Editor.h"

#include "Editor/EdPanelComponentEditor.h"
#include "Editor/EdPanelSceneHierarchy.h"

#include "System/Scene/ScnCore.h"
#include "System/Scene/Rendering/ScnRenderableComponent.h"
#include "System/Scene/Rendering/ScnViewComponent.h"

#include <vector>

namespace Editor
{	
	EdSceneContext SceneContext_;


	struct EditableHandle
	{
		BcU32 ID_ = 0;
		const char* Name_ = nullptr;
		MaVec3d Position_;
		BcU32 CombinedID_ = 0;
	};
	std::vector< EditableHandle > EditableHandles_;

	BcU32 SelectedID_ = 0;

	// View callback.
	class ViewCallback : public ScnViewCallback
	{
	public:
		ViewCallback()
		{
		}

		void onViewDrawPreRender( ScnRenderContext& RenderContext ) override
		{
		}

		void onViewDrawPostRender( ScnRenderContext& RenderContext ) override
		{
			// Intercept transparent passes to render what we need.
			if( BcContainsAllFlags( RenderContext.View_->getPasses(), RsRenderSortPassFlags::TRANSPARENT ) )
			{				
				// TODO: Multiple view support.
				ImGui::Begin( "#editor", nullptr, ImGui::GetIO().DisplaySize, 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus );
				auto DrawList = ImGui::GetWindowDrawList();
				ImGui::End();

				for( const auto& Handle : EditableHandles_ )
				{
					const BcF32 HandleRadius = 8.0f;
					const auto ScreenPosition = ( RenderContext.View_->getScreenPosition( Handle.Position_ ) +
						MaVec2d( ImGui::GetIO().DisplaySize ) * 0.5f );

					bool MouseOver = false;
					const auto Distance = ( ScreenPosition - ImGui::GetIO().MousePos ).magnitude();
					if( Distance < HandleRadius )
					{
						MouseOver = true;
					}

					DrawList->AddCircleFilled( ScreenPosition, HandleRadius, MouseOver ? RsColour::GREEN.asRGBA() : RsColour::WHITE.asRGBA() );
					DrawList->AddCircle( ScreenPosition, HandleRadius, RsColour::BLACK.asRGBA() );

					if( ImGui::GetIO().MouseClicked[0] && MouseOver )
					{
						SelectedID_ = Handle.CombinedID_;
					}					
				}
			}
		}
	};

	ViewCallback ViewCallback_;

	void Init()
	{
		if( ScnCore::pImpl() )
		{
			// Register scene context for callbacks.
			ScnCore::pImpl()->addCallback( &SceneContext_ );

			// Register panels.
			EdPanelComponentEditor::Register( SceneContext_ );
			EdPanelSceneHierarchy::Register( SceneContext_ );

			// Register for view callback.
			ScnCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN,
				[]( EvtID, const EvtBaseEvent& )
				{
					ScnViewProcessor::pImpl()->registerViewCallback( &ViewCallback_ );
					return evtRET_REMOVE;
				} );

			// Register for next frame callback.
			ScnCore::pImpl()->subscribe( sysEVT_SYSTEM_PRE_UPDATE,
				[]( EvtID, const EvtBaseEvent& )
				{
					EditableHandles_.clear();
					return evtRET_PASS;
				} );
			
		}
	}

	bool Handle( BcU32 ID, const char* Name, const MaVec3d Position )
	{
		EditableHandle EditableHandle;
		EditableHandle.ID_ = ID;
		EditableHandle.Name_ = Name;
		EditableHandle.Position_ = Position;
		EditableHandle.CombinedID_ = BcHash::GenerateCRC32( EditableHandle.CombinedID_, &ID, sizeof( ID ) );
		EditableHandle.CombinedID_ = BcHash::GenerateCRC32( EditableHandle.CombinedID_, Name, BcStrLength( Name ) );

		// No need to push handle into list if it's selected.
		if( EditableHandle.CombinedID_ == SelectedID_ )
		{
			return true;
		}

		EditableHandles_.push_back( EditableHandle );
		return false;
	}
}
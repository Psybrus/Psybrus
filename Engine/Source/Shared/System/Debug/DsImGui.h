#pragma once 

#include "Math/MaVec2d.h"
#include "Math/MaVec4d.h"

//////////////////////////////////////////////////////////////////////////
// Cast operators.
#define IM_VEC2_CLASS_EXTRA                                                 \
		ImVec2(const class MaCPUVec2d& f);                                  \
		operator class MaCPUVec2d() const;

#define IM_VEC4_CLASS_EXTRA                                                 \
		ImVec4(const class MaCPUVec4d& f);                                  \
		operator class MaCPUVec4d() const;

//////////////////////////////////////////////////////////////////////////
// ImGui include.
#include "imgui.h"

//////////////////////////////////////////////////////////////////////////
// Forward declarations.
class RsContext;
class RsFrame;
class RsTexture;
class RsSamplerState;

//////////////////////////////////////////////////////////////////////////
// ImGui functions.
namespace ImGui
{
	class ScopedID
	{
	public:
		ScopedID( const char* str_id )
		{
			ImGui::PushID( str_id );
		}

		ScopedID( const char* str_id_begin, const char* str_id_end )
		{
			ImGui::PushID( str_id_begin, str_id_end );
		}

		ScopedID( const void* ptr_id )
		{
			ImGui::PushID( ptr_id );
		}

		ScopedID( const int int_id )
		{
			ImGui::PushID( int_id );
		}

		~ScopedID()
		{
			ImGui::PopID();
		}
	};

	class ScopedIndent
	{
	public:
		ScopedIndent()
		{
			ImGui::Indent();
		}

		~ScopedIndent()
		{
			ImGui::Unindent();
		}
	};

	/**
	 * Add shadowed text.
	 */
	void AddShadowedText( ImDrawList* DrawList, MaVec2d Position, int Colour, const char* Format, ... );

	namespace Psybrus
	{
		ImTextureID AddTexture( RsTexture* Texture, RsSamplerState* SamplerState = nullptr );
		RsTexture* GetTexture( ImTextureID ID );
		RsSamplerState* GetSamplerState( ImTextureID ID );

		bool Init();
		void WaitFrame();
		bool NewFrame();
		void Render( RsContext* Context, RsFrame* Frame );
		void Shutdown();


	} // end Psybrus
} // end ImGui

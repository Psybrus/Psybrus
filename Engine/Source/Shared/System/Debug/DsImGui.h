#pragma once 

#include "Math/MaVec2d.h"
#include "Math/MaVec4d.h"
#include "Math/MaMat4d.h"

//////////////////////////////////////////////////////////////////////////
// Cast operators.
#define IM_VEC2_CLASS_EXTRA                                                 \
		ImVec2(const class MaCPUVec2d& f);                                  \
		operator class MaCPUVec2d() const;

#define IM_VEC4_CLASS_EXTRA                                                 \
		ImVec4(const class MaCPUVec4d& f);                                  \
		operator class MaCPUVec4d() const;

//////////////////////////////////////////////////////////////////////////
// ImGui includes.
#include "imgui.h"

#if COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wbackslash-newline-escape"
#endif

#include "ImGuizmo.h"

#if COMPILER_CLANG
#pragma clang diagnostic pop
#endif

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

namespace ImGuizmo
{
	void SetMatrices( const MaMat4d& View, const MaMat4d& Projection );
	void DrawCube( const MaMat4d& Matrix );
	void Translate( MaMat4d& Matrix, MaMat4d* DeltaMatrix = nullptr );
	void Scale( MaMat4d& Matrix, MaMat4d* DeltaMatrix = nullptr );
	void Rotate( MaMat4d& Matrix, MaMat4d* DeltaMatrix = nullptr );
}


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

//////////////////////////////////////////////////////////////////////////
// ImGui functions.
namespace ImGui
{
namespace Psybrus
{
		bool Init();
		void WaitFrame();
		bool NewFrame();
		void Render( RsContext* Context, RsFrame* Frame );
		void Shutdown();
} // emnd ImGui
} // end Psybrus


#include "System/Scene/ScnImGui.h"

#include "System/Renderer/RsCore.h"
#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsProgram.h"
#include "System/Renderer/RsRenderState.h"
#include "System/Renderer/RsSamplerState.h"
#include "System/Renderer/RsVertexDeclaration.h"

#include "System/Os/OsCore.h"
#include "System/Os/OsClient.h"

//////////////////////////////////////////////////////////////////////////
// Cast operators.
ImVec2::ImVec2(const MaCPUVec2d& f):
	x( f.x() ),
	y( f.y() )
{

}

ImVec2::operator MaCPUVec2d() const
{
	return MaVec2d( x, y );
}

ImVec4::ImVec4(const MaCPUVec4d& f):
	x( f.x() ),
	y( f.y() ),
	z( f.z() ),
	w( f.w() )
{

}

ImVec4::operator MaCPUVec4d() const
{
	return MaVec4d( x, y, z, w );
}

//////////////////////////////////////////////////////////////////////////
// Private.
namespace 
{
	/// Render state.
	RsRenderStateUPtr RenderState_;
	/// Font sampler.
	RsSamplerStateUPtr FontSampler_;
	/// Font texture.
	RsTextureUPtr FontTexture_;
	/// Vertex declarartion.
	RsVertexDeclarationUPtr VertexDeclaration_;
	/// Vertex buffer used by internal implementation.
	RsBufferUPtr VertexBuffer_;
	/// Constant buffer.
	RsBufferUPtr ConstantBuffer_;
	/// Constant buffer structure.
	struct ConstantBuffer
	{
		MaMat4d ProjectionMatrix_;
	};
	/// Program.
	RsProgramUPtr Program_;

	/// Current draw context.
	RsContext* DrawContext_ = nullptr;
	/// Current draw frame.
	RsFrame* DrawFrame_ = nullptr;
	/// Scoped draw class to manage current draw properties.
	class ScopedDraw
	{
	public:
		ScopedDraw( RsContext* Context, RsFrame* Frame )
		{
			BcAssert( DrawContext_ == nullptr );
			BcAssert( DrawFrame_ == nullptr );
			DrawContext_ = Context;
			DrawFrame_ = Frame;
		}

		~ScopedDraw()
		{
			DrawContext_ = nullptr;
			DrawFrame_ = nullptr;
		}
	};

	/**
	 * Perform the draw.
	 */
	void RenderDrawLists( ImDrawList** const CmdLists, int CmdListsCount )
	{
		BcAssert( DrawContext_ != nullptr );
		BcAssert( DrawFrame_ != nullptr );
	}

}

//////////////////////////////////////////////////////////////////////////
// Implementation for Psybrus.
namespace ImGui
{
namespace Psybrus
{
	bool Init()
	{
		VertexDeclaration_.reset( RsCore::pImpl()->createVertexDeclaration(
			RsVertexDeclarationDesc( 3 )
				.addElement( RsVertexElement( 0, 0, 4, RsVertexDataType::FLOAT32, RsVertexUsage::POSITION, 0 ) )
				.addElement( RsVertexElement( 0, 16, 2, RsVertexDataType::FLOAT32, RsVertexUsage::TEXCOORD, 0 ) )
				.addElement( RsVertexElement( 0, 24, 4, RsVertexDataType::UBYTE_NORM, RsVertexUsage::COLOUR, 0 ) ) ) );

		VertexBuffer_.reset( RsCore::pImpl()->createBuffer( 
			RsBufferDesc( 
				RsBufferType::VERTEX,
				RsResourceCreationFlags::STREAM, 
				10000 * VertexDeclaration_->getDesc().getMinimumStride() ) ) );

		ConstantBuffer_.reset( RsCore::pImpl()->createBuffer(
			RsBufferDesc(
				RsBufferType::UNIFORM,
				RsResourceCreationFlags::STREAM,
				sizeof( ConstantBuffer ) ) ) );

		auto RenderStateDesc = RsRenderStateDesc();
		RenderStateDesc.BlendState_.RenderTarget_[ 0 ].Enable_ = BcTrue;
		RenderStateDesc.BlendState_.RenderTarget_[ 0 ].SrcBlend_ = RsBlendType::SRC_ALPHA;
		RenderStateDesc.BlendState_.RenderTarget_[ 0 ].DestBlend_ = RsBlendType::INV_SRC_ALPHA;
		RenderStateDesc.BlendState_.RenderTarget_[ 0 ].BlendOp_ = RsBlendOp::ADD;
		RenderStateDesc.BlendState_.RenderTarget_[ 0 ].SrcBlendAlpha_ = RsBlendType::SRC_ALPHA;
		RenderStateDesc.BlendState_.RenderTarget_[ 0 ].DestBlendAlpha_ = RsBlendType::ZERO;
		RenderStateDesc.BlendState_.RenderTarget_[ 0 ].BlendOpAlpha_ = RsBlendOp::ADD;
		RenderStateDesc.BlendState_.RenderTarget_[ 0 ].WriteMask_ = 0xf;
		RenderState_ = RsCore::pImpl()->createRenderState( RenderStateDesc );

		unsigned char* Pixels = nullptr;
		int Width, Height;
		ImGuiIO& IO = ImGui::GetIO();
		IO.Fonts->GetTexDataAsRGBA32( &Pixels, &Width, &Height );
		FontTexture_.reset( RsCore::pImpl()->createTexture(
			RsTextureDesc( 
				RsTextureType::TEX2D,
				RsResourceCreationFlags::STATIC,
				RsResourceBindFlags::SHADER_RESOURCE,
				RsTextureFormat::R8G8B8A8,
				1,
				Width, 
				Height,
				1 ) ) );

		RsTextureSlice Slice = { 0, RsTextureFace::NONE };
		RsCore::pImpl()->updateTexture( FontTexture_.get(), Slice, RsResourceUpdateFlags::ASYNC,
			[ Pixels, Width, Height ]( class RsTexture*, const RsTextureLock& Lock )
			{
				BcAssert( Lock.Pitch_ == ( Width * 4 ) );
				memcpy( Lock.Buffer_, Pixels, Width * Height * 4 );
			} );


		return true;
	}


	void Shutdown()
	{
		ImGui::Shutdown();

		BcAssert( DrawContext_ == nullptr );
		BcAssert( DrawFrame_ == nullptr );
		VertexDeclaration_.reset();
		VertexBuffer_.reset();
		ConstantBuffer_.reset();
		RenderState_.reset();
		FontSampler_.reset();
		FontTexture_.reset();
	}

	void NewFrame( RsContext* Context, RsFrame* Frame )
	{
		ScopedDraw ScopedDraw( Context, Frame );

		ImGuiIO& IO = ImGui::GetIO();

		// Grab client to get current size.
		OsClient* Client = OsCore::pImpl()->getClient( 0 );
		IO.DisplaySize = ImVec2( Client->getWidth(), Client->getHeight() );

	    // Start the frame
	    ImGui::NewFrame();
	}
} // end Psybrus
} // end ImGui

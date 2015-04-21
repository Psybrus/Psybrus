#include "System/Scene/ScnImGui.h"
#include "System/Scene/Rendering/ScnShader.h"

#include "System/Renderer/RsCore.h"
#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsProgram.h"
#include "System/Renderer/RsRenderState.h"
#include "System/Renderer/RsSamplerState.h"
#include "System/Renderer/RsVertexDeclaration.h"

#include "System/Content/CsCore.h"

#include "System/Os/OsCore.h"
#include "System/Os/OsClient.h"
#include "System/Os/OsEvents.h"


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
	/// Constant block.
	ScnShaderViewUniformBlockData ConstantBlock_;
	/// Program.
	RsProgramUPtr Program_;
	/// Imgui package.
	CsPackage* Package_ = nullptr;

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
		PSY_LOGSCOPEDCATEGORY( "ImGui" );
		BcAssert( DrawContext_ != nullptr );
		BcAssert( DrawFrame_ != nullptr );
		BcAssert( Package_ != nullptr );
		if( !Package_->isReady() )
		{
			PSY_LOG( "Still waiting on assets to load. Skipping render." );
		}

		// Create appropriate projection matrix.
		ImGuiIO& IO = ImGui::GetIO();
	
		ConstantBlock_.ProjectionTransform_.orthoProjection( 
			0.0f, IO.DisplaySize.x,
			IO.DisplaySize.y, 0,
			-1.0f, 1.0f );
		ConstantBlock_.InverseProjectionTransform_ = ConstantBlock_.ProjectionTransform_;
		ConstantBlock_.InverseProjectionTransform_.inverse();
		ConstantBlock_.InverseViewTransform_ = MaMat4d();
		ConstantBlock_.ViewTransform_ = ConstantBlock_.InverseViewTransform_;
		ConstantBlock_.ViewTransform_.inverse();
		ConstantBlock_.ClipTransform_ = ConstantBlock_.ViewTransform_ * ConstantBlock_.ProjectionTransform_;

		// TODO: Update.
		//RsCore::pImpl()->

		// Update vertex buffer.


		BcU32 VertexOffset = 0;
		for( int CmdListIdx = 0; CmdListIdx < CmdListsCount; ++CmdListIdx )
		{
			const ImDrawList* CmdList = CmdLists[ CmdListIdx ];

			for( size_t CmdIdx = 0; CmdIdx < CmdList->commands.size(); ++CmdIdx )
			{
	            const ImDrawCmd* Cmd = &CmdList->commands[ CmdIdx ];
				if( Cmd->user_callback )
				{
					Cmd->user_callback( CmdList, Cmd );
				}
				else
				{
					//const D3D11_RECT r = { (LONG)pcmd->clip_rect.x, (LONG)pcmd->clip_rect.y, (LONG)pcmd->clip_rect.z, (LONG)pcmd->clip_rect.w };
					//g_pd3dDeviceContext->PSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)&pcmd->texture_id);
					//g_pd3dDeviceContext->RSSetScissorRects(1, &r); 
					//g_pd3dDeviceContext->Draw(pcmd->vtx_count, vtx_offset);
				}
				VertexOffset += Cmd->vtx_count;
			}
		}
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
				sizeof( ConstantBlock_ ) ) ) );

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

		IO.RenderDrawListsFn = RenderDrawLists;
#if PLATFORM_WINDOWS		
		IO.ImeWindowHandle = (HWND)OsCore::pImpl()->getClient( 0 )->getWindowHandle();
#endif

		IO.KeyMap[ ImGuiKey_Tab ] = OsEventInputKeyboard::KEYCODE_TAB;
		IO.KeyMap[ ImGuiKey_LeftArrow ] = OsEventInputKeyboard::KEYCODE_LEFT;
		IO.KeyMap[ ImGuiKey_RightArrow ] = OsEventInputKeyboard::KEYCODE_RIGHT;
		IO.KeyMap[ ImGuiKey_UpArrow ] = OsEventInputKeyboard::KEYCODE_UP;
		IO.KeyMap[ ImGuiKey_DownArrow ] = OsEventInputKeyboard::KEYCODE_DOWN;
		IO.KeyMap[ ImGuiKey_Home ] = OsEventInputKeyboard::KEYCODE_HOME;
		IO.KeyMap[ ImGuiKey_End ] = OsEventInputKeyboard::KEYCODE_END;
		IO.KeyMap[ ImGuiKey_Delete ] = OsEventInputKeyboard::KEYCODE_DELETE;
		IO.KeyMap[ ImGuiKey_Backspace ] = OsEventInputKeyboard::KEYCODE_BACKSPACE;
		IO.KeyMap[ ImGuiKey_Enter ] = OsEventInputKeyboard::KEYCODE_RETURN;
		IO.KeyMap[ ImGuiKey_Escape ] = OsEventInputKeyboard::KEYCODE_ESCAPE;
		IO.KeyMap[ ImGuiKey_A ] = 'A';
		IO.KeyMap[ ImGuiKey_C ] = 'C';
		IO.KeyMap[ ImGuiKey_V ] = 'V';
		IO.KeyMap[ ImGuiKey_X ] = 'X';
		IO.KeyMap[ ImGuiKey_Y ] = 'Y';
		IO.KeyMap[ ImGuiKey_Z ] = 'Z';

		// Request imgui packge.
		Package_ = CsCore::pImpl()->requestPackage( "imgui" );
		Package_->acquire();
		return true;
	}

	void NewFrame()
	{
		ImGuiIO& IO = ImGui::GetIO();

		// Grab client to get current size.
		OsClient* Client = OsCore::pImpl()->getClient( 0 );
		IO.DisplaySize = ImVec2( Client->getWidth(), Client->getHeight() );

		// Start the frame
		ImGui::NewFrame();
	}

	void Render( RsContext* Context, RsFrame* Frame )
	{
		ScopedDraw ScopedDraw( Context, Frame );

		ImGui::Render();
	}

	void Shutdown()
	{
		ImGui::Shutdown();

		Package_->release();

		BcAssert( DrawContext_ == nullptr );
		BcAssert( DrawFrame_ == nullptr );
		VertexDeclaration_.reset();
		VertexBuffer_.reset();
		ConstantBuffer_.reset();
		RenderState_.reset();
		FontSampler_.reset();
		FontTexture_.reset();
	}
} // end Psybrus
} // end ImGui

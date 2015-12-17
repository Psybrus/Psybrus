#include "System/Debug/DsImGui.h"

#include "System/Scene/Rendering/ScnShader.h"

#include "System/Renderer/RsCore.h"
#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsProgram.h"
#include "System/Renderer/RsRenderNode.h"
#include "System/Renderer/RsRenderState.h"
#include "System/Renderer/RsSamplerState.h"
#include "System/Renderer/RsVertexDeclaration.h"

#include "System/Content/CsCore.h"

#include "System/Os/OsCore.h"
#include "System/Os/OsClient.h"
#include "System/Os/OsEvents.h"

#include "System/SysFence.h"

#include "Base/BcMath.h"

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
	/// Vertex declarartion.
	RsVertexDeclarationUPtr VertexDeclaration_;
	/// Vertex buffer used by internal implementation.
	RsBufferUPtr VertexBuffer_;
	/// Index buffer used by internal implementation.
	RsBufferUPtr IndexBuffer_;
	/// Geometry binding.
	RsGeometryBindingUPtr GeometryBinding_;
	/// Font sampler.
	RsSamplerStateUPtr FontSampler_;
	/// Font texture.
	RsTextureUPtr FontTexture_;
	/// White texture.
	RsTextureUPtr WhiteTexture_;
	/// Uniform buffer.
	RsBufferUPtr UniformBuffer_;
	/// Program binding.
	RsProgram* Program_ = nullptr;
	/// Program binding desc.
	RsProgramBindingDesc ProgramBindingDesc_;
	/// Program binding for default.
	RsProgramBindingUPtr ProgramBinding_;
	/// Render state.
	RsRenderStateUPtr RenderState_;
	/// Uniform block.
	ScnShaderViewUniformBlockData UniformBlock_;
	/// Imgui package.
	CsPackage* Package_ = nullptr;
	/// Fence for synchronisation.
	SysFence RenderThreadFence_;
	/// Program bindings to destroy.
	std::vector< RsProgramBindingUPtr > DestroyProgramBindings_;

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

	/// Mouse wheel.
	BcF32 MouseWheel_ = 0.0f;
	/// Text input.
	std::vector< BcU32 > TextInput_;

	/**
	 * Perform the draw.
	 */
	void RenderDrawLists( ImDrawData* DrawData )
	{
		PSY_LOGSCOPEDCATEGORY( "ImGui" );
		BcAssert( DrawContext_ != nullptr );
		BcAssert( DrawFrame_ != nullptr );
		BcAssert( Package_ != nullptr );
		if( DrawData == nullptr || DrawData->CmdListsCount == 0 )
		{
			return;
		}
		
		ImDrawData CachedDrawData = *DrawData;

		if( !Package_->isReady() )
		{
			PSY_LOG( "Still waiting on assets to load. Skipping render." );
		}

		// Create appropriate projection matrix.
		ImGuiIO& IO = ImGui::GetIO();
	
		UniformBlock_.ProjectionTransform_.orthoProjection( 
			0.0f, IO.DisplaySize.x,
			IO.DisplaySize.y, 0,
			-1.0f, 1.0f );
		UniformBlock_.InverseProjectionTransform_ = UniformBlock_.ProjectionTransform_;
		UniformBlock_.InverseProjectionTransform_.inverse();
		UniformBlock_.InverseViewTransform_ = MaMat4d();
		UniformBlock_.ViewTransform_ = UniformBlock_.InverseViewTransform_;
		UniformBlock_.ViewTransform_.inverse();
		UniformBlock_.ClipTransform_ = UniformBlock_.ViewTransform_ * UniformBlock_.ProjectionTransform_;

		RsRenderSort Sort;
		Sort.Value_ = 0;
		Sort.Pass_ = RS_SORT_PASS_OVERLAY;		
		Sort.Viewport_ = RS_SORT_VIEWPORT_MAX;
		Sort.RenderTarget_ = RS_SORT_RENDERTARGET_MAX;
		Sort.NodeType_ = RS_SORT_NODETYPE_MAX;

		RenderThreadFence_.increment();

		// Destroy last frames program bindings.
		DestroyProgramBindings_.clear();

		auto Width = IO.DisplaySize.x;
		auto Height = IO.DisplaySize.y;
		DrawFrame_->queueRenderNode( Sort,
			[ CachedDrawData, Width, Height ]( RsContext* Context )
			{
				RsFrameBuffer* BackBuffer = Context->getBackBuffer();
				RsViewport Viewport( 0, 0, Width, Height );

				// Update constant buffr.
				Context->updateBuffer( 
					UniformBuffer_.get(), 0, sizeof( UniformBlock_ ), 
					RsResourceUpdateFlags::NONE,
					[]( RsBuffer* Buffer, const RsBufferLock& Lock )
					{
						memcpy( Lock.Buffer_, &UniformBlock_, sizeof( UniformBlock_ ) );
					} );

				const BcU32 UniformBufferSlot = Program_->findUniformBufferSlot( "ScnShaderViewUniformBlockData" );
				const BcU32 TextureSlot = Program_->findShaderResourceSlot( "aDiffuseTex" );
				const BcU32 SamplerSlot = Program_->findSamplerSlot( "aDiffuseTex" );

				// Update vertex buffer.
				Context->updateBuffer( 
					VertexBuffer_.get(), 0, VertexBuffer_->getDesc().SizeBytes_, 
					RsResourceUpdateFlags::NONE,
					[ CachedDrawData, UniformBufferSlot ]( RsBuffer* Buffer, const RsBufferLock& Lock )
					{
						ImDrawVert* Vertices = reinterpret_cast< ImDrawVert* >( Lock.Buffer_ );
						BcU32 NoofVertices = 0;
						for ( int CmdListIdx = 0; CmdListIdx < CachedDrawData.CmdListsCount; ++CmdListIdx )
						{
							const ImDrawList* CmdList = CachedDrawData.CmdLists[ CmdListIdx ];
							memcpy( Vertices, &CmdList->VtxBuffer[0], CmdList->VtxBuffer.size() * sizeof( ImDrawVert ) );
							// No uniform buffer slot in shader, so we need to transform before the vertex shader.
							if( UniformBufferSlot == BcErrorCode )
							{
								for( int VertIdx = 0; VertIdx < CmdList->VtxBuffer.size(); ++VertIdx )
								{
									Vertices[VertIdx].pos = Vertices[VertIdx].pos * UniformBlock_.ClipTransform_;
								}
							}
							Vertices += CmdList->VtxBuffer.size();
							NoofVertices += CmdList->VtxBuffer.size();
							BcAssert( (BcU8*)Vertices <= ((BcU8*)Lock.Buffer_ ) + Buffer->getDesc().SizeBytes_ );
						}
					} );

				// Update index buffer.
				Context->updateBuffer( 
					IndexBuffer_.get(), 0, IndexBuffer_->getDesc().SizeBytes_, 
					RsResourceUpdateFlags::NONE,
					[ CachedDrawData ]( RsBuffer* Buffer, const RsBufferLock& Lock )
					{
						static_assert( sizeof( ImDrawIdx ) == sizeof( BcU16 ), "Indices must be 16 bit." );
						ImDrawIdx* Indices = reinterpret_cast< ImDrawIdx* >( Lock.Buffer_ );
						BcU32 NoofIndices = 0;
		 				BcU32 VertexOffset = 0;
						for ( int CmdListIdx = 0; CmdListIdx < CachedDrawData.CmdListsCount; ++CmdListIdx )
						{
							const ImDrawList* CmdList = CachedDrawData.CmdLists[ CmdListIdx ];
							for( BcU32 Idx = 0; Idx < CmdList->IdxBuffer.size(); ++Idx )
							{
								BcU32 Index = VertexOffset + static_cast< BcU32 >( CmdList->IdxBuffer[ Idx ] );
								BcAssert( Index < 0xffff );
								Indices[ Idx ] = static_cast< BcU16 >( Index );
							}
							Indices += CmdList->IdxBuffer.size();
							NoofIndices += CmdList->IdxBuffer.size();
							VertexOffset += CmdList->VtxBuffer.size();
							BcAssert( (BcU8*)Indices <= ((BcU8*)Lock.Buffer_ ) + Buffer->getDesc().SizeBytes_ );
						}
					} );

 				BcU32 IndexOffset = 0;
				for( int CmdListIdx = 0; CmdListIdx < CachedDrawData.CmdListsCount; ++CmdListIdx )
				{
					const ImDrawList* CmdList = CachedDrawData.CmdLists[ CmdListIdx ];

					for( size_t CmdIdx = 0; CmdIdx < CmdList->CmdBuffer.size(); ++CmdIdx )
					{
						const ImDrawCmd* Cmd = &CmdList->CmdBuffer[ CmdIdx ];
						if( Cmd->UserCallback )
						{
							Cmd->UserCallback( CmdList, Cmd );
						}
						else
						{
							RsScissorRect ScissorRect(
								(BcS32)Cmd->ClipRect.x, 
								(BcS32)Cmd->ClipRect.y, 
								(BcS32)( Cmd->ClipRect.z - Cmd->ClipRect.x ), 
								(BcS32)( Cmd->ClipRect.w - Cmd->ClipRect.y ) );

							// Regenerate program binding if we need to.
							bool RecreateProgramBinding = false;
							if( Cmd->TextureId != nullptr )
							{
								if( TextureSlot == BcErrorCode || 
									ProgramBindingDesc_.ShaderResourceSlots_[ TextureSlot ].Texture_ != Cmd->TextureId )
								{
									if( TextureSlot != BcErrorCode && SamplerSlot != BcErrorCode )
									{
										RecreateProgramBinding |= ProgramBindingDesc_.setShaderResourceView( TextureSlot, (RsTexture*)Cmd->TextureId );
										RecreateProgramBinding |= ProgramBindingDesc_.setSamplerState( SamplerSlot, FontSampler_.get() );
									}
									RecreateProgramBinding |= ProgramBindingDesc_.setUniformBuffer( UniformBufferSlot, UniformBuffer_.get() );
								}
							}
							else
							{
								if( TextureSlot != BcErrorCode && SamplerSlot != BcErrorCode )
								{
									RecreateProgramBinding |= ProgramBindingDesc_.setShaderResourceView( TextureSlot, WhiteTexture_.get() );
									RecreateProgramBinding |= ProgramBindingDesc_.setSamplerState( SamplerSlot, FontSampler_.get() );
								}
								RecreateProgramBinding |= ProgramBindingDesc_.setUniformBuffer( UniformBufferSlot, UniformBuffer_.get() );
							}
							RecreateProgramBinding = true;
							if( RecreateProgramBinding && ProgramBinding_ )
							{
								DestroyProgramBindings_.emplace_back( std::move( ProgramBinding_ ) );
							}
							if( ProgramBinding_ == nullptr )
							{
								// Not typical recommended usage as it subverts RsCore.
								ProgramBinding_.reset( new RsProgramBinding( Context, Program_, ProgramBindingDesc_ ) );
								Context->createProgramBinding( ProgramBinding_.get() );
							}

							Context->drawIndexedPrimitives( 
								GeometryBinding_.get(),
								ProgramBinding_.get(),
								RenderState_.get(),
								BackBuffer,
								&Viewport,
								&ScissorRect,
								RsTopologyType::TRIANGLE_LIST,
								IndexOffset,
								Cmd->ElemCount,
								0 );
						}
						IndexOffset += Cmd->ElemCount;
					}
				}
				RenderThreadFence_.decrement();
			} );
	}
	
	/**
	 * Handle key down.
	 */
	eEvtReturn OnKeyDown( EvtID, const EvtBaseEvent& BaseEvent )
	{
		auto Event = BaseEvent.get< OsEventInputKeyboard >();
		ImGuiIO& IO = ImGui::GetIO();
		if( Event.KeyCode_ < 512 )
		{
			IO.KeysDown[ Event.KeyCode_ ] = 1;
		}

		if( Event.KeyCode_ == OsEventInputKeyboard::KEYCODE_CONTROL )
		{
			IO.KeyCtrl = true;
		}
		if( Event.KeyCode_ == OsEventInputKeyboard::KEYCODE_SHIFT )
		{
			IO.KeyShift = true;
		}
		if( Event.KeyCode_ == OsEventInputKeyboard::KEYCODE_ALT )
		{
			IO.KeyAlt = true;
		}
		return evtRET_PASS;
	}

	/**
	 * Handle key up.
	 */
	eEvtReturn OnKeyUp( EvtID, const EvtBaseEvent& BaseEvent )
	{
		auto Event = BaseEvent.get< OsEventInputKeyboard >();
		ImGuiIO& IO = ImGui::GetIO();
		if( Event.KeyCode_ < 512 )
		{
			IO.KeysDown[ Event.KeyCode_ ] = 0;
		}

		if( Event.KeyCode_ == OsEventInputKeyboard::KEYCODE_CONTROL )
		{
			IO.KeyCtrl = false;
		}
		if( Event.KeyCode_ == OsEventInputKeyboard::KEYCODE_SHIFT )
		{
			IO.KeyShift = false;
		}
		if( Event.KeyCode_ == OsEventInputKeyboard::KEYCODE_ALT )
		{
			IO.KeyAlt = false;
		}
		return evtRET_PASS;
	}

	/**
	 * Handle text input..
	 */
	eEvtReturn OnTextInput( EvtID, const EvtBaseEvent& BaseEvent )
	{
		auto Event = BaseEvent.get< OsEventInputText >();
		for( BcU32 Idx = 0; Idx < 32; ++Idx )
		{
			BcU32 Char = Event.Text_[ Idx ];
			if( Char > 0 && Char < 0x10000 )
			{
				TextInput_.push_back( Char );
			}
			else
			{
				break;
			}
		}
		return evtRET_PASS;
	}

	/**
	 * Handle mouse up.
	 */
	eEvtReturn OnMouseDown( EvtID, const EvtBaseEvent& BaseEvent )
	{
		auto Event = BaseEvent.get< OsEventInputMouse >();
		ImGuiIO& IO = ImGui::GetIO();
		if( Event.ButtonCode_ < 2 )
		{
			IO.MouseDown[ Event.ButtonCode_ ] = true;
			IO.MousePos.x = (signed short)Event.MouseX_;
			IO.MousePos.y = (signed short)Event.MouseY_;
		}
		return evtRET_PASS;
	}

	/**
	 * Handle mouse up.
	 */
	eEvtReturn OnMouseUp( EvtID, const EvtBaseEvent& BaseEvent )
	{
		auto Event = BaseEvent.get< OsEventInputMouse >();
		ImGuiIO& IO = ImGui::GetIO();
		if( Event.ButtonCode_ < 2 )
		{
			IO.MouseDown[ Event.ButtonCode_ ] = false;
			IO.MousePos.x = (signed short)Event.MouseX_;
			IO.MousePos.y = (signed short)Event.MouseY_;
		}
		return evtRET_PASS;
	}

	/**
	 * Handle mouse move.
	 */
	eEvtReturn OnMouseMove( EvtID, const EvtBaseEvent& BaseEvent )
	{
		auto Event = BaseEvent.get< OsEventInputMouse >();
		ImGuiIO& IO = ImGui::GetIO();
		IO.MousePos.x = (signed short)Event.MouseX_;
		IO.MousePos.y = (signed short)Event.MouseY_;
		return evtRET_PASS;
	}

	/**
	 * Handle mouse wheel.
	 */
	eEvtReturn OnMouseWheel( EvtID, const EvtBaseEvent& BaseEvent )
	{
		auto Event = BaseEvent.get< OsEventInputMouseWheel >();
		MouseWheel_ += Event.ScrollY_;
		return evtRET_PASS;
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
		PSY_LOGSCOPEDCATEGORY( "ImGui" );
		VertexDeclaration_ = RsCore::pImpl()->createVertexDeclaration(
			RsVertexDeclarationDesc( 3 )
				.addElement( RsVertexElement( 0, (size_t)(&((ImDrawVert*)0)->pos),  2, RsVertexDataType::FLOAT32, RsVertexUsage::POSITION, 0 ) )
				.addElement( RsVertexElement( 0, (size_t)(&((ImDrawVert*)0)->uv), 2, RsVertexDataType::FLOAT32, RsVertexUsage::TEXCOORD, 0 ) )
				.addElement( RsVertexElement( 0, (size_t)(&((ImDrawVert*)0)->col), 4, RsVertexDataType::UBYTE_NORM, RsVertexUsage::COLOUR, 0 ) ),
			"DsImGui" );

		VertexBuffer_ = RsCore::pImpl()->createBuffer( 
			RsBufferDesc( 
				RsBufferType::VERTEX,
				RsResourceCreationFlags::STREAM, 
				65536 * VertexDeclaration_->getDesc().getMinimumStride() ),
			"DsImGui" );

		IndexBuffer_ = RsCore::pImpl()->createBuffer( 
			RsBufferDesc( 
				RsBufferType::INDEX,
				RsResourceCreationFlags::STREAM, 
				128 * 1024 * sizeof( BcU16 ) ),
			"DsImGui" );

		RsGeometryBindingDesc GeometryBindingDesc;
		GeometryBindingDesc.setVertexDeclaration( VertexDeclaration_.get() );
		GeometryBindingDesc.setVertexBuffer( 0, VertexBuffer_.get(), sizeof( ImDrawVert ) );
		GeometryBindingDesc.setIndexBuffer( IndexBuffer_.get() );
		GeometryBinding_ = RsCore::pImpl()->createGeometryBinding( GeometryBindingDesc, "DsImGui" );

		UniformBuffer_ = RsCore::pImpl()->createBuffer(
			RsBufferDesc(
				RsBufferType::UNIFORM,
				RsResourceCreationFlags::STREAM,
				sizeof( UniformBlock_ ) ),
			"DsImGui" );

		auto SamplerStateDesc = RsSamplerStateDesc();
		SamplerStateDesc.AddressU_ = RsTextureSamplingMode::CLAMP;
		SamplerStateDesc.AddressV_ = RsTextureSamplingMode::CLAMP;
		SamplerStateDesc.AddressW_ = RsTextureSamplingMode::CLAMP;
		FontSampler_ = RsCore::pImpl()->createSamplerState( SamplerStateDesc, "DsImGui" );
		unsigned char* Pixels = nullptr;
		int Width, Height;
		ImGuiIO& IO = ImGui::GetIO();
		IO.Fonts->GetTexDataAsRGBA32( &Pixels, &Width, &Height );

		PSY_LOG( "Creating texture %ux%u", Width, Height );
		FontTexture_ = RsCore::pImpl()->createTexture(
			RsTextureDesc( 
				RsTextureType::TEX2D,
				RsResourceCreationFlags::STATIC,
				RsResourceBindFlags::SHADER_RESOURCE,
				RsTextureFormat::R8G8B8A8,
				1,
				Width, 
				Height,
				1 ),
			"DsImGui" );

		RsTextureSlice Slice = { 0, RsTextureFace::NONE };
		RsCore::pImpl()->updateTexture( FontTexture_.get(), Slice, RsResourceUpdateFlags::ASYNC,
			[ Pixels, Width, Height ]( class RsTexture*, const RsTextureLock& Lock )
			{
				const BcU32 SourcePitch = Width * 4;
				for( BcU32 Row = 0; Row < Height; ++Row )
				{
					BcU8* DestData = reinterpret_cast< BcU8* >( Lock.Buffer_ ) + ( Lock.Pitch_ * Row );
					memcpy( DestData, Pixels + ( SourcePitch * Row ), SourcePitch );
				}
			} );

		WhiteTexture_ = RsCore::pImpl()->createTexture(
			RsTextureDesc( 
				RsTextureType::TEX2D,
				RsResourceCreationFlags::STATIC,
				RsResourceBindFlags::SHADER_RESOURCE,
				RsTextureFormat::R8G8B8A8,
				1,
				1, 
				1,
				1 ),
			"DsImGui" );

		RsCore::pImpl()->updateTexture( WhiteTexture_.get(), Slice, RsResourceUpdateFlags::ASYNC,
			[ Pixels ]( class RsTexture* Texture, const RsTextureLock& Lock )
			{
				BcU32* Texel = reinterpret_cast< BcU32* >( Lock.Buffer_ );
				*Texel = 0xffffffff;
			} );

		auto RenderStateDesc = RsRenderStateDesc();
		RenderStateDesc.BlendState_.RenderTarget_[ 0 ].Enable_ = BcTrue;
		RenderStateDesc.BlendState_.RenderTarget_[ 0 ].SrcBlend_ = RsBlendType::SRC_ALPHA;
		RenderStateDesc.BlendState_.RenderTarget_[ 0 ].DestBlend_ = RsBlendType::INV_SRC_ALPHA;
		RenderStateDesc.BlendState_.RenderTarget_[ 0 ].BlendOp_ = RsBlendOp::ADD;
		RenderStateDesc.BlendState_.RenderTarget_[ 0 ].SrcBlendAlpha_ = RsBlendType::SRC_ALPHA;
		RenderStateDesc.BlendState_.RenderTarget_[ 0 ].DestBlendAlpha_ = RsBlendType::ZERO;
		RenderStateDesc.BlendState_.RenderTarget_[ 0 ].BlendOpAlpha_ = RsBlendOp::ADD;
		RenderStateDesc.BlendState_.RenderTarget_[ 0 ].WriteMask_ = 0xf;
		RenderStateDesc.DepthStencilState_.DepthTestEnable_ = BcFalse;
		RenderStateDesc.DepthStencilState_.DepthWriteEnable_ = BcFalse;
		RenderStateDesc.RasteriserState_.ScissorEnable_ = BcTrue;
		RenderStateDesc.RasteriserState_.FillMode_ = RsFillMode::SOLID;
		RenderState_ = RsCore::pImpl()->createRenderState( RenderStateDesc, "DsImGui" );

		IO.Fonts->TexID = FontTexture_.get();

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

		// Register for input.
		OsCore::pImpl()->subscribe( osEVT_INPUT_KEYDOWN, OnKeyDown );
		OsCore::pImpl()->subscribe( osEVT_INPUT_KEYUP, OnKeyUp );
		OsCore::pImpl()->subscribe( osEVT_INPUT_TEXT, OnTextInput );
		OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEDOWN, OnMouseDown );
		OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEUP, OnMouseUp );
		OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEMOVE, OnMouseMove );
		OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEWHEEL, OnMouseWheel );

		// Request imgui packge.
		Package_ = CsCore::pImpl()->requestPackage( "imgui" );
		Package_->acquire();
		CsCore::pImpl()->requestPackageReadyCallback( "imgui", 
			[]( CsPackage* Package, BcU32 )
			{
				BcAssert( Package == Package_ );
				ScnShaderRef Textured;
				CsCore::pImpl()->requestResource( 
					"imgui", "textured_shader", Textured );
				BcAssert( Textured );
				ScnShaderPermutationFlags Permutation = 
					ScnShaderPermutationFlags::RENDER_FORWARD |
					ScnShaderPermutationFlags::PASS_MAIN |
					ScnShaderPermutationFlags::MESH_STATIC_2D |
					ScnShaderPermutationFlags::LIGHTING_NONE;

				Program_ = Textured->getProgram( Permutation );



				BcAssert( Program_ );
			}, 0 );
		return true;
	}

	void WaitFrame()
	{
		PSY_LOGSCOPEDCATEGORY( "ImGui" );
		// Wait till render thread has done the last frame.
		RenderThreadFence_.wait();
	}

	bool NewFrame()
	{
		PSY_LOGSCOPEDCATEGORY( "ImGui" );
		if( Package_ != nullptr )
		{
			WaitFrame();
			ImGuiIO& IO = ImGui::GetIO();

			// Grab client to get current size.
			if( OsCore::pImpl() )
			{
				OsClient* Client = OsCore::pImpl()->getClient( 0 );
				BcAssert( Client );
				IO.DisplaySize = ImVec2( Client->getWidth(), Client->getHeight() );
			}

			// Update mouse wheel.
			IO.MouseWheel = MouseWheel_;
			MouseWheel_ = 0.0f;

			// Update keyboard input.
			for( BcU32 Char : TextInput_ )
			{
				if( Char > 0 && Char < 0x10000 )
				{
					IO.AddInputCharacter( (BcU16)Char );
				}
				else
				{
					break;
				}
			}
			TextInput_.clear();

			// Start the frame
			ImGui::NewFrame();
			return true;
		}
		return false;
	}

	void Render( RsContext* Context, RsFrame* Frame )
	{
		PSY_LOGSCOPEDCATEGORY( "ImGui" );
		if( Program_ != nullptr )
		{
			RenderThreadFence_.wait();
			ScopedDraw ScopedDraw( Context, Frame );

			ImGui::Render();
		}
	}

	void Shutdown()
	{
		PSY_LOGSCOPEDCATEGORY( "ImGui" );
		WaitFrame();
		ImGui::Shutdown();

		// Unregister input.
		OsCore::pImpl()->unsubscribe( osEVT_INPUT_KEYDOWN, OnKeyDown );
		OsCore::pImpl()->unsubscribe( osEVT_INPUT_KEYUP, OnKeyUp );
		OsCore::pImpl()->unsubscribe( osEVT_INPUT_TEXT, OnTextInput );
		OsCore::pImpl()->unsubscribe( osEVT_INPUT_MOUSEDOWN, OnMouseDown );
		OsCore::pImpl()->unsubscribe( osEVT_INPUT_MOUSEUP, OnMouseUp );
		OsCore::pImpl()->unsubscribe( osEVT_INPUT_MOUSEMOVE, OnMouseMove );
		OsCore::pImpl()->unsubscribe( osEVT_INPUT_MOUSEWHEEL, OnMouseWheel );

		Package_->release();
		Package_ = nullptr;
		BcAssert( DrawContext_ == nullptr );
		BcAssert( DrawFrame_ == nullptr );
		GeometryBinding_.reset();
		ProgramBinding_.reset();
		VertexDeclaration_.reset();
		VertexBuffer_.reset();
		IndexBuffer_.reset();
		UniformBuffer_.reset();
		RenderState_.reset();
		FontSampler_.reset();
		FontTexture_.reset();
		WhiteTexture_.reset();
	}
} // end Psybrus
} // end ImGui

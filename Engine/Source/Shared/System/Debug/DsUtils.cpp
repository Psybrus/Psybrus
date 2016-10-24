#include "System/Debug/DsUtils.h"

// TODO: Use RsProgram directly.
#include "System/Scene/Rendering/ScnShader.h"

#include "System/Content/CsCore.h"

#include "System/Renderer/RsCore.h"
#include "System/Renderer/RsContext.h"
#include "System/Renderer/RsFrame.h"
#include "System/Renderer/RsFrameBuffer.h"
#include "System/Renderer/RsViewport.h"

#include "Base/BcProfiler.h"

//////////////////////////////////////////////////////////////////////////
// Debug utilities.
#if !PSY_PRODUCTION
namespace Debug
{
	struct TextSection
	{
		MaVec3d WorldPosition_;
		RsColour Colour_;
		std::string Text_;
	};

	struct PrimitiveSection
	{
		RsTopologyType Type_;
		BcU32 VertexIndex_;
		BcU32 NoofVertices_;
		BcU32 Layer_;
		BcU32 CategoryMask_;
	};

	class PrimitiveSectionCompare
	{
	public:
		bool operator()( const PrimitiveSection& A, const PrimitiveSection& B )
		{
			return A.Layer_ < B.Layer_;
		}
	};

	RsVertexDeclarationUPtr VertexDeclaration_;
	RsBufferUPtr UniformBuffer_;

	// Submission data.
	std::vector< TextSection > TextSections_;
	Vertex* pWorkingVertices_ = nullptr;
	Vertex* pVertices_ = nullptr;
	Vertex* pVerticesEnd_ = nullptr;
	BcSize NoofVertices_ = 0;
	BcSize VertexIndex_ = 0;
	SysFence UploadFence_;
	
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

	std::vector< PrimitiveSection > PrimitiveSectionList_;
	size_t LastPrimitiveSection_;

	BcU32 DrawCategoryMask_ = 0;
	BcU32 CurrCategoryMask_ = 0;
	BcU32 CurrCategoryMaskAlloc_ = 1;
	std::unordered_map< std::string, BcU32 > MaskNameMap_;  

	BcU32 GetDrawCategoryMask()
	{
		return DrawCategoryMask_;
	}

	void SetDrawCategoryMask( BcU32 CategoryMask )
	{
		DrawCategoryMask_ = CategoryMask;
	}
	
	BcU32 GetCurrentDrawCategoryMask()
	{ 
		return CurrCategoryMask_;
	}


	void SetCurrentDrawCategoryMask( BcU32 Mask )
	{
		CurrCategoryMask_ = Mask;
	}

	BcU32 GetDrawCategoryMask( const char* Name )
	{
		if( MaskNameMap_.find( Name ) == MaskNameMap_.end() )
		{
			MaskNameMap_[ Name ] = CurrCategoryMaskAlloc_;
			CurrCategoryMaskAlloc_ <<= 1;
		}
		return MaskNameMap_[ Name ];
	}

	size_t GetDrawCategories( const char** OutCategoryNames, BcU32* OutCategoryMasks, size_t MaxCategories )
	{
		size_t RetVal = 0;
		for( const auto& Pair : MaskNameMap_ )
		{
			if( RetVal >= MaxCategories )
			{
				break;
			}

			if( Pair.second != 0 )
			{
				OutCategoryNames[ RetVal ] = Pair.first.c_str();
				OutCategoryMasks[ RetVal ] = Pair.second;
				++RetVal;
			}
		}

		return RetVal;
	}

	DrawCategory::DrawCategory( const char* Name )
	{
		OldMask_ = GetCurrentDrawCategoryMask();
		SetCurrentDrawCategoryMask( GetDrawCategoryMask( Name ) );
	}

	DrawCategory::DrawCategory( BcU32 Category )
	{
		OldMask_ = GetCurrentDrawCategoryMask();
		SetCurrentDrawCategoryMask( Category );
	}

	DrawCategory::~DrawCategory()
	{
		SetCurrentDrawCategoryMask( OldMask_ );
	}

	void Init( size_t NoofVertices )
	{
		//
		CurrCategoryMask_ = GetDrawCategoryMask( "Default" );
		DrawCategoryMask_ = CurrCategoryMask_;

		// Allocate working vertices.
		NoofVertices_ = NoofVertices;
		pWorkingVertices_ = new Vertex[ NoofVertices_ ];

		// Allocate our own vertex buffer data.
		VertexDeclaration_ = RsCore::pImpl()->createVertexDeclaration( 
			RsVertexDeclarationDesc( 2 )
				.addElement( RsVertexElement( 0, 0,				4,		RsVertexDataType::FLOAT32,		RsVertexUsage::POSITION,		0 ) )
				.addElement( RsVertexElement( 0, 16,			4,		RsVertexDataType::UBYTE_NORM,	RsVertexUsage::COLOUR,			0 ) ),
			"Debug" );
	
		// Allocate uniform buffer object.
		UniformBuffer_ = RsCore::pImpl()->createBuffer( 
			RsBufferDesc( 
				RsBindFlags::UNIFORM_BUFFER,
				RsResourceCreationFlags::STREAM,
				sizeof( ScnShaderObjectUniformBlockData ) ),
			"Debug" );

		auto RenderStateDesc = RsRenderStateDesc();
		RenderStateDesc.BlendState_.RenderTarget_[ 0 ].Enable_ = BcTrue;
		RenderStateDesc.BlendState_.RenderTarget_[ 0 ].SrcBlend_ = RsBlendType::SRC_ALPHA;
		RenderStateDesc.BlendState_.RenderTarget_[ 0 ].DestBlend_ = RsBlendType::INV_SRC_ALPHA;
		RenderStateDesc.BlendState_.RenderTarget_[ 0 ].BlendOp_ = RsBlendOp::ADD;
		RenderStateDesc.BlendState_.RenderTarget_[ 0 ].SrcBlendAlpha_ = RsBlendType::SRC_ALPHA;
		RenderStateDesc.BlendState_.RenderTarget_[ 0 ].DestBlendAlpha_ = RsBlendType::ZERO;
		RenderStateDesc.BlendState_.RenderTarget_[ 0 ].BlendOpAlpha_ = RsBlendOp::ADD;
		RenderStateDesc.BlendState_.RenderTarget_[ 0 ].WriteMask_ = 0xf;
		RenderStateDesc.DepthStencilState_.DepthTestEnable_ = BcTrue;
		RenderStateDesc.DepthStencilState_.DepthWriteEnable_ = BcFalse;
		RenderStateDesc.RasteriserState_.ScissorEnable_ = BcTrue;
		RenderStateDesc.RasteriserState_.FillMode_ = RsFillMode::SOLID;
		RenderStateDesc.RasteriserState_.AntialiasedLineEnable_ = BcTrue;
		RenderState_ = RsCore::pImpl()->createRenderState( RenderStateDesc, "DsImGui" );

		// Request imgui packge.
		// TODO: Have one big debug package.
		Package_ = CsCore::pImpl()->requestPackage( "imgui" );
		Package_->acquire();
		CsCore::pImpl()->requestPackageReadyCallback( "imgui", 
			[]( CsPackage* Package, BcU32 )
			{
				BcAssert( Package == Package_ );
				ScnShaderRef Shader;
				CsCore::pImpl()->requestResource( 
					"imgui", "default_shader", Shader );
				BcAssert( Shader );
				ScnShaderPermutationFlags Permutation = 
					ScnShaderPermutationFlags::RENDER_FORWARD |
					ScnShaderPermutationFlags::PASS_MAIN |
					ScnShaderPermutationFlags::MESH_STATIC_3D;

				Program_ = Shader->getProgram( Permutation );
				BcAssert( Program_ );
			}, 0 );
	}

	void Shutdown()
	{
		delete [] pWorkingVertices_;
		pWorkingVertices_ = nullptr;

		VertexDeclaration_.reset();
		UniformBuffer_.reset();
		ProgramBinding_.reset();
		RenderState_.reset();
		Package_->release();
		Package_ = nullptr;
	}

	void DrawViewOverlay( ImDrawList* DrawList, const MaMat4d& View, const MaMat4d& Projection, const RsViewport& Viewport, RsContext* Context )
	{
		MaMat4d ClipTransform = View * Projection;

		auto getScreenPos = [ & ]( MaVec3d WorldPos )
		{
			MaVec4d ScreenSpace = MaVec4d( WorldPos, 1.0f ) * ClipTransform;
			MaVec2d ScreenPosition = MaVec2d( ScreenSpace.x() / ScreenSpace.w(), -ScreenSpace.y() / ScreenSpace.w() );

			BcF32 HalfW = BcF32( Viewport.width() ) * 0.5f;
			BcF32 HalfH = BcF32( Viewport.height() ) * 0.5f;
			return MaVec2d( ( ScreenPosition.x() * HalfW ), ( ScreenPosition.y() * HalfH ) ) + MaVec2d( HalfW, HalfH );
		};

		for( const auto& TextSection: TextSections_ )
		{
			MaVec2d WorldPos = getScreenPos( TextSection.WorldPosition_ );
			ImGui::AddShadowedText( DrawList, WorldPos, TextSection.Colour_.asRGBA(), "%s", TextSection.Text_.c_str() );
		}
	}


	void Render( RsFrame* Frame, RsFrameBuffer* FrameBuffer, const RsViewport& Viewport, RsBuffer* ViewUniformBuffer, RsRenderSort Sort )
	{
		if( Program_ == nullptr )
		{
			return;
		}

		if( ProgramBinding_ == nullptr )
		{
			ProgramBindingDesc_.setUniformBuffer( Program_->findUniformBufferSlot( "ScnShaderViewUniformBlockData" ), ViewUniformBuffer, 0, sizeof( ScnShaderViewUniformBlockData ) );
			ProgramBinding_ = RsCore::pImpl()->createProgramBinding( Program_, ProgramBindingDesc_, "Debug" );
		}

		// Upload.
		BcSize VertexDataSize = VertexIndex_ * sizeof( Vertex );
		RsBufferAlloc VBAlloc = RsCore::pImpl()->allocTransientBuffer( RsBindFlags::VERTEX_BUFFER, (BcU32)VertexDataSize );
		if( VertexDataSize > 0 )
		{
			UploadFence_.increment();
			RsCore::pImpl()->updateBuffer( 
				VBAlloc.Buffer_, VBAlloc.Offset_, VBAlloc.Size_, 
				RsResourceUpdateFlags::ASYNC,
				[ VertexDataSize ]
				( RsBuffer* Buffer, const RsBufferLock& BufferLock )
				{
					BcAssert( VertexDataSize <= Buffer->getDesc().SizeBytes_ );
					BcMemCopy( BufferLock.Buffer_, pWorkingVertices_, 
						VertexDataSize );
					UploadFence_.decrement();
				} );
		}

		// HUD pass.
		Sort.Layer_ = 0;
		Sort.Pass_ = RS_SORT_PASS_MAX;

		// NOTE: Could do this sort inside of the renderer, but I'm just gonna keep the canvas
		//       as one solid object as to not conflict with other canvas objects when rendered
		//       to the scene. Will not sort by transparency or anything either.
		//std::stable_sort( PrimitiveSectionList_.begin(), PrimitiveSectionList_.end(), ScnCanvasComponentPrimitiveSectionCompare() );
	
		for( BcU32 Idx = 0; Idx < PrimitiveSectionList_.size(); ++Idx )
		{
			// Copy primitive sections in.
			auto* PrimSection = Frame->alloc< PrimitiveSection >( 1 );
			BcMemZero( PrimSection, sizeof( PrimitiveSection ) );
			*PrimSection = PrimitiveSectionList_[ Idx ];
		
			// Allocate geometry binding.
			RsGeometryBindingDesc GeometryBindingDesc;
			GeometryBindingDesc.setVertexDeclaration( VertexDeclaration_.get() );
			GeometryBindingDesc.setVertexBuffer( 0, VBAlloc.Buffer_, sizeof( Vertex ), VBAlloc.Offset_ );
			auto GeometryBinding = RsCore::pImpl()->createGeometryBinding( GeometryBindingDesc, "Debug" );

			// Add to frame.
			UploadFence_.increment();
			Frame->queueRenderNode( Sort,
				[
					GeometryBinding = GeometryBinding.get(),
					ProgramBinding = ProgramBinding_.get(),
					RenderState = RenderState_.get(),
					FrameBuffer = FrameBuffer,
					Viewport = Viewport,
					PrimSection 
				]
				( RsContext* Context )
				{
					if( PrimSection->Type_ != RsTopologyType::INVALID )
					{
						Context->drawPrimitives( 
							GeometryBinding,
							ProgramBinding,
							RenderState,
							FrameBuffer,
							&Viewport,
							nullptr,
							PrimSection->Type_, PrimSection->VertexIndex_, PrimSection->NoofVertices_,
							0, 1 );
					}

					PrimSection->~PrimitiveSection();
					UploadFence_.decrement();
				} );
		}
	
		// Reset vertices.
		pVertices_ = pVerticesEnd_ = nullptr;

		// Reset program binding for next frame update.
		ProgramBinding_.reset();
	}

	void NextFrame()
	{
		PSY_PROFILE_FUNCTION;
		// Wait for vertex buffer to finish uploading.
		UploadFence_.wait();

		// Set vertices up.
		pVertices_ = pVerticesEnd_ = pWorkingVertices_;
		pVerticesEnd_ += NoofVertices_;
		VertexIndex_ = 0;
	
		// Empty primitive sections.
		PrimitiveSectionList_.clear();
	
		// Clear last primitive.
		LastPrimitiveSection_ = BcErrorCode;

		TextSections_.clear();
	}


	Vertex* AllocVertices( size_t NoofVertices )
	{
		BcAssertMsg( pVertices_ != nullptr, "Debug: Don't have a working buffer." );
		Vertex* pCurrVertex = nullptr;
		if( ( VertexIndex_ + NoofVertices ) <= NoofVertices_ )
		{
			pCurrVertex = &pVertices_[ VertexIndex_ ];
			VertexIndex_ += NoofVertices;
		}
		return pCurrVertex;
	}


	bool CanDraw( const char* CategoryName )
	{
		if( CategoryName != nullptr )
		{
			return CanDraw( GetDrawCategoryMask( CategoryName ) );
		}
		return true;
	}

	
	bool CanDraw( BcU32 CategoryMask )
	{
		return !!( CategoryMask & GetDrawCategoryMask() );
	}


	BcU32 ConvertVertexPointerToIndex( Vertex* pVertex )
	{
		// NOTE: Will probably warn due to converting a 64-bit pointer to 32-bit value, but
		//       it's actually ok because we should never have over 4GB worth of vertices!
		BcU32 ByteOffset = BcU32( ( (BcU8*)pVertex - (BcU8*)pVertices_ ) & 0xffffffff );
		return ByteOffset / sizeof( Vertex );
	}


	void AddPrimitive( RsTopologyType Type, Vertex* pVertices, BcU32 NoofVertices, BcU32 Layer )
	{
		// Check if the vertices are owned by us, if not copy in.
		if( pVertices < pVertices_ || pVertices_ >= pVerticesEnd_ )
		{
			Vertex* pNewVertices = AllocVertices( NoofVertices );
			if( pNewVertices != NULL )
			{
				BcMemCopy( pNewVertices, pVertices, sizeof( Vertex ) * NoofVertices );
				pVertices = pNewVertices;
			}
		}
	
		// TODO: If there was a previous primitive which we can marge into, attempt to.
		BcU32 VertexIndex = ConvertVertexPointerToIndex( pVertices );
		PrimitiveSection PrimitiveSection = 
		{
			Type,
			VertexIndex,
			NoofVertices,
			Layer,
			CurrCategoryMask_,
		};
	
		PrimitiveSectionList_.push_back( PrimitiveSection );
		LastPrimitiveSection_ = (BcU32)PrimitiveSectionList_.size() - 1;
	}


	void DrawLine( const MaVec3d& PointA, const MaVec3d& PointB, const RsColour& Colour, BcU32 Layer )
	{
		if( !CanDraw( CurrCategoryMask_ ) )
		{
			return;
		}

		Vertex* pVertices = AllocVertices( 2 );
		Vertex* pFirstVertex = pVertices;
	
		// Only draw if we can allocate vertices.
		if( pVertices != NULL )
		{
			// Now copy in data.
			BcU32 RGBA = Colour.asRGBA();
		
			pVertices->X_ = PointA.x();
			pVertices->Y_ = PointA.y();
			pVertices->Z_ = PointA.z();
			pVertices->W_ = 1.0f;
			pVertices->RGBA_ = RGBA;
			++pVertices;
			pVertices->X_ = PointB.x();
			pVertices->Y_ = PointB.y();
			pVertices->Z_ = PointB.z();
			pVertices->W_ = 1.0f;
			pVertices->RGBA_ = RGBA;

			// Quickly check last primitive.
			BcBool AddNewPrimitive = BcTrue;
			if( LastPrimitiveSection_ != BcErrorCode )
			{
				PrimitiveSection& PrimitiveSection = PrimitiveSectionList_[ LastPrimitiveSection_ ];

				// If the last primitive was the same type as ours we can append to it.
				// NOTE: Need more checks here later.
				if( PrimitiveSection.Type_ == RsTopologyType::LINE_LIST &&
					PrimitiveSection.Layer_ == Layer &&
					PrimitiveSection.CategoryMask_ == CurrCategoryMask_ )
				{
					PrimitiveSection.NoofVertices_ += 2;
				
					AddNewPrimitive = BcFalse;
				}
			}
		
			// Add primitive.
			if( AddNewPrimitive == BcTrue )
			{
				AddPrimitive( RsTopologyType::LINE_LIST, pFirstVertex, 2, Layer );
			}
		}
	}

	void DrawLines( const MaVec3d* pPoints, BcU32 NoofLines, const RsColour& Colour, BcU32 Layer )
	{
		if( !CanDraw( CurrCategoryMask_ ) )
		{
			return;
		}

		BcU32 NoofVertices = 2 * NoofLines;
		Vertex* pVertices = AllocVertices( NoofVertices );
		Vertex* pFirstVertex = pVertices;

		// Only draw if we can allocate vertices.
		if( pVertices != NULL )
		{	
			// Now copy in data.
			BcU32 RGBA = Colour.asRGBA();

			for( BcU32 Idx = 0; Idx < NoofVertices; ++Idx )
			{
				pVertices->X_ = pPoints[ Idx ].x();
				pVertices->Y_ = pPoints[ Idx ].y();
				pVertices->Z_ = pPoints[ Idx ].z();
				pVertices->W_ = 1.0f;
				pVertices->RGBA_ = RGBA;
				++pVertices;
			}
		
			// Add primitive.		
			AddPrimitive( RsTopologyType::LINE_STRIP, pFirstVertex, NoofVertices, Layer );
		}
	}

	void DrawMatrix( const MaMat4d& Matrix, const RsColour& Colour, BcU32 Layer )
	{
		if( !CanDraw( CurrCategoryMask_ ) )
		{
			return;
		}

		MaVec3d Centre( Matrix.translation() );	
		MaVec3d X( Matrix.row0().x(), Matrix.row0().y(), Matrix.row0().z() );	
		MaVec3d Y( Matrix.row1().x(), Matrix.row1().y(), Matrix.row1().z() );	
		MaVec3d Z( Matrix.row2().x(), Matrix.row2().y(), Matrix.row2().z() );	

		DrawLine( Centre, Centre + X, RsColour::RED * Colour, Layer );
		DrawLine( Centre, Centre + Y, RsColour::GREEN * Colour, Layer );
		DrawLine( Centre, Centre + Z, RsColour::BLUE * Colour, Layer );
	}

	void DrawGrid( const MaVec3d& Position, const MaVec3d& Size, BcF32 StepSize, BcF32 SubDivideMultiple, BcU32 Layer )
	{
		if( !CanDraw( CurrCategoryMask_ ) )
		{
			return;
		}

		BcU32 NoofAxis = ( Size.x() > 0 ? 1 : 0  ) + ( Size.y() > 0 ? 1 : 0  ) + ( Size.z() > 0 ? 1 : 0 );
		BcAssertMsg( NoofAxis == 2, "Only supports 2 axis in the grid!" );
		BcAssert( SubDivideMultiple > 1.0f );
		BcUnusedVar( NoofAxis );
	
	
		// Determine which axis to draw along.
		MaVec3d XAxis;
		MaVec3d YAxis;
		BcF32 XSize = 0.0f;
		BcF32 YSize = 0.0f;

		if( Size.x() > 0.0f )
		{
			XAxis = MaVec3d( Size.x(), 0.0f, 0.0f );
			XSize = Size.x();
			if( Size.y() > 0.0f )
			{
				YAxis = MaVec3d( 0.0f, Size.y(), 0.0f );
				YSize = Size.y();
			}
			else
			{
				YAxis = MaVec3d( 0.0f, 0.0f, Size.z() );
				YSize = Size.z();
			}
		}
		else
		{
			XAxis = MaVec3d( 0.0f, Size.y(), 0.0f );
			YAxis = MaVec3d( 0.0f, 0.0f, Size.z() );
			XSize = Size.y();
			YSize = Size.z();
		}

		// Normalise.
		XAxis.normalise();
		YAxis.normalise();

		while( StepSize < XSize && StepSize < YSize )
		{
			// Draw grid.
			for( BcF32 X = 0.0f; X <= XSize; X += StepSize )
			{
				MaVec3d A1( Position + ( XAxis * X ) + ( YAxis *  YSize ) );
				MaVec3d B1( Position + ( XAxis * X ) + ( YAxis * -YSize ) );
				MaVec3d A2( Position + ( XAxis * -X ) + ( YAxis *  YSize ) );
				MaVec3d B2( Position + ( XAxis * -X ) + ( YAxis * -YSize ) );
				DrawLine( A1, B1, RsColour( 1.0f, 1.0f, 1.0f, 0.05f ), Layer );
				DrawLine( A2, B2, RsColour( 1.0f, 1.0f, 1.0f, 0.05f ), Layer );
			}

			for( BcF32 Y = 0.0f; Y <= YSize; Y += StepSize )
			{
				MaVec3d A1( Position + ( XAxis *  YSize ) + ( YAxis * Y ) );
				MaVec3d B1( Position + ( XAxis * -YSize ) + ( YAxis * Y ) );
				MaVec3d A2( Position + ( XAxis *  YSize ) + ( YAxis * -Y ) );
				MaVec3d B2( Position + ( XAxis * -YSize ) + ( YAxis * -Y ) );
				DrawLine( A1, B1, RsColour( 1.0f, 1.0f, 1.0f, 0.05f ), Layer );
				DrawLine( A2, B2, RsColour( 1.0f, 1.0f, 1.0f, 0.05f ), Layer );
			}

			StepSize *= SubDivideMultiple;
		}
	}

	void DrawEllipsoid( const MaVec3d& Position, const MaVec3d& Size, const RsColour& Colour, BcU32 Layer )
	{
		if( !CanDraw( CurrCategoryMask_ ) )
		{
			return;
		}

		// Draw outer circles for all axis.
		BcU32 LOD = 16;
		BcF32 Angle = 0.0f;
		BcF32 AngleInc = ( BcPI * 2.0f ) / BcF32( LOD );

		// Draw axis lines.
		for( BcU32 i = 0; i < LOD; ++i )
		{
			MaVec2d PosA( BcCos( Angle ), -BcSin( Angle ) );
			MaVec2d PosB( BcCos( Angle + AngleInc ), -BcSin( Angle + AngleInc ) );

			MaVec3d XAxisA = MaVec3d( 0.0f,                 PosA.x() * Size.y(), PosA.y() * Size.z() );
			MaVec3d YAxisA = MaVec3d( PosA.x() * Size.x(), 0.0f,                 PosA.y() * Size.z() );
			MaVec3d ZAxisA = MaVec3d( PosA.x() * Size.x(), PosA.y() * Size.y(), 0.0f                 );
			MaVec3d XAxisB = MaVec3d( 0.0f,                 PosB.x() * Size.y(), PosB.y() * Size.z() );
			MaVec3d YAxisB = MaVec3d( PosB.x() * Size.x(), 0.0f,                 PosB.y() * Size.z() );
			MaVec3d ZAxisB = MaVec3d( PosB.x() * Size.x(), PosB.y() * Size.y(), 0.0f                 );

			DrawLine( XAxisA + Position, XAxisB + Position, Colour, 0 );
			DrawLine( YAxisA + Position, YAxisB + Position, Colour, 0 );
			DrawLine( ZAxisA + Position, ZAxisB + Position, Colour, 0 );

			Angle += AngleInc;
		}

		// Draw a cross down centre.
		MaVec3d XAxis = MaVec3d( Size.x(), 0.0f, 0.0f );
		MaVec3d YAxis = MaVec3d( 0.0f, Size.y(), 0.0f );
		MaVec3d ZAxis = MaVec3d( 0.0f, 0.0f, Size.z() );
		DrawLine( Position - XAxis, Position + XAxis, Colour, Layer );
		DrawLine( Position - YAxis, Position + YAxis, Colour, Layer );
		DrawLine( Position - ZAxis, Position + ZAxis, Colour, Layer );
	}

	void DrawCircle( const MaVec3d& Position, const MaVec3d& Size, const RsColour& Colour, BcU32 Layer )
	{
		if( !CanDraw( CurrCategoryMask_ ) )
		{
			return;
		}

		// Draw outer circles for all axis.
		BcU32 LOD = 12;
		BcF32 Angle = 0.0f;
		BcF32 AngleInc = ( BcPI * 2.0f ) / BcF32( LOD );

		// Draw axis lines.
		for( BcU32 i = 0; i < LOD; ++i )
		{
			MaVec2d PosA( BcCos( Angle ), -BcSin( Angle ) );
			MaVec2d PosB( BcCos( Angle + AngleInc ), -BcSin( Angle + AngleInc ) );

			//MaVec3d XAxisA = MaVec3d( 0.0f,                 PosA.x() * Size.y(), PosA.y() * Size.z() );
			MaVec3d YAxisA = MaVec3d( PosA.x() * Size.x(), 0.0f,                 PosA.y() * Size.z() );
			//MaVec3d ZAxisA = MaVec3d( PosA.x() * Size.x(), PosA.y() * Size.y(), 0.0f                 );
			//MaVec3d XAxisB = MaVec3d( 0.0f,                 PosB.x() * Size.y(), PosB.y() * Size.z() );
			MaVec3d YAxisB = MaVec3d( PosB.x() * Size.x(), 0.0f,                 PosB.y() * Size.z() );
			//MaVec3d ZAxisB = MaVec3d( PosB.x() * Size.x(), PosB.y() * Size.y(), 0.0f                 );

			//DrawLine( XAxisA + Position, XAxisB + Position, Colour, 0 );
			DrawLine( YAxisA + Position, YAxisB + Position, Colour, 0 );
			//DrawLine( ZAxisA + Position, ZAxisB + Position, Colour, 0 );

			Angle += AngleInc;
		}

		// Draw a cross down centre.
		MaVec3d XAxis = MaVec3d( Size.x(), 0.0f, 0.0f );
		MaVec3d YAxis = MaVec3d( 0.0f, Size.y(), 0.0f );
		MaVec3d ZAxis = MaVec3d( 0.0f, 0.0f, Size.z() );
		DrawLine( Position - XAxis, Position + XAxis, Colour, Layer );
		DrawLine( Position - YAxis, Position + YAxis, Colour, Layer );
		DrawLine( Position - ZAxis, Position + ZAxis, Colour, Layer );
	}

	void DrawAABB( const MaAABB& AABB, const RsColour& Colour, BcU32 Layer )
	{
		if( !CanDraw( CurrCategoryMask_ ) )
		{
			return;
		}

		DrawLine( AABB.corner( 0 ), AABB.corner( 1 ), Colour, Layer );
		DrawLine( AABB.corner( 1 ), AABB.corner( 3 ), Colour, Layer );
		DrawLine( AABB.corner( 2 ), AABB.corner( 0 ), Colour, Layer );
		DrawLine( AABB.corner( 3 ), AABB.corner( 2 ), Colour, Layer );

		DrawLine( AABB.corner( 4 ), AABB.corner( 5 ), Colour, Layer );
		DrawLine( AABB.corner( 5 ), AABB.corner( 7 ), Colour, Layer );
		DrawLine( AABB.corner( 6 ), AABB.corner( 4 ), Colour, Layer );
		DrawLine( AABB.corner( 7 ), AABB.corner( 6 ), Colour, Layer );

		DrawLine( AABB.corner( 0 ), AABB.corner( 4 ), Colour, Layer );
		DrawLine( AABB.corner( 1 ), AABB.corner( 5 ), Colour, Layer );
		DrawLine( AABB.corner( 2 ), AABB.corner( 6 ), Colour, Layer );
		DrawLine( AABB.corner( 3 ), AABB.corner( 7 ), Colour, Layer );
	}

	void DrawShadowedText( const MaVec3d& WorldPosition, const RsColour& Colour, const char* Text, ... )
	{
		if( !CanDraw( CurrCategoryMask_ ) )
		{
			return;
		}

		std::array< char, 1024 > TextBuffer;
		va_list Args;
		va_start( Args, Text );
		BcVSPrintf( TextBuffer.data(), TextBuffer.size(), Text, Args );
		va_end( Args );

		TextSection TextSection;
		TextSection.WorldPosition_ = WorldPosition;
		TextSection.Colour_ = Colour;
		TextSection.Text_ = TextBuffer.data();
		TextSections_.push_back( TextSection );
	}


}
#endif // !PSY_PRODUCTION
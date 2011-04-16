/**************************************************************************
*
* File:		RsStateBlockGL.cpp
* Author:	Neil Richardson 
* Ver/Date:	16/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "RsStateBlockGL.h"

#include "RsGL.h"

//////////////////////////////////////////////////////////////////////////
// State value translation.
static GLenum gCompareMode[] = 
{
	GL_NEVER,
	GL_LESS,
	GL_EQUAL,
	GL_LEQUAL,
	GL_GREATER,
	GL_NOTEQUAL,
	GL_GEQUAL,
	GL_ALWAYS
};

static GLenum gTextureFiltering[] = 
{
	// No mipmapping.
	GL_NEAREST,
	GL_LINEAR,
	
	// Mipmapping nearest
	GL_NEAREST_MIPMAP_NEAREST,
	GL_LINEAR_MIPMAP_NEAREST,
	
	// Mipmapping linear
	GL_NEAREST_MIPMAP_LINEAR,
	GL_LINEAR_MIPMAP_LINEAR
};

static GLenum gTextureSampling[] = 
{
	GL_REPEAT,
	GL_MIRRORED_REPEAT,
	GL_CLAMP_TO_EDGE,
	GL_DECAL
};

//////////////////////////////////////////////////////////////////////////
// Ctor
RsStateBlockGL::RsStateBlockGL()
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsStateBlockGL::~RsStateBlockGL()
{

}

//////////////////////////////////////////////////////////////////////////
// bind
//virtual
void RsStateBlockGL::bind()
{
	// TODO: Compare against previous state block.
	
	
	// Bind render states.
	for( BcU32 RenderStateIdx = 0; RenderStateIdx < NoofRenderStateBinds_; ++RenderStateIdx )
	{
		BcU32 RenderStateID = RenderStateBinds_[ RenderStateIdx ];
		TRenderStateValue& RenderStateValue = RenderStateValues_[ RenderStateID ];
		
		// Some states are dependant, and set in batches.
		if( RenderStateValue.Dirty_ )
		{
			const BcS32 Value = RenderStateValue.Value_;
		
			switch( RenderStateID )
			{
				case rsRS_DEPTH_WRITE_ENABLE:
					glDepthMask( Value );
					break;
				case rsRS_DEPTH_TEST_ENABLE:
					Value ? glEnable( GL_DEPTH_TEST ) : glDisable( GL_DEPTH_TEST );
					break;
				case rsRS_DEPTH_TEST_COMPARE:
					glDepthFunc( gCompareMode[ Value ] );
					break;
				case rsRS_DEPTH_BIAS:
					// NOT IMPLEMENTED YET.
					break;
				case rsRS_ALPHA_TEST_ENABLE:
					Value ? glEnable( GL_ALPHA_TEST ) : glDisable( GL_ALPHA_TEST );
					break;
				case rsRS_ALPHA_TEST_COMPARE:
					bindAlphaFunc();
					break;
				case rsRS_ALPHA_TEST_THRESHOLD:
					bindAlphaFunc();
					break;
				case rsRS_BLEND_MODE:
					bindBlendMode( (eRsBlendingMode)Value );
					break;
			}
			
			// No longer dirty.
			RenderStateValue.Dirty_ = BcFalse;
		}
	}
	
	// Bind texture states.
	for( BcU32 TextureStateIdx = 0; TextureStateIdx < NoofTextureStateBinds_; ++TextureStateIdx )
	{
		BcU32 TextureStateID = TextureStateBinds_[ TextureStateIdx ];
		TTextureStateValue& TextureStateValue = TextureStateValues_[ TextureStateID ];

		if( TextureStateValue.Dirty_ )
		{
			RsTexture* pTexture = TextureStateValue.pTexture_;
			const RsTextureParams& Params = TextureStateValue.Params_;

			// TODO: Support texture types (1D, 2D, 3D, Cube)
			glActiveTexture( GL_TEXTURE0 + TextureStateID );
			glEnable( GL_TEXTURE_2D );
			glBindTexture( GL_TEXTURE_2D, pTexture ? pTexture->getHandle< GLuint >() : 0 );
			
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gTextureFiltering[ Params.MinFilter_ ] );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gTextureFiltering[ Params.MagFilter_ ] );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gTextureSampling[ Params.UMode_ ] );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gTextureSampling[ Params.VMode_ ] );	
		
			TextureStateValue.Dirty_ = BcFalse;
		}
	}
	
	// Reset binds.
	NoofRenderStateBinds_ = 0;
	NoofTextureStateBinds_ = 0;
}

//////////////////////////////////////////////////////////////////////////
// bindAlphaFunc
void RsStateBlockGL::bindAlphaFunc()
{
	TRenderStateValue& CompareValue = RenderStateValues_[ rsRS_ALPHA_TEST_COMPARE ];
	TRenderStateValue& ThresholdValue = RenderStateValues_[ rsRS_ALPHA_TEST_THRESHOLD ];

	glAlphaFunc( gCompareMode[ CompareValue.Value_ ], ThresholdValue.Value_ / 255.0f );

	CompareValue.Dirty_ = BcFalse;
	ThresholdValue.Dirty_ = BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// bindBlendMode
void RsStateBlockGL::bindBlendMode( eRsBlendingMode BlendMode )
{
	switch( BlendMode )
	{
		case rsBM_NONE:
			glDisable( GL_BLEND );
			glBlendFunc( GL_ONE, GL_ZERO );
			break;
					
		case rsBM_BLEND:
			glEnable( GL_BLEND );
			glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
			break;
					
		case rsBM_ADD:
			glEnable( GL_BLEND );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE );
			break;

		case rsBM_SUBTRACT:
			glEnable( GL_BLEND );
			glBlendEquation( GL_FUNC_SUBTRACT );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE );
			break;
			
		default:
			break;
	}
}


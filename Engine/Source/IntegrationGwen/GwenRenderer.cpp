/**************************************************************************
*
* File:		GwenRenderer.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Based on the Gwen OpenGL Renderer.
*		
*
*
* 
**************************************************************************/

#include "GwenRenderer.h"

#include "CsCore.h"

GwenRenderer::GwenRenderer( ScnEntityRef Entity, ScnGwenCanvasComponentRef ParentComponent ):
	Entity_( Entity ),
	ParentComponent_( ParentComponent ),
	Colour_( 1.0f, 1.0f, 1.0f, 1.0f )
{

}

GwenRenderer::~GwenRenderer()
{
	Entity_->detach( SolidMaterialComponent_ );
}

void GwenRenderer::Init()
{
	// Find components we depend upon.
	BcU32 Components = Entity_->getNoofComponents();
	for( BcU32 Idx = 0; Idx < Components; ++Idx )
	{
		ScnCanvasComponentRef CanvasComponent( Entity_->getComponent( Idx ) );
		
		if( CanvasComponent.isValid() )
		{
			Canvas_ = CanvasComponent;
		}
	}

	ParentComponent_->createMaterialComponentByAlias( "solid", SolidMaterialComponent_ );
	Entity_->attach( SolidMaterialComponent_ );
}

void GwenRenderer::Begin()
{
	Canvas_->clear();

	BcMat4d Projection;
	Projection.orthoProjection( 0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f );
	Canvas_->pushMatrix( Projection );
}

void GwenRenderer::End()
{
	Canvas_->popMatrix();
}

void GwenRenderer::DrawFilledRect( Gwen::Rect rect )
{
	Translate( rect );

	BcVec2d Size( BcVec2d( rect.w, rect.h ) );
	BcVec2d CornerMin( BcVec2d( rect.x, rect.y ) );
	BcVec2d CornerMax( CornerMin + Size );

	if ( ClippingEnabled_ )
	{
		if ( ( CornerMax.x() < ClipMin_.x() ) || ( CornerMin.x() > ClipMax_.x() ) || ( CornerMax.y() < ClipMin_.y() ) || ( CornerMin.y() > ClipMax_.y() ) )
		{
			return;
		}

		if ( CornerMin.x() < ClipMin_.x() )
		{
			CornerMin.x( ClipMin_.x() );
		}

		if ( CornerMax.x() > ClipMax_.x() )
		{
			CornerMax.x( ClipMax_.x() );
		}

		if ( CornerMin.y() < ClipMin_.y() )
		{
			CornerMin.y( ClipMin_.y() );
		}

		if ( CornerMax.y() > ClipMax_.y() )
		{
			CornerMax.y( ClipMax_.y() );
		}
	}
	
	Canvas_->setMaterialComponent( SolidMaterialComponent_ );
	Canvas_->drawSprite( CornerMin, CornerMax - CornerMin, 0, Colour_, 0 );
} 

void GwenRenderer::SetDrawColor(Gwen::Color color)
{
	Colour_.set( BcReal( color.r ) / 255.0f, BcReal( color.g ) / 255.0f, BcReal( color.b ) / 255.0f, BcReal( color.a ) / 255.0f );
}

void GwenRenderer::StartClip()
{
	Gwen::Rect rect = ClipRegion();
	
	ClipMin_ = BcVec2d( rect.x, rect.y );
	ClipMax_ = BcVec2d( rect.w, rect.h ) + ClipMin_;
	ClippingEnabled_ = BcTrue;
}

void GwenRenderer::EndClip()
{
	Gwen::Rect rect = ClipRegion();
	ClippingEnabled_ = BcFalse;
}

void GwenRenderer::DrawTexturedRect( Gwen::Texture* pTexture, Gwen::Rect rect, float U0, float V0, float U1, float V1 )
{
	ScnMaterialComponentRef MaterialComponent( (ScnMaterialComponent*)pTexture->data );
	if ( !MaterialComponent.isValid() ) return;

	// Handle no material.
	if ( MaterialComponent.isReady() == BcFalse )
	{
		return;
	}
	
	Translate( rect );
	
	BcVec2d Size( BcVec2d( rect.w, rect.h ) );
	BcVec2d CornerMin( BcVec2d( rect.x, rect.y ) );
	BcVec2d CornerMax( CornerMin + Size );

	if ( ClippingEnabled_ )
	{
		if ( ( CornerMax.x() < ClipMin_.x() ) || ( CornerMin.x() > ClipMax_.x() ) || ( CornerMax.y() < ClipMin_.y() ) || ( CornerMin.y() > ClipMax_.y() ) )
		{
			return;
		}

		BcReal TexWidth = U1 - U0;
		BcReal TexHeight = V1 - V0;

		if ( CornerMin.x() < ClipMin_.x() )
		{
			U0 -= ( ( CornerMin.x() - ClipMin_.x() ) / Size.x() ) * TexWidth;
			CornerMin.x( ClipMin_.x() );
		}

		if ( CornerMax.x() > ClipMax_.x() )
		{
			U1 -= ( ( CornerMax.x() - ClipMax_.x() ) / Size.x() ) * TexWidth;
			CornerMax.x( ClipMax_.x() );
		}

		if ( CornerMin.y() < ClipMin_.y() )
		{
			V0 -= ( ( CornerMin.y() - ClipMin_.y() ) / Size.y() ) * TexHeight;
			CornerMin.y( ClipMin_.y() );
		}

		if ( CornerMax.y() > ClipMax_.y() )
		{
			V1 -= ( ( CornerMax.y() - ClipMax_.y() ) / Size.y() ) * TexHeight;
			CornerMax.y( ClipMax_.y() );
		}
	}

	// Allocate 2 triangles.
	ScnCanvasComponentVertex* pVertices = Canvas_->allocVertices( 6 );
	ScnCanvasComponentVertex* pFirstVertex = pVertices;
		
	// Only draw if we can allocate vertices.
	if( pVertices != NULL )
	{
		// Now copy in data.
		BcU32 RGBA = Colour_.asABGR();
		
		pVertices->X_ = CornerMin.x();
		pVertices->Y_ = CornerMin.y();
		pVertices->Z_ = 0.0f;
		pVertices->U_ = U0;
		pVertices->V_ = V0;
		pVertices->RGBA_ = RGBA;
		++pVertices;
		
		pVertices->X_ = CornerMax.x();
		pVertices->Y_ = CornerMin.y();
		pVertices->Z_ = 0.0f;
		pVertices->U_ = U1;
		pVertices->V_ = V0;
		pVertices->RGBA_ = RGBA;
		++pVertices;
		
		pVertices->X_ = CornerMin.x();
		pVertices->Y_ = CornerMax.y();
		pVertices->Z_ = 0.0f;
		pVertices->U_ = U0;
		pVertices->V_ = V1;
		pVertices->RGBA_ = RGBA;
		++pVertices;
		
		pVertices->X_ = CornerMin.x();
		pVertices->Y_ = CornerMax.y();
		pVertices->Z_ = 0.0f;
		pVertices->U_ = U0;
		pVertices->V_ = V1;
		pVertices->RGBA_ = RGBA;
		++pVertices;

		pVertices->X_ = CornerMax.x();
		pVertices->Y_ = CornerMin.y();
		pVertices->Z_ = 0.0f;
		pVertices->U_ = U1;
		pVertices->V_ = V0;
		pVertices->RGBA_ = RGBA;
		++pVertices;

		pVertices->X_ = CornerMax.x();
		pVertices->Y_ = CornerMax.y();
		pVertices->Z_ = 0.0f;
		pVertices->U_ = U1;
		pVertices->V_ = V1;
		pVertices->RGBA_ = RGBA;

		Canvas_->setMaterialComponent( MaterialComponent );
		Canvas_->addPrimitive( rsPT_TRIANGLELIST, pFirstVertex, 6, 0, BcTrue );
	}
}

void GwenRenderer::LoadTexture( Gwen::Texture* pTexture )
{
	// Setup texture params for failure.
	pTexture->data = NULL;
	pTexture->failed = 1;
	pTexture->width = 0;
	pTexture->height = 0;

	// Create material component and attach it to our parent.
	std::string FileName = pTexture->name.Get();
	ScnMaterialComponentRef MaterialComponent;
	if( !ParentComponent_->createMaterialComponentByAlias( FileName.c_str(), MaterialComponent ) )
	{
		return;
	}
	
	// Attach to entity.
	Entity_->attach( MaterialComponent );
	
	// Get texture from material component.
	BcU32 DiffuseTexParam = MaterialComponent->findParameter( "aDiffuseTex" );
	ScnTextureRef Texture( MaterialComponent->getTexture( DiffuseTexParam ) );

	// Setup texture params.
	pTexture->data = &(*MaterialComponent);
	pTexture->failed = 0;
	pTexture->width = Texture->getWidth();
	pTexture->height = Texture->getHeight();	
}

void GwenRenderer::FreeTexture( Gwen::Texture* pTexture )
{
	ScnMaterialComponentRef MaterialComponent( (ScnMaterialComponent*)pTexture->data );
	if ( !MaterialComponent.isValid() ) return;

	// Detach from entity.
	Entity_->detach( MaterialComponent );
	
	// Null the data.
	pTexture->data = NULL;
}

Gwen::Color GwenRenderer::PixelColour( Gwen::Texture* pTexture, unsigned int x, unsigned int y, const Gwen::Color& col_default )
{
	ScnMaterialComponentRef MaterialComponent( (ScnMaterialComponent*)pTexture->data );
	if ( !MaterialComponent.isValid() ) return Gwen::Color();

	// Get texture from material.
	BcU32 DiffuseTexParam = MaterialComponent->findParameter( "aDiffuseTex" );
	ScnTextureRef Texture( MaterialComponent->getTexture( DiffuseTexParam ) );

	// Get texel from texture.
	RsColour Colour = Texture->getTexel( x, y );

	// Convert to Gwen colour.
	Gwen::Color GwenColour
	(
		(unsigned char)BcClamp( BcU32( Colour.r() * 255.0f ), 0, 255 ),
		(unsigned char)BcClamp( BcU32( Colour.g() * 255.0f ), 0, 255 ),
		(unsigned char)BcClamp( BcU32( Colour.b() * 255.0f ), 0, 255 ),
		(unsigned char)BcClamp( BcU32( Colour.a() * 255.0f ), 0, 255 )
	);

	return GwenColour;
}

void GwenRenderer::LoadFont( Gwen::Font* pFont )
{
	// Setup font params for failure.
	pFont->data = NULL;
	pFont->realsize = 0.0f;
	pFont->size = 0.0f;
	pFont->bold = false;
	pFont->dropshadow = false;
	
	// Create font component and attach it to our parent.
	Gwen::String FaceName =  Gwen::Utility::UnicodeToString( pFont->facename );
	ScnFontComponentRef FontComponent;
	if( !ParentComponent_->createFontComponentByAlias( FaceName.c_str(), FontComponent ) )
	{
		return;
	}
	
	// Attach to entity.
	Entity_->attach( FontComponent );
	
	// Setup texture params.
	pFont->data = &(*FontComponent);
}

void GwenRenderer::FreeFont( Gwen::Font* pFont )
{
	ScnFontComponentRef FontComponent( (ScnFontComponent*)pFont->data );
	if ( !FontComponent.isValid() ) return;

	// Detach from entity.
	Entity_->detach( FontComponent );
	
	// Null the data.
	pFont->data = NULL;
}

void GwenRenderer::RenderText( Gwen::Font* pFont, Gwen::Point pos, const Gwen::UnicodeString& text )
{
	if( pFont->data == NULL )
	{
		LoadFont( pFont );
	}

	ScnFontComponentRef FontComponent( (ScnFontComponent*)pFont->data );
	if ( !FontComponent.isValid() ) return;

	// Handle components that aren't attached.
	if ( !FontComponent->isAttached() )
	{
		BcPrintf( "GwenRenderer::RenderText: FontComponent \"%s\" is not attached yet.\n", (*FontComponent->getName()).c_str() );
		return; 
	}

	// Convert to UTF-8.
	Gwen::String OutString =  Gwen::Utility::UnicodeToString( text );
	
	ScnMaterialComponentRef MaterialComponent( FontComponent->getMaterialComponent() );
	BcU32 AlphaStepParam = MaterialComponent->findParameter( "aAlphaTestStep" );
	MaterialComponent->setParameter( AlphaStepParam, BcVec2d( 0.3f, 0.5f ) );

	// Set clipping and draw.
	Translate( pos.x, pos.y );

	FontComponent->setClipping( ClippingEnabled_, ClipMin_, ClipMax_ );
	//BcVec2d Size = FontComponent->draw( Canvas_, BcVec2d( 0.0f, 0.0f ), OutString.c_str(), Colour_, BcTrue );
	//Canvas_->setMaterialComponent( SolidMaterialComponent_ );
	//Canvas_->drawBox( BcVec2d( pos.x, pos.y ), BcVec2d( pos.x, pos.y ) + Size, RsColour::GREEN, 0 );
	FontComponent->draw( Canvas_, BcVec2d( pos.x, pos.y ), OutString.c_str(), Colour_, BcFalse );
}

Gwen::Point GwenRenderer::MeasureText( Gwen::Font* pFont, const Gwen::UnicodeString& text )
{
	if( pFont->data == NULL )
	{
		LoadFont( pFont );
	}

	ScnFontComponentRef FontComponent( (ScnFontComponent*)pFont->data );
	if ( !FontComponent.isValid() ) return Gwen::Point();

	// Convert to UTF-8.
	Gwen::String OutString =  Gwen::Utility::UnicodeToString( text );
	BcVec2d Size = FontComponent->draw( Canvas_, BcVec2d( 0.0f, 0.0f ), OutString.c_str(), Colour_, BcTrue );
	
	return Gwen::Point( Size.x(), Size.y() );
}

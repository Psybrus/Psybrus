/**************************************************************************
*
* File:		GwenRenderer.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Based on the Gwen OpenGL Renderer.
*		
*
*
* 
**************************************************************************/

#ifndef GWEN_RENDERERS_GwenRenderer_H
#define GWEN_RENDERERS_GwenRenderer_H

#include "ScnEntity.h"
#include "ScnCanvasComponent.h"
#include "ScnFont.h"

#include "ScnGwenCanvas.h"

#include "Gwen/Gwen.h"
#include "Gwen/Utility.h"
#include "Gwen/Font.h"
#include "Gwen/Texture.h"
#include "Gwen/BaseRender.h"
#include "Gwen/Skins/TexturedBase.h"

class GwenRenderer : public Gwen::Renderer::Base
{
public:
	GwenRenderer( ScnEntityRef Entity, ScnGwenCanvasComponentRef ParentComponent );
	virtual ~GwenRenderer();

	virtual void Init();

	virtual void Begin();
	virtual void End();

	virtual void SetDrawColor( Gwen::Color color );
	virtual void DrawFilledRect( Gwen::Rect rect );

	void StartClip();
	void EndClip();

	void LoadTexture( Gwen::Texture* pTexture );
	void FreeTexture( Gwen::Texture* pTexture );
	void DrawTexturedRect( Gwen::Texture* pTexture, Gwen::Rect pTargetRect, float u1=0.0f, float v1=0.0f, float u2=1.0f, float v2=1.0f );
	Gwen::Color PixelColour( Gwen::Texture* pTexture, unsigned int x, unsigned int y, const Gwen::Color& col_default );

	virtual void LoadFont( Gwen::Font* pFont );
	virtual void FreeFont( Gwen::Font* pFont );
	virtual void RenderText( Gwen::Font* pFont, Gwen::Point pos, const Gwen::UnicodeString& text );
	virtual Gwen::Point MeasureText( Gwen::Font* pFont, const Gwen::UnicodeString& text );

protected:
	ScnEntityRef Entity_;
	ScnGwenCanvasComponentRef ParentComponent_;
	RsColour Colour_;

	BcBool ClippingEnabled_;
	BcVec2d ClipMin_;
	BcVec2d ClipMax_;

	ScnCanvasComponentRef Canvas_;

	ScnMaterialComponentRef SolidMaterialComponent_;
};

#endif

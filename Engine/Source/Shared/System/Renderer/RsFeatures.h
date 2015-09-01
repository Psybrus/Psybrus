#pragma once

#include "System/Renderer/RsTypes.h"

//////////////////////////////////////////////////////////////////////////
// RsFeatureRenderTargetOrigin
enum class RsFeatureRenderTargetOrigin
{
	TOP_LEFT,
	BOTTOM_LEFT,
};

//////////////////////////////////////////////////////////////////////////
// RsFeatures
struct RsFeatures
{
	// Misc.
	bool MRT_ = false;
	bool DepthTextures_ = false;
	bool NPOTTextures_ = false;
	bool SeparateBlendState_ = false;
	bool AnisotropicFiltering_ = false;
	bool AntialiasedLines_ = false;
	RsFeatureRenderTargetOrigin RTOrigin_ = RsFeatureRenderTargetOrigin::TOP_LEFT;

	// Texture types.
	bool Texture1D_ = false;
	bool Texture2D_ = false;
	bool Texture3D_ = false;
	bool TextureCube_ = false;

	// Texture formats.
	bool TextureFormat_[ (int)RsTextureFormat::MAX ] = { false };
	bool RenderTargetFormat_[ (int)RsTextureFormat::MAX ] = { false };
	bool DepthStencilTargetFormat_[ (int)RsTextureFormat::MAX ] = { false };

	// TODO: Buffer types.


	// TODO: Buffer formats.


	// TODO: Shader types.


	// TODO: Shader code types.


	// TODO: Draw types.


};

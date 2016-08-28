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
	RsFeatures()
	{
		TextureFormat_.fill( false );
		RenderTargetFormat_.fill( false );
		DepthStencilTargetFormat_.fill( false );
	}

	// Misc.
	bool MRT_ = false;
	bool NPOTTextures_ = false;
	bool SeparateBlendState_ = false;
	bool AnisotropicFiltering_ = false;
	bool AntialiasedLines_ = false;
	RsFeatureRenderTargetOrigin RTOrigin_ = RsFeatureRenderTargetOrigin::TOP_LEFT;
	bool ComputeShaders_ = false;
	bool Instancing_ = false;

	// Texture types.
	bool Texture1D_ = false;
	bool Texture2D_ = false;
	bool Texture3D_ = false;
	bool TextureCube_ = false;

	// Texture formats.
	std::array< bool, (int)RsResourceFormat::MAX > TextureFormat_;
	std::array< bool, (int)RsResourceFormat::MAX > RenderTargetFormat_;
	std::array< bool, (int)RsResourceFormat::MAX > DepthStencilTargetFormat_;

	// TODO: Buffer types.


	// TODO: Buffer formats.


	// TODO: Shader types.


	// TODO: Shader code types.


	// TODO: Draw types.


};

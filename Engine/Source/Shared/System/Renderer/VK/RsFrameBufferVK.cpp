#include "System/Renderer/VK/RsFrameBufferVK.h"
#include "System/Renderer/VK/RsTextureVK.h"
#include "System/Renderer/VK/RsUtilsVK.h"

#include "System/Renderer/RsFrameBuffer.h"
#include "System/Renderer/RsTexture.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsFrameBufferVK::RsFrameBufferVK( class RsFrameBuffer* Parent, VkDevice Device ):
	Parent_( Parent ),
	Device_( Device )
{
	Parent->setHandle( this );
	createRenderPass();
	createFrameBuffer();
}

//////////////////////////////////////////////////////////////////////////
// Dtor
RsFrameBufferVK::~RsFrameBufferVK()
{
	if( RenderPass_ )
	{
		vkDestroyRenderPass( Device_, RenderPass_ );
		RenderPass_ = 0;
	}

	if( FrameBuffer_ )
	{
		vkDestroyFramebuffer( Device_, FrameBuffer_ );
		FrameBuffer_ = 0;
	}

	Parent_->setHandle( 0 );
}
	
//////////////////////////////////////////////////////////////////////////
// createFrameBuffer
void RsFrameBufferVK::createFrameBuffer()
{
	const auto Desc = Parent_->getDesc();
	const size_t NoofAttachments = Desc.RenderTargets_.size() + ( Desc.DepthStencilTarget_ ? 1 : 0 );

	// Setup frame buffer.
	uint32_t Width = std::numeric_limits< uint32_t >::max();
	uint32_t Height = std::numeric_limits< uint32_t >::max();

	std::vector< VkImageView > ImageViews( NoofAttachments );
	uint32_t AttachmentIdx = 0;
	for( uint32_t Idx = 0; Idx < Desc.RenderTargets_.size(); ++Idx )
	{
		if( Desc.RenderTargets_[ Idx ] )
		{
			auto& RenderTargetDesc = Desc.RenderTargets_[ Idx ]->getDesc();
			auto* TextureVK = Desc.RenderTargets_[ Idx ]->getHandle< RsTextureVK* >();
			
			Width = std::min( Width, RenderTargetDesc.Width_ );
			Height = std::min( Height, RenderTargetDesc.Height_ );

			ImageViews[ AttachmentIdx ] = TextureVK->getImageView();
			BcAssert( ImageViews[ AttachmentIdx ] != 0 );
			++AttachmentIdx;

			// Append hash.
			FormatHash_ = BcHash::GenerateCRC32( FormatHash_, &RenderTargetDesc.Format_, sizeof( RenderTargetDesc.Format_ ) );
		}
	}

	if( Desc.DepthStencilTarget_ )
	{
		auto& DepthStencilDesc = Desc.DepthStencilTarget_->getDesc();
		auto* TextureVK = Desc.DepthStencilTarget_->getHandle< RsTextureVK* >();

		Width = std::min( Width, DepthStencilDesc.Width_ );
		Height = std::min( Height, DepthStencilDesc.Height_ );

		ImageViews[ AttachmentIdx ] = TextureVK->getImageView();
		BcAssert( ImageViews[ AttachmentIdx ] != 0 );
		++AttachmentIdx;

		// Append hash.
		FormatHash_ = BcHash::GenerateCRC32( FormatHash_, &DepthStencilDesc.Format_, sizeof( DepthStencilDesc.Format_ ) );
	}

	VkFramebufferCreateInfo FrameBufferCreateInfo = {};
	FrameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	FrameBufferCreateInfo.pNext = nullptr;
	FrameBufferCreateInfo.renderPass = RenderPass_;
	FrameBufferCreateInfo.attachmentCount = static_cast< uint32_t >( AttachmentIdx );
	FrameBufferCreateInfo.pAttachments = ImageViews.data();
	FrameBufferCreateInfo.width  = Width;
	FrameBufferCreateInfo.height = Height;
	FrameBufferCreateInfo.layers = 1;

	auto RetVal = vkCreateFramebuffer( Device_, &FrameBufferCreateInfo, &FrameBuffer_ );
	BcAssert( !RetVal );

}

//////////////////////////////////////////////////////////////////////////
// createRenderPass
void RsFrameBufferVK::createRenderPass()
{
	const auto Desc = Parent_->getDesc();
	const size_t NoofAttachments = Desc.RenderTargets_.size() + ( Desc.DepthStencilTarget_ ? 1 : 0 );

	// Setup render pass.
	std::vector< VkAttachmentDescription > Attachments( NoofAttachments );
	std::vector< VkAttachmentReference > ColourAttachmentReferences( Desc.RenderTargets_.size() );
	VkAttachmentReference DepthStencilAttachmentReference = {};
	uint32_t AttachmentIdx = 0;
	for( uint32_t Idx = 0; Idx < Desc.RenderTargets_.size(); ++Idx )
	{
		if( Desc.RenderTargets_[ Idx ] )
		{
			auto& RenderTargetDesc = Desc.RenderTargets_[ Idx ]->getDesc();

			auto& Attachment = Attachments[ AttachmentIdx ];
			Attachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION;
			Attachment.pNext = nullptr;
			Attachment.format = RsUtilsVK::GetTextureFormat( RenderTargetDesc.Format_ );
			Attachment.samples = 1;
			Attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			Attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			Attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			Attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
			Attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			Attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			auto& ColourAttachmentReference = ColourAttachmentReferences[ Idx ];
			ColourAttachmentReference.attachment = AttachmentIdx;
			ColourAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			++AttachmentIdx;
		}
	}
	
	if( Desc.DepthStencilTarget_ )
	{
		auto& DepthStencilDesc = Desc.DepthStencilTarget_->getDesc();

		auto& Attachment = Attachments[ AttachmentIdx ];
		Attachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION;
		Attachment.pNext = nullptr;
		Attachment.format = RsUtilsVK::GetTextureFormat( DepthStencilDesc.Format_ );
		Attachment.samples = 1;
		Attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		Attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		Attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		Attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		Attachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		Attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		DepthStencilAttachmentReference.attachment = AttachmentIdx;
		DepthStencilAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		++AttachmentIdx;
	}

	VkSubpassDescription Subpass = {};
	Subpass.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION;
	Subpass.pNext = nullptr;
	Subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	Subpass.flags = 0;
	Subpass.inputCount = 0;
	Subpass.pInputAttachments = nullptr;
	Subpass.colorCount = static_cast< uint32_t >( ColourAttachmentReferences.size() );
	Subpass.pColorAttachments = ColourAttachmentReferences.data();
	Subpass.pResolveAttachments = nullptr;
	Subpass.depthStencilAttachment = DepthStencilAttachmentReference;
	Subpass.preserveCount = 0;
	Subpass.pPreserveAttachments = nullptr;

	VkRenderPassCreateInfo RenderPassCreateInfo = {};
	RenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	RenderPassCreateInfo.pNext = nullptr;
	RenderPassCreateInfo.attachmentCount = static_cast< uint32_t >( AttachmentIdx );
	RenderPassCreateInfo.pAttachments = Attachments.data();
	RenderPassCreateInfo.subpassCount = 1;
	RenderPassCreateInfo.pSubpasses = &Subpass;
	RenderPassCreateInfo.dependencyCount = 0;
	RenderPassCreateInfo.pDependencies = nullptr;

	auto RetVal = vkCreateRenderPass( Device_, &RenderPassCreateInfo, &RenderPass_ );
	BcAssert( !RetVal );
}



/*!
\brief Function definitions for the RenderPass.
\file PVRVk/RenderPassVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRVk/RenderPassVk.h"

namespace pvrvk {
namespace impl {
//!\cond NO_DOXYGEN
RenderPass_::RenderPass_(make_shared_enabler, const DeviceWeakPtr& device, const RenderPassCreateInfo& createInfo) : PVRVkDeviceObjectBase(device), DeviceObjectDebugUtils()
{
	_createInfo = createInfo;
	if (createInfo.extendedSupportRequired())
	{
		const uint32_t vk12 = VK_MAKE_VERSION(1, 2, 0);
		if (getDevice()->getPhysicalDevice()->getProperties().getApiVersion() <= vk12)
		{ throw std::runtime_error("attempting to create a renderpass using vulkan 1.2+ feature(s) with an unsupported physical device"); }
		createRenderPass2(createInfo);
	}
	else
	{
		createRenderPass(createInfo);
	}
}

void RenderPass_::createRenderPass(const RenderPassCreateInfo& createInfo) 
{
	VkRenderPassCreateInfo renderPassInfoVk;
	memset(&renderPassInfoVk, 0, sizeof(renderPassInfoVk));
	renderPassInfoVk.sType = static_cast<VkStructureType>(StructureType::e_RENDER_PASS_CREATE_INFO);

	renderPassInfoVk.attachmentCount = createInfo.getNumAttachmentDescription();
	pvrvk::ArrayOrVector<VkAttachmentDescription, 4> attachmentDescVk(renderPassInfoVk.attachmentCount);

	for (uint32_t attachmentIndex = 0; attachmentIndex < createInfo.getNumAttachmentDescription(); ++attachmentIndex)
	{
		const AttachmentDescription& attachmentDesc = createInfo.getAttachmentDescription(attachmentIndex);
		attachmentDescVk[attachmentIndex].flags = static_cast<VkAttachmentDescriptionFlags>(AttachmentDescriptionFlags::e_NONE);
		attachmentDescVk[attachmentIndex].samples = static_cast<VkSampleCountFlagBits>(attachmentDesc.getSamples());

		if ((attachmentDescVk[attachmentIndex].format = static_cast<VkFormat>(attachmentDesc.getFormat())) == Format::e_UNDEFINED)
		{ throw ErrorValidationFailedEXT("RenderPassVk: Unsupported Color VkFormat"); }
		attachmentDescVk[attachmentIndex].loadOp = static_cast<VkAttachmentLoadOp>(attachmentDesc.getLoadOp());
		attachmentDescVk[attachmentIndex].storeOp = static_cast<VkAttachmentStoreOp>(attachmentDesc.getStoreOp());
		attachmentDescVk[attachmentIndex].initialLayout = static_cast<VkImageLayout>(attachmentDesc.getInitialLayout());
		attachmentDescVk[attachmentIndex].finalLayout = static_cast<VkImageLayout>(attachmentDesc.getFinalLayout());
		attachmentDescVk[attachmentIndex].stencilLoadOp = static_cast<VkAttachmentLoadOp>(attachmentDesc.getStencilLoadOp());
		attachmentDescVk[attachmentIndex].stencilStoreOp = static_cast<VkAttachmentStoreOp>(attachmentDesc.getStencilStoreOp());
		attachmentDescVk[attachmentIndex].samples = static_cast<VkSampleCountFlagBits>(attachmentDesc.getSamples());
	} // next attachment

	renderPassInfoVk.pAttachments = attachmentDescVk.get();

	// Subpass
	renderPassInfoVk.subpassCount = createInfo.getNumSubpasses();
	pvrvk::ArrayOrVector<VkSubpassDescription, 2> subPassesVk(renderPassInfoVk.subpassCount);

	// calculate the attachmentRefs total sizes
	uint32_t totalNumAttachmentRef = 0;

	for (uint32_t i = 0; i < renderPassInfoVk.subpassCount; ++i)
	{
		const SubpassDescription& subpass = createInfo.getSubpass(i);
		totalNumAttachmentRef += subpass.getNumColorAttachmentReference();
		totalNumAttachmentRef += subpass.getNumInputAttachmentReference();
		totalNumAttachmentRef += static_cast<uint32_t>(subpass.getDepthStencilAttachmentReference().getLayout() != ImageLayout::e_UNDEFINED);
		totalNumAttachmentRef += subpass.getNumResolveAttachmentReference();
		totalNumAttachmentRef += subpass.getNumPreserveAttachmentReference();
	}
	pvrvk::ArrayOrVector<VkAttachmentReference, 4> attachmentReferenceVk(totalNumAttachmentRef);

	uint32_t attachmentOffset = 0;
	for (uint32_t subpassId = 0; subpassId < renderPassInfoVk.subpassCount; ++subpassId)
	{
		const SubpassDescription& subpass = createInfo.getSubpass(subpassId);
		VkSubpassDescription subPassVk = {};
		subPassVk.pipelineBindPoint = static_cast<VkPipelineBindPoint>(subpass.getPipelineBindPoint());

		// input attachments
		if (subpass.getNumInputAttachmentReference() > 0)
		{
			subPassVk.pInputAttachments = &attachmentReferenceVk[attachmentOffset];
			for (uint8_t j = 0; j < subpass.getNumInputAttachmentReference(); ++j)
			{
				attachmentReferenceVk[attachmentOffset].attachment = subpass.getInputAttachmentReference(j).getAttachment();
				attachmentReferenceVk[attachmentOffset].layout = static_cast<VkImageLayout>(subpass.getInputAttachmentReference(j).getLayout());
				++attachmentOffset;
			}
			subPassVk.inputAttachmentCount = subpass.getNumInputAttachmentReference();
		}

		// Color attachments
		if (subpass.getNumColorAttachmentReference() > 0)
		{
			subPassVk.pColorAttachments = &attachmentReferenceVk[attachmentOffset];
			for (uint8_t j = 0; j < subpass.getNumColorAttachmentReference(); ++j)
			{
				attachmentReferenceVk[attachmentOffset].attachment = subpass.getColorAttachmentReference(j).getAttachment();
				attachmentReferenceVk[attachmentOffset].layout = static_cast<VkImageLayout>(subpass.getColorAttachmentReference(j).getLayout());
				++attachmentOffset;
			}
			subPassVk.colorAttachmentCount = subpass.getNumColorAttachmentReference();
		}

		// resolve attachments
		if (subpass.getNumResolveAttachmentReference() > 0)
		{
			subPassVk.pResolveAttachments = &attachmentReferenceVk[attachmentOffset];
			for (uint8_t j = 0; j < subpass.getNumResolveAttachmentReference(); ++j)
			{
				attachmentReferenceVk[attachmentOffset].attachment = subpass.getResolveAttachmentReference(j).getAttachment();
				attachmentReferenceVk[attachmentOffset].layout = static_cast<VkImageLayout>(subpass.getResolveAttachmentReference(j).getLayout());
				++attachmentOffset;
			}
		}

		// preserve attachments
		if (subpass.getNumPreserveAttachmentReference() > 0)
		{
			subPassVk.pPreserveAttachments = subpass.getAllPreserveAttachments();
			subPassVk.preserveAttachmentCount = subpass.getNumPreserveAttachmentReference();
		}

		// depth-stencil attachment
		if (subpass.getDepthStencilAttachmentReference().getLayout() != ImageLayout::e_UNDEFINED)
		{
			subPassVk.pDepthStencilAttachment = &attachmentReferenceVk[attachmentOffset];
			attachmentReferenceVk[attachmentOffset].attachment = subpass.getDepthStencilAttachmentReference().getAttachment();
			attachmentReferenceVk[attachmentOffset].layout = static_cast<VkImageLayout>(subpass.getDepthStencilAttachmentReference().getLayout());
			++attachmentOffset;
		}
		subPassesVk[subpassId] = subPassVk;
	} // next subpass
	renderPassInfoVk.pSubpasses = subPassesVk.get();

	// Sub pass dependencies
	renderPassInfoVk.dependencyCount = createInfo.getNumSubpassDependencies();
	pvrvk::ArrayOrVector<VkSubpassDependency, 4> subPassDependenciesVk(renderPassInfoVk.dependencyCount);

	for (uint32_t attachmentIndex = 0; attachmentIndex < createInfo.getNumSubpassDependencies(); ++attachmentIndex)
	{
		const SubpassDependency& subPassDependency = createInfo.getSubpassDependency(attachmentIndex);
		VkSubpassDependency subPassDependencyVk;
		memset(&subPassDependencyVk, 0, sizeof(subPassDependencyVk));

		subPassDependencyVk.srcSubpass = subPassDependency.getSrcSubpass();
		subPassDependencyVk.dstSubpass = subPassDependency.getDstSubpass();
		subPassDependencyVk.srcStageMask = static_cast<VkPipelineStageFlags>(subPassDependency.getSrcStageMask());
		subPassDependencyVk.dstStageMask = static_cast<VkPipelineStageFlags>(subPassDependency.getDstStageMask());
		subPassDependencyVk.srcAccessMask = static_cast<VkAccessFlags>(subPassDependency.getSrcAccessMask());
		subPassDependencyVk.dstAccessMask = static_cast<VkAccessFlags>(subPassDependency.getDstAccessMask());
		subPassDependencyVk.dependencyFlags = static_cast<VkDependencyFlags>(subPassDependency.getDependencyFlags());

		subPassDependenciesVk[attachmentIndex] = subPassDependencyVk;
	} // next subpass dependency
	renderPassInfoVk.pDependencies = subPassDependenciesVk.get();

	vkThrowIfFailed(getDevice()->getVkBindings().vkCreateRenderPass(getDevice()->getVkHandle(), &renderPassInfoVk, NULL, &_vkHandle), "Create RenderPass Failed");
}

void RenderPass_::createRenderPass2(const RenderPassCreateInfo& createInfo)
{
	VkRenderPassCreateInfo2 renderPassInfoVk;
	memset(&renderPassInfoVk, 0, sizeof(renderPassInfoVk));
	renderPassInfoVk.sType = static_cast<VkStructureType>(StructureType::e_RENDER_PASS_CREATE_INFO_2);

	renderPassInfoVk.attachmentCount = createInfo.getNumAttachmentDescription();
	pvrvk::ArrayOrVector<VkAttachmentDescription2, 4> attachmentDescVk(renderPassInfoVk.attachmentCount);

	for (uint32_t attachmentIndex = 0; attachmentIndex < createInfo.getNumAttachmentDescription(); ++attachmentIndex)
	{
		attachmentDescVk[attachmentIndex].sType = static_cast<VkStructureType>(StructureType::e_ATTACHMENT_DESCRIPTION_2);
		attachmentDescVk[attachmentIndex].pNext = nullptr;

		const AttachmentDescription& attachmentDesc = createInfo.getAttachmentDescription(attachmentIndex);
		attachmentDescVk[attachmentIndex].flags = static_cast<VkAttachmentDescriptionFlags>(AttachmentDescriptionFlags::e_NONE);
		attachmentDescVk[attachmentIndex].samples = static_cast<VkSampleCountFlagBits>(attachmentDesc.getSamples());

		if ((attachmentDescVk[attachmentIndex].format = static_cast<VkFormat>(attachmentDesc.getFormat())) == Format::e_UNDEFINED)
		{ throw ErrorValidationFailedEXT("RenderPassVk: Unsupported Color VkFormat"); }
		attachmentDescVk[attachmentIndex].loadOp = static_cast<VkAttachmentLoadOp>(attachmentDesc.getLoadOp());
		attachmentDescVk[attachmentIndex].storeOp = static_cast<VkAttachmentStoreOp>(attachmentDesc.getStoreOp());
		attachmentDescVk[attachmentIndex].initialLayout = static_cast<VkImageLayout>(attachmentDesc.getInitialLayout());
		attachmentDescVk[attachmentIndex].finalLayout = static_cast<VkImageLayout>(attachmentDesc.getFinalLayout());
		attachmentDescVk[attachmentIndex].stencilLoadOp = static_cast<VkAttachmentLoadOp>(attachmentDesc.getStencilLoadOp());
		attachmentDescVk[attachmentIndex].stencilStoreOp = static_cast<VkAttachmentStoreOp>(attachmentDesc.getStencilStoreOp());
		attachmentDescVk[attachmentIndex].samples = static_cast<VkSampleCountFlagBits>(attachmentDesc.getSamples());
	} // next attachment

	renderPassInfoVk.pAttachments = attachmentDescVk.get();

	// Subpass
	renderPassInfoVk.subpassCount = createInfo.getNumSubpasses();
	pvrvk::ArrayOrVector<VkSubpassDescription2, 2> subPassesVk(renderPassInfoVk.subpassCount);
	pvrvk::ArrayOrVector<VkFragmentShadingRateAttachmentInfoKHR, 2> fragmentShadingRateAttachmentInfos(renderPassInfoVk.subpassCount);

	// calculate the attachmentRefs total sizes
	uint32_t totalNumAttachmentRef = 0;

	for (uint32_t i = 0; i < renderPassInfoVk.subpassCount; ++i)
	{
		const SubpassDescription& subpass = createInfo.getSubpass(i);
		totalNumAttachmentRef += subpass.getNumColorAttachmentReference();
		totalNumAttachmentRef += subpass.getNumInputAttachmentReference();
		totalNumAttachmentRef += static_cast<uint32_t>(subpass.getDepthStencilAttachmentReference().getLayout() != ImageLayout::e_UNDEFINED);
		totalNumAttachmentRef += subpass.getNumResolveAttachmentReference();
		totalNumAttachmentRef += subpass.getNumPreserveAttachmentReference();
		totalNumAttachmentRef += static_cast<uint32_t>(subpass.getFragmentShadingRateAttachment().getEnabled());
	}
	pvrvk::ArrayOrVector<VkAttachmentReference2, 4> attachmentReferenceVk(totalNumAttachmentRef);

	uint32_t attachmentOffset = 0;
	for (uint32_t subpassId = 0; subpassId < renderPassInfoVk.subpassCount; ++subpassId)
	{
		const SubpassDescription& subpass = createInfo.getSubpass(subpassId);
		VkSubpassDescription2 subPassVk = {};
		subPassVk.sType = static_cast<VkStructureType>(StructureType::e_SUBPASS_DESCRIPTION_2);
		subPassVk.pNext = nullptr;
		subPassVk.pipelineBindPoint = static_cast<VkPipelineBindPoint>(subpass.getPipelineBindPoint());

		// multiview not currently supported by the pvrsdk
		subPassVk.viewMask = 0;

		// input attachments
		if (subpass.getNumInputAttachmentReference() > 0)
		{
			subPassVk.pInputAttachments = &attachmentReferenceVk[attachmentOffset];
			for (uint8_t j = 0; j < subpass.getNumInputAttachmentReference(); ++j)
			{
				attachmentReferenceVk[attachmentOffset].sType = static_cast<VkStructureType>(StructureType::e_ATTACHMENT_REFERENCE_2);
				attachmentReferenceVk[attachmentOffset].pNext = nullptr;
				attachmentReferenceVk[attachmentOffset].attachment = subpass.getInputAttachmentReference(j).getAttachment();
				attachmentReferenceVk[attachmentOffset].layout = static_cast<VkImageLayout>(subpass.getInputAttachmentReference(j).getLayout());
				attachmentReferenceVk[attachmentOffset].aspectMask = static_cast<VkImageAspectFlags>(pvrvk::ImageAspectFlags::e_ALL_BITS);
				++attachmentOffset;
			}
			subPassVk.inputAttachmentCount = subpass.getNumInputAttachmentReference();
		}

		// Color attachments
		if (subpass.getNumColorAttachmentReference() > 0)
		{
			subPassVk.pColorAttachments = &attachmentReferenceVk[attachmentOffset];
			for (uint8_t j = 0; j < subpass.getNumColorAttachmentReference(); ++j)
			{
				attachmentReferenceVk[attachmentOffset].sType = static_cast<VkStructureType>(StructureType::e_ATTACHMENT_REFERENCE_2);
				attachmentReferenceVk[attachmentOffset].pNext = nullptr;
				attachmentReferenceVk[attachmentOffset].attachment = subpass.getColorAttachmentReference(j).getAttachment();
				attachmentReferenceVk[attachmentOffset].layout = static_cast<VkImageLayout>(subpass.getColorAttachmentReference(j).getLayout());
				attachmentReferenceVk[attachmentOffset].aspectMask = static_cast<VkImageAspectFlags>(pvrvk::ImageAspectFlags::e_ALL_BITS);
				++attachmentOffset;
			}
			subPassVk.colorAttachmentCount = subpass.getNumColorAttachmentReference();
		}

		// resolve attachments
		if (subpass.getNumResolveAttachmentReference() > 0)
		{
			subPassVk.pResolveAttachments = &attachmentReferenceVk[attachmentOffset];
			for (uint8_t j = 0; j < subpass.getNumResolveAttachmentReference(); ++j)
			{
				attachmentReferenceVk[attachmentOffset].sType = static_cast<VkStructureType>(StructureType::e_ATTACHMENT_REFERENCE_2);
				attachmentReferenceVk[attachmentOffset].pNext = nullptr;
				attachmentReferenceVk[attachmentOffset].attachment = subpass.getResolveAttachmentReference(j).getAttachment();
				attachmentReferenceVk[attachmentOffset].layout = static_cast<VkImageLayout>(subpass.getResolveAttachmentReference(j).getLayout());
				attachmentReferenceVk[attachmentOffset].aspectMask = static_cast<VkImageAspectFlags>(pvrvk::ImageAspectFlags::e_ALL_BITS);
				++attachmentOffset;
			}
		}

		// preserve attachments
		if (subpass.getNumPreserveAttachmentReference() > 0)
		{
			subPassVk.pPreserveAttachments = subpass.getAllPreserveAttachments();
			subPassVk.preserveAttachmentCount = subpass.getNumPreserveAttachmentReference();
		}

		// depth-stencil attachment
		if (subpass.getDepthStencilAttachmentReference().getLayout() != ImageLayout::e_UNDEFINED)
		{
			subPassVk.pDepthStencilAttachment = &attachmentReferenceVk[attachmentOffset];
			attachmentReferenceVk[attachmentOffset].sType = static_cast<VkStructureType>(StructureType::e_ATTACHMENT_REFERENCE_2);
			attachmentReferenceVk[attachmentOffset].pNext = nullptr;
			attachmentReferenceVk[attachmentOffset].attachment = subpass.getDepthStencilAttachmentReference().getAttachment();
			attachmentReferenceVk[attachmentOffset].layout = static_cast<VkImageLayout>(subpass.getDepthStencilAttachmentReference().getLayout());
			attachmentReferenceVk[attachmentOffset].aspectMask = static_cast<VkImageAspectFlags>(pvrvk::ImageAspectFlags::e_ALL_BITS);
			++attachmentOffset;
		}

		// fragment shading rate attachment
		if (subpass.getFragmentShadingRateAttachment().getEnabled())
		{
			subpass.getFragmentShadingRateAttachment().getAttachment();
			attachmentReferenceVk[attachmentOffset].sType = static_cast<VkStructureType>(StructureType::e_ATTACHMENT_REFERENCE_2);
			attachmentReferenceVk[attachmentOffset].pNext = nullptr;
			attachmentReferenceVk[attachmentOffset].attachment = subpass.getFragmentShadingRateAttachment().getAttachment().getAttachment();
			attachmentReferenceVk[attachmentOffset].layout = static_cast<VkImageLayout>(subpass.getFragmentShadingRateAttachment().getAttachment().getLayout());
			attachmentReferenceVk[attachmentOffset].aspectMask = static_cast<VkImageAspectFlags>(pvrvk::ImageAspectFlags::e_ALL_BITS);

			// fragment shading rate attachment info
			fragmentShadingRateAttachmentInfos[subpassId].sType = static_cast<VkStructureType>(StructureType::e_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR);
			fragmentShadingRateAttachmentInfos[subpassId].pNext = subPassVk.pNext;
			fragmentShadingRateAttachmentInfos[subpassId].pFragmentShadingRateAttachment = &attachmentReferenceVk[attachmentOffset];
			fragmentShadingRateAttachmentInfos[subpassId].shadingRateAttachmentTexelSize = subpass.getFragmentShadingRateAttachment().getTexelSize().get();

			subPassVk.pNext = static_cast<void*>(&fragmentShadingRateAttachmentInfos[subpassId]);
			++attachmentOffset;
		}

		subPassesVk[subpassId] = subPassVk;
	} // next subpass
	renderPassInfoVk.pSubpasses = subPassesVk.get();

	// Sub pass dependencies
	renderPassInfoVk.dependencyCount = createInfo.getNumSubpassDependencies();
	pvrvk::ArrayOrVector<VkSubpassDependency2, 4> subPassDependenciesVk(renderPassInfoVk.dependencyCount);

	for (uint32_t attachmentIndex = 0; attachmentIndex < createInfo.getNumSubpassDependencies(); ++attachmentIndex)
	{
		const SubpassDependency& subPassDependency = createInfo.getSubpassDependency(attachmentIndex);
		VkSubpassDependency2 subPassDependencyVk;
		memset(&subPassDependencyVk, 0, sizeof(subPassDependencyVk));

		subPassDependencyVk.sType = static_cast<VkStructureType>(StructureType::e_SUBPASS_DEPENDENCY_2);
		subPassDependencyVk.pNext = nullptr;
		subPassDependencyVk.srcSubpass = subPassDependency.getSrcSubpass();
		subPassDependencyVk.dstSubpass = subPassDependency.getDstSubpass();
		subPassDependencyVk.srcStageMask = static_cast<VkPipelineStageFlags>(subPassDependency.getSrcStageMask());
		subPassDependencyVk.dstStageMask = static_cast<VkPipelineStageFlags>(subPassDependency.getDstStageMask());
		subPassDependencyVk.srcAccessMask = static_cast<VkAccessFlags>(subPassDependency.getSrcAccessMask());
		subPassDependencyVk.dstAccessMask = static_cast<VkAccessFlags>(subPassDependency.getDstAccessMask());
		subPassDependencyVk.dependencyFlags = static_cast<VkDependencyFlags>(subPassDependency.getDependencyFlags());

		subPassDependenciesVk[attachmentIndex] = subPassDependencyVk;
	} // next subpass dependency
	renderPassInfoVk.pDependencies = subPassDependenciesVk.get();

	// multiview correlated view masks not currently supported by the pvrsdk
	renderPassInfoVk.correlatedViewMaskCount = 0;
	renderPassInfoVk.pCorrelatedViewMasks = nullptr;

	vkThrowIfFailed(getDevice()->getVkBindings().vkCreateRenderPass2(getDevice()->getVkHandle(), &renderPassInfoVk, NULL, &_vkHandle), "Create RenderPass Failed");
}

RenderPass_::~RenderPass_()
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		if (!_device.expired())
		{
			getDevice()->getVkBindings().vkDestroyRenderPass(getDevice()->getVkHandle(), getVkHandle(), NULL);
			_vkHandle = VK_NULL_HANDLE;
		}
		else
		{
			reportDestroyedAfterDevice();
		}
	}
}
//!\endcond
} // namespace impl
} // namespace pvrvk

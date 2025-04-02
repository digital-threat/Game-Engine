#include <vk_descriptors.h>

#include <stdexcept>

void DescriptorLayoutBuilder::AddBinding(u32 pBinding, VkDescriptorType pType)
{
	VkDescriptorSetLayoutBinding layoutBinding{};
	layoutBinding.binding = pBinding;
	layoutBinding.descriptorCount = 1;
	layoutBinding.descriptorType = pType;

	mBindings.push_back(layoutBinding);
}

void DescriptorLayoutBuilder::Clear()
{
	mBindings.clear();
}

VkDescriptorSetLayout DescriptorLayoutBuilder::Build(const VkDevice pDevice, const VkShaderStageFlags pShaderStages, const void *pNext, const VkDescriptorSetLayoutCreateFlags pFlags)
{
	for (auto& binding : mBindings)
	{
		binding.stageFlags |= pShaderStages;
	}

	VkDescriptorSetLayoutCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	info.pNext = pNext;
	info.pBindings = mBindings.data();
	info.bindingCount = static_cast<u32>(mBindings.size());
	info.flags = pFlags;

	VkDescriptorSetLayout set;
	if (vkCreateDescriptorSetLayout(pDevice, &info, nullptr, &set) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor set layout.");
	}

	return set;
}

void DescriptorAllocator::InitializePool(const VkDevice pDevice, const u32 pMaxSets, std::span<PoolSizeRatio> pPoolRatios)
{
	std::vector<VkDescriptorPoolSize> poolSizes;
	poolSizes.reserve(pPoolRatios.size());

	for (PoolSizeRatio ratio : pPoolRatios)
	{
		poolSizes.emplace_back(ratio.type,static_cast<u32>(ratio.ratio * pMaxSets));
	}

	VkDescriptorPoolCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	info.flags = 0;
	info.maxSets = pMaxSets;
	info.poolSizeCount = static_cast<u32>(poolSizes.size());
	info.pPoolSizes = poolSizes.data();

	vkCreateDescriptorPool(pDevice, &info, nullptr, &mPool);
}

void DescriptorAllocator::ClearDescriptors(const VkDevice pDevice)
{
	vkResetDescriptorPool(pDevice, mPool, 0);
}

void DescriptorAllocator::DestroyPool(const VkDevice pDevice)
{
	vkDestroyDescriptorPool(pDevice, mPool, nullptr);
}

VkDescriptorSet DescriptorAllocator::Allocate(const VkDevice pDevice, const VkDescriptorSetLayout pLayout)
{
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.descriptorPool = mPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &pLayout;

	VkDescriptorSet descriptorSet;
	if (vkAllocateDescriptorSets(pDevice, &allocInfo, &descriptorSet) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate descriptor sets.");
	}

	return descriptorSet;
}

void
DescriptorWriter::WriteImage(u32 binding, VkImageView image, VkSampler sampler, VkImageLayout layout, VkDescriptorType type)
{
	VkDescriptorImageInfo imageInfo{};
	imageInfo.sampler = sampler;
	imageInfo.imageView = image;
	imageInfo.imageLayout = layout;

	VkDescriptorImageInfo& info = mImageInfos.emplace_back(imageInfo);

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstBinding = binding;
	write.dstSet = VK_NULL_HANDLE; //left empty for now until we need to write it
	write.descriptorCount = 1;
	write.descriptorType = type;
	write.pImageInfo = &info;

	mWrites.push_back(write);
}

void
DescriptorWriter::WriteBuffer(u32 binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type)
{
	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = buffer;
	bufferInfo.offset = offset;
	bufferInfo.range = size;

	VkDescriptorBufferInfo& info = mBufferInfos.emplace_back(bufferInfo);

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstBinding = binding;
	write.dstSet = VK_NULL_HANDLE; //left empty for now until we need to write it
	write.descriptorCount = 1;
	write.descriptorType = type;
	write.pBufferInfo = &info;

	mWrites.push_back(write);
}

void DescriptorWriter::WriteTlas(u32 binding, VkAccelerationStructureKHR tlas)
{
	VkWriteDescriptorSetAccelerationStructureKHR writeAS{};
	writeAS.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
	writeAS.accelerationStructureCount = 1;
	writeAS.pAccelerationStructures = &tlas;

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	// NOTE(Sergei): If you ever forget - the specialized descriptor has to be chained.
	write.pNext = &writeAS;
	write.dstBinding = binding;
	write.dstSet = VK_NULL_HANDLE;
	write.descriptorCount = 1;
	write.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

	mWrites.push_back(write);
}

void
DescriptorWriter::Clear()
{
	mImageInfos.clear();
	mBufferInfos.clear();
	mWrites.clear();
}

void
DescriptorWriter::UpdateSet(VkDevice pDevice, VkDescriptorSet pSet)
{
	for (VkWriteDescriptorSet& write : mWrites)
	{
		write.dstSet = pSet;
	}

	vkUpdateDescriptorSets(pDevice, static_cast<u32>(mWrites.size()), mWrites.data(), 0, nullptr);
}

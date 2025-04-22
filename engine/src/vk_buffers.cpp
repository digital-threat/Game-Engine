#include <vk_buffers.h>

#include <engine.h>
#include <stdexcept>

VulkanBuffer CreateBuffer(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;
	bufferInfo.size = size;
	bufferInfo.usage = usage;

	VmaAllocationCreateInfo vmaAllocationInfo{};
	vmaAllocationInfo.usage = memoryUsage;
	vmaAllocationInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

	VulkanBuffer buffer;
	if (vmaCreateBuffer(allocator, &bufferInfo, &vmaAllocationInfo, &buffer.buffer, &buffer.allocation, &buffer.info) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create buffer.");
	}

	return buffer;
}
VulkanBuffer CreateBufferAligned(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage,
								 VkDeviceSize alignment)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;
	bufferInfo.size = size;
	bufferInfo.usage = usage;

	VmaAllocationCreateInfo vmaAllocationInfo{};
	vmaAllocationInfo.usage = memoryUsage;
	vmaAllocationInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

	VulkanBuffer buffer;
	if (vmaCreateBufferWithAlignment(allocator, &bufferInfo, &vmaAllocationInfo, alignment, &buffer.buffer, &buffer.allocation,
									 &buffer.info) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create buffer.");
	}

	return buffer;
}

// TODO(Sergei): Create a staging memory manager to avoid having to use immediate submit.
// Just get some staging memory and release it sometime after.
VulkanBuffer CreateBufferAndUploadData(Engine& engine, VkDeviceSize size, void* data, VkBufferUsageFlags usage)
{
	VulkanBuffer stagingBuffer = CreateBuffer(engine.mAllocator, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	memcpy(stagingBuffer.info.pMappedData, data, size);

	VkBufferUsageFlags bufferUsage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	VulkanBuffer dstBuffer = CreateBuffer(engine.mAllocator, size, bufferUsage, VMA_MEMORY_USAGE_GPU_ONLY);

	auto func = [&](VkCommandBuffer cmd) { CopyBuffer(cmd, stagingBuffer.buffer, dstBuffer.buffer, size); };

	ImmediateSubmit(engine.mDevice, engine.mGraphicsQueue, engine.mImmediate, func);

	DestroyBuffer(engine.mAllocator, stagingBuffer);

	return dstBuffer;
}

void DestroyBuffer(VmaAllocator allocator, const VulkanBuffer& buffer)
{
	vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation);
}

void CopyBuffer(VkCommandBuffer cmd, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(cmd, srcBuffer, dstBuffer, 1, &copyRegion);
}

void CopyBuffer(VkCommandBuffer cmd, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize srcOffset, VkDeviceSize size)
{
	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = srcOffset;
	copyRegion.size = size;
	vkCmdCopyBuffer(cmd, srcBuffer, dstBuffer, 1, &copyRegion);
}

VkDeviceAddress GetBufferDeviceAddress(VkDevice device, VkBuffer buffer)
{
	VkBufferDeviceAddressInfo addressInfo{};
	addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	addressInfo.buffer = buffer;
	return vkGetBufferDeviceAddress(device, &addressInfo);
}

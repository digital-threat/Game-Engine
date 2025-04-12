#include <engine.h>

void Engine::InitRaytracing()
{
	VkPhysicalDeviceProperties2 properties2{};
	properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	properties2.pNext = &mRtProperties;
	vkGetPhysicalDeviceProperties2(mPhysicalDevice, &properties2);
}

void Engine::InitRaytracingDescriptorLayout()
{
	DescriptorLayoutBuilder builder;
	builder.AddBinding(0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
	builder.AddBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
	mRaytracingDescriptorLayout = builder.Build(mDevice);
}

void Engine::InitRaytracingSceneDescriptorLayout()
{
	u32 textureCount = TextureManager::Instance().GetTextureCount();

	DescriptorLayoutBuilder builder;
	builder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_RAYGEN_BIT_KHR);
	builder.AddBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
	builder.AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, textureCount, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
	mSceneDescriptorLayout = builder.Build(mDevice);
}

void Engine::InitRaytracingPipeline()
{

}

void Engine::RenderRaytracing(VkCommandBuffer cmd, FrameData& currentFrame)
{
	VkDescriptorSet raytracingSet = currentFrame.descriptorAllocator.Allocate(mDevice, mSceneDescriptorLayout);

	DescriptorWriter writer;
	writer.WriteTlas(0, mApplication->mRenderContext.raytracing.tlas);
	writer.WriteImage(1, mColorTarget.imageView, nullptr, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	writer.UpdateSet(mDevice, raytracingSet);

	//vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mRaytracingPipelineLayout, 0, 1, &raytracingSet, 0, nullptr);
}
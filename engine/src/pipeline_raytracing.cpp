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
	builder.AddBinding(0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR);
	builder.AddBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	mRaytracingDescriptorLayout = builder.Build(mDevice, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
}

void Engine::InitRaytracingSceneDescriptorLayout()
{
	// Camera matrices (global uniforms)
	//m_descSetLayoutBind.addBinding(SceneBindings::eGlobals, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_RAYGEN_BIT_KHR);
	// Obj descriptions (vertex buffer address, index buffer address, material buffer address, material index address)
	//m_descSetLayoutBind.addBinding(SceneBindings::eObjDescs, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
	// Textures
	//m_descSetLayoutBind.addBinding(SceneBindings::eTextures, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nbTxt, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);

	DescriptorLayoutBuilder builder;
	builder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	builder.AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	mSceneDescriptorLayout = builder.Build(mDevice, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
}

void Engine::InitRaytracingMaterialDescriptorLayout()
{
	DescriptorLayoutBuilder builder;
	builder.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	builder.AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	mMaterialDescriptorLayout = builder.Build(mDevice, VK_SHADER_STAGE_FRAGMENT_BIT);
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
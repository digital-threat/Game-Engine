#include <engine.h>
#include <vk_helpers.h>
#include <vk_pipelines.h>
#include <glm/gtc/quaternion.hpp>

void Engine::InitRasterSceneDescriptorLayout()
{
    // TODO(Sergei): This requires all textures to be preloaded, should a build a new layout when a scene loads?
    u32 textureCount = TextureManager::Instance().GetTextureCount();

    DescriptorLayoutBuilder builder;
    builder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    builder.AddBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    builder.AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, textureCount, VK_SHADER_STAGE_FRAGMENT_BIT);
    builder.AddBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    mSceneDescriptorLayout = builder.Build(mDevice);
}

void Engine::InitRasterPipeline()
{
    VkShaderModule vertexShader;
    LoadShaderModule("shaders/lit_phong_vert.spv", mDevice, &vertexShader);

    VkShaderModule fragmentShader;
    LoadShaderModule("shaders/lit_phong_frag.spv", mDevice, &fragmentShader);

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(RasterPushConstants);
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = PipelineLayoutCreateInfo();
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pSetLayouts = &mSceneDescriptorLayout;
    pipelineLayoutInfo.setLayoutCount = 1;
    VK_CHECK(vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &mMeshPipelineLayout));

    PipelineBuilder builder;
    builder.mPipelineLayout = mMeshPipelineLayout;
    builder.SetShaders(vertexShader, fragmentShader);
    builder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    builder.SetPolygonMode(VK_POLYGON_MODE_FILL);
    builder.SetCullMode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE);
    builder.SetMultisamplingNone();
    builder.DisableBlending();
    builder.EnableDepthTest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);
    builder.SetColorAttachmentFormat(mColorTarget.format);
    builder.SetDepthFormat(mDepthTarget.format);
    mMeshPipeline = builder.Build(mDevice);

    vkDestroyShaderModule(mDevice, vertexShader, nullptr);
    vkDestroyShaderModule(mDevice, fragmentShader, nullptr);
}

void Engine::UpdateSceneDescriptorSet(VkDescriptorSet sceneSet, FrameData& currentFrame)
{
    float aspect = static_cast<float>(mRenderExtent.width) / static_cast<float>(mRenderExtent.height);

    SceneRenderData sceneRenderData = mApplication->mRenderContext.scene;
    glm::mat4 mainLightP = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 100.0f, 0.1f);
    glm::mat4 mainLightV = glm::lookAt(sceneRenderData.mainLightPos, glm::vec3(0.0f), glm::vec3(0, 1, 0));

    CameraRenderData& camera = mApplication->mRenderContext.camera;
    SceneData scene{};
    scene.matrixV = glm::lookAt(camera.pos, camera.pos + camera.forward, camera.up);
    scene.matrixP = glm::perspective(glm::radians(camera.fov), aspect, 100.0f, 0.1f);
    scene.matrixVP = scene.matrixP * scene.matrixV;
    scene.mainLightVP = mainLightP * mainLightV;
    scene.mainLightColor = sceneRenderData.mainLightColor;
    scene.mainLightDir = glm::normalize(glm::vec4(sceneRenderData.mainLightPos - glm::vec3(0.0f), 1.0f));
    scene.ambientColor = glm::vec4(sceneRenderData.ambientColor, 1.0f);
    scene.lightBuffer = mApplication->mRenderContext.light.lightBuffer;
    scene.lightCount =  mApplication->mRenderContext.light.lightCount;
    scene.cameraPos = glm::vec4(camera.pos, 0.0f);

    memcpy(currentFrame.sceneDataBuffer.info.pMappedData, &scene, sizeof(SceneData));

    // TODO(Sergei): Object data buffer shouldn't be updated every frame, only on scene load.
    u32 meshCount = MeshManager::Instance().mMeshes.size();
    std::vector<ObjectData> objects;
    objects.reserve(meshCount);

    for (u32 j = 0; j < meshCount; j++)
    {
        GpuMesh& mesh = MeshManager::Instance().mMeshes[j];

        ObjectData renderObject{};
        renderObject.textureOffset = mesh.textureOffset;
        renderObject.vertexBufferAddress = mesh.vertexBufferAddress;
        renderObject.indexBufferAddress = mesh.indexBufferAddress;
        renderObject.materialBufferAddress = mesh.materialBufferAddress;
        renderObject.matIdBufferAddress = mesh.matIdBufferAddress;

        objects.push_back(renderObject);
    }

    memcpy(currentFrame.objectDataBuffer.info.pMappedData, objects.data(), objects.size() * sizeof(ObjectData));

    DescriptorWriter writer;
    writer.WriteBuffer(0, currentFrame.sceneDataBuffer.buffer, sizeof(SceneData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    writer.WriteBuffer(1, currentFrame.objectDataBuffer.buffer, objects.size() * sizeof(ObjectData), 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    for (u32 i = 0; i < TextureManager::Instance().mTextures.size(); i++)
    {
        Texture& texture = TextureManager::Instance().mTextures[i];
        writer.WriteImage(2, texture.image.imageView, texture.sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    }
    writer.WriteImage(3, mShadowmapTarget.imageView, TextureManager::Instance().GetSampler("NEAREST_MIPMAP_LINEAR"), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    writer.UpdateSet(mDevice, sceneSet);
}

void Engine::RenderRaster(VkCommandBuffer cmd, FrameData& currentFrame)
{
    VkClearValue clear{};
    clear.color = {0.0f, 0.0f, 0.0f, 1.0f};

    VkRenderingAttachmentInfo colorAttachment = ColorAttachmentInfo(mColorTarget.imageView, &clear, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkRenderingAttachmentInfo depthAttachment = DepthAttachmentInfo(mDepthTarget.imageView, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

    VkRenderingInfo renderInfo = RenderingInfo(mRenderExtent, &colorAttachment, &depthAttachment);

    vkCmdBeginRendering(cmd, &renderInfo);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mMeshPipeline);

    VkViewport viewport{};
    viewport.x = 0;
    viewport.y = static_cast<float>(mRenderExtent.height);
    viewport.width = static_cast<float>(mRenderExtent.width);
    viewport.height = -static_cast<float>(mRenderExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = mRenderExtent.width;
    scissor.extent.height = mRenderExtent.height;

    vkCmdSetScissor(cmd, 0, 1, &scissor);

    VkDescriptorSet sceneSet = currentFrame.descriptorAllocator.Allocate(mDevice, mSceneDescriptorLayout);
    UpdateSceneDescriptorSet(sceneSet, currentFrame);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mMeshPipelineLayout, 0, 1, &sceneSet, 0, nullptr);

     for (auto& object : mApplication->mRenderContext.instances)
     {
         RasterPushConstants pushConstants;
         pushConstants.matrixM = object.transform;
         pushConstants.matrixITM = glm::transpose(glm::inverse(object.transform));
         pushConstants.meshHandle = object.meshHandle;

         GpuMesh mesh = MeshManager::Instance().GetMesh(object.meshHandle);

         vkCmdPushConstants(cmd, mMeshPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(RasterPushConstants), &pushConstants);
         vkCmdBindIndexBuffer(cmd, mesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
         vkCmdDrawIndexed(cmd, mesh.indexCount, 1, 0, 0, 0);
     }

    vkCmdEndRendering(cmd);
}

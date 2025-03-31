#include <engine.h>
#include <vk_helpers.h>
#include <vk_pipelines.h>
#include <glm/gtc/quaternion.hpp>

void Engine::InitRasterizedPipeline()
{
    VkShaderModule vertexShader;
    LoadShaderModule("assets/shaders/lit_phong_vert.spv", mDevice, &vertexShader);

    VkShaderModule fragmentShader;
    LoadShaderModule("assets/shaders/lit_phong_frag.spv", mDevice, &fragmentShader);

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(GeometryPushConstants);
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    std::array<VkDescriptorSetLayout, 2> descriptorSets = { mSceneDescriptorLayout, mMaterialDescriptorLayout };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = PipelineLayoutCreateInfo();
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pSetLayouts = descriptorSets.data();
    pipelineLayoutInfo.setLayoutCount = 2;
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

void Engine::RenderRasterized(VkCommandBuffer cmd, FrameData& currentFrame)
{
    VkRenderingAttachmentInfo colorAttachment = ColorAttachmentInfo(mColorTarget.imageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkRenderingAttachmentInfo depthAttachment = DepthAttachmentInfo(mDepthTarget.imageView, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

    VkRenderingInfo renderInfo = RenderingInfo(mRenderExtent, &colorAttachment, &depthAttachment);

    vkCmdBeginRendering(cmd, &renderInfo);

    //vkCmdClearColorImage(cmd, mColorTarget.image)

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

    SceneData* sceneUniformData = static_cast<SceneData*>(currentFrame.sceneDataBuffer.info.pMappedData);
    *sceneUniformData = scene;

    VkDescriptorSet sceneSet = currentFrame.descriptorAllocator.Allocate(mDevice, mSceneDescriptorLayout);

    DescriptorWriter writer;
    writer.WriteBuffer(0, currentFrame.sceneDataBuffer.buffer, sizeof(SceneData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    writer.WriteImage(1, mShadowmapTarget.imageView, TextureManager::Get().GetSampler("NEAREST_MIPMAP_LINEAR"), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    writer.UpdateSet(mDevice, sceneSet);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mMeshPipelineLayout, 0, 1, &sceneSet, 0, nullptr);

    for (auto& model : mApplication->mRenderContext.renderObjects)
    {
        GeometryPushConstants pushConstants;
        pushConstants.matrixM = model.transform;
        pushConstants.matrixITM = glm::transpose(glm::inverse(model.transform));
        pushConstants.vertexBuffer = model.vertexBufferAddress;

        vkCmdPushConstants(cmd, mMeshPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GeometryPushConstants), &pushConstants);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mMeshPipelineLayout, 1, 1, &model.materialSet, 0, nullptr);
        vkCmdBindIndexBuffer(cmd, model.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(cmd, model.indexCount, 1, 0, 0, 0);
    }

    vkCmdEndRendering(cmd);
}
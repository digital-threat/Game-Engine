#include <engine.h>
#include <forward_renderer.h>
#include <vk_helpers.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <components/camera.h>
#include <ecs/coordinator.h>
#include <systems/camera_system.h>
#include <systems/render_system.h>
#include <systems/transform_gui_system.h>

ForwardRenderer::ForwardRenderer(Engine& engine) : Application(engine)
{
	mRenderContext.renderScale = 1.0f;
	mRenderContext.samplesPerPixel = 1;
	isSimulating = true;
	mCurrentScene = 0;
}

void ForwardRenderer::Awake()
{
	mGlobalCoordinator.RegisterComponent<Camera>();

	{
		Entity entity = mGlobalCoordinator.CreateEntity();

		Camera camera{};
		camera.position = glm::vec3(0.0f, 2.0f, -3.0f);
		camera.sensitivity = 0.1f;
		camera.speed = 1.0f;
		camera.yaw = 90.0f;
		camera.fov = 60.0f;
		mGlobalCoordinator.AddComponent<Camera>(entity, camera);
	}

	mScenes.push_back(DefaultScene());

	mCurrentScene = 1;

	InitRenderTargets();
	InitSceneDescriptorLayout();
	InitRasterPipeline();
	InitShadowmapPipeline();
}

void ForwardRenderer::Update(f64 deltaTime)
{
	CameraSystem::Update(mGlobalCoordinator.mEntityManager, mGlobalCoordinator.mComponentManager, mRenderContext.camera, deltaTime);
}

void ForwardRenderer::PhysicsUpdate(f64 deltaTime)
{
	// if (isSimulating)
	// {
	// 	PhysicsSystem::Update(mGlobalCoordinator.mEntityManager, mGlobalCoordinator.mComponentManager, deltaTime);
	// }
}

void ForwardRenderer::Render()
{
	Scene& scene = mScenes[mCurrentScene];

	CameraSystem::OnGUI(mGlobalCoordinator.mEntityManager, mGlobalCoordinator.mComponentManager);
	TransformGUISystem::Update(scene.coordinator.mEntityManager, scene.coordinator.mComponentManager);

	ImGuiApplication();
	ImGuiScene(scene);

	mRenderContext.scene.mainLightPos = scene.mainLightPosition;
	mRenderContext.scene.mainLightColor = glm::vec4(scene.mainLightColor, scene.mainLightIntensity);
	mRenderContext.scene.skyColor = scene.skyColor;
	mRenderContext.instances.clear();
	mRenderContext.light.lightCount = 0;
	mRenderContext.raytracing.tlas = scene.tlas.handle;

	RenderSystem::Update(mEngine, scene.coordinator.mEntityManager, scene.coordinator.mComponentManager, scene);

	TransitionImageLayout(cmd, mShadowmapTarget.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
	RenderShadowmap(cmd);
	TransitionImageLayout(cmd, mShadowmapTarget.image, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
						  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	TransitionImageLayout(cmd, mColorTarget.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	TransitionImageLayout(cmd, mDepthTarget.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

	RenderRaster(cmd, currentFrame);

	TransitionImageLayout(cmd, mColorTarget.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
}

void ForwardRenderer::Destroy() {}

void ForwardRenderer::InitRenderTargets()
{
	// Depth

	std::array<VkFormat, 3> candidates = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
	VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

	mDepthTarget.format = FindSupportedFormat(mEngine.mPhysicalDevice, candidates, tiling, features);
	mDepthTarget.extent = {mEngine.mVkbSwapchain.extent.width, mEngine.mVkbSwapchain.extent.height, 1};

	VkImageUsageFlags depthTargetUsage{};
	depthTargetUsage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	VkImageCreateInfo depthInfo = ImageCreateInfo(mDepthTarget.format, depthTargetUsage, mDepthTarget.extent);
	CreateImage(mEngine.mAllocator, depthInfo, VMA_MEMORY_USAGE_GPU_ONLY, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mDepthTarget);
	VkImageViewCreateInfo depthViewInfo = ImageViewCreateInfo(mDepthTarget.image, mDepthTarget.format, VK_IMAGE_ASPECT_DEPTH_BIT);
	if (vkCreateImageView(mEngine.mDevice, &depthViewInfo, nullptr, &mDepthTarget.imageView) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create depth image view.");
	}

	// Shadowmap

	mShadowmapTarget.format = mDepthTarget.format;
	mShadowmapTarget.extent = {1024, 1024, 1};

	VkImageUsageFlags shadowmapDepthTargetUsage{};
	shadowmapDepthTargetUsage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	shadowmapDepthTargetUsage |= VK_IMAGE_USAGE_SAMPLED_BIT;

	VkImageCreateInfo shadowmapInfo = ImageCreateInfo(mShadowmapTarget.format, shadowmapDepthTargetUsage, mShadowmapTarget.extent);
	CreateImage(mEngine.mAllocator, shadowmapInfo, VMA_MEMORY_USAGE_GPU_ONLY, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mShadowmapTarget);
	VkImageViewCreateInfo shadowmapViewInfo =
			ImageViewCreateInfo(mShadowmapTarget.image, mShadowmapTarget.format, VK_IMAGE_ASPECT_DEPTH_BIT);
	if (vkCreateImageView(mEngine.mDevice, &shadowmapViewInfo, nullptr, &mShadowmapTarget.imageView) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create depth image view.");
	}
}

Scene ForwardRenderer::DefaultScene() {}

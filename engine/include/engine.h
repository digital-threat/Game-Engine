#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS

#include <VkBootstrap.h>

#include <vector>

#include <application.h>
#include <material_manager.h>
#include <mesh_manager.h>
#include <texture_manager.h>
#include <types.h>
#include <vk_descriptors.h>
#include <vk_images.h>
#include <vk_immediate.h>
#include <vk_types.h>

constexpr u32 WIDTH = 1280;
constexpr u32 HEIGHT = 720;

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

class Engine
{
public:
	// Resize window
	bool mResizeRequested = false;
	bool mFramebufferResized = false;

	GLFWwindow* mWindow = nullptr;

	VmaAllocator mAllocator{};

	vkb::Swapchain mVkbSwapchain;
	vkb::DispatchTable mVkbDT;

	VkInstance mInstance = VK_NULL_HANDLE;
	VkSurfaceKHR mSurface = VK_NULL_HANDLE;

	VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
	VkDevice mDevice = VK_NULL_HANDLE;

	// Immediate
	ImmediateData mImmediate{};

	VkQueue mGraphicsQueue = VK_NULL_HANDLE;
	VkQueue mPresentQueue = VK_NULL_HANDLE;
	u32 mGraphicsQueueIndex;
	u32 mPresentQueueIndex;

	std::vector<VkImage> mSwapchainImages;
	std::vector<VkImageView> mSwapchainImageViews;

	VulkanImage mColorTarget{};
	VulkanImage mDepthTarget{};
	VulkanImage mShadowmapTarget{};

	DescriptorAllocator mGlobalDescriptorAllocator{};

	VkDescriptorSetLayout mSceneDescriptorLayout;

	VkExtent2D mRenderExtent{};

	// Pipelines
	VkPipelineLayout mMeshPipelineLayout = VK_NULL_HANDLE;
	VkPipeline mMeshPipeline = VK_NULL_HANDLE;

	VkPipelineLayout mShadowmapPipelineLayout = VK_NULL_HANDLE;
	VkPipeline mShadowmapPipeline = VK_NULL_HANDLE;

	Application* mApplication;

	// Ray tracing
	VkPhysicalDeviceRayTracingPipelinePropertiesKHR mRtProperties{
			VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR};
	VkDescriptorSetLayout mRtDescriptorLayout;
	VkDescriptorSetLayout mRtSceneDescriptorLayout;
	std::vector<VkRayTracingShaderGroupCreateInfoKHR> mRtShaderGroups;
	VkPipelineLayout mRtPipelineLayout;
	VkPipeline mRtPipeline;
	VulkanBuffer mRtSBTBuffer;
	VkStridedDeviceAddressRegionKHR mRgenRegion{};
	VkStridedDeviceAddressRegionKHR mMissRegion{};
	VkStridedDeviceAddressRegionKHR mHitRegion{};
	VkStridedDeviceAddressRegionKHR mCallRegion{};

public:
	Engine()
	{
		MeshManager::Allocate(*this);
		TextureManager::Allocate(*this);
		MaterialManager::Allocate(*this);
	}

	void Run(Application* application);

private:
	void InitWindow();
	void InitVulkan(FrameData* frames);
	void MainLoop(FrameData* frames);
	void Cleanup();

	void ResizeSwapchain();

	static vkb::Instance CreateInstance();
	void CreateSurface();
	static vkb::PhysicalDevice SelectPhysicalDevice(vkb::Instance& vkbInstance, VkSurfaceKHR vkSurface);
	vkb::Device CreateDevice(vkb::PhysicalDevice& vkbPhysicalDevice);
	void CreateDispatchTable(vkb::Device vkbDevice);
	void GetQueues(vkb::Device& device);
	void CreateAllocator();
	void CreateSwapchain(u32 width, u32 height);
	void CreateCommandObjects(vkb::Device& device, FrameData* frames);
	void InitSyncObjects(FrameData* frames);

	// Descriptors
	void InitDescriptors(FrameData* frames);
	void InitGlobalDescriptorAllocator();
	void InitFrameDescriptorAllocators(FrameData* frames);

	// Buffers
	void InitBuffers(FrameData* frames);

	// Drawing
	void Render(FrameData& currentFrame);

	// ImGui
	void InitImGui();
	void RenderImgui(VkCommandBuffer cmd, VkImageView targetImageView);

	// Rasterized
	void InitRasterSceneDescriptorLayout();
	void InitRasterPipeline();
	void UpdateSceneDescriptorSet(VkDescriptorSet sceneSet, FrameData& currentFrame);
	void RenderRaster(VkCommandBuffer cmd, FrameData& currentFrame);

	// Rasterized shadow mapping
	void InitShadowmapPipeline();
	void RenderShadowmap(VkCommandBuffer cmd);

	// Ray tracing
	void InitRt();
	void InitRtDescriptorLayout();
	void InitRtSceneDescriptorLayout();
	void InitRtPipeline();
	void InitRtSBT();
	void UpdateRtSceneDescriptorSet(VkDescriptorSet sceneSet, FrameData& currentFrame);
	void RenderRt(VkCommandBuffer cmd, FrameData& currentFrame);

	// Compute background
	void InitBackgroundDescriptorLayout();
	void UpdateBackgroundDescriptorSet();
	void InitBackgroundPipeline();
	void RenderBackground(VkCommandBuffer cmd);

	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto app = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
		app->mFramebufferResized = true;
	}

public:
	void UploadMesh(CpuMesh& inMesh, GpuMesh& outMesh);
};

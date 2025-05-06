#pragma once

#include <acceleration_structures.h>
#include <application.h>
#include <ecs/coordinator.h>
#include <scene.h>

class Sandbox : public Application
{
private:
	Coordinator mGlobalCoordinator;
	bool isSimulating;

	std::vector<Scene> mScenes;
	u32 mCurrentScene;

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
	explicit Sandbox(Engine& engine);
	void Awake() override;
	void Update(f64 deltaTime) override;
	void PhysicsUpdate(f64 deltaTime) override;
	void Render(VkCommandBuffer cmd, FrameData& currentFrame) override;
	void OnGUI() override;
	void Destroy() override;

private:
	void ImGuiCustomStyle();
	void ImGuiApplication();
	void ImGuiMaterials();
	void ImGuiScene(Scene& scene);

	Scene MirrorScene();
	Scene SponzaScene();
	void CornellBoxScene();

	void InitRt();
	void InitRtDescriptorLayout();
	void InitRtSceneDescriptorLayout();
	void InitRtPipeline();
	void InitRtSBT();
	void UpdateRtSceneDescriptorSet(VkDescriptorSet sceneSet, FrameData& currentFrame);
	void RenderRt(VkCommandBuffer cmd, FrameData& currentFrame);
};

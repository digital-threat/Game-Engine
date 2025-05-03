#pragma once

#include <application.h>
#include <ecs/coordinator.h>
#include <scene.h>

struct RayHit;
struct Ray;

class ForwardRenderer : public Application
{
private:
	Coordinator mGlobalCoordinator;
	bool isSimulating;

	std::vector<Scene> mScenes;
	u32 mCurrentScene;

	VkPipelineLayout mMeshPipelineLayout = VK_NULL_HANDLE;
	VkPipeline mMeshPipeline = VK_NULL_HANDLE;

	VkPipelineLayout mShadowmapPipelineLayout = VK_NULL_HANDLE;
	VkPipeline mShadowmapPipeline = VK_NULL_HANDLE;

	VulkanImage mColorTarget;
	VulkanImage mDepthTarget;
	VulkanImage mShadowmapTarget;

	VkDescriptorSetLayout mSceneDescriptorLayout;


public:
	explicit ForwardRenderer(Engine& engine);
	void Awake() override;
	void Update(f64 deltaTime) override;
	void PhysicsUpdate(f64 deltaTime) override;
	void Render() override;
	void Destroy() override;

private:
	void ImGuiApplication();
	void ImGuiMaterials();
	void ImGuiScene(Scene& scene);

	void InitRenderTargets();
	void InitSceneDescriptorLayout();
	void InitRasterPipeline();

	void InitRasterSceneDescriptorLayout();
	void UpdateSceneDescriptorSet(VkDescriptorSet sceneSet, FrameData& currentFrame);
	void RenderRaster(VkCommandBuffer cmd, FrameData& currentFrame);

	void InitShadowmapPipeline();
	void RenderShadowmap(VkCommandBuffer cmd);

	Scene DefaultScene();
};

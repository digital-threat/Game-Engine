#pragma once


struct Scene;
class EntityManager;
class ComponentManager;

class RenderSystem
{
public:
	static void Update(VkCommandBuffer cmd, FrameData& currentFrame, Engine& engine, Scene& scene);
};

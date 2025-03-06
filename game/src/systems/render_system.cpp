#include <material_manager.h>
#include <mesh_structs.h>
#include <renderer_vk_types.h>
#include <ecs/component_manager.h>
#include <ecs/entity_manager.h>
#include <systems/render_system.h>
#include <components/renderer.h>
#include <components/transform.h>

#include <glm/gtc/quaternion.hpp>

void RenderSystem::Update(EntityManager& entityManager, ComponentManager& componentManager, RenderContext &context)
{
	Archetype archetype;
	archetype |= Archetype(componentManager.GetComponentType<Transform>());
	archetype |= Archetype(componentManager.GetComponentType<Renderer>());

	context.renderObjects.clear();

	auto func = [&](Entity entity)
	{
		Transform transform = componentManager.GetComponent<Transform>(entity);
		Renderer renderer = componentManager.GetComponent<Renderer>(entity);

		RenderObject renderObject;

		glm::mat4 matrixM = glm::translate(glm::mat4(1.0f), transform.position);
		matrixM *= glm::mat4_cast(transform.rotation);
		matrixM = glm::scale(matrixM, glm::vec3(transform.scale));

		renderObject.transform = matrixM;

		assert(renderer.mesh != nullptr);
		assert(renderer.material.index != -1);

		renderObject.indexBuffer = renderer.mesh->meshBuffers.indexBuffer;
		renderObject.indexCount = renderer.mesh->indexCount;
		renderObject.vertexBuffer = renderer.mesh->meshBuffers.vertexBuffer;
		renderObject.vertexBufferAddress = renderer.mesh->meshBuffers.vertexBufferAddress;
		renderObject.materialSet = MaterialManager::Get().GetDescriptorSet(renderer.material);
	};

	entityManager.Each(archetype, func);
}

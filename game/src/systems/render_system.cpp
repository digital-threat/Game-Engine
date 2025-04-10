#include <material_manager.h>
#include <mesh_structs.h>
#include <render_context.h>
#include <ecs/component_manager.h>
#include <ecs/entity_manager.h>
#include <systems/render_system.h>
#include <components/renderer.h>
#include <components/transform.h>

#include <glm/gtc/quaternion.hpp>

void RenderSystem::Update(EntityManager& entityManager, ComponentManager& componentManager, RenderContext &context)
{
	Archetype archetype;
	archetype.set(componentManager.GetComponentType<Transform>());
	archetype.set(componentManager.GetComponentType<Renderer>());

	context.instances.clear();

	auto func = [&](Entity entity)
	{
		Transform& transform = componentManager.GetComponent<Transform>(entity);
		Renderer& renderer = componentManager.GetComponent<Renderer>(entity);

		glm::mat4 matrixM = glm::translate(glm::mat4(1.0f), transform.position);
		matrixM *= glm::mat4_cast(transform.rotation);
		matrixM = glm::scale(matrixM, glm::vec3(transform.scale));

		RenderInstance instance{};
		instance.transform = matrixM;
		instance.meshHandle = renderer.meshHandle;

		context.instances.push_back(instance);
	};

	entityManager.Each(archetype, func);
}

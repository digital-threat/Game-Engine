#include <collision.h>
#include <ecs/component_manager.h>
#include <ecs/entity_manager.h>
#include <systems/physics_system.h>

void PhysicsSystem::Update(EntityManager& entityManager, ComponentManager& componentManager, float deltaTime)
{
    auto sphereColliders = componentManager.GetComponentsOfType<SphereCollider>();
    auto boxColliders = componentManager.GetComponentsOfType<BoxCollider>();

    std::vector<Collision> collisions;

    for (u32 i = 0; i < sphereColliders.mSize; i++)
    {
        for (u32 j = i + 1; j < sphereColliders.mSize; j++)
        {
            auto c1 = sphereColliders.mComponentArray[i];
            auto c2 = sphereColliders.mComponentArray[j];

            Collision collision { c1, c2 };
            if (IntersectSphereSphere(c1, c2, collision))
            {
                collisions.push_back(collision);
            }
        }
    }

    for (u32 i = 0; i < boxColliders.mSize; i++)
    {
        for (u32 j = i + 1; j < boxColliders.mSize; j++)
        {
            auto c1 = boxColliders.mComponentArray[i];
            auto c2 = boxColliders.mComponentArray[j];

            Collision collision { c1, c2 };
            if (IntersectBoxBox(c1, c2, collision))
            {
                collisions.push_back(collision);
            }
        }
    }

    for (u32 i = 0; i < sphereColliders.mSize; i++)
    {
        for (u32 j = 0; j < boxColliders.mSize; j++)
        {
            auto c1 = sphereColliders.mComponentArray[i];
            auto c2 = boxColliders.mComponentArray[j];

            Collision collision { c1, c2 };
            if (IntersectSphereBox(c1, c2, collision))
            {
                collisions.push_back(collision);
            }
        }
    }

    // // Apply gravity
    // for (ColliderComponent* colliderComponent : colliders)
    // {
    //     Collider& collider = colliderComponent->GetCollider();
    //     if (collider.hasGravity && !collider.isKinematic)
    //     {
    //         collider.velocity += glm::vec3(0, mGravity, 0) * static_cast<float>(deltaTime);
    //     }
    // }
    //
    // // Handle collisions
    // for (Collision collision : collisions)
    // {
    //     if (!collision.c1.isKinematic || !collision.c2.isKinematic)
    //     {
    //         glm::vec3 relativeVelocity = collision.c2.velocity - collision.c1.velocity;
    //         float velocityAlongNormal = glm::dot(relativeVelocity, collision.normal);
    //
    //         if (velocityAlongNormal < 0)
    //         {
    //             float impulse = (1 + mRestitution) * velocityAlongNormal;
    //
    //             if (!collision.c1.isKinematic)
    //             {
    //                 glm::vec3 impulseVector = impulse * collision.normal;
    //                 collision.c1.velocity += impulseVector;
    //             }
    //
    //             if (!collision.c2.isKinematic)
    //             {
    //                 glm::vec3 impulseVector = impulse * collision.normal;
    //                 collision.c2.velocity -= impulseVector;
    //             }
    //         }
    //     }
    // }
    //
    // // Apply velocity
    // for (ColliderComponent* colliderComponent : colliders)
    // {
    //     Collider& collider = colliderComponent->GetCollider();
    //     if (!collider.isKinematic)
    //     {
    //         auto transform = static_cast<TransformComponent*>(colliderComponent->parent.GetComponent(ComponentType::TRANSFORM));
    //         transform->mPosition += collider.velocity * static_cast<float>(deltaTime);
    //     }
    // }
}

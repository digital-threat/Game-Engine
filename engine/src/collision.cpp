#include "collision.h"

#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

bool CheckIntersection(Collider& c1, Collider& c2)
{
	if (c1.type == ColliderType::SPHERE && c2.type == ColliderType::SPHERE)
	{
		return Intersection(static_cast<SphereCollider&>(c1), static_cast<SphereCollider&>(c2));
	}
	if (c1.type == ColliderType::BOX && c2.type == ColliderType::SPHERE)
	{
		return Intersection(static_cast<SphereCollider&>(c2), static_cast<BoxCollider&>(c1));
	}
	if (c1.type == ColliderType::SPHERE && c2.type == ColliderType::BOX)
	{
		return Intersection(static_cast<SphereCollider&>(c1), static_cast<BoxCollider&>(c2));
	}

	return false;
}

bool Intersection(SphereCollider sphere1, SphereCollider sphere2)
{
	float distance = glm::distance(sphere1.position, sphere2.position);
	if (distance < sphere1.radius + sphere2.radius)
	{
		std::cout << "Sphere to sphere intersection" << std::endl;
		return true;
	}

	return false;
}

bool Intersection(SphereCollider sphere, BoxCollider box)
{
	glm::mat4 translationMatrix = translate(glm::mat4(1.0f), box.position);
	glm::mat4 rotationMatrix = toMat4(glm::quat(radians(box.rotation)));
	glm::mat4 boxTransform = translationMatrix * rotationMatrix;

	glm::vec3 localSpherePosition = glm::inverse(boxTransform) * glm::vec4(sphere.position, 1.0f);
	glm::vec3 closestPoint = glm::clamp(localSpherePosition, -box.extents, box.extents);
	float distSqr = glm::distance2(localSpherePosition, closestPoint);
	if (distSqr < sphere.radius * sphere.radius)
	{
		std::cout << "Sphere to box intersection" << std::endl;
		return true;
	}

	return false;
}

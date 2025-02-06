#include "collision.h"

#include <iostream>
#include <ostream>
#include <glm/detail/func_geometric.inl>

bool CheckIntersection(Collider* c1, Collider* c2)
{
	if (c1->type == ColliderType::SPHERE && c2->type == ColliderType::SPHERE)
	{
		return Intersection(static_cast<SphereCollider*>(c1), static_cast<SphereCollider*>(c2));
	}

	return false;
}

bool Intersection(SphereCollider* c1, SphereCollider* c2)
{
	float distance = glm::distance(c1->position, c2->position);
	if (distance < c1->radius + c2->radius)
	{
		std::cout << "Sphere to sphere intersection" << std::endl;
		return true;
	}

	return false;
}

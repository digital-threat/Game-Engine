#pragma once

#include <glm/vec3.hpp>

enum class ColliderType
{
	INVALID,
	SPHERE,
  	BOX,
};

struct Collider
{
	ColliderType type;
	glm::vec3 position;
};

struct SphereCollider : Collider
{
	float radius;
};

struct BoxCollider : Collider
{
	glm::vec3 rotation;
	glm::vec3 extents;
};


bool CheckIntersection(Collider& c1, Collider& c2);
bool Intersection(SphereCollider sphere1, SphereCollider sphere2);
bool Intersection(SphereCollider sphere, BoxCollider box);
bool Intersection(BoxCollider* c1, BoxCollider* c2);


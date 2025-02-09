#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

enum class ColliderType
{
	INVALID,
	SPHERE,
  	BOX,
};

struct Collider
{
	ColliderType type;
};

struct SphereCollider : Collider
{
	glm::vec3 position;
	float radius;
};

struct BoxCollider : Collider
{
	glm::mat4 transform;
	glm::vec3 extents;
};


bool CheckIntersection(Collider& c1, Collider& c2);
bool Intersection(SphereCollider& sphere1, SphereCollider& sphere2);
bool Intersection(SphereCollider& sphere, BoxCollider& box);
bool Intersection(BoxCollider& box1, BoxCollider& box2);


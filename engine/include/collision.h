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

struct Ray
{
	glm::vec3 origin;
	glm::vec3 direction;
};

struct RayHit
{
	glm::vec3 point;
	float distance;
};

bool CheckIntersection(Collider& c1, Collider& c2);
bool Intersection(SphereCollider& sphere1, SphereCollider& sphere2);
bool Intersection(SphereCollider& sphere, BoxCollider& box);
bool Intersection(BoxCollider& box1, BoxCollider& box2);

bool CheckRayIntersection(Ray& ray, RayHit& hit, Collider& collider);
bool CheckRayIntersection(Ray& ray, Collider& collider);

bool IntersectRaySphere(Ray& ray, RayHit& hit, SphereCollider& sphere);
bool TestRaySphere(Ray& ray, SphereCollider& sphere);

bool IntersectRayOBB(Ray& ray, RayHit& hit, BoxCollider &box);
bool IntersectRayAABB(Ray& ray, RayHit& hit, BoxCollider &box);


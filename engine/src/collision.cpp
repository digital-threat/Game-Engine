#include "collision.h"

#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

bool IntersectSphereSphere(SphereCollider& sphere1, SphereCollider& sphere2, Collision& collision)
{
	float distSqr = distance2(sphere1.position, sphere2.position);
	if (distSqr < (sphere1.radius + sphere2.radius) * (sphere1.radius + sphere2.radius))
	{
		float dist = glm::sqrt(distSqr);
		collision.normal = (dist > 0.0f) ? (sphere2.position - sphere1.position) / dist : glm::vec3(1, 0, 0);
		collision.point = sphere1.position + collision.normal * sphere1.radius;

		std::cout << "Sphere to sphere intersection" << std::endl;
		return true;
	}

	return false;
}

bool IntersectSphereBox(SphereCollider& sphere, BoxCollider& box, Collision& collision)
{
	glm::vec3 localSpherePosition = inverse(box.transform) * glm::vec4(sphere.position, 1.0f);
	glm::vec3 closestPoint = clamp(localSpherePosition, -box.extents, box.extents);
	float distSqr = distance2(localSpherePosition, closestPoint);
	if (distSqr < sphere.radius * sphere.radius)
	{
		glm::vec3 delta = localSpherePosition - closestPoint;
		collision.normal = glm::length(delta) > 0.0f ? glm::normalize(delta) : glm::vec3(1, 0, 0);
		collision.point = box.transform * glm::vec4(closestPoint, 1.0f);
		std::cout << "Sphere to box intersection" << std::endl;
		return true;
	}

	return false;
}

bool IntersectBoxBox(BoxCollider& box1, BoxCollider& box2, Collision& collision)
{
	glm::mat3 rotation1 = glm::mat3(box1.transform);
	glm::mat3 rotation2 = glm::mat3(box2.transform);

	glm::vec3 translation = glm::vec3(box2.transform[3]) - glm::vec3(box1.transform[3]);
	translation = glm::transpose(rotation1) * translation;

	glm::mat3 rotation = glm::transpose(rotation1) * rotation2;
	glm::mat3 absRotation = glm::mat3(glm::abs(rotation[0]), glm::abs(rotation[1]), glm::abs(rotation[2])) + glm::mat3(glm::epsilon<float>());

	float r1, r2;
	float minPenetration = FLT_MAX;
	glm::vec3 bestAxis = glm::vec3(0.0f);

	for (int i = 0; i < 3; i++)
	{
		r1 = box1.extents[i];
		r2 = glm::dot(absRotation[i], box2.extents);

		float penetration = (r1 + r2) - glm::abs(glm::dot(translation, rotation1[i]));

		if (penetration < 0.0f) return false;

		if (penetration < minPenetration)
		{
			minPenetration = penetration;
			bestAxis = rotation1[i];
		}
	}

	for (int i = 0; i < 3; i++)
	{
		r1 = glm::dot(absRotation[i], box1.extents);
		r2 = box2.extents[i];

		float penetration = (r1 + r2) - glm::abs(glm::dot(translation, rotation2[i]));

		if (penetration < 0.0f) return false;

		if (penetration < minPenetration)
		{
			minPenetration = penetration;
			bestAxis = rotation2[i];
		}
	}

	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			glm::vec3 axis = glm::cross(rotation1[i], rotation2[j]);

			if (glm::length(axis) > 0.0f)
			{
				axis = glm::normalize(axis);

				float ra = glm::dot(box1.extents, glm::abs(rotation1 * axis));
				float rb = glm::dot(box2.extents, glm::abs(rotation2 * axis));
				float penetration = (ra + rb) - glm::abs(glm::dot(glm::normalize(translation), axis)) * glm::length(translation);

				if (penetration < 0.0f) return false;

				if (penetration < minPenetration)
				{
					minPenetration = penetration;
					bestAxis = axis;
				}
			}
		}
	}

	collision.normal = glm::normalize(bestAxis);
	if (glm::dot(collision.normal, translation) < 0.0f)
	{
		collision.normal = -collision.normal;
	}

	collision.point = glm::vec3(box1.transform[3]) + collision.normal * minPenetration * 0.5f;

	std::cout << "Box to box intersection" << std::endl;
	return true;
}

bool CheckRayIntersection(Ray &ray, RayHit &hit, Collider &collider)
{
	if (collider.type == ColliderType::SPHERE)
	{
		return IntersectRaySphere(ray, hit, static_cast<SphereCollider&>(collider));
	}
	if (collider.type == ColliderType::BOX)
	{
		return IntersectRayOBB(ray, hit, static_cast<BoxCollider&>(collider));
	}

	return false;
}

bool CheckRayIntersection(Ray &ray, Collider &collider)
{
	if (collider.type == ColliderType::SPHERE)
	{
		return TestRaySphere(ray, static_cast<SphereCollider&>(collider));
	}

	return false;
}

bool IntersectRaySphere(Ray& ray, RayHit& hit, SphereCollider& sphere)
{
	glm::vec3 m = ray.origin - sphere.position;
	float b = glm::dot(m, ray.direction);
	float c = glm::dot(m, m) - sphere.radius * sphere.radius;

	if (c > 0.0f && b > 0.0f) return false;

	float discriminant = b * b - c;
	if (discriminant < 0.0f) return false;

	hit.distance = -b - glm::sqrt(discriminant);

	if (hit.distance < 0.0f) hit.distance = 0.0f;
	hit.point = ray.origin + hit.distance * ray.direction;

	std::cout << "Ray to sphere intersection" << std::endl;
	return true;
}

bool TestRaySphere(Ray &ray, SphereCollider &sphere)
{
	glm::vec3 m = ray.origin - sphere.position;
	float c = glm::dot(m, m) - sphere.radius * sphere.radius;

	if (c <= 0.0f)
	{
		std::cout << "Ray to sphere intersection" << std::endl;
		return true;
	}

	float b = glm::dot(m, ray.direction);

	if (b > 0.0f) return false;
	float discriminant = b * b - c;

	if (discriminant < 0.0f) return false;

	std::cout << "Ray to sphere intersection" << std::endl;
	return true;
}

bool IntersectRayOBB(Ray &ray, RayHit &hit, BoxCollider &box)
{
	glm::vec3 center = glm::vec3(box.transform[3]);
	glm::mat3 rotation = glm::mat3(box.transform);

	glm::vec3 localOrigin = glm::transpose(rotation) * (ray.origin - center);
	glm::vec3 localDirection = glm::transpose(rotation) * ray.direction;

	BoxCollider localBox;
	localBox.transform = glm::mat4(1.0f);
	localBox.extents = box.extents;

	Ray localRay;
	localRay.origin = localOrigin;
	localRay.direction = localDirection;

	return IntersectRayAABB(localRay, hit, localBox);
}

bool IntersectRayAABB(Ray &ray, RayHit &hit, BoxCollider &box)
{
	float tMin = 0.0f; // Set to -FLT_MAX to get first hit on line
	float tMax = FLT_MAX; // Set to max distance ray can travel (for segment)

	for (int i = 0; i < 3; i++)
	{
		if (glm::abs(ray.direction[i]) < glm::epsilon<float>())
		{
			if (ray.origin[i] < -box.extents[i] || ray.origin[i] > box.extents[i]) return false;
		}
		else
		{
			float ood = 1.0f / ray.direction[i];
			float t1 = (-box.extents[i] - ray.origin[i]) * ood;
			float t2 = (box.extents[i] - ray.origin[i]) * ood;
			if (t1 > t2)
			{
				float temp = t1;
				t1 = t2;
				t2 = temp;
			}
			tMin = glm::max(tMin, t1);
			tMax = glm::min(tMax, t2);
			if (tMin > tMax) return false;
		}
	}

	hit.distance = tMin;
	hit.point = ray.origin + ray.direction * tMin;
	std::cout << "Ray to box intersection" << std::endl;
	return true;
}
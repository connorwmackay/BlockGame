#pragma once
#include <glm/vec3.hpp>
#include "transformComponent.h"
#include "mesh.h"

enum class FrustumPlane
{
	Left,
	Right,
	Top,
	Bottom,
	Near,
	Far
};

struct Plane
{
	glm::vec3 normal = {0.0f, 1.0f, 0.0f};
	float distanceFromOrigin = 0.0f;

	Plane(const glm::vec3& p1, glm::vec3 normal);
	Plane() = default;
};

struct Frustum
{
	Plane top;
	Plane bottom;
	Plane right;
	Plane left;
	Plane near;
	Plane far;
};

struct AABB
{
	glm::vec3 origin;
	glm::vec3 size;
};

Frustum CreateFrustum(TransformComponent* cameraTransform, float fovy, float aspectRatio, float zNear, float zFar);
bool IsBoundingBoxInsideFrustum(const Frustum& frustum, const AABB& aabb);

bool IsPointInsidePlane(const Plane& plane, const glm::vec3& point);
bool IsPointInsidePlane(const Plane& plane, const Vertex& vertex);

bool IsBoundingBoxInsidePlane(const Plane& plane, const AABB& aabb);

#include "frustum.h"

#include "logging.h"

Plane::Plane(const glm::vec3& p1, glm::vec3 normal_)
{
	normal = normal_;
	distanceFromOrigin = glm::dot(normal_, p1);
}

Frustum CreateFrustum(TransformComponent* cameraTransform, float fovy, float aspectRatio, float zNear, float zFar)
{
	Frustum frustum{};

	float halfVSide = zFar * tanf(fovy * 0.5f);
	float halfHSide = halfVSide * aspectRatio;
	glm::vec3 forwardMultFar = zFar * cameraTransform->GetForwardVector();

	frustum.near = { cameraTransform->GetTranslation() + zNear * cameraTransform->GetForwardVector(), cameraTransform->GetForwardVector() };
	frustum.far = { cameraTransform->GetTranslation() + forwardMultFar, -cameraTransform->GetForwardVector() };
	frustum.right = { cameraTransform->GetTranslation(), glm::cross(forwardMultFar - cameraTransform->GetRightVector() * halfHSide, cameraTransform->GetUpVector()) };
	frustum.left = { cameraTransform->GetTranslation(), glm::cross(cameraTransform->GetUpVector(), forwardMultFar + cameraTransform->GetRightVector() * halfHSide) };
	frustum.top = { cameraTransform->GetTranslation(), glm::cross(cameraTransform->GetRightVector(), forwardMultFar - cameraTransform->GetUpVector() * halfVSide) };
	frustum.bottom = { cameraTransform->GetTranslation(), glm::cross(forwardMultFar + cameraTransform->GetUpVector() * halfVSide, cameraTransform->GetRightVector()) };
	return frustum;
}

bool IsBoundingBoxInsidePlane(const Plane& plane, const AABB& aabb)
{
	float r = aabb.size.x * abs(plane.normal.x) + aabb.size.y * abs(plane.normal.y) + aabb.size.z * abs(plane.normal.z);
	float distanceToPlane = glm::dot(plane.normal, aabb.origin) - plane.distanceFromOrigin;
	return -r <= distanceToPlane;
}

bool IsBoundingBoxInsideFrustum(const Frustum& frustum, const AABB& aabb)
{
	if (IsBoundingBoxInsidePlane(frustum.right, aabb) &&
		IsBoundingBoxInsidePlane(frustum.left, aabb) &&
		IsBoundingBoxInsidePlane(frustum.top, aabb) &&
		IsBoundingBoxInsidePlane(frustum.bottom, aabb) &&
		IsBoundingBoxInsidePlane(frustum.near, aabb) &&
		IsBoundingBoxInsidePlane(frustum.far, aabb))
	{
		return true;
	}

	return false;
}
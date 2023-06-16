#include "transformComponent.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "entity.h"
#include "logging.h"
#include "meshComponent.h"

TransformComponent::TransformComponent()
{}

TransformComponent::TransformComponent(Entity* owner, glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale)
	:Component(owner)
{
	SetTranslation(translation);
	SetRotation(rotation);
	SetScale(scale);
	hasChanged_.store(true);
}

void TransformComponent::SetTranslation(glm::vec3 const& translation)
{
	SetHasChanged(true);
	translation_.store(translation);
}

void TransformComponent::SetRotation(glm::vec3 const& rotation)
{
	SetHasChanged(true);
	rotation_.store(rotation);
}

void TransformComponent::SetScale(glm::vec3 const& scale)
{
	SetHasChanged(true);
	scale_.store(scale);
}

glm::vec3 TransformComponent::GetTranslation()
{
	return translation_;
}

glm::vec3 TransformComponent::GetRotation()
{
	return rotation_;
}

glm::vec3 TransformComponent::GetScale()
{
	return scale_;
}

void TransformComponent::RotateX(float value)
{
	SetHasChanged(true);

	auto localRotation = rotation_.load();
	localRotation.x += value;
	rotation_.store(localRotation);
}

void TransformComponent::RotateY(float value)
{
	SetHasChanged(true);
	auto localRotation = rotation_.load();
	localRotation.y += value;
	rotation_.store(localRotation);
}

void TransformComponent::RotateZ(float value)
{
	SetHasChanged(true);
	auto localRotation = rotation_.load();
	localRotation.z += value;
	rotation_.store(localRotation);
}

glm::vec3 TransformComponent::GetForwardVector()
{
	glm::vec3 forward;
	forward.x = glm::cos(rotation_.load().y) * glm::sin(rotation_.load().x);
	forward.y = -glm::sin(rotation_.load().y);
	forward.z = glm::cos(rotation_.load().y) * glm::cos(rotation_.load().x);
	return forward;
}

glm::vec3 TransformComponent::GetRightVector()
{
	glm::vec3 right;
	right.x = glm::cos(rotation_.load().x);
	right.y = 0.0f;
	right.z = -glm::sin(rotation_.load().x);

	return right;
}

glm::vec3 TransformComponent::GetUpVector()
{
	glm::vec3 forward = GetForwardVector();
	glm::vec3 right = GetRightVector();

	glm::vec3 up = glm::cross(forward, right);
	return up;
}

glm::mat4 TransformComponent::GetModel()
{
	glm::mat4 model = glm::mat4(1.0f);

	model = glm::translate(model, translation_.load());
	model = glm::rotate(model, rotation_.load().x, glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, rotation_.load().y, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, rotation_.load().z, glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, scale_.load());

	return model;
}

void TransformComponent::SetHasChanged(bool hasChanged)
{
	hasChanged_.store(hasChanged);
}

bool TransformComponent::GetHasChanged()
{
	return hasChanged_.load();
}
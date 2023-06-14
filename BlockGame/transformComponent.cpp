#include "transformComponent.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

TransformComponent::TransformComponent()
{}

TransformComponent::TransformComponent(glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale)
{
	SetTranslation(translation);
	SetRotation(rotation);
	SetScale(scale);
}

void TransformComponent::SetTranslation(glm::vec3 const& translation)
{
	translation_ = translation;
}

void TransformComponent::SetRotation(glm::vec3 const& rotation)
{
	rotation_ = rotation;
}

void TransformComponent::SetScale(glm::vec3 const& scale)
{
	scale_ = scale;
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
	rotation_.x += value;
}

void TransformComponent::RotateY(float value)
{
	rotation_.y += value;
}

void TransformComponent::RotateZ(float value)
{
	rotation_.z += value;
}

glm::vec3 TransformComponent::GetForwardVector()
{
	glm::vec3 forward;
	forward.x = glm::cos(rotation_.y) * glm::sin(rotation_.x);
	forward.y = -glm::sin(rotation_.y);
	forward.z = glm::cos(rotation_.y) * glm::cos(rotation_.x);
	return forward;
}

glm::vec3 TransformComponent::GetRightVector()
{
	glm::vec3 right;
	right.x = glm::cos(rotation_.x);
	right.y = 0.0f;
	right.z = -glm::sin(rotation_.x);

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

	model = glm::translate(model, translation_);
	model = glm::rotate(model, rotation_.x, glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, rotation_.y, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, rotation_.z, glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, scale_);

	return model;
}

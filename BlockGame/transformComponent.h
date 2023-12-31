#pragma once
#include <atomic>

#include "component.h"
#include <glm/glm.hpp>

class TransformComponent : public Component
{
	std::atomic<glm::vec3> translation_;
	std::atomic<glm::vec3> rotation_;
	std::atomic<glm::vec3> scale_;

	std::atomic<bool> hasChanged_ {true};
public:
	TransformComponent();
	TransformComponent(Entity* owner, glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale);

	void SetTranslation(glm::vec3 const& translation);
	void SetRotation(glm::vec3 const& rotation);
	void SetScale(glm::vec3 const& scale);

	void RotateX(float value);
	void RotateY(float value);
	void RotateZ(float value);

	glm::vec3 GetTranslation();
	glm::vec3 GetRotation();
	glm::vec3 GetScale();

	glm::vec3 GetForwardVector();
	glm::vec3 GetUpVector();
	glm::vec3 GetRightVector();

	glm::mat4 GetModel();

	void SetHasChanged(bool hasChanged);
	bool GetHasChanged();
};
#pragma once
#include "component.h"
#include <glm/glm.hpp>

#include "transformComponent.h"

class CameraComponent : public Component
{
public:
	CameraComponent();

	glm::mat4 GetView(TransformComponent* transformComponent);
};

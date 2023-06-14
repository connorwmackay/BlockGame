#pragma once
#include "component.h"
#include <glm/glm.hpp>

#include "transformComponent.h"

class CameraComponent : public Component
{
public:
	CameraComponent();
	CameraComponent(Entity* owner);

	glm::mat4 GetView(TransformComponent* transformComponent);
};

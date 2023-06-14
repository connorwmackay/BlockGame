#include "cameraComponent.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

CameraComponent::CameraComponent()
{}

CameraComponent::CameraComponent(Entity* owner)
	: Component(owner)
{}


glm::mat4 CameraComponent::GetView(TransformComponent* transformComponent)
{
	glm::mat4 view = glm::lookAt(
		transformComponent->GetTranslation(),
		transformComponent->GetTranslation() + transformComponent->GetForwardVector(),
		transformComponent->GetUpVector()
	);

	return view;
}
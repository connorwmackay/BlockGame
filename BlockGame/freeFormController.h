#pragma once
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>

#include "cameraComponent.h"
#include "entity.h"
#include "transformComponent.h"
#include "collisionDetection.h"

#include "world.h"

class FreeFormController : public Entity
{
protected:
	TransformComponent* transformComponent_;
	CameraComponent* cameraComponent_;
	GLFWwindow* window_;

	float moveSpeed_;
	float sensitivity_;
	double prevMouseXPos_, prevMouseYPos_;
public:
	FreeFormController(GLFWwindow* window, glm::vec3 position, glm::vec3 rotation);

	void Start() override;
	void Update(World* world);

	CollisionDetection::CollisionBox getCollisionBox();
};
